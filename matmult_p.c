/* File: matmult_p.c
 * Course: Spring 2014 BU CS 410 with Rich West, A2
 * Purpose: Multiplies two matrices from the standard input using a multi-processing approach.
 * Special Notes: Huge matrices can cause a problem, e.g. a 40x40 matrix generating 1600 processes won't work
 * References and Citations:
 * BUCS410 A2 assignment sheet
 * http://linux.die.net/man/2/shmget
 *
 * Creator: Joel Mough, joelm@bu.edu BUID U95138815
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include "matrix_read.h"




 int main(int argc, char **argv) {
	int i;
	int leftArrayKey;
	int rightArrayKey;
	int numRows1;
	int itemsCol1;
	int numRows2;
	int itemsCol2;

	int val = readTwoMatrices(&leftArrayKey, &rightArrayKey, &numRows1, &itemsCol1, &numRows2, &itemsCol2);
	if (val != 0) {
		return -1;
	}

	
 	// This is the meat and potatoes; it runs through all row column combos in the new array and works them out.
 	// numRows1 x itemsCol2
 	int sharedMemKey = shmget(IPC_PRIVATE, numRows1*itemsCol2*4, IPC_CREAT | S_IRUSR | S_IWUSR);
 	int *result = (int *)shmat(sharedMemKey, NULL, 0);
 	int pid;
 	int numChildren;
	int keys[3];
	keys[0] = leftArrayKey;
	keys[1] = rightArrayKey;
	keys[2] = sharedMemKey;

	int pids[MAX_MATRIX_DIMENSION][MAX_MATRIX_DIMENSION];
 	for (i = 0; i < numRows1; i++) {
 		int j;
 		for (j = 0; j < itemsCol2; j++) {
 			// Cluaes that need piped; num in a col in 1, num in a row in 2, the row and col number of the new matrix
 			int fd[2];
 			pipe(fd);
 			//printf("i: %d\tj: %d\t\n", i, j);
 			numChildren++;
 			if ((pid = fork()) < 0) {
 				printf("error creating child process\n");
 				perror(NULL);
				releaseSharedMemory(keys, 3);
 				return -1;
 			}
 			// in a child process
 			if (pid == 0) {
 				// Redirects the new pipe to be the standard input of the newly execed multiple, which will deal with that.
 				close(fd[1]);
 				char *descriptor = calloc(sizeof(int)*8+1, 1);
 				sprintf(descriptor, "%d", fd[0]);
 				execl("./multiply", "multiply", descriptor, (char *)0);
 				printf("exec error; abborintg\n");
				releaseSharedMemory(keys, 3);
 				return -1;
 			} else {
 				pids[i][j] = pid;
 				close(fd[0]);
 				dprintf(fd[1], "%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n", itemsCol1, itemsCol2, numRows2, i, j, leftArrayKey, rightArrayKey, sharedMemKey);
 				close(fd[1]);
 			}

 		}
 	}

 	int normal = 1;
 	for (i = 0; i < numRows1; i++) {
 		int j;
 		for (j = 0; j < itemsCol2; j++) {
			//printf("here\n");
			int status;
 			waitpid(pids[i][j], &status,0);
 			if (status != 0) {
 				printf("error in child process\n");
 				normal = 0;
 			}
 		}
 	}



 	for (i = 0; i < numRows1 && normal; i++) {
 		int j;
 		for (j = 0; j < itemsCol2; j++) {
 			printf("%d", result[i*itemsCol2 + j]);
 			if (j != (itemsCol2 - 1)) {
 				printf(" ");
 			}
 		}
 		if (i != (numRows1 - 1)) {
 			printf("\n");
 		}
 		
 	}

	releaseSharedMemory(keys, 3);
	if (!normal) {
		return -1;
	}
 	return 0;

 }
