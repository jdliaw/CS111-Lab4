# CS 111 Lab 4 Makefile

CC = gcc
CFLAGS = -std=gnu99 -pthread -lrt 
DISTDIR = lab4-jenniferliawbrandonliu

default:

addtest: addtest.c
	$(CC) $(CFLAGS) addtest.c -o $@

SortedList: SortedList.c SortedList.h
	$(CC) -c SortedList.c 
	# generates object file SortedList.o
	# gcc SortedList.o sltest.o -o sltest 
	# generates executable

sltest: sltest.c
	$(CC) $(CFLAGS) SortedList.o sltest.c -o $@

check:

dist: $(DISTDIR).tar.gz

SOURCES = README Makefile addtest.c sltest.c SortedList.h SortedList.c answers.txt
$(DISTDIR).tar.gz: $(SOURCES) check-dist
	rm -fr $(DISTDIR)
	tar -czf $@.tmp --transform='s,^,$(DISTDIR)/,' $(SOURCES)
	./check-dist $(DISTDIR)
	mv $@.tmp $@

clean:
	rm -fr *.o *~ *.tar.gz *.tmp core *.core addtest

.PHONY: all dist check clean