#define _XOPEN_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpointer.h>
#include <libxml/xpath.h>
#include <libxml/HTMLparser.h>
#include <libxml/uri.h>

int append_scheda(char *buf, xmlNodePtr node, int pos, int bufSize)
{
	if (node!= NULL)
	{
		if (node->type == XML_ELEMENT_NODE)
		{
			pos = append_scheda(buf, node->children, pos, bufSize);
			
			append_scheda(buf, node->next, pos, bufSize);
		}
		else if(node->type == XML_TEXT_NODE)
		{
			if (node->content != NULL && (strlen(buf)+strlen((char *)node->content))<bufSize )
			{
				char *src = (char *)node->content;
				int i=0;
				char c;
				while ((c=src[i++])!='\0' && pos < bufSize-1)
				{
					if ( !iscntrl(c))
					{
						if (c=='"' || c=='\\')
							buf[pos++] = '\\';

						buf[pos++] = c;
					}
				}
				buf[pos] = '\0';

				append_scheda(buf, node->next, pos, bufSize);
			}
		}
	}
	return pos;
}

char * read_scheda(char *baseUrl, xmlChar *url, char *epgdb_root)
{
	static char buf[2048];
	int i;
	FILE *fd;
	char cachefile[strlen(epgdb_root) + strlen((char*)url) + 2];
	htmlDocPtr docScheda = NULL;
	xmlXPathContextPtr contextScheda = NULL;
	xmlXPathObjectPtr par = NULL;
	xmlChar *urlScheda = NULL;

	/* build cache filename */
	buf[0]='\0';
	strcpy(cachefile, epgdb_root);
	cachefile[strlen(epgdb_root)] = '/';
	for (i=0; i<strlen((char*)url); i++)
		if (url[i] == '/' || url[i] == '\\' || url[i] == '?' || url[i] == '&' || url[i] == '=')
			cachefile[i+strlen(epgdb_root)+1] = '_';
		else
			cachefile[i+strlen(epgdb_root)+1] = url[i];

	cachefile[i+strlen(epgdb_root)+1] = '\0';
	
	/* try to read from cache */
	fd = fopen(cachefile, "r");
	if (fd)
	{
		fread(buf, 2048, 1, fd);
		fclose(fd);
		return buf;
	}
	
	/* ok... no cache... download it! */
	urlScheda = xmlBuildURI(url, (xmlChar *)baseUrl);
	
	if (urlScheda != NULL )
	{
		docScheda = htmlReadFile((char *)urlScheda, NULL, HTML_PARSE_RECOVER|HTML_PARSE_NOERROR|HTML_PARSE_NOWARNING);

		if (docScheda != NULL )
		{
			contextScheda = xmlXPathNewContext(docScheda);

			if (contextScheda != NULL)
			{
				// Prende il primo paragrafo sotto la div con id="box"
				par = xmlXPathEvalExpression((const xmlChar *)"//div[@id='box']/p[1]", contextScheda);

				if (par != NULL && !xmlXPathNodeSetIsEmpty(par->nodesetval))
				{
					append_scheda(buf, par->nodesetval->nodeTab[0]->children, 0, 2048);
					
					xmlXPathFreeObject(par);
				}

				xmlXPathFreeContext(contextScheda);
			}

			xmlFreeDoc(docScheda);
		}
	
		xmlFree(urlScheda);
	}

	/* save the cache */
	if (strlen(buf) > 0)
	{
		fd = fopen(cachefile, "w");
		if (fd)
		{
			fwrite(buf, strlen(buf)+1, 1, fd);
			fclose(fd);
		}
	}
	
	return buf;
}
