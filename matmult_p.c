/* File: matmult_p.c
 * Course: Spring 2014 BU CS 410 with Rich West, A2
 * Purpose:
 * Special Notes:
 * References and Citations:
 * BUCS410 A2 assignment sheet
 * http://linux.die.net/man/2/shmget
 *
 * Creator: Joel Mough, joelm@bu.edu BUID U95138815
 * Team: 
 */

 // How to read in the matrix is what is confusing me...hmm. Have to look over this. 
// Should we assumed it will be input put [1, 2, 3][4, 5, 6][7, 8, 9 ] and then another matrix?
 // Like a string?

 // Basic idea is not that hard. Use shmtcl to set up the shared memory segment. Then basically for each id in the new matrix
 // you will fork at the moment, and the the "m" and "n" variables to find the appropriate row column combos. Process them
 // and store in the stored memory segment. Once all this is done, we "write" to STD_OUTFILENO and are good.

 // Each line imported from STDIN will will hold a matrix; blank line separates the two. So, like this
 // 1 2
 // 1 2
 // *blan line*
 // and continue; WIll read in char at a time, convert, and move along. 

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
 	//printf("%d\t%d\n", numRows1, itemsCol2);
 	for (i = 0; i < numRows1; i++) {
 		int j;
 		for (j = 0; j < itemsCol2; j++) {
 			//printf("currentRow: %d\tcurrentCol: %d\n", i, j);
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
 				//printf("in parent: %d\t%d\n", i, j);
 				close(fd[0]);
 				dprintf(fd[1], "%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n", itemsCol1, itemsCol2, numRows2, i, j, leftArrayKey, rightArrayKey, sharedMemKey);
 				close(fd[1]);
 			}

 		}
 	}

 	// ADD MORE sophisticated error checking later on. 
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



 	//printf("%d\t%d\n%d\t%d\n", result[0], result[1], result[2], result[3]);
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
 	// Ref http://stackoverflow.com/questions/279729/how-to-wait-untill-all-child-processes-called-by-fork-complete
 	return 0;

 }
