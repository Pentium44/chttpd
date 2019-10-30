# Makefile - (C) Chris Dorman, 2013 <cddo@riseup.net>

PREFIX = /usr/local
CC = gcc

CFLAGS = -O2 -Wall -Wextra
LDFLAGS = 

BIN = chttpd
CONFIGFILE = inc/chttpd.conf
OBJECTS = src/chttpd.o src/functions.o src/cgi.o src/check.o src/log.o

all: main

fresh: clean all

main: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(BIN) $(LDFLAGS) $(CFLAGS)
	
clean:
	rm -f $(OBJECTS) $(BIN)
	
install:
	cp $(BIN) $(PREFIX)/bin/$(BIN)
	cp $(CONFIGFILE) /etc/chttpd.conf
	cp inc/$(BIN) /etc/init.d/$(BIN)
