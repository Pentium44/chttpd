// CTHTTPD - Simple Web Server - GPLv2
// Chris Dorman, 2012-2014 (cddo@riseup.net)
// Help from Nickolai Vurgaft (slipperygate@gmail.com)
#include "check.h"
#include "dep.h"

// Check if a file exists (index.html)
int file_exists(char *fname) {
	struct stat   buffer;   
	if (stat (fname, &buffer) == 0)
		return 0;
	else
		return 1;
}

// Check if the path is a directory (darklight)
int is_dir(char* p) {
	char * stripslash;
	struct stat st;
	stripslash = p + 1; // strip the first forward 'slash' from the string
	if (stat(stripslash, &st) == 0 && (st.st_mode & S_IFDIR)) {
		return 1;
	}
	else if (stat(stripslash, &st) == 0 && (st.st_mode & S_IFREG)) {
		return 2;
	}
	return 0;
}
