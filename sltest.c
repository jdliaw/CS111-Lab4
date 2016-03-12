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
int exit_status = 0;

SortedList_t* list = NULL;
SortedListElement_t* elements = NULL;
char** keys = NULL;

// mutex global vars
int mutex = 0;
int spin = 0;
static pthread_mutex_t lock;
volatile int lock_s = 0;

// generate random key of length len
void random_key(char* s, int len) { // TODO: s as pointer is ok?
	// all possible chars
	static const char alpha[] = "0123456789"
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	// randomly choose one of possible char for each char in s
	for (int i = 0; i < len; i++) {
	  int index = rand() % (sizeof(alpha)-1);
	  s[i] = alpha[index];
	}

	s[len] = '\0'; //terminate with nullbyte
}

void* threadfunc(void* arg) {
	int tid = (int) arg;

	// insert elements into the list
	for (int i = 0; i < iterations; i++) {
		long long index = (tid*iterations) + i;
		if (mutex) {
			// use pthread_mutex
		  pthread_mutex_lock(&lock);
			SortedList_insert(list, &elements[index]);
			pthread_mutex_unlock(&lock);
		}
		else if (spin) {
			// use test and set
			while(__sync_lock_test_and_set(&lock_s, 1));
			SortedList_insert(list, &elements[index]);
			__sync_lock_release(&lock_s, 1);
		}
		else {
			// unprotected
			SortedList_insert(list, &elements[index]);
		}
	}

	// get list length
	int len;
	if (mutex) {
		pthread_mutex_lock(&lock);
		len = SortedList_length(list);
		pthread_mutex_unlock(&lock);
	}
	else if (spin) {
		while(__sync_lock_test_and_set(&lock_s, 1));
		len = SortedList_length(list);
		__sync_lock_release(&lock_s, 1);
	}
	else {
		len = SortedList_length(list);
	}

	// look up each of keys inserted & delete each returned element
	SortedListElement_t* target = malloc(sizeof(SortedListElement_t));
	int ret;

	for (int i = 0; i < iterations; i++) {
		long long index = (tid * iterations) + i;
		if (mutex) {
			pthread_mutex_lock(&lock);
			target = SortedList_lookup(list, keys[index]);
			ret = SortedList_delete(target);
			pthread_mutex_unlock(&lock);
		}
		else if (spin) {
			while(__sync_lock_test_and_set(&lock_s, 1));
			target = SortedList_lookup(list, keys[index]);
			ret = SortedList_delete(target);
			__sync_lock_release(&lock_s, 1);
		}
		else {
		  	target = SortedList_lookup(list, keys[index]);
		  	ret = SortedList_delete(target);
		}
		// Error handling
		if (target == NULL) {
			fprintf(stderr, "Target not found\n");
			exit_status = 1;
		}

		// Error handling
		if (ret != 0) {
			fprintf(stderr, "Failed to delete target %s from SortedList\n", target->key);
			exit_status = 1;
		}
	}
}

void sltest(long nthreads, long niter, char opt_yield) {
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

	// malloc array for random keys
	keys = malloc(sizeof(char*) * nelements);
	if (keys == NULL) {
		fprintf(stderr, "Error allocating memory for keys array\n");
		exit_status = 1;
	}

	// generate array of random keys
	for (int i = 0; i < nelements; i++) {
        int len = (rand() % 15) + 1; // generate a random size for each key
		keys[i] = malloc(sizeof(char*) * len);
		random_key(keys[i], len);
	}

	// initialize elements with random keys
	for (int i = 0; i < nelements; i++) {
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

	// create and start threads
	pthread_t *tids = malloc(nthreads * sizeof(pthread_t));
	if (tids == NULL) {
		fprintf(stderr, "Error allocating memory for threads\n");
		exit_status = 1;
	}

	for (int i = 0; i < nthreads; i++) {
		int pthread_ret = pthread_create(&tids[i], NULL, threadfunc, (void*)i);
		if (pthread_ret != 0) {
			fprintf(stderr, "Error creating threads\n");
			exit_status = 1;
		}
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
	      	case 's':
	      		if (optarg) {
	      			sync = *optarg;
	      		}
	      		else {
	      			fprintf(stderr, "Invalid sync option\n");
	      			exit(1);
	      		}
	      		// set sync flags accordingly
	      		switch(sync) {
	      			case 'm':
	      				mutex = 1;
	      				fprintf(stderr, "sync = m\n");
	      				break;
					case 's':
	      				spin = 1;
	      				fprintf(stderr, "sync = s\n");
	      				break;
	      			default:
	      				break;
	      		}
	      		break;
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

 	sltest(nthreads, iterations, opt_yield);
	return 0;

}
