#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include "SortedList.h"

void* threadfunc(void* arg) {
	// keep track of keys inserted
	const char* keys = malloc(nelements * sizeof(char*));

	// insert elements into the list
	for (int i = 0; i < nelements; i++) {
		insert(list, &elements[i]);
		keys[i] = elements[i]->key;
	}

	// get list length
	int len = length(list);

	// look up each of keys inserted & delete each returned element
	SortedListElement_t* element;
	for (int i = 0; i < nelements; i++) {
		element = lookup(list, &keys[i]);
		delete(element);
	}
}

// takes a parameter for the number of parallel threads (--threads=#, default 1)
// takes a parameter for the number of iterations (--iterations=#, default 1)
// takes a parameter to enable the optional critical section yields (--yield=[ids], i for insert, d for delete, and s for searches)
// initializes an empty list.
// creates and initializes (with random keys) the required number (threads × iterations) of list elements. We do this before creating the threads so that this time is not included in our start-to-finish measurement.
// notes the (high resolution) starting time for the run (using clock_gettime(2))
// starts the specified number of threads, each of which will use the above list function to
// 		insert each of its elements (iterations parameter) into the list
// 		gets the list length
// 		look up each of the keys it inserted
// 		deletes each returned element from the list
// 		exits to re-join the parent thread
// wait for all threads to complete, and notes the (high resolution) ending time for the run.
// checks the length of the list to confirm that it is zero, and logs an error to stderr if it is not.
// prints to stdout
// 		the number of operations performed
// 		the total run time (in nanoseconds), and the average time per operation (in nanoseconds).
// exits with a status of zero if there were no errors, otherwise non-zero


void sltest(long nthreads, long niter, char opt_yield) {
	int exit_status = 0;

	// initialize an empty list
	SortedList_t *list;
	list->key = NULL; // head key is null

	// create and initialize (threads x iterations) list elements
	long nelements = nthreads * niter;
	SortedListElement_t *elements = malloc(nelements * sizeof(SortedListElement_t));
	// TODO: random keys????

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

	for (int i = 0; i < nthreads; i++) {
		// TODO: threadfunc args? list head, elements to insert, iterations, .....
		int pthread_ret = pthread_create(&tids[i], NULL, threadfunc, "FIGURE OUT LATER");
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
	int clock_ret = clock_gettime(CLOCK_MONOTONIC, &tp_end);
	if (clock_ret != 0) {
		fprintf(stderr, "Error getting end time\n");
		exit_status = 1;
	}

	// check length of list to confirm that it is zero
	int len = length(list)
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
	long long iterations = 0;
	char opt_yield;

	while (1) {
	    static struct option long_options[] =
	    {
		    { "threads", optional_argument, 0, 't' },
			{ "iter", optional_argument, 0, 'i' },
			{ "sync", optional_argument, 0, 's' },
			{ "yield", optional_argument, 0, 'y' },
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
		      	break;
	      	/* niterations */
	      	case 'i':
				if(optarg) {
					iterations = atoi(optarg);
				}
				else {
					iterations = 1;
				}
	      		break;
	      	/* sync option
	      		m = mutex
	      		s = spinlock
	      		c = compare and swap */
	      	case 's':
	      		if (optarg) {
	      			sync = *optarg;
	      		}
	      		else {
	      			fprintf(stderr, "Invalid sync option\n");
	      			exit(1);
	      		}
	      		break;
	      	/* yield option 
	      		i = insert
	      		d = delete
	      		s = search */
	      	case 'y':
	      		if (optarg) {
	      			opt_yield = atoi(optarg);
	      		}
	      		else {
	      			fprintf(stderr, "Invalid yield option\n");
	      			exit(1);
	      		}
	      		break;
	      	default: 
	      		break;
	  	}
 	}

 	sltest(nthreads, iterations, opt_yield);

 	return 0;
}