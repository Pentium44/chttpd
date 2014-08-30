// CTHTTPD - Simple Web Server - GPLv2
// Chris Dorman, 2012-2014 (cddo@riseup.net)
// Help from Nickolai Vurgaft (slipperygate@gmail.com)

// CGI base by Lisa 'darkrose' Milne

#include "dep.h"
#include "cgi.h"
#include "chttpd.h"
#include "functions.h"

void do_cgi(int file, char *fpath, int fd, char *cgiroot)
{
	static char buffer[BUFSIZE+1];
	char *cgidir;

	char *dname = dirname(fpath);
	
	strcpy(cgidir, cgiroot);
	strcat(cgidir, fpath+1);
	
	// Write HTTP protocol to socket before executing cgi
	sprintf(buffer,"HTTP/1.0 200 OK\r\n");
	write(fd,buffer,strlen(buffer));
	/*
		start parsing the cgi code
	*/
	
	/* test code */
	
	/*
	sprintf(buffer,"Content-Type: text/plain\r\n\r\n%s, %s, %s", cgiroot, fpath, cgidir);
	write(fd,buffer,strlen(buffer));
	
	free(dummy);
	*/
	
	fd = dup2(fd,STDOUT_FILENO);
	chdir(cgidir);
	
	execl("/bin/sh", file, NULL);
}
