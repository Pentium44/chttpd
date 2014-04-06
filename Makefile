# Makefile - (C) Chris Dorman, 2013 <cddo@riseup.net>

CC=gcc
FLAGS=-w

all: chttpd

chttpd: chttpd.o
		$(CC) chttpd.o check.o -o chttpd
		
chttpd.o: chttpd.c check.c
		$(CC) -c chttpd.c check.c $(FLAGS)
		
clean:
		rm *o chttpd
		
install: 
		cp chttpd /usr/sbin
