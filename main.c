// CS 111 Lab 4

#include <stdio.h>

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
}