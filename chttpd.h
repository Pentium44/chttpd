// CTHTTPD - Simple Web Server - GPLv2
// Chris Dorman, 2012-2014 (cddo@riseup.net)
// Help from Nickolai Vurgaft (slipperygate@gmail.com)

// definitions
#define BUFSIZE 4096
#define LIST_BUFSIZE 3068
#define ERROR 42
#define SORRY 43
#define LOG   44
#define SEND_ERROR 45

const char *client = "chttpd";
const char *version = "1.2.6";
const char *sys_lable = "Linux";

int forward_slash = 47; // forward slash in ascii