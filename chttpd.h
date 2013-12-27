#define BUFSIZE 4096
#define LIST_BUFSIZE 3068
#define ERROR 42
#define SORRY 43
#define LOG   44
#define SEND_ERROR 45

const char *client = "chttpd";
const char *version = "1.2.2";


// Check if a file exists (index.html)
int file_exists(char *fname) {
	struct stat   buffer;   
	if (stat (fname, &buffer) == 0)
		return 0;
	else
		return 1;
}

// Check if the path is a directory (darklight)
int is_dir(char* p)
{
	char * stripslash;
	struct stat st;
	stripslash = p + 1;
	if (stat(stripslash, &st) == 0 && (st.st_mode & S_IFDIR)) {
		return 1;
	}
	else if (stat(stripslash, &st) == 0 && (st.st_mode & S_IFREG)) {
		return 2;
	}
	return 0;
}
