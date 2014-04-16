================================================
Assignment 2 - submitted through jingzhi gsubmit
================================================

Team member: Jingzhi Gao, Tianyou Luo, Joel Mough
Make file commands:
"make all" makes everything
And then everything else is just "make <part name>" e.g. make matmult_p
make clean cleans everything up

=========
Myshell.c
=========
Worked on by: Tianyou Lou, piping added by Joel Mough
References: BU CS 410 A2 assignment sheet and links and course textbook (all throughout this assignment actually)
Limitations: cannot do arguments (e.g. ./finds -p <name> -s "hello" will not work) -- not required but noted. 
Purpose: 
"to implement a basic shell called "myshell", to learn how a command interpreter works" -- Rich West
Notes:
ctrl+d -- kills the shell
ctrl+c -- kills the outermost process
commands must be in the form "./" or have a full path name. Does not deal with the environment variable. 
spacing is sensitive; you cannot do ./matmult_p <input4 and expect it to work. 

Works like a shell with the operators in the assignment sheet supported. 

==================================================================================
Matmult_p -- needs files matmult_p.c, matrix_read.c, matrix_read.h, and multiply.c
==================================================================================
Worked on by: Joel Mough, joelm@bu.edu
References:
 * BUCS410 A2 assignment sheet
 * http://linux.die.net/man/2/shmget
 * Various man pages on shared memory
 * Piazza as needed

Purpose:
matmult_p uses multiple processes to multiply two matrices read in via the standard input. 
The format for the matrices MUST be as follows:
a b c d
e f g h
i j k l

m n o p
q r s t
u v w x
----------
With only one space between each line and NO EXTRA SPACES at the end of the line. Having extra spaces will cuase
error messages to appear, because the program must verify matrix size after reading it in. 

After succesffully reading in the matrix, it then spawns m x n children (where m is the number of columsn in matrix 1 and n is the number of rows in matrix 2), having each
child process fill in the shared memory matrix. It waits on all child processes, and when done, it outputs the matrix to the standard output. 

LIMITATION NOTE: Due to the resource intensive nature of multi-processing matrix multiplication, on csa2 multiple processses being spawned can crash the program due to a resource
allocation refused error. 

The program relies on a matrix reader program I wrote in two separate files (because I also used the matrix reader in the matformatter).
matrix_read.c takes in a matrix of the above format, and reads it in, line at a time. If there exists no second matrix (e.g. it reaches EOF after reading in
only one matrix), it will only read in and store the first matrix. It converts the two (or one) matrices to integer matrices, and puts them in a stored memory segment. 
Additionally, it provides functions for freeing shared memory from the system and deallocating memory. Both of these are encapsulated via the matrix_read.h

Multiply.c takes one argument, a file descriptor, and uses it to open a shared pipe file as needed. It reads in various fields pretaining to the matrix being worked on
and the row and column in the shared memory matrix being edited. It then performs the matrix row/column computation, and stores it in shared memory. 

-------------------------
Matformatter -- needs files matrix_read.c and matrix_read.h
--------------------------------------------------------------
** See explanation of matrix_read above. 
Worked on by: joelm@bu.edu

It reads in a matrix of the same matrix as above, and computes its transpose, printing it out to the standard out. 

As above, note that formatting can cause issues; spaces at the end of lines can cause issues. Preferably, the input matrix should be as follows:
a b
c d(EOF right here)
----



===========
Matmult_t.c
===========
Worked on by: Jingzhi Gao, jingzhi@bu.edu
References:
 * "Portable Multithreading: The Signal Stack Trick For User-Space Thread Creation" by Ralf S. Engelschall

Purpose:
matmult_t uses a multi-threaded approach to multiply two integer matrices. It creates machine contexts within a single process address space, and switches between these contexts to multiply one row by one column of the input matrices per thread, and each thread computes the result of its corresponding slot in the output matrix. The context-switching is based on "Portable Multithreading: The Signal Stack Trick For User-Space Thread Creation" by Ralf S. Engelschall, which is implemented with a user-defined signal, as well as setjmp, longjmp, and sigaltstack.


Note:
1. each command line supports up to 10 programs once.
