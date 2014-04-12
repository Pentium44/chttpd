# Makefile - (C) Chris Dorman, 2013 <cddo@riseup.net>

CC=gcc
FLAGS=-w

all: chttpd

chttpd: chttpd.o
		$(CC) chttpd.o check.o cgi.o -o chttpd
chttpd.o: 
		$(CC) -c src/chttpd.c $(FLAGS)
		$(CC) -c src/check.c $(FLAGS)
		$(CC) -c src/cgi.c $(FLAGS)
clean:
		rm *o chttpd
		
install: 
		cp chttpd /usr/sbin
		cp inc/chttpd.conf /etc
