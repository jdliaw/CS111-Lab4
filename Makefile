# CS 111 Lab 4 Makefile

OPTIMIZE = -O2

# gcc -std=gnu99 -lrt main.c -o addtest

CC = gcc
CFLAGS = $(OPTIMIZE) -std=gnu99 -lrt 
# may need -lpthread for pthread_mutex stuff

default: addtest

addtest: main.c
	$(CC) $(CFLAGS) main.c -o $@

