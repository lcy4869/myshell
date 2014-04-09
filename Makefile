
generated_files_matmult_p = multiply matmult_p matmult_p.o multiply.o matformatter matformatter.o

all: matmult_p matmult_t matformatter myshell test

matmult_t: matmult_t.c
	gcc matmult_t.c -lm -o matmult_t

matmult_p: matmult_p.c multiply.c matrix_read.h matrix_read.c
	gcc -c matrix_read.c -o matrix_read.o
	gcc -c matmult_p.c -o matmult_p.o
	gcc -o matmult_p matmult_p.o matrix_read.o
	gcc multiply.c -o multiply

matformatter: matformatter.c matrix_read.h matrix_read.c
	gcc -c matrix_read.c -o matrix_read.o
	gcc -c matformatter.c -o matformatter.o
	gcc -o matformatter matformatter.o matrix_read.o

myshell: myshell.c
	gcc myshell.c -o myshell

test: test.c
	gcc test.c -o test

make clean:
	rm -f myshell *.o *~ *.so $(generated_files_matmult_p)
