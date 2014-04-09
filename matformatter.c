/* File: multiply.c
 * Course: Spring 2014 BU CS 410 with Rich West, A2
 * Purpose:
 * Special Notes:
 * References and Citations:
 * BUCS410 A2 assignment sheet
 * http://linux.die.net/man/2/shmget
 *
 * Creator: Joel Mough, joelm@bu.edu BUID U95138815
 * Team: 
 * Purpose: Takes in a matrix from the standard in, computes it's transpose, and outputs it to the standard output
 */

#include "matrix_read.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

 int main(int argc, char **argv) {
 	int i;
	int leftArrayKey;
	int rightArrayKey;
	int numRows1;
	int itemsCol1;
	int numRows2 = 8675309;
	int itemsCol2;
	int val = readTwoMatrices(&leftArrayKey, &rightArrayKey, &numRows1, &itemsCol1, &numRows2, &itemsCol2);
	if (val != 1) {
		return -1;
	}

	int *array = (int *)shmat(leftArrayKey, NULL, 0);
	if (array == (void *) -1) {
 		printf("error accessing shared memory\n");
 		return -1;
	}
	int transpose[itemsCol1][numRows1];
	for (i = 0; i < itemsCol1; i++) {
		int j;
		for (j = 0; j < numRows1; j++) {
			transpose[i][j] = array[j*itemsCol1 + i];
			printf("%d", transpose[i][j]);
			if (j != (numRows1 - 1)) {
				printf(" ");
			}
		}
		if (i != (itemsCol1 - 1)) {
			printf("\n");
		}
	}

	int keys[1];
	keys[0] = leftArrayKey;
	releaseSharedMemory(keys,1);

	return 0;
 }


