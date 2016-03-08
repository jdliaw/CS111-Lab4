// CS 111 Lab 4

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h> // for clock_gettime
#include <pthread.h>

/* Part 1 */

void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    *pointer = sum;
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

void addtest(int nthreads, int niter) {
	// nthreads, niter default = 1
	long long counter = 0;
	pthread_t *threads;
	clockid_t clk_id;
	struct timespec *tp;
	/* time_t tv_sec; // whole secs >= 0
	long tv_nsec; // nanoseconds */
	clock_gettime(clk_id, tp);
	time_t start_time = tp->tv_sec; // want "high resolution".. does that mean ns??
	// start threads
	for (unsigned i = 0; i < nthreads; i++) {
		int ret = pthread_create(&thread[i], NULL, thread_func, args);
		if (ret != 0) {
			// error handling
		}
	}
	// use add function to do stuff

	// wait for threads to complete
	// get ending time for run

	if (counter != 0) {
		fprintf(stderr, "some error message");
		return;
	}

	// print stuff to stdout
	fprintf(stdout, )
}

int main(int argc, char **argv) {
	 while (1) {
    static struct option long_options[] =
    {
	    { "threads=", required_argument, 0, 't' },
		{ "iterations=", required_argument, 0, 'i' },
		{ 0, 0, 0, 0 }
    };
    int option_index = 0;
    c = getopt_long(argc, argv, "", long_options, &option_index);

    /* Detect the end of options */
    if (c == -1) {
      break;
    }

    switch (c)
      {
      /* nthreads */ 
      case 't':
      	if (optind < argc) {
	    	int nthreads = argv[optind];
	  	}
      	break;
      /* niterations */
      case 'i':
      	if (optind < argc) {
	    	int niter = argv[optind];
	  	}
      	break;
      default: 
      	break;
}