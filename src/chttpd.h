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

void do_chttpd_log(int type, char *s1, char *s2, int num);