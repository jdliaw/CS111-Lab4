// CS 111 Lab 4

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h> // for clock_gettime
#include <pthread.h>

/* Part 1 */

static long long counter;
static pthread_mutex_t lock;
volatile int lock_s = 0;

int opt_yield;
char sync;

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

/* add function with mutex */

void madd(long long *pointer, long long value) {
	pthread_mutex_lock(&lock);
	long long sum = *pointer + value;
	if (opt_yield)
		pthread_yield();
	*pointer = sum;
	pthread_mutex_unlock(&lock);
}

/* add function with spinlock */

void sadd(long long *pointer, long long value) {
	while(__sync_lock_test_and_set(&lock_s, 1));
	long long sum = *pointer + value;
	if (opt_yield)
		pthread_yield();
	*pointer = sum;
	__sync_lock_release(&lock_s, 1);
}

/* add function with cmp and swap */
void cadd(long long pointer, long long value) {
	long long sum;
	long long orig;
	do {
		orig = *pointer;
		sum = orig + value;
	} while (__sync_val_compare_and_swap(pointer, orig, sum) != orig);
}

/* thread function */
void* threadfunc(void* arg) {
	long iterations = (long)arg;
	long i;
	if (sync == 'm') {
		for (i = 0; i < iterations; i++) {
			madd(&counter, 1);
		}
		for (i = 0; i < iterations; i++) {
			madd(&counter, -1);
		}
	}
	else if (sync == 's') {
		for (i = 0; i < iterations; i++) {
			sadd(&counter, 1);
		}
		for (i = 0; i < iterations; i++) {
			sadd(&counter, -1);
		}
	}
	else if (sync == 'c') {
		for (i = 0; i < iterations; i++) {
			cadd(&counter, 1);
		}
		for (i = 0; i < iterations; i++) {
			cadd(&counter, -1);
		}
	}
	else {
		for (i = 0; i < iterations; i++) {
			add(&counter, 1);
		}
		for (i = 0; i < iterations; i++) {
			add(&counter, -1);
		}
	}
}

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

	// start threads
	unsigned i;
	for (i = 0; i < nthreads; i++) {
		int thread_ret = pthread_create(&tids[i], NULL, threadfunc, (void*)niter);
		// error handling
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
	      	/* sync option
	      		m = mutex
	      		s = spinlock
	      		c = compare and swap */
	      	case 's':
	      		if (optarg) {
	      			sync = *optarg; //sdfslfdslflsd
	      			fprintf(stderr, "sync=%c\n", sync);
	      		}
	      		else {
	      			fprintf(stderr, "Invalid sync option\n");
	      			exit(1);
	      		}
	      		break;
	      	/* yield option */
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
