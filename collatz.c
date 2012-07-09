/*
 * Prints the A006877 sequence of numbers that break the record for
 * stopping time in the Collatz algorithm
 *
 * Written by Shawn Paul Smith
 * Public Domain
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define uint unsigned int
#define uint16 unsigned short
#define REALLOC_FCT 2
#define COLLATZ_LIM 360000000

/* Defines a convenient wrapper around a uint16 array. */
typedef struct {
    uint16 * ar;
    uint size;
    uint cap;
} uint16ArrayStr, *uint16Array;

/* Creates a convenient wrapper around a uint16 array. */
uint16Array uint16ArrayInit(uint capacity) {
    uint16Array results = (uint16Array) malloc(sizeof(uint16ArrayStr));
    results->ar = (uint16 *) malloc(sizeof(uint16) * capacity);
    results->cap = capacity;
    results->size = 0;
    return results;
}

/* Frees up ram allocated to given array. */
void uint16ArrayFree(uint16Array tofree) {
    free(tofree->ar);
    free(tofree);
}

/* Appends a value to the given array. */
void uint16ArrayAppend(uint16Array uintArr, uint16 append) {
    //Ensure there is room to append, reallocing if needed
    if (uintArr->size >= uintArr->cap) {
        uint newcap = uintArr->cap * REALLOC_FCT;
        uintArr->ar = (uint16 *) realloc(uintArr->ar, sizeof(uint16) * newcap);
        if (uintArr->ar == NULL) {
            printf("Failed to grow array, buy more ram\n");
            return;
        } else {
            uintArr->cap = newcap;
        }
    }
    uintArr->ar[uintArr->size] = append;
    uintArr->size++;
}

/* Determines the Collatz algo steps from n until 1,
   reading cached values from given array. */
uint16 collatz(uint16Array cache, uint n) {
    if (n < cache->size) { // In cache
        return cache->ar[n];
    } else if ((n & 1) == 0) {// even
        return 1 + collatz(cache, n>>1);
    } // odd
    return 1 + collatz(cache, (3*n)+1);
}

int main(void) {
    // cache->ar[n] holds how many steps of the
    // collatz algo it takes to get from n to 1.
    uint16Array cache = uint16ArrayInit(256);

    // Write results as hex to a file
    FILE * resultsfile;
    resultsfile = fopen("collatz-data.bin","w+");
    if (resultsfile == NULL) {
        printf("Couldn't open data file.\nWhat have you done?\n");
        return(EXIT_FAILURE);
    }

    // Prefill with initial values
    uint16ArrayAppend(cache, 0); // 0 isn't defined
    uint16ArrayAppend(cache, 0); // 1 is the goal, the root of the tree ...
    uint i = 1; //  1 is the first element in the sequence.
    size_t one = sizeof(i) / sizeof(uint); // We're writing one int at a time.
    fwrite(&i, sizeof(uint), one, resultsfile);
    printf("%X\n", i);

    // Do the real work
    uint16 steps;
    uint16 maxSteps = 0;

    for (i = 2; i < COLLATZ_LIM; i++) {
        steps = collatz(cache, i);
        uint16ArrayAppend(cache, steps);
        if (steps > maxSteps) {
            fwrite(&i, sizeof(uint), one, resultsfile);
            printf("%X\n", i);
            maxSteps = steps;
        }
    }

    // Clean up after yourself.
    // you were not raised in a barn.
    fclose(resultsfile);
    uint16ArrayFree(cache);

    // I said *good day*, sir!
    return(EXIT_SUCCESS);
}

