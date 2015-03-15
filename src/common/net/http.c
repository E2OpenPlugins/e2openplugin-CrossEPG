#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>

#include "../../common.h"

#include "../core/log.h"

#include "http.h"

char *_build_get_query (char *host, char *page)
{
	char *query;
	char *getpage = page;
	char *tpl = "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n";
	char useragent[32];
	sprintf (useragent, "CrossEPG %s", RELEASE);
	if(getpage[0] == '/') getpage++;
	// -5 is to consider the %s %s %s in tpl and the ending \0
	query = _malloc (strlen (host)+strlen (getpage)+strlen (useragent)+strlen (tpl)-5);
	sprintf (query, tpl, getpage, host, useragent);
	return query;
}

size_t http_file_write(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	return fwrite(ptr, size, nmemb, stream);
}
 
size_t http_file_read(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	return fread(ptr, size, nmemb, stream);
}
 
int http_progress(void(*progress_callback)(int, int), double t, double d, double ultotal, double ulnow)
{
	if (progress_callback != NULL && t > 0) progress_callback ((int)d, (int)t);
	return 0;
}

bool http_get (char *host, char *page, int port, int tempfile, void(*progress_callback)(int, int), volatile bool *stop)
{
	log_add ("HTTP Get (host:%s, port:%d, page:%s)", host, port, page);

	FILE *fd = fdopen (tempfile, "w");
	if (fd == NULL)
	{
		log_add ("Cannot open temporary file");
	}

	CURL *curl = curl_easy_init();
	if (curl)
	{
		//outfile = fopen("test.curl", "w");
		char url[4096];

		sprintf(url, "http://%s:%d/%s", host, port, page);

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fd);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_file_write);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, http_file_read);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, http_progress);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, progress_callback);

		curl_easy_perform(curl);

		fclose(fd);
		curl_easy_cleanup(curl);
		return true;
	}

	return false;
}


bool http_get_old (char *host, char *page, int port, int tempfile, void(*progress_callback)(int, int), volatile bool *stop)
{
	struct sockaddr_in *remote;
	struct hostent *hent;
	int sock;
	int tmpres;
	char ip[16];
	char *get;
	char buf[BUFSIZ+1];
	FILE *fd;
	int size = 0;
	int total_size = 0;

	log_add ("HTTP Get (host:%s, port:%d, page:%s)", host, port, page);

	/* create socket */
	if ((sock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		log_add ("Can't create TCP socket");
		return false;
	}
	
	/* get ip */
	memset (ip, 0, sizeof (ip));
	if ((hent = gethostbyname (host)) == NULL)
	{
		log_add ("Can't get IP address");
		return false;
	}
	
	if (inet_ntop (AF_INET, hent->h_addr_list[0], ip, 15) == NULL)
	{
		log_add ("Can't resolve host");
		return false;
	}
	
	remote = _malloc (sizeof (struct sockaddr_in *));
	remote->sin_family = AF_INET;
	tmpres = inet_pton (AF_INET, ip, (void *)(&(remote->sin_addr.s_addr)));
	if (tmpres < 0)
	{
		log_add ("Can't set remote->sin_addr.s_addr");
		_free (remote);
		return false;
	}
	else if (tmpres == 0)
	{
		log_add ("%s is not a valid IP address");
		_free (remote);
		return false;
	}
	remote->sin_port = htons (port);
	
	if (connect (sock, (struct sockaddr *)remote, sizeof (struct sockaddr)) < 0)
	{
		log_add ("Could not connect");
		_free (remote);
		return false;
	}
	
	get = _build_get_query (host, page);
	//fprintf(stderr, "Query is:\n<<START>>\n%s<<END>>\n", get);
	
	/* send the query to the server */
	int sent = 0;
	while (sent < strlen (get))
	{
		tmpres = send(sock, get+sent, strlen(get)-sent, 0);
		if(tmpres == -1)
		{
			log_add ("Can't send query");
			_free (get);
			_free (remote);
			close (sock);
			return false;
		}
		sent += tmpres;
	}
	
	//now it is time to receive the page
	fd = fdopen (tempfile, "w");
	if (fd == NULL)
	{
		log_add ("Cannot open temporary file");
		_free (get);
		_free (remote);
		close (sock);
	}
	memset (buf, 0, sizeof (buf));
	bool htmlstart = false;
	char *htmlcontent;
	
	bool error = false;
	while ((tmpres = recv (sock, buf, BUFSIZ, 0)) > 0 && *stop == false)
	{
		if (htmlstart == false)
		{
			char *line = buf;
			char *next_line;
			while ((next_line = strstr (line, "\r\n")) != NULL)
			{
				line[next_line - line] = '\0';
				next_line += 2;
				if (strlen (line) == 0)
				{
					htmlcontent = next_line;
					htmlstart = true;
					tmpres -= (next_line - buf);
					break;
				}
				int response;
				char http_version[256];
				if (sscanf (line, "HTTP/%s %d", http_version, &response) == 2)
				{
					if (response != 200)
					{
						log_add ("HTTP error %d", response);
						error = true;
					}
				}
				
				// Content-Length: 210060
				int length;
				if (sscanf (line, "Content-Length: %d", &length) == 1)
					total_size = length;
				
				line = next_line;
			}
		}
		else
		{
			htmlcontent = buf;
		}
		
		if (error) break;
		
		if ((htmlstart) && (tmpres > 0))
		{
			fwrite (htmlcontent, tmpres, 1, fd);
			size += tmpres;
			if (progress_callback != NULL) progress_callback (size, total_size);
		}
		memset (buf, 0, tmpres);
	}
	
	if (tmpres < 0)
	{
		log_add ("Error receiving data");
		error = true;
	}
	else if (size <= 0)
	{
		log_add ("ERROR!! No data found");
		error = true;
	}
	else
		log_add ("Downloaded %d bytes", total_size);
		
	fclose (fd);
	_free (get);
	_free (remote);
	close (sock);
	return !error;
}
