// CTHTTPD - Simple Web Server - GPLv2
// Chris Dorman, 2012-2014 (cddo@riseup.net)
// Help from Nickolai Vurgaft (slipperygate@gmail.com)
#include "dep.h"
#include "functions.h"

// Addition by OldCoder for replacing HEX values in GET paths.
// BEGINNING OF OLDCODER :P
int hexchartonum (char c)
{
    if ((c >= '0') && (c <= '9')) return c - '0';
    if ((c >= 'a') && (c <= 'f')) return c - 'a' + 10;
    if ((c >= 'A') && (c <= 'F')) return c - 'A' + 10;
    return -1;
}

char *fixpath (char *input)
{
    static char xbuf [1024];
    int ch;
    int ii;
    char *cp;
    strcpy (xbuf, input);

    while ((cp = strchr (xbuf, '%')) != NULL)
    {
        int h1 = hexchartonum (cp [1]);
        if (h1 < 0) return "";
        int h2 = hexchartonum (cp [2]);
        if (h2 < 0) return "";
        int hh = (h1 * 16) + h2;

        *cp = hh;
        for (ii = 0 ;; ii++)
            { cp [ii+1] = ch = cp [ii+3]; if (ch == '\0') break; }
    }

    return xbuf;
}

// END OF OLDCODER :P
