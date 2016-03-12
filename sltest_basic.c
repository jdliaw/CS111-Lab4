#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include <string.h>
#include "SortedList.h"

// global vars
char sync;
int opt_yield;
long long iterations;

SortedList_t* list = NULL;
SortedListElement_t* elements = NULL;

char** keys = NULL;

// generate random key of length len
void random_key(char* s, int len) { // TODO: s as pointer is ok?
	// all possible chars
	static const char alpha[] = "0123456789"
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	//fprintf(stderr, "Alpha size: %d\n", sizeof(alpha));
	//	fprintf(stderr, "alpha[62]: %c\talpha[61]: %c\n", alpha[62], alpha[61]);
	// randomly choose one of possible char for each char in s
	for (int i = 0; i < len; i++) {
	  int index = rand() % (sizeof(alpha)-1);
	  //	  fprintf(stderr, "Generating random key\tIndex: %d\tKey: %c\n",index, alpha[index]);
	  s[i] = alpha[index];
	}

	s[len] = '\0'; //terminate with nullbyte
}

// need different implementation for insert, lenth, and lookup/delete
void* threadfunc(void* arg) {
	fprintf(stderr, "Entered threadfunc\n");
	//long long iterations = (long long) arg;
	int tid = (int) arg;
	fprintf(stderr, "tid = %d\n", tid);
	fprintf(stderr, "iterations = %llu\n", iterations);


	// insert elements into the list
	for (int i = 0; i < iterations; i++) {
         	fprintf(stderr, "Inserting element with key %s\n", elements[i].key);
		SortedList_insert(list, &elements[(tid*iterations) + i]);
	}

	// get list length
	int len = SortedList_length(list);
	fprintf(stderr, "len = %d\n", len);

	// look up each of keys inserted & delete each returned element
	SortedListElement_t* target = malloc(sizeof(SortedListElement_t));

	for (int i = 0; i < iterations; i++) {
		target = SortedList_lookup(list, keys[i]);
		if (target == NULL) {
			fprintf(stderr, "Target not found\n");
			// TODO: error handling
		}
		else
		  fprintf(stderr, "Target key: %s\n", target->key);

		int ret = SortedList_delete(target);
		if (ret != 0) {
			fprintf(stderr, "Failed to delete target from SortedList\n");
			// TODO: error handling
		}
	}
}

void sltest(long nthreads, long niter, char opt_yield) {
	int exit_status = 0;

	// malloc list array
	list = malloc(sizeof(SortedList_t));
	list->key = NULL;
	list->next = list;
	list->prev = list;

	// create and initialize (threads x iterations) list elements
	long nelements = nthreads * niter;
	elements = malloc(nelements * sizeof(SortedListElement_t));
	if (elements == NULL) {
		fprintf(stderr, "Error allocating memory for list elements\n");
		exit_status = 1;
	}
	else
		fprintf(stderr, "Success in allocating list array\n");

	// malloc array for random keys
	keys = malloc(sizeof(char*) * nelements);
	if (keys == NULL) {
		fprintf(stderr, "Error allocating memory for keys array\n");
		exit_status = 1;
	}


	//char* p = keys;
	// generate array of random keys
	for (int i = 0; i < nelements; i++) {
                int len = (rand() % 15) + 1; // generate a random size for each key
		keys[i] = malloc(sizeof(char*) * len);
		random_key(keys[i], len);
	}

	fprintf(stderr, "did we get here\n");
	// initialize elements with random keys
	for (int i = 0; i < nelements; i++) {
		fprintf(stderr, "Initializing element %d with key %s\n", i, keys[i]);
		elements[i].key = keys[i];
		elements[i].next = NULL;
		elements[i].prev = NULL;
	}

	// get start time
	struct timespec tp_start;
	struct timespec tp_end;

	int clock_ret = clock_gettime(CLOCK_MONOTONIC, &tp_start);
	if (clock_ret != 0) {
		fprintf(stderr, "Error getting start time\n");
		exit_status = 1;
	}
	else
		fprintf(stderr, "Got start time\n");

	// create and start threads
	pthread_t *tids = malloc(nthreads * sizeof(pthread_t));
	if (tids == NULL) {
		fprintf(stderr, "Error allocating memory for threads\n");
		exit_status = 1;
	}
	else
		fprintf(stderr, "Success in creating thread array\n");

	for (int i = 0; i < nthreads; i++) {
		int pthread_ret = pthread_create(&tids[i], NULL, threadfunc, (void*)i);
		if (pthread_ret != 0) {
			fprintf(stderr, "Error creating threads\n");
			exit_status = 1;
		}
		else
			fprintf(stderr, "Success creating thread %d\n", i);
	}

	// wait for all threads to complete
	for (int i = 0; i < nthreads; i++) {
		int pthread_ret = pthread_join(tids[i], NULL);
		if (pthread_ret != 0) {
			fprintf(stderr, "Error joining threads\n");
			exit_status = 1;
		}
	}

	// get end time
	clock_ret = clock_gettime(CLOCK_MONOTONIC, &tp_end);
	if (clock_ret != 0) {
		fprintf(stderr, "Error getting end time\n");
		exit_status = 1;
	}

	// check length of list to confirm that it is zero
	int len = SortedList_length(list);
	fprintf(stderr,"Final length = %d", len);
	if (len != 0) {
		fprintf(stderr, "ERROR: final length = %lld\n", len);
		exit_status = 1;
	}

	long long noperations = nthreads * niter * 2 * 50; // ????
	long long elapsed_time = 1000000000 * (tp_end.tv_sec - tp_start.tv_sec) + tp_end.tv_nsec - tp_start.tv_nsec;
	long long average_time = elapsed_time / noperations;

	//print stuff to stdout
	fprintf(stdout, "%lli threads x %lli iterations x (insert + lookup/del) + (100/2 avg len) = %lli operations\n", nthreads, niter, noperations);
	fprintf(stdout, "elapsed time: %lld ns\n", elapsed_time);
	fprintf(stdout, "per operation: %lld ns\n", average_time);

	// exit non-zero status if errors, exit 0 if no errors.
	exit(exit_status);
}

int main(int argc, char **argv) {
	long long nthreads = 0;
	//	long long iterations = 0;
	char* yield;

	while (1) {
	    static struct option long_options[] =
	    {
		    { "threads", optional_argument, 0, 't' },
			{ "iter", optional_argument, 0, 'i' },
			{ "sync", optional_argument, 0, 's' },
			{ "yield", optional_argument, 0, 'y' },
			{ "lists", optional_argument, 0, 'l'},
			{ 0, 0, 0, 0 }
	    };
	    int option_index = 0;
	    int c = getopt_long(argc, argv, "", long_options, &option_index);

	    /* Detect the end of options */
	    if (c == -1) {
	      break;
	    }

	    switch (c)
	    {
	      	/* nthreads */ 
	    	case 't':
	    		if(optarg) {
	    			nthreads = atoi(optarg);	       
	    		}
	    		else {
	    			nthreads = 1;	//default 1
	    		}
	    		fprintf(stderr, "Threads: %lu\n", nthreads);
		      	break;
	      	/* niterations */
	      	case 'i':
				if(optarg) {
					iterations = atoi(optarg);
				}
				else {
					iterations = 1;
				}
				fprintf(stderr, "Iterations: %lu\n", iterations);
	      		break;
	      	/* sync option
	      		m = mutex
	      		s = spinlock s*/
	    //   	case 's':
	    //   		if (optarg) {
	    //   			sync = *optarg;
	    //   		}
	    //   		else {
	    //   			fprintf(stderr, "Invalid sync option\n");
	    //   			exit(1);
	    //   		}
	    //   		// set sync flags accordingly
	    //   		switch(sync) {
	    //   			case 'm':
	    //   				mutex = 1;
	    //   				break;
					// case 's':
	    //   				spin = 1;
	    //   				break;
	    //   			default:
	    //   				break;
	    //   		}
	    //   		break;
	      	/* yield option 
	      		i = insert
	      		d = delete
	      		s = search */
	      	// case 'y':
	      	// 	if (optarg) {
	      	// 		// TODO: opt_yield a char or int????? (see flags)
	      	// 		yield = optarg;
	      	// 	}
	      	// 	else {
	      	// 		fprintf(stderr, "Invalid yield option\n");
	      	// 		exit(1);
	      	// 	}
	      		// get correct opt_yield flags
	      		/* including yield=i
								  =d
								  =is
								  =ds
					etc. */		
	      	// 	int i = 0;
	      	// 	int run = 1;
	      	// 	while (run) {
	      	// 		switch(yield[i]) {
	      	// 			case 'i':
	      	// 				opt_yield |= INSERT_YIELD; // opt_yield should be an int bc insert_yield is int
	      	// 				break;
	      	// 			case 'd':
	      	// 				opt_yield |= DELETE_YIELD;
	      	// 				break;
	      	// 			case 's':
	      	// 				opt_yield |= SEARCH_YIELD;
	      	// 				break;
	      	// 			default:
	      	// 				run = 0;
	      	// 				break;
	      	// 		}
	      	// 		i++;
	      	// 	}
	      	// 	break;
	      	// case 'l':
	      	// 	if (optarg) {
	      	// 		nlists = atoi(optarg);
	      	// 	}
	      	// 	else {
	      	// 		nlists = 1;
	      	// 	}
	      	default: 
	      		break;
	  	}
 	}

 	fprintf(stderr, "About to enter sltest\n");
 	sltest(nthreads, iterations, opt_yield);
	return 0;

}
