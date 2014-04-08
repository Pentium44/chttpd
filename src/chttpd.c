// CTHTTPD - Simple Web Server - GPLv2
// Chris Dorman, 2012-2014 (cddo@riseup.net)
// Help from Nickolai Vurgaft (slipperygate@gmail.com)

#include "chttpd.h"
#include "mimetypes.h"
#include "check.h"
#include "dep.h"


const char *client = "chttpd";
const char *version = "1.3.0";
const char *sys_lable = "Linux";

int forward_slash = 47; // forward slash in ascii

//
//
// /// Simple configuration parser - Pacific
//
//

#define CONFBUF 1024
char *equal = "=";

struct config
{
	// struct for htdocs string
	char htdocs[CONFBUF];
	// struct for port string
	char port[CONFBUF];
	// struct for config status (failed to open config etc)
	char status[CONFBUF];
	// struct for cgi 
	char cgi[CONFBUF];
};
    
// write to struct
struct config get_config(char *filename)
{
	struct config configstruct;

	// open config as readable
	FILE *file = fopen (filename, "r");
    
    // check if config opens successfully
	if( access( filename, F_OK ) == -1 ) {
		memcpy(configstruct.status,"1",1);
	}
	else
	{
		memcpy(configstruct.status,"0",1);
	}
    
    // if file is null, end
	if (file != NULL)
	{
		// line buffer for config
		char line[CONFBUF];
		// int used to track config line
		int i = 0;
     
		// config while loop, loops fgets until end of file
		while(fgets(line, sizeof(line), file) != NULL)
		{
			
			char *cfline; // setup string
			cfline = strtok(line, equal);
		
			
			// if line is commented out, skip
			if (strncmp("#",line,1)==0)
				continue;
				
			if (strncmp("HTDOCS",cfline,6)==0 || strncmp("htdocs",cfline,6)==0) {
				
					cfline = strtok(NULL, equal); // call strtok to get value
					// if newline is found, remove newline from string
					if(cfline[strlen(cfline)-1] == '\n')
						cfline[strlen(cfline)-1] = 0;
					// write htdocs path to struct
					memcpy(configstruct.htdocs,cfline,strlen(cfline));
					
			} else if (strncmp("PORT",cfline,4)==0 || strncmp("port",cfline,4)==0){
				
					cfline = strtok(NULL, equal); // call strtok to get value
					// if newline is found, remove newline from string
					if(cfline[strlen(cfline)-1] == '\n')
						cfline[strlen(cfline)-1] = 0;
					// write port to struct
					memcpy(configstruct.port,cfline,strlen(cfline));
					
			} else if (strncmp("ENABLE_CGI",cfline,10)==0 || strncmp("enable_cgi",cfline,10)==0){
					
					cfline = strtok(NULL, equal); // call strtok to get value
					// if newline is found, remove newline from string
					if(cfline[strlen(cfline)-1] == '\n')
						cfline[strlen(cfline)-1] = 0;
					// write cgi status to struct
					memcpy(configstruct.cgi,cfline,strlen(cfline));
			}
                           
		} // End while
	} // End if file
           
	fclose(file);   
           
	return configstruct;
     
}

void log(int type, char *s1, char *s2, int num)
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
	case ERROR: (void)sprintf(logbuffer,"%s %s (%s) %s : Error: %s %s",client, version, sys_lable, timebuf, s1, s2); break;
	case SORRY: (void)sprintf(logbuffer, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<html><head><title>CHTTPD: Error</title>\n</head><body><h2>CHTTPD Error:</h2> %s %s <hr /><address>%s %s (%s)</address></body></html>\r\n", s1, s2, client, version, sys_lable); 
				(void)write(num,logbuffer,strlen(logbuffer));
				break;
	case LOG: (void)sprintf(logbuffer,"%s %s (%s) %s : Info: %s:%s:5d",client, version, sys_lable, timebuf, s1, s2); break;
	case SEND_ERROR: (void)sprintf(logbuffer,"HTTP/1.0 500 Internal Server Error\r\nContent-Type: text/html\r\n\r\n<html><head><title>CHTTPD: Found error</title></head><body><h2>Index error</h2>%s<hr /><address>%s %s (%s)</address></body></html>\r\n", s1, client, version, sys_lable);
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

void web(int fd, int hit, char *datadir, char *cgistatus)
{
	int j, file_fd, buflen, len, contentfs;
	long i, filesize;
	char * fstr;
	char * exten;
	char * path; 
	char * protocol;
	char * stripslash_index;
	char * stripslash_path;
	size_t pathlen; 
	static char buffer[BUFSIZE+1];
	static char listbuffer[LIST_BUFSIZE*2];

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
	
	if( !strncmp(&buffer[0],"GET /\0",6) || !strncmp(&buffer[0],"get /\0",6) ) 
		if(file_exists("index.html") == 0) {
			(void)strcpy(buffer,"GET /index.html");
		} else {
			DIR *d = opendir(".");
			struct dirent* dirp; // struct dirp for directory listing
			
			(void)sprintf(listbuffer,"HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n");
			(void)write(fd,listbuffer,strlen(listbuffer)); // write header socket
			
			(void)sprintf(listbuffer,"<!DOCTYPE html>\r\n"
									"<html>\r\n"
									"<head>\r\n"
									"\t<title>Directory listing of /</title>\r\n"
									"</head>\r\n"
									"<body>\r\n"
									"\t<h2>Directory listing of /</h2>\r\n"
									"\t<hr />\r\n<table>\r\n");
			(void)write(fd,listbuffer,strlen(listbuffer)); // write list html to socket
			
			// There is no parent directory at the root of the web servers filesystem xD
			//(void)sprintf(listbuffer,"\t<tr><td><a href=\"..\">Parent Directory</a></td></tr>\r\n");
			//(void)write(fd,listbuffer,strlen(listbuffer));
			
			// Start listing files and directories
			while ((dirp = readdir(d)))
			{
				if (dirp->d_name[0] == '.')
					continue;
					
				(void)sprintf(listbuffer,"\t<tr><td><a href=\"%s\">%s</a></td></tr>\r\n", dirp->d_name, dirp->d_name);
				(void)write(fd,listbuffer,strlen(listbuffer));
			}
			(void)sprintf(listbuffer,"\t</table>\r\n<hr /><address>%s %s (%s)</address>\r\n</body>\r\n</html>\r\n", client, version, sys_lable);
			(void)write(fd,listbuffer,strlen(listbuffer));
			exit(0);
		}
	
	
	// set uri path
	path = strchr(buffer,' '); 
		path++; 
	// get protocol
	protocol = strchr(path,' ');
		protocol++;
		
	pathlen = strlen(path);
	if(is_dir(path) == 1) {
		if(path[pathlen - 1] != forward_slash) // if there is no "/" at the end of the url, add it
		{
			strcat(path,"/");
			(void)sprintf(listbuffer,"HTTP/1.0 301 Moved Permanently\r\nLocation: %s\r\n\r\n", path); //header to buffer
			(void)write(fd,listbuffer,strlen(listbuffer)); // write header to socket
			//(void)sprintf(listbuffer,"<html><meta http-equiv=\"refresh\" content=\"0;url=%s\"></html>",path);
			//(void)write(fd,listbuffer,strlen(listbuffer)); // write redirect
			exit(0); // stop here, let the browser reconnect with a new url
		}	
	}
		
	// Check if directory was requested, if so, send index.html
	if (is_dir(path) == 1) {
		char getindex[PATH_MAX];
		
		strcpy(getindex,path);
		strcat(getindex,"index.html");
		
		stripslash_index = getindex + 1; // directory + index (for index redirection)
		stripslash_path = path + 1; // get full path
		
		if(file_exists(stripslash_index) != 0) 
		{
			DIR *d = opendir(stripslash_path);
			struct dirent* dirp; // struct dirp for directory listing
			
			(void)sprintf(listbuffer,"HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n");
			(void)write(fd,listbuffer,strlen(listbuffer)); // write header socket
			
			(void)sprintf(listbuffer,"<!DOCTYPE html>\r\n"
									"<html>\r\n"
									"<head>\r\n"
									"\t<title>Directory listing of %s</title>\r\n"
									"</head>\r\n"
									"<body>\r\n"
									"\t<h2>Directory listing of %s</h2>\r\n"
									"\t<hr />\r\n<table>\r\n", path, path);
			(void)write(fd,listbuffer,strlen(listbuffer)); // write list html to socket
			
			(void)sprintf(listbuffer,"\t<tr><td><a href=\"..\">Parent Directory</a></td></tr>\r\n");
			(void)write(fd,listbuffer,strlen(listbuffer));
			
			// Start listing files and directories
			while ((dirp = readdir(d)))
			{
				if (dirp->d_name[0] == '.')
					continue;
				(void)sprintf(listbuffer,"\t<tr><td><a href=\"%s\">%s</a></td></tr>\r\n", dirp->d_name, dirp->d_name);
				(void)write(fd,listbuffer,strlen(listbuffer));
			}
			(void)sprintf(listbuffer,"\t</table>\r\n<hr /><address>%s %s (%s)</address>\r\n</body>\r\n</html>\r\n", client, version, sys_lable);
			(void)write(fd,listbuffer,strlen(listbuffer));
			exit(0);
		} 
		else 
		{
			strcat(path,"index.html");
		}
	}
	
	// Check file extensions and mime types before sending headers
	buflen=strlen(buffer);
	fstr = (char *)0;
	exten = (char *)0;
	for(i=0;extensions[i].ext != 0;i++) {
		len = strlen(extensions[i].ext);
		if( !strncmp(&buffer[buflen-len], extensions[i].ext, len)) {
			fstr =extensions[i].filetype;
			exten =extensions[i].ext;
			break;
		}
	}
	
	/* Just download the file if the extension is missing :D */
	//if(atoi(fstr) == 0) strcpy(fstr,"application/octet-stream");
	
	if(strncmp("server/log",fstr,10)==0) log(SORRY,"Cannot retrieve server logs, forbidden!",buffer,fd);

	if(( file_fd = open(&buffer[5],O_RDONLY)) == -1) 
		log(SORRY, "failed to open file",&buffer[5],fd);

	if(!strcmp(cgistatus, "yes")) {
		if(strncmp("server/cgi",fstr,10)==0) {
			(void)do_cgi(file_fd,fd,datadir);
			exit(0);
		}
	}
	else
	{
		if(strncmp("server/cgi",fstr,10)==0) {
			log(SORRY, "CGI disabled - ", "Cannot access CGI script", fd);
		}
	}

		
	struct stat filesz;
	stat(&buffer[5], &filesz);
	contentfs = filesz.st_size;

	log(LOG,"SEND",&buffer[5],hit);

	(void)sprintf(buffer,"HTTP/1.0 200 OK\r\nContent-Type: %s\r\n", fstr);
	(void)write(fd,buffer,strlen(buffer));
	
	// Add content length to http header
	(void)sprintf(buffer,"Content-Length: %d\r\n\r\n", contentfs);
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
	socklen_t length;
	static struct sockaddr_in cli_addr; 
	static struct sockaddr_in serv_addr;
	
	struct config configstruct; // config struct

	if(argc > 2 || argc < 2) {
		(void)printf("usage: chttpd [chttpd config] &\n"
	"Example: chttpd /path/to/config.conf &\n");
		exit(0); // give exit code error
	}
	
	if(argc == 2) {
		configstruct = get_config(argv[1]);
		if(atoi(configstruct.status) == 1) {
			(void)printf("ERROR: Can't find configuration file at %s.\n", argv[1]);
			exit(1); // give exit code error
		}
	}
	
	//
	// Parse the config file
	//
	
	(void)printf("chttpd: set port: %s\n", configstruct.port);
	(void)printf("chttpd: set htdocs directory: %s\n", configstruct.htdocs);
	(void)printf("chttpd: CGI enabled: %s\n", configstruct.cgi);
	
	if(chdir(configstruct.htdocs) == -1) {
		(void)printf("Warning: failed to chdir Errno: %d\n", errno);
		(void)printf("Warning: Failed to set htdocs value: %s\n", configstruct.htdocs);
		exit(1);
	}
	
	if(fork() != 0)
		return 1; 
		
	(void)signal(SIGCLD, SIG_IGN); 
	(void)signal(SIGHUP, SIG_IGN); 
	for(i=0;i<32;i++)
		(void)close(i);	
	(void)setpgrp();	

	port = (int) strtol(configstruct.port, NULL, 0);

	log(LOG,"CHTTPD server starting",configstruct.port,getpid());

	if((listenfd = socket(AF_INET, SOCK_STREAM,0)) <0)
		log(ERROR, "system call","socket",0);
	if(port < 0 || port > 60000)
		log(ERROR,"Invalid port number try [1,60000], tried starting on ",configstruct.port,0);
	
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);
	
	if(bind(listenfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) <0)
		log(ERROR,"Failed to ","bind",0);
	if( listen(listenfd,64) <0)
		log(ERROR,"Failed to","listen",0);

	for(hit=1; ;hit++) {
		length = sizeof(cli_addr);
		if((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, (socklen_t*) &length)) < 0)
			log(ERROR,"Failed to","accept",0);

		if((pid = fork()) < 0) {
			log(ERROR,"Failed to","fork",0);
		}
		else {
			if(pid == 0) {
				(void)close(listenfd);
				web(socketfd,hit,configstruct.htdocs,configstruct.cgi);
			} else {
				(void)close(socketfd);
			}
		}
	}
}
