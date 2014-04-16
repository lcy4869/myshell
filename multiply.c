/* File: multiply.c
 * Course: Spring 2014 BU CS 410 with Rich West, A2
 * Purpose: Reads in a pipe file descriptor, opens it, and gets certain info about the row/column of two matrices it will be multiplying
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

 #define MAX_DIGITS_INT 20

// Pass the pipe descriptor in as an argument; the rest is read out of the pipe.
// Reda sin the pipe file descriptor
 int main(int argc, char **argv) {
    if (argc < 2) {
    	printf("invalid; must pass in pipe file desc\n");
    	return -1;
    }
    //printf("here\n");
 	int pipeDescriptor = atoi(argv[1]);
 	FILE *pipe = fdopen(pipeDescriptor, "r");
 	if (pipe == NULL) {
 		printf("error opening pipe\n");
 		return -1;
 	}
 	// So we can set up the appropriate for loops when going through the arrays
 	int numCol1;
 	int numCol2;
 	int numRows2;

 	// Which row and column to computer and where in the new array the result will go
 	int currentRow;
 	int currentCol;

 	// These three values are needed to access the three shared memory segments
 	int idMat1;
 	int idMat2;
 	int idSolution;
 	// Add error checking as we do this. 
 	char *temp = calloc(MAX_DIGITS_INT, 1);
 	fgets(temp, MAX_DIGITS_INT, pipe);
 	numCol1 = atoi(temp);

 	fgets(temp, MAX_DIGITS_INT, pipe);
 	numCol2 = atoi(temp);

 	fgets(temp, MAX_DIGITS_INT, pipe);
 	numRows2 = atoi(temp);

 	fgets(temp, MAX_DIGITS_INT, pipe);
 	currentRow = atoi(temp);


 	fgets(temp, MAX_DIGITS_INT, pipe);
 	currentCol = atoi(temp);

 	fgets(temp, MAX_DIGITS_INT, pipe);
 	idMat1 = atoi(temp);

 	fgets(temp, MAX_DIGITS_INT, pipe);
 	idMat2 = atoi(temp);

 	fgets(temp, MAX_DIGITS_INT, pipe);
 	idSolution = atoi(temp);
 	free(temp);

 	int *leftArray = (int *)shmat(idMat1, NULL, 0);
 	int *rightArray = (int *)shmat(idMat2, NULL, 0);
 	int *newArray = (int *)shmat(idSolution, NULL, 0);
 	if (leftArray == (void *) -1 || rightArray == (void *) -1 || newArray == (void *) -1) {
 		printf("error accessing shared memory\n");
 		return -1;
	}
 	int runningSol = 0;

 	int i;
 	for (i = 0; i < numRows2; i++) {
 		runningSol += leftArray[currentRow*numCol1 + i]*rightArray[i*numCol2 + currentCol];
 	}

 	newArray[currentRow*numCol2 + currentCol] = runningSol;

 	return 0;

 }
