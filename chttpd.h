#define BUFSIZE 4068
#define ERROR 42
#define SORRY 43
#define LOG   44
#define SEND_ERROR 45

const char *client = "chttpd";
const char *version = "1.1";


// Check if a file exists (index.html)

int file_exists(char *fname) {
	struct stat   buffer;   
	return (stat (fname, &buffer) == 0);
}

int ends_with_slash(char *str) {
	return (str && *str && str[strlen(str) - 1] == '/') ? 0 : 1;
}
