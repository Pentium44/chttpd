# Makefile - (C) Chris Dorman, 2013 <cddo@riseup.net>

CC=gcc
FLAGS=-w

all: chttpd

chttpd: chttpd.o
		$(CC) chttpd.o check.o cgi.o -o chttpd
		
chttpd.o: chttpd.c check.c cgi.c
		$(CC) -c chttpd.c $(FLAGS)
		$(CC) -c check.c $(FLAGS)
		$(CC) -c cgi.c $(FLAGS)
clean:
		rm *o chttpd
		
install: 
		cp chttpd /usr/sbin
		cp chttpd.conf /etc
