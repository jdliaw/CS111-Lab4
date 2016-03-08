// CS 111 Lab 4

#include <stdio.h>

// Part 1
void add(long long *pointer, long long value) {
        long long sum = *pointer + value;
        *pointer = sum;
    }