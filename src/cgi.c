// CTHTTPD - Simple Web Server - GPLv2
// Chris Dorman, 2012-2014 (cddo@riseup.net)
// Help from Nickolai Vurgaft (slipperygate@gmail.com)

// CGI base by Lisa 'darkrose' Milne

#include "dep.h"
#include "cgi.h"
#include "chttpd.h"

void do_cgi(char *file, int fd, char *cgiroot)
{
	static char buffer[BUFSIZE+1];
	
	// Write HTTP protocol to socket before executing cgi
	(void)sprintf(buffer,"HTTP/1.0 200 OK\r\n");
	(void)write(fd,buffer,strlen(buffer));
	// /
	// start parsing the cgi code
	// /
	
	// pipe stdout to socket
	fd = dup2(fd,STDOUT_FILENO);
	// chroot cgi
	chdir(cgiroot);
	// execute cgi
	execl("/bin/sh", file, NULL);
}