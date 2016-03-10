# CS 111 Lab 4 Makefile

CC = gcc
CFLAGS = -std=gnu99 -pthread -lrt 

default: addtest

addtest: main.c
	$(CC) $(CFLAGS) main.c -o $@

clean:
	rm -fr *.o *~ *.tar.gz *.tmp core *.core addtest

.PHONY: all clean