// CS 111 Lab 4

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h> // for clock_gettime
#include <pthread.h>

/* Part 1 */

static long long counter;

int opt_yield;

struct Threadargs {
	long long counter;
	long iterations;
};

void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    if (opt_yield)
            pthread_yield();
    *pointer = sum;
}

void* thread_func(void* arg) {
  //struct Threadargs *args = (struct Threadargs*)arg;
  //	long long counter = args->counter;
  //	long iterations = args->iterations;
  long iterations = (long)arg;
  long i;
	for (i = 0; i < iterations; i++) {
		add(&counter, 1);
	}
	for (i = 0; i < iterations; i++) {
		add(&counter, -1);
	}
}

/* addtest:
takes a parameter for the number of parallel threads (--threads=#, default 1)
takes a parameter for the number of iterations (--iterations=#, default 1)
initializes a (long long) counter to zero.
notes the (high resolution) starting time for the run (using clock_gettime(2))
starts the specified number of threads, each of which will use the above add function to:
	add 1 to the counter the specified number of times
	add −1 to the counter the specified number of times
	exits to re-join the parent thread
wait for all threads to complete, and notes the (high resolution) ending time for the run.
checks to see if the counter value is zero, and if not log an error message to stderr
prints to stdout
	the total number of operations performed
	the total run time (in nanoseconds), and the average time per operation (in nanoseconds).
If there were no errors, exit with a status of zero, else a non-zero status */

void addtest(long nthreads, long niter) {
	int exit_status = 0;
	counter = 0;
	struct timespec tp;  /* time_t tv_sec; // whole secs >= 0
							long tv_nsec; // nanoseconds */
	int clock_ret = clock_gettime(CLOCK_MONOTONIC, &tp);
	if (clock_ret != 0) {
	  fprintf(stderr, "Error in clock_gettime()\n");
	  exit_status = 1;
	}
	long start_time = tp.tv_nsec; // want "high resolution" aka in ns
	fprintf(stderr, "start_time: %lu\n", start_time);

	//malloc threads
	pthread_t *tids = malloc(nthreads * sizeof(pthread_t));

	// struct for thread args
	struct Threadargs *args = malloc(sizeof(args));
	args->counter = counter;
	args->iterations = niter;

	// start threads
	unsigned i;
	for (i = 0; i < nthreads; i++) {
		int thread_ret = pthread_create(&tids[i], NULL, thread_func, (void*)niter);
		if (thread_ret != 0) {
		  fprintf(stderr,"Error creating threads\n");
		  exit_status = 1;
		}
	}

	// wait for threads to complete, join.
	for (i = 0; i < nthreads; i++) {
		int thread_ret = pthread_join(tids[i], NULL);
		if(thread_ret != 0) {
		  fprintf(stderr, "Error joining threads\n");
			exit_status = 1;
		}
	}

	// get ending time for run
	clock_gettime(CLOCK_MONOTONIC, &tp);
	long end_time = tp.tv_nsec;
	long elapsed_time = end_time - start_time;
	long noperations = nthreads * niter * 2;
	long average_time = elapsed_time / noperations;

	// error message if counter not zero
	if (counter != 0) {
		fprintf(stderr, "ERROR: final count = %lld\n", counter);
		exit_status = 1;
		return;
	}

	// print stuff to stdout
	fprintf(stdout, "%lu threads x %lu iterations x (add + subtract) = %lu operations\n", nthreads, niter, noperations);
	fprintf(stdout, "elapsed time: %lu ns\n", elapsed_time);
	fprintf(stdout, "per operation: %lu ns\n", average_time);

	// exit non-zero status if errors, exit 0 if no errors.
	exit(exit_status);
}

int main(int argc, char **argv) {
	long nthreads = 0;
	long iterations = 0;

	while (1) {
	    static struct option long_options[] =
	    {
		    { "threads", optional_argument, 0, 't' },
			{ "iter", optional_argument, 0, 'i' },
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
	    		if(optarg) {		//threads=<something>
	    			nthreads = atoi(optarg);
	    			fprintf(stderr, "threads=%lu\n", nthreads);
	    		}
	    		else {
	    			nthreads = 1;	//default 1
	    			fprintf(stderr, "threads=%lu\n", nthreads);
	    		}
		      	break;
	      	/* niterations */
	      	case 'i':
				if(optarg) {
					iterations = atoi(optarg);
					fprintf(stderr, "iter=%lu\n", iterations);
				}
				else {
					iterations = 1;
					fprintf(stderr, "iter=%lu\n", iterations);
				}
	      		break;
	      	case 'y':
	      		if (optarg) {
	      			opt_yield = atoi(optarg);
	      			fprintf(stderr, "yield=%d\n", opt_yield);
	      		}
	      		else {
	      			opt_yield = 1;
	      			fprintf(stderr, "yield=%d\n", opt_yield);
	      		}
	      		break;
	      	default: 
	      		break;
	  	}

 	}
 	addtest(nthreads, iterations);
 	return 0;
}
