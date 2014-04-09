/*
 * 
 * Filename: matmult_t.c
 * Jingzhi Gao
 *
 * Description:
 *   This program uses a multi-threaded approach to multiply two integer 
 *   matrices. This task is accomplished by creating machine contexts within a 
 *   single process address space, and switches between these contexts two 
 *   multiply one row by one column of the input matrices per thread. The 
 *   context-switching is implemented with a user-defined signal, as well as 
 *   setjmp, longjmp, and sigaltstack.
 * 
 */

#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <string.h>
#include <setjmp.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>


#define MAX_THREADS 100
#define STACK_SIZE  16384
#define TEMP_FILE   "./matmult_t_temp.txt"

enum boolean{false, true};

int max_threads;           // Defines the number of threads needed.

int **a;                   // Matrices.
int **b;
int **c;

int a_rows = 0;            // Dimensions of matrices.
int a_cols = 0;
int b_rows = 0;
int b_cols = 0;

static jmp_buf finish;     // Defines goto location after last thread finishes.




typedef struct mctx_t {
    jmp_buf jb;
} mctx_t;

static mctx_t uc[MAX_THREADS]; 
static char * stacks[MAX_THREADS];

static mctx_t mctx_caller;
static sig_atomic_t mctx_called;

static mctx_t *mctx_creat;
static void (*mctx_creat_func)(void *);
static void *mctx_creat_arg;
static sigset_t mctx_creat_sigs;
//double ceil(double x);

/* Save machine context. */
#define mctx_save(mctx) setjmp((mctx)->jb)

/* Restore machine context. */
#define mctx_restore(mctx) longjmp((mctx)->jb, 1)

/* Switch machine context. */
#define mctx_switch(mctx_old, mctx_new) if (setjmp((mctx_old)->jb) == 0) longjmp((mctx_new)->jb,1) 




int countline(char *line) {

    int total = 0;

    while (*line != '\n') {
        if (isdigit(*line) || (*line == '-')) {
            total++;
            while (isdigit(*(++line)));
        } 
        else if (isspace(*line)) {
            line++;
        }
        else {
            printf("Invalid input character '%c'. Terminating.\n", *line);
            exit(1);
        }
    }

    return total;
}


int **initialize_array(int rows, int cols) {

    int **array;
    array = (int**) malloc(rows * sizeof (int*));
    int i;
    for (i = 0; i < rows; i++)
        array[i] = (int*) malloc(cols * sizeof (int));

    return array;
}


void free_array(int **array, int rows) {
    int i;

    for (i = 0; i < rows; i++)
        free(array[i]);
    
    free(array);
}


void fill_array(int *array, char *line) {

    char *pch;
    int i = 0;

    pch = strtok(line, " ");

    while (pch != NULL) {
        array[i++] = atoi(pch);
        pch = strtok(NULL, " ");
    }
}


void multiply(int num) {

    int k = (num - 1) % (b_cols);

    double w = floor((num - 1)/b_cols);

    int i = (int)w;

    int j;

    //printf("here\nnum = %d, k = %d, i = %d\n", num, k, i); //debug

    for (j = 0; j < a_cols; j++) 
        c[i][k] += a[i][j] * b[j][k];

    if (num < max_threads)
        mctx_switch(&uc[num], &uc[num + 1]);

    longjmp(finish, 1);

}



/*
 * mctx_create_boot(): bootstraps into a clean stack frame by calling a 
 * second function. This also loads the function information, saves the 
 * current machine context, and restores the previously saved machine 
 * context of mctx_create to transfer execution control back to it.
 * 
 * Some or all of this code was written by:
 *   Ralf S. Engelschall, rse@engelschall.com
 *   "Portable Multithreading: The Signal Stack Trick For User-Space Thread Creation"
 */
void mctx_create_boot(void) {

    void (*mctx_start_func)(void *);
    void *mctx_start_arg;

    /* Step 10: */
    sigprocmask(SIG_SETMASK, &mctx_creat_sigs, NULL);

    /* Step 11: */
    mctx_start_func = mctx_creat_func;
    mctx_start_arg = mctx_creat_arg;

    /* Step 12 and Step 13: */
    mctx_switch(mctx_creat, &mctx_caller);

    /* The thread "magically" starts... */
    mctx_start_func(mctx_start_arg);

    /* NOTREACHED */
    abort();
}

/*
 * mctx_create_trampoline(): first entered asynchronously, at which time it 
 * saves its machine context and returns to terminate the signal handler. The 
 * second time it is entered, control is transferred to the alternate stack, 
 * but without the signal handler scope. It can then call the bootstrap function.
 * 
 * Some or all of this code was written by:
 *   Ralf S. Engelschall, rse@engelschall.com
 *   "Portable Multithreading: The Signal Stack Trick For User-Space Thread Creation"
 */
void mctx_create_trampoline(int sig) {

    /* Step 5: */
    if (mctx_save(mctx_creat) == 0) {
        mctx_called = true;
        return;
    }

    /* Step 9: */
    mctx_create_boot();

}

/*
 * mctx_create(): creates a machine context for a thread. It defines the signals,
 * signal handlers, signal masks, etc. It eventually sends itself a user-defined
 * signal to transfer control to the trampoline function, which sets in motion
 * the context-switching implementation.
 * 
 * Some or all of this code was written by:
 *   Ralf S. Engelschall, rse@engelschall.com
 *   "Portable Multithreading: The Signal Stack Trick For User-Space Thread Creation"
 */
void mctx_create(mctx_t *mctx, void (*sf_addr)(void *), void *sf_arg, void *sk_addr, size_t sk_size) {

    struct sigaction sa;
    struct sigaction osa;
    struct sigaltstack ss;
    struct sigaltstack oss;
    sigset_t osigs;
    sigset_t sigs;

    /* Step 1: */
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGUSR1);
    sigprocmask(SIG_BLOCK, &sigs, &osigs);

    /* Step 2: */
    memset((void *) &sa, 0, sizeof (struct sigaction));
    sa.sa_handler = mctx_create_trampoline;
    sa.sa_flags = SA_ONSTACK;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, &osa);

    /* Step 3: */
    ss.ss_sp = sk_addr;
    ss.ss_size = sk_size;
    ss.ss_flags = 0;
    sigaltstack(&ss, &oss);

    /* Step 4: */
    mctx_creat = mctx;
    mctx_creat_func = sf_addr;
    mctx_creat_arg = sf_arg;
    mctx_creat_sigs = osigs;
    mctx_called = false;
    kill(getpid(), SIGUSR1);
    sigfillset(&sigs);
    sigdelset(&sigs, SIGUSR1);

    while (!mctx_called) {
        sigsuspend(&sigs);
    }

    /* Step 6: */
    sigaltstack(NULL, &ss);
    ss.ss_flags = SS_DISABLE;
    sigaltstack(&ss, NULL);

    if (!(oss.ss_flags & SS_DISABLE))
        sigaltstack(&oss, NULL);

    sigaction(SIGUSR1, &osa, NULL);
    sigprocmask(SIG_SETMASK, &osigs, NULL);

    /* Step 7 and Step 8: */
    mctx_switch(&mctx_caller, mctx);

    /* Step 14: */
    return;
}

/*
 * my_thr_create(): creates a thread by calling the machine context-creating
 * function. Each thread has an associated "stack," function, and thread id.
 */
void my_thr_create(void (*func) (int), int thr_id) {

    int size = STACK_SIZE / 2;

    stacks[thr_id] = (char *) malloc(STACK_SIZE);

    mctx_create(&uc[thr_id], (void (*) (void *)) func,
            (void *) thr_id, stacks[thr_id] + size, size);

}


/*
 * main(): reads the input array information from stdin and checks to make sure
 * that it is valid. It then calls the thread-creating functions and eventually
 * prints out the product matrix. 
 */
int main(int argc, char *argv[]) {

    int nbytes = 256;
    char *line = (char *) malloc(nbytes + 1);

    int in_a = true;
    int in_b = true;

    FILE *fout;

    if ((fout = fopen(TEMP_FILE, "w+")) == NULL) {
        printf("Error: Cannot open file for processing matrices. Terminating.\n");
        exit(1);
    }

    while (in_a || in_b) {

        getline((char **)&line, (size_t *)&nbytes, stdin);
        fprintf(fout, "%s", line);

        int num = countline(line);

        if (num == 0) {

            if (in_a) {
                in_a = false;
                if (a_rows == 0) {
                    printf("Error: No input for matrix A. Terminating.\n");
                    exit(1);
                }
            } else {
                in_b = false;
                if (b_rows == 0) {
                    printf("Error: No input for matrix B. Terminating.\n");
                    exit(1);
                }
            }

        } else if (in_a) {
            a_rows++;
            if (a_cols == 0) {
                a_cols = num;
            } else if (num != a_cols) {
                printf("Error: Inconsistent number of columns in A. "
                       "Terminating.\n");
                exit(1);
            }
        } else {
            b_rows++;
            if (b_cols == 0) {
                b_cols = num;
            } else if (num != b_cols) {
                printf("Error: Inconsistent number of columns in B. "
                       "Terminating.\n");
                exit(1);
            }
        }

    }
    
    if (a_cols != b_rows) {
        printf("Error: Number of columns in A does not match number of"
               " rows in B; cannot multiply. Terminating.\n");
        exit(1);
    } 

    if (fseek(fout, 0, SEEK_SET) < 0) {
        printf("Error with file to process matrices. Terminating.\n");
        exit(1);
    }

    a = initialize_array(a_rows, a_cols);
    b = initialize_array(b_rows, b_cols);
    c = initialize_array(a_rows, b_cols);

    int i;

    for (i = 0; i < a_rows; i++) {
        getline((char **)&line, (size_t *)&nbytes, fout);
        fill_array(a[i], line);
    }



    getline((char **)&line, (size_t *)&nbytes, fout); // space between input matrices

    for (i = 0; i < b_rows; i++) {
        getline((char **)&line, (size_t *)&nbytes, fout);
        fill_array(b[i], line);
    }


    free(line);
    fclose(fout);
    remove(TEMP_FILE);

    max_threads = a_rows * b_cols;

    for (i = 1; i <= max_threads; i++)
        my_thr_create(multiply, i);

    //printf("a_rows = %d.\na_cols = %d.\nb_rows = %d.\nb_cols = %d.\n", a_rows, a_cols, b_rows, b_cols); // debug

    
    if (setjmp(finish) == 0)
        mctx_switch(&mctx_caller, &uc[1]);



    int j;

    // Print out product matrix.
    for (i = 0; i < a_rows; i++) {
        for (j = 0; j < b_cols; j++) {
            printf("%d", c[i][j]);
	    if (j != b_cols - 1) {
		printf(" ");
	    }
        }
        printf("\n");   
    }
    //printf("\n");

    free_array(a, a_rows);
    free_array(b, b_rows);
    free_array(c, a_rows);

    for (i = 1; i <= max_threads; i++)
        free(stacks[i]);
    
    return 0;

}
