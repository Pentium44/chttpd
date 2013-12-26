// CTHTTPD - Simple Web Server - GPLv2
// Chris Dorman, 2012-2013 (cddo@riseup.net)
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// For directory listings
#include <dirent.h>

#include "chttpd.h"
#include "mimetypes.h"

void log(int type, char *s1, char *s2, int num)
{
	int fd ;
	char logbuffer[BUFSIZE*2];

	switch (type) {
	case ERROR: (void)sprintf(logbuffer,"Error: %s %s\n",s1, s2); break;
	case SORRY: (void)sprintf(logbuffer, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<html><head><title>CHTTPD: Error</title>\n</head><body><h2>CHTTPD Error:</h2> %s %s <hr />%s %s</body></html>\r\n", s1, s2, client, version); 
				(void)write(num,logbuffer,strlen(logbuffer));
				break;
	case LOG: (void)sprintf(logbuffer,"HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<html><head><title>CHTTPD: Information</title></head><body><h2>CHTTPD info:</h2> %s:%s:5d</body></html>\r\n",s1, s2); break;
	case SEND_ERROR: (void)sprintf(logbuffer,"HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<html><head><title>CHTTPD: Found error</title></head><body><h2>Index error</h2>%s<hr />%s %s</body></html>\r\n", s1, client, version);
				(void)write(num,logbuffer,strlen(logbuffer));
				break;
	}	
	
	if(type == ERROR) {
		if((fd = open("server.log", O_CREAT| O_WRONLY | O_APPEND,0644)) >= 0) {
			(void)write(fd,logbuffer,strlen(logbuffer)); 
			(void)write(fd,"\n",1);      
			(void)close(fd);
		}
	}
	
	if(type == ERROR || type == SORRY || type == SEND_ERROR) exit(3);
}

void web(int fd, int hit, char *datadir)
{
	int j, file_fd, buflen, len;
	long i, filesize;
	char * fstr, path, wholepath;
	static char buffer[BUFSIZE+1];

	// Check to see if file is corrupted
	filesize =read(fd,buffer,BUFSIZE); 
	if(filesize == 0 || filesize == -1) {
		log(SORRY,"failed to read browser request","",fd);
	}
	
	if(filesize > 0 && filesize < BUFSIZE)	
		buffer[filesize]=0;	
	else buffer[0]=0;

	for(i=0;i<filesize;i++)	
		if(buffer[i] == '\r' || buffer[i] == '\n')
			buffer[i]='*';
	log(LOG,"request",buffer,hit);

	if( strncmp(buffer,"GET ",4) && strncmp(buffer,"get ",4) )
		log(SORRY,"Only simple GET operation supported",buffer,fd);

	for(i=4;i<BUFSIZE;i++) { 
		if(buffer[i] == ' ') { 
			buffer[i] = 0;
			break;
		}
	}

	for(j=0;j<i-1;j++) 	
		if(buffer[j] == '.' && buffer[j+1] == '.')
			log(SORRY,"Parent directory (..) path names not supported",buffer,fd);
	
	if( !strncmp(&buffer[0],"GET /\0",6) || !strncmp(&buffer[0],"get /\0",6) ) {
		if(file_exists("index.html")) {
			(void)strcpy(buffer,"GET /index.html");
		} else {
			log(SEND_ERROR,"Index file not found!","",fd);
		}
	}
	
	buflen=strlen(buffer);
	fstr = (char *)0;
	for(i=0;extensions[i].ext != 0;i++) {
		len = strlen(extensions[i].ext);
		if( !strncmp(&buffer[buflen-len], extensions[i].ext, len)) {
			fstr =extensions[i].filetype;
			break;
		}
	}
	if(fstr == 0) log(SORRY,"file extension type not supported",buffer,fd);

	if(( file_fd = open(&buffer[5],O_RDONLY)) == -1) 
		log(SORRY, "failed to open file",&buffer[5],fd);

	log(LOG,"SEND",&buffer[5],hit);

	(void)sprintf(buffer,"HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", fstr);
	(void)write(fd,buffer,strlen(buffer));

	while (	(filesize = read(file_fd, buffer, BUFSIZE)) > 0 ) {
		(void)write(fd,buffer,filesize);
	}
#ifdef LINUX
	sleep(1);
#endif
	exit(1);
}


int main(int argc, char **argv)
{
	int i, port, pid, listenfd, socketfd, hit;
	size_t length;
	static struct sockaddr_in cli_addr; 
	static struct sockaddr_in serv_addr;

	if( argc < 3  || argc > 3 || !strcmp(argv[1], "-?") ) {
		(void)printf("usage: server [port] [server directory] &"
	"\tExample: server 80 ./ &\n\n"
	"\tOnly Supports:");
		for(i=0;extensions[i].ext != 0;i++)
			(void)printf("%s, ",extensions[i].ext);
		exit(0);
	}
	if(chdir(argv[2]) == -1){ 
		(void)printf("ERROR: Can't Change to directory %s\n",argv[2]);
		exit(4);
	}

	if(fork() != 0)
		return 0; 
	(void)signal(SIGCLD, SIG_IGN); 
	(void)signal(SIGHUP, SIG_IGN); 
	for(i=0;i<32;i++)
		(void)close(i);	
	(void)setpgrp();	

	log(LOG,"CHTTPD server starting",argv[1],getpid());

	if((listenfd = socket(AF_INET, SOCK_STREAM,0)) <0)
		log(ERROR, "system call","socket",0);
	port = atoi(argv[1]);
	if(port < 0 || port >60000)
		log(ERROR,"Invalid port number try [1,60000], tried starting on ",argv[1],0);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);
	if(bind(listenfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) <0)
		log(ERROR,"Failed to ","bind",0);
	if( listen(listenfd,64) <0)
		log(ERROR,"Failed to","listen",0);

	for(hit=1; ;hit++) {
		length = sizeof(cli_addr);
		if((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length)) < 0)
			log(ERROR,"Failed to","accept",0);

		if((pid = fork()) < 0) {
			log(ERROR,"Failed to","fork",0);
		}
		else {
			if(pid == 0) {
				(void)close(listenfd);
				web(socketfd,hit,argv[2]);
			} else {
				(void)close(socketfd);
			}
		}
	}
}
