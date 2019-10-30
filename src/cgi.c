// CTHTTPD - Simple Web Server - GPLv2
// Chris Dorman, 2012-2014 (cddo@riseup.net)
// Help from Nickolai Vurgaft (slipperygate@gmail.com)

// CGI base by Lisa 'darkrose' Milne

#include "dep.h"
#include "cgi.h"
#include "chttpd.h"
#include "functions.h"

void do_cgi(char fpath[], int fd, char *cgiroot)
{
	static char buffer[BUFSIZE+1];
	char *cgidir;
	char *cgi_p;
	
	cgi_p = malloc(strlen(cgiroot) + strlen(fpath) + 1);
	strcpy(cgi_p, cgiroot);
	strcat(cgi_p, fpath);
	
	char *cgipath = dirname(fpath);
	cgidir = malloc(strlen(cgiroot) + strlen(fpath) + 1);
	strcpy(cgidir, cgiroot);
	strcat(cgidir, cgipath);
	
	// Write HTTP protocol to socket before executing cgi
	sprintf(buffer,"HTTP/1.0 200 OK\r\n");
	write(fd,buffer,strlen(buffer));
	/*
		start parsing the cgi code
	*/
	
	
	fd = dup2(fd,STDOUT_FILENO);
	chdir(cgidir);
	
	execl("/bin/bash", cgi_p, NULL);
	
}
