#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include "SortedList.h"

// global vars
char sync;
char* opt_yield;
int nlists = 0;
int mutex = 0;
int spin = 0;
static pthread_mutex_t lock;
volatile int lock_s = 0;

SortedList_t* list = NULL;
SortedListElement_t* elements = NULL;
char* keys = NULL;

// generate random key of length len
void random_key(char* s, int len) { // TODO: s as pointer is ok?
	// all possible chars
	static const char alpha[] = "0123456789"
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	// randomly choose one of possible char for each char in s
	for (int i = 0; i < len; i++) {
		s[i] = alpha[rand() % (sizeof(alpha)-1)];
	}

	s[len] = 0; //terminate with nullbyte
}

// determine which list to insert the key into (returns an index)
int hash(const char* key) {
	// hash = key % nlists
	int n = 0;
	int len = strlen(key);
	for (int i = 0; i < len; i++) {
		n += key[i];
	}
	return n % nlists;
}

// need different implementation for insert, lenth, and lookup/delete
void* threadfunc(void* arg) {
	long long nelements = (long long) arg;

	// keep track of keys inserted
	const char* keys = malloc(nelements * sizeof(char*));

	// insert elements into the list
	for (int i = 0; i < nelements; i++) {
		if (mutex) {
			// use pthread_mutex
			pthread_mutex_lock(&lock);
			SortedList_insert(list, &elements[i]);
			pthread_mutex_unlock(&lock);
		}
		else if (spin) {
			// use test and set
			while(__sync_lock_test_and_set(&lock_s, 1));
			SortedList_insert(list, &elements[i]);
			__sync_lock_release(&lock_s, 1);
		}
		else {
			// unprotected
			SortedList_insert(list, &elements[i]);
		}
	}

	// get list length
	int len;
	if (mutex) {
		// pthread_mutex
		pthread_mutex_lock(&lock);
		len = SortedList_length(list);
		pthread_mutex_unlock(&lock);
	}
	else if (spin) {
		// test and set
		while(__sync_lock_test_and_set(&lock_s, 1));
		len = SortedList_length(list);
		__sync_lock_release(&lock_s, 1);
	}
	else {
		// unprotected
		len = SortedList_length(list);
	}

	// look up each of keys inserted & delete each returned element
	SortedListElement_t* target = malloc(sizeof(SortedListElement_t));

	for (int i = 0; i < nelements; i++) {
		if (mutex) {
			pthread_mutex_lock(&lock);
			target = SortedList_lookup(list, &keys[i]);
				if (target == NULL) {
					fprintf(stderr, "Target not found\n");
					// TODO: further error handling?
					pthread_mutex_unlock(&lock);
					continue;
				}
			SortedList_delete(target);
			pthread_mutex_unlock(&lock);
		}
		else if (spin) {
			while(__sync_lock_test_and_set(&lock_s, 1));
			target = SortedList_lookup(list, &keys[i]);
			if (target == NULL) {
					fprintf(stderr, "Target not found\n");
					__sync_lock_release(&lock_s, 1);
					continue;
				}
			SortedList_delete(target);
			__sync_lock_release(&lock_s, 1);
		}
		else {
			target = SortedList_lookup(list, &keys[i]);
			SortedList_delete(target);
		}
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

	// malloc list array
	list = malloc(sizeof(SortedList_t) * nlists);

	// TODO: initialize mutexes and spinlocks

	// initialize lists to empty
	for (int i = 0; i < nlists; i++) {
		list[i].key = NULL;
		list[i].next = NULL; // TODO: check? should this be null?
		list[i].prev = NULL;
	}

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
		int len = rand() % 15; // generate a random size for each key
		keys[i] = malloc(sizeof(char*) * len);
		if (keys[i] == NULL) {
			fprintf(stderr, "Error allocating memory for key\n");
			exit_status = 1;
		}
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

	for (int i = 0; i < nthreads; i++) {
		int pthread_ret = pthread_create(&tids[i], NULL, threadfunc, (void*)nelements);
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
	int len = SortedList_Length(list);
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
	      				break;
			case 's':
	      				spin = 1;
	      				break;
	      			default:
	      				break;
	      		}
	      		break;
	      	/* yield option 
	      		i = insert
	      		d = delete
	      		s = search */
	      	case 'y':
	      		if (optarg) {
	      			// TODO: opt_yield a char or int????? (see flags)
	      			opt_yield = *optarg;
	      		}
	      		else {
	      			fprintf(stderr, "Invalid yield option\n");
	      			exit(1);
	      		}
	      		// get correct opt_yield flags
	      		/* including yield=i
								  =d
								  =is
								  =ds
					etc. */		
	      		int i = 0;
	      		int run = 1;
	      		while (run) {
	      			switch(opt_yield[i]) {
	      				case 'i':
	      					opt_yield |= INSERT_YIELD;
	      					break;
	      				case 'd':
	      					opt_yield |= DELETE_YIELD;
	      					break;
	      				case 's':
	      					opt_yield |= SEARCH_YIELD;
	      					break;
	      				default:
	      					run = 0;
	      					break;
	      			}
	      			i++;
	      		}
	      		break;
	      	case 'l':
	      		if (optarg) {
	      			nlists = atoi(optarg);
	      		}
	      		else {
	      			nlists = 1;
	      		}
	      	default: 
	      		break;
	  	}
 	}

 	sltest(nthreads, iterations, opt_yield);

 	return 0;
}
