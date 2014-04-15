// CTHTTPD - Simple Web Server - GPLv2
// Chris Dorman, 2012-2014 (cddo@riseup.net)
// Help from Nickolai Vurgaft (slipperygate@gmail.com)

#include "log.h"
#include "dep.h"
#include "chttpd.h"

void do_chttpd_log(int type, char *s1, char *s2, int num)
{
	// logs the local time of the event 
	time_t now = time(NULL);
	struct tm* tm_info;
	char timebuf[30];
	memset(timebuf, 0, strlen(timebuf));
	tm_info = localtime(&now);
	strftime(timebuf, sizeof(timebuf), "%Y/%m/%d, %H:%M:%S", tm_info);

	int fd ;
	char logbuffer[BUFSIZE*2];

	switch (type) {
	case ERROR: (void)sprintf(logbuffer,"chttpd - %s : Error: %s %s", timebuf, s1, s2); break;
	case SORRY: (void)sprintf(logbuffer, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<html><head><title>CHTTPD: Error</title>\n</head><body><h2>CHTTPD Error:</h2> %s %s <hr /><address>chttpd</address></body></html>\r\n", s1, s2); 
				(void)write(num,logbuffer,strlen(logbuffer));
				break;
	case LOG: (void)sprintf(logbuffer,"chttpd - %s : Info: %s:%s:5d", timebuf, s1, s2); break;
	case SEND_ERROR: (void)sprintf(logbuffer,"HTTP/1.0 500 Internal Server Error\r\nContent-Type: text/html\r\n\r\n<html><head><title>CHTTPD: Found error</title></head><body><h2>Index error</h2>%s<hr /><address>chttpd</address></body></html>\r\n", s1);
				(void)write(num,logbuffer,strlen(logbuffer));
				break;
	}	
	
	if(type == ERROR || type == LOG) { // Log important data
		if((fd = open("server.log", O_CREAT| O_WRONLY | O_APPEND,0644)) >= 0) {
			(void)write(fd,logbuffer,strlen(logbuffer)); 
			(void)write(fd,"\n",1);      
			(void)close(fd);
		}
	}
	
	if(type == ERROR || type == SORRY || type == SEND_ERROR) exit(3);
}