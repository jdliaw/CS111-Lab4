#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include "SortedList.h"

// takes a parameter for the number of parallel threads (--threads=#, default 1)
// takes a parameter for the number of iterations (--iterations=#, default 1)
// takes a parameter to enable the optional critical section yields (--yield=[ids], i for insert, d for delete, and s for searches)
// initializes an empty list.
// creates and initializes (with random keys) the required number (threads × iterations) of list elements. We do this before creating the threads so that this time is not included in our start-to-finish measurement.
// notes the (high resolution) starting time for the run (using clock_gettime(2))
// starts the specified number of threads, each of which will use the above list function to
// insert each of its elements (iterations parameter) into the list
// gets the list length
// look up each of the keys it inserted
// deletes each returned element from the list
// exits to re-join the parent thread
// wait for all threads to complete, and notes the (high resolution) ending time for the run.
// checks the length of the list to confirm that it is zero, and logs an error to stderr if it is not.
// prints to stdout
// the number of operations performed
// the total run time (in nanoseconds), and the average time per operation (in nanoseconds).
// exits with a status of zero if there were no errors, otherwise non-zero


void sltest(long nthreads, long niter, char opt_yield) {
	// initialize an empty list
	SortedList_t *list;
	list->key = NULL; // head key is null

	long nelements = nthreads * niter;

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