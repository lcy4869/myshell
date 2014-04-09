/* File: matrix_read.h
 * Course: Spring 2014 BU CS 410 with Rich West, A2
 * Purpose: Serves as a header file to matrix_read.h, which provides a function that reads in two matrixes of the following format in front stdin:
 * 1 2 3 4 5 -- single space delimited lines
   (one space in between the matrices)
 * It reads in two matrices, converts them to strings, and puts them in shared memory regions, and then stores the keys in the shared memory redions. 
 *
 * Special Notes:
 * References and Citations:
 * BUCS410 A2 assignment sheet
 *
 * Creator: Joel Mough, joelm@bu.edu BUID U95138815
 * Team: 
 */
#ifndef MATRIX_READ_H
#define MATRIX_READ_H

// Max number of digits in a 64 digit interger
#define MAX_DIGITS_INT 20
#define MAX_MATRIX_DIMENSION 100

// Function reads in two matrices from the standard input and maps them into shared memory regions, and returns
// the keys into passed pointers int *leftMatrix, int *rightMatrix. NOTE; the returning process
// must map them back into their memory addresses as needed AND is responsible for cleaning up (e.g. closing all shared
// memory segments) Returns 0 on success, -1 on failure, 1 if it only read in one matrix. 
 // IF it only read in one matrix, it will store the key to it in "leftMatrix," and the row and columds in numRows1
 // and numColumns1
void releaseSharedMemory(int *keys, int numKeys);
void freeMem(char **stringArray, int numRows);
int readTwoMatrices(int *leftMatrix, int *rightMatrix, int *numRows1, int *numColumns1, int *numRows2, int *numColumns2);

#endif
