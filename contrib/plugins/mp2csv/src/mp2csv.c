#define _XOPEN_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpointer.h>
#include <libxml/xpath.h>

#include "scheda.h"
#include "log.h"

#define DEFAULT_CONFIG "mp2csv.config"
#define URL_PALINSESTO "http://www.mediasetpremium.mediaset.it/export/palinsesto.xml"

static xmlDocPtr palinsestoDoc = NULL;
static char epgdb_root[512];

typedef struct exportInfo_s
{
	unsigned short int nid;		// origin network id
	unsigned short int tsid;		// transport stream id
	unsigned short int sid;		// service id

	unsigned short int offset;		// time offset
	bool EOA;				// End Of Array
} exportInfo;

static xmlXPathContextPtr configContext = NULL;
static xmlDocPtr configDoc = NULL;

exportInfo *searchConf(xmlChar *channel_name)
{
	xmlChar buf[1024];
	xmlXPathObjectPtr result;
	xmlChar *nid;
	xmlChar *tsid;
	xmlChar *sid;
	xmlChar *offset;
	exportInfo *exp;
	int i;

	xmlStrPrintf(buf, 1024, (const xmlChar *)"//channel[@name='%s']/export", channel_name);

	log_add("..xPath=\"%s\"", (char *)buf);

	result = xmlXPathEvalExpression(buf, configContext);
	if (result != NULL && !xmlXPathNodeSetIsEmpty(result->nodesetval))
	{
		exp = malloc(sizeof(exportInfo) * (result->nodesetval->nodeNr+1));

		for (i=0; i<result->nodesetval->nodeNr; i++)
		{
			nid  = xmlGetProp(result->nodesetval->nodeTab[i], (const xmlChar *)"nid");
			tsid = xmlGetProp(result->nodesetval->nodeTab[i], (const xmlChar *)"tsid");
			sid  = xmlGetProp(result->nodesetval->nodeTab[i], (const xmlChar *)"sid");
			offset = xmlGetProp(result->nodesetval->nodeTab[i], (const xmlChar *)"offset");

			log_add("..nid|tsid|sid +offset = %s|%s|%s +%s", (char *)nid, (char *)tsid, (char *)sid, (char *)offset);
	
			exp[i].nid = atoi((char *)nid);
			exp[i].tsid = atoi((char *)tsid);
			exp[i].sid = atoi((char *)sid);
			exp[i].offset = atoi((char *)offset);
			exp[i].EOA = false;

			xmlFree(nid);
			xmlFree(tsid);
			xmlFree(sid);
			xmlFree(offset);
		}
		exp[i].EOA = true;

		xmlXPathFreeObject(result);
		
		return exp;
	}
	else
	{
		return NULL;
	}
}

xmlXPathContextPtr openConf(char *config_filename)
{
	configDoc = xmlReadFile(config_filename, NULL, 0);

	if (configDoc != NULL )
	{
		configContext = xmlXPathNewContext(configDoc);

		if (configContext != NULL)
		{
			return configContext;
		}
	}

	log_add("Non riesco a leggere il file di configurazione.");

	return NULL;
}

void closeConf()
{
	if (configDoc != NULL )
	xmlFreeDoc(configDoc);

	if (configContext != NULL)
		xmlXPathFreeContext(configContext);

	return;
}

static int optdst = 0;
time_t parseDataOra(xmlChar *data, xmlChar *ora)
{
	char buf[30];
	struct tm tempo;
	time_t t;

	sprintf(buf, "%s %s:00", data, ora);
	strptime(buf, "%Y/%m/%d %H:%M:%S", &tempo);
	tempo.tm_isdst = -1;

	t = mktime(&tempo);

	if (optdst==1)
		return t-3600;
	else
		return t;
}

void parseProg(xmlNodePtr prg, xmlChar *data, xmlChar *canale, exportInfo *exp, int cnt)
{
	time_t inizio;
	time_t fine;
	static time_t previnizio;
	static time_t prevfine;

	xmlChar *orainizio;
	xmlChar *orafine;

	int	 durata;
	xmlChar *tipo;
	xmlChar *titolo;
	xmlChar *descrizione;
	xmlChar *anno;
	xmlChar *paese;
	xmlChar *linkScheda;
	char *scheda;

	xmlXPathContextPtr context;

	int i;

	orainizio = xmlGetProp(prg, (const xmlChar *)"orainizio");
	orafine = xmlGetProp(prg, (const xmlChar *)"orafine");

	context = xmlXPathNewContext(palinsestoDoc);
	if (context == NULL)
	{
		printf("Error in xmlXPathNewContext\n");
		return;
	}
	context->node = prg;

	durata	  = xmlXPathCastToNumber(xmlXPathEvalExpression((const xmlChar *)"durata", context));
	tipo		= xmlXPathCastToString(xmlXPathEvalExpression((const xmlChar *)"tipologia", context));
	titolo	  = xmlXPathCastToString(xmlXPathEvalExpression((const xmlChar *)"titolo", context));
	descrizione = xmlXPathCastToString(xmlXPathEvalExpression((const xmlChar *)"descrizione", context));
	anno		= xmlXPathCastToString(xmlXPathEvalExpression((const xmlChar *)"anno", context));
	paese	   = xmlXPathCastToString(xmlXPathEvalExpression((const xmlChar *)"paese", context));
	linkScheda  = xmlXPathCastToString(xmlXPathEvalExpression((const xmlChar *)"linkScheda", context));

	inizio = parseDataOra(data, orainizio);
	fine   = parseDataOra(data, orafine);

	// Aggiusta la data aggiungendo un giorno (24h) in caso di wrap: 
	// i palinsesti vanno dalle 6 (circa) del giorno corrente alle 6 (circa del giorno seguente)
	if (cnt>0 && inizio<previnizio)
	{
		inizio = inizio + 24*60*60;
	}
	if (cnt>0 && fine<prevfine)
	{
		fine = fine + 24*60*60;
	}

	previnizio = inizio;
	prevfine = fine;

	// Se non è un evento iniziato ieri, stampa la riga con le informazioni
	//
	//	1 => organization id
	//	2 => transport service id
	//	3 => service id
	//	4 => start time (unix timestamp)
	//	5 => length (seconds)
	//	6 => event title
	//	7 => event description

	if (xmlStrstr(titolo, (const xmlChar *)"evento iniziato ieri")==NULL && xmlStrstr(titolo, (const xmlChar *)"iniziato alle")==NULL)
	{
		scheda = read_scheda(URL_PALINSESTO, linkScheda, epgdb_root);
		for (i=0; !exp[i].EOA; i++)
		{
			printf("%d,%d,%d,%d,%d,\"%s\",\"%s\"\n", exp[i].nid, exp[i].tsid, exp[i].sid, (int)inizio+exp[i].offset*60, durata*60, titolo, scheda);
		}
	}

	xmlXPathFreeContext(context);

	xmlFree(orainizio);
	xmlFree(orafine);

	xmlFree(tipo);
	xmlFree(titolo);
	xmlFree(descrizione);
	xmlFree(anno);
	xmlFree(paese);
	xmlFree(linkScheda);

	return;
}

void parseCanale(xmlNodePtr canale, xmlChar *data)
{
	xmlChar *desc;
	int cnt=0;
	xmlNodePtr prog;
	exportInfo *exports;
	
	desc = xmlGetProp(canale, (const xmlChar *)"description");

	log_add("..canale %s", desc);

	exports = searchConf(desc);
	if (exports != NULL)
	{
		log_add("..match");
		prog = canale->xmlChildrenNode;
		while (prog != NULL)
		{
			if ((!xmlStrcmp(prog->name, (const xmlChar *)"prg")))
			{
				parseProg(prog, data, desc, exports, cnt++);
			}
			prog = prog->next;
		}
	
		free(exports);
	}
	else
	{
		log_add("..nessun match");
	}
	xmlFree(desc);

	return;
}

void parseGiorno(xmlNodePtr giorno)
{
	xmlChar *data;
	xmlNodePtr canale;
	
	data = xmlGetProp(giorno, (const xmlChar *)"data");

	log_add(".giorno %s", data);

	canale = giorno->xmlChildrenNode;
	while (canale != NULL)
	{
		if ((!xmlStrcmp(canale->name, (const xmlChar *)"canale")))
		{
			parseCanale(canale, data);
		}
		canale = canale->next;
	}
	xmlFree(data);

	return;
}

void parseDoc(char *url)
{
	xmlNodePtr cur;

	palinsestoDoc = xmlReadFile(url, NULL, 0);
	
	if (palinsestoDoc == NULL )
	{
		log_add("Errore in xmlReadFile()");
		return;
	}

	cur = xmlDocGetRootElement(palinsestoDoc);
	
	if (cur == NULL)
	{
		log_add("Il palinsesto non ha un nodo radice");
		xmlFreeDoc(palinsestoDoc);
		return;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "palinsesto"))
	{
		log_add("La radice del palinsesto non è <palinsesto>");
		xmlFreeDoc(palinsestoDoc);
		return;
	}
	
	cur = cur->xmlChildrenNode;
	while (cur != NULL)
	{
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"giorno")))
		{
			parseGiorno(cur);
		}
		
		cur = cur->next;
	}
	
	xmlFreeDoc(palinsestoDoc);

	return;
}

void usage(char *me)
{
	printf ("Usage: %s [opt]\n", me);
	printf ("  -D  fix for daylight saving time\n");
	printf ("  -f  channel configuration file \n");
	printf ("	  default: %s\n", DEFAULT_CONFIG);
	printf ("  -L  enable debug to /tmp\n");
	printf ("  -h  show this help\n");

	return;
}

static char *config_trim_spaces (char *text)
{
	char *tmp = text;
	while (tmp[0] == ' ') tmp++;
	while (strlen (tmp) > 1)
		if (tmp[strlen (tmp) - 1] == ' ') tmp[strlen (tmp) - 1] = '\0';
		else break;
	
	if (tmp[0] == ' ') tmp[0] = '\0';
	return tmp;
}

void loadCrossepgConf()
{
	FILE *fd;
	char line[512];
	char key[512];
	char value[512];
	
	strcpy(value, "/tmp/mp2csv_cache"); // default value
	
	/* try to open the config from all possible paths */
	fd = fopen("/usr/crossepg/crossepg.config", "r");
	if (!fd)
		fd = fopen("/var/crossepg/crossepg.config", "r");
	if (!fd)
		fd = fopen("/var/plug-in/crossepg.config", "r");
	if (!fd)
		return; // no config found.. sorry
	
	while (fgets(line, sizeof(line), fd)) 
	{
		char *tmp_key, *tmp_value;
		
		memset(key, 0, sizeof(key));
		memset(value, 0, sizeof(value));
		
		if (sscanf (line, "%[^#=]=%s\n", key, value) != 2)
			continue;
		
		tmp_key = config_trim_spaces(key);
		tmp_value = config_trim_spaces(value);
		
		if (strcmp("db_root", tmp_key) == 0)
		{
			sprintf(epgdb_root, "%s/mp2csv_cache", tmp_value);
			break;
		}
	}
	fclose(fd);
}

void deleteOldCache()
{
	DIR *dp;
	struct dirent *ep;
	time_t now = time(NULL);
	
	dp = opendir(epgdb_root);
	if (dp != NULL)
	{
		while ((ep = readdir(dp)) != NULL)
		{
			char filename[512];
			struct stat fstat;
			sprintf(filename, "%s/%s", epgdb_root, ep->d_name);
			if (stat(filename, &fstat) == 0)
				if (fstat.st_atime < (now - (60*60*24*7)))	// if last access is older than 7 days
					unlink(filename);
		}
	}
	closedir(dp);
}

int main(int argc, char **argv)
{
	char configName[256];
	char c;
	int optlog = 0;

	strcpy(configName, DEFAULT_CONFIG);

	while ((c = getopt (argc, argv, "Df:L")) != -1)
	{
		switch (c)
		{
			case 'D':
				optdst = 1;
				break;
			case 'f':
				strncpy(configName, optarg, 256);
				break;
			case 'L':
				optlog = 1;
				break;
			case '?':
			usage(argv[0]);
				return 0;
		}
	}

	if (optind != argc)
	{
		usage(argv[0]);
		return 0;
	}

	if (optlog==1)
	{
		log_enable();
	}
	else
	{
		log_disable();
	}
	log_open("/tmp/mp2csv.log", "MP2csv - elabora il palinsesto di MP");

	log_add("File config=%s", configName);

	loadCrossepgConf();
	mkdir(epgdb_root, S_IRWXU | S_IRWXG | S_IRWXO);
	
	if (openConf(configName)!=NULL)
	{
		log_add("Inizio l'elaborazione del palinsesto.");
		parseDoc(URL_PALINSESTO);
		log_add("Finisco l'elaborazione del palinsesto.");
	}

	closeConf();
	deleteOldCache();

	log_close();

	return 1;
}
