# CS 111 Lab 4 Makefile

CC = gcc
CFLAGS = -std=gnu99 -pthread -lrt 
DISTDIR = lab4-jenniferliawbrandonliu

default: sltest addtest

addtest: addtest.c
	$(CC) $(CFLAGS) addtest.c -o $@

sltest: sltest.c
	$(CC) $(CFLAGS) sltest.c -o $@

check:

dist: $(DISTDIR).tar.gz

SOURCES = README Makefile addtest.c sltest.c SortedList.h answers.txt graphs.pdf
$(DISTDIR).tar.gz: $(SOURCES) check-dist
	rm -fr $(DISTDIR)
	tar -czf $@.tmp --transform='s,^,$(DISTDIR)/,' $(SOURCES)
	./check-dist $(DISTDIR)
	mv $@.tmp $@

clean:
	rm -fr *.o *~ *.tar.gz *.tmp core *.core addtest sltest $(DISTDIR)

.PHONY: all dist check clean