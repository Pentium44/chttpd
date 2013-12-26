# Compile chttpd
CC=gcc
EXECUTABLE=chttpd

chttpd: chttpd.c
	$(CC) -o $(EXECUTABLE) chttpd.c

