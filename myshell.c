/* File:    myshell.c
 * Author:  Tianyou Luo
 * Date:    03/24/14
 */
 
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_LEN_CMD 1024

static void signal_handler(int);
static void cmdParser(char *);
static int tokenize(char * , char * []);

static int backend = 0; /* whether execute program in backend */

int
main(int argc, char * argv[])
{
    /* handle signals */
    if (signal(SIGINT, signal_handler))
    {
    }
    if (signal(SIGQUIT, signal_handler))
    {
    }

    char cmd[MAX_LEN_CMD]; /* max length of command is 1024 */    
    int num_char;   /* length of command */
    char c, *ptr;
    int status;
    pid_t pid;
    
    while (1)
    {
        if (isatty(0) == 1) /* from terminal */
        {
        printf("%s", "myshell>");
        }
    
        ptr = cmd;
        num_char = 0;
        while (num_char < MAX_LEN_CMD && ((c = getchar()) != EOF && c != '\n'))
        {
            *ptr++ = c;
            num_char++;
        }
        *ptr = 0;
    
        cmdParser(cmd);

        
        if (backend)
        {
            if ((pid = vfork()) < 0)   
            {
                write(2, "ERROR: fork error!\n", 19);
            } 
            else if (pid == 0)    /* child */
            {
            /*
                if (execvp(cmdList[0][0], cmdList[0]) < 0)
                {
                    write(2, "ERROR: execvp error!\n", 31);
                    break;
                }
                break;
                */
            }
            else    /* parent */
            {
                backend = 0;
                break;
            }
        }
    }
}

static void
cmdParser(char * cmd_line)
{
    char * ptr = cmd_line + strlen(cmd_line)-1;
    if (*ptr == '&')
    {
        *ptr = 0;
        backend = 1;
        
    }
    
    char * buf[1024];
    int i = tokenize(cmd_line, buf);
    printf("%s\n", cmd_line);
    
    
}

/* store tokens in buf
 * and return number of tokens
 */
static int
tokenize(char * s, char * buf[])
{
    int i = 0;
    char * t;
    char * tmp = strcpy(tmp, s);

    while ((t = strtok(tmp, " ")) != NULL)
    {
    	tmp = NULL;
        buf[i++] = t;
    }
    /*
    int j;
    for (j = 0; j < i; j++)
    {
        printf("%s\n", buf[j]);
    }    
    */
    return i;
}

static void 
signal_handler(int signo)
{
    if (signo == SIGINT)
        printf("\n%s\n", "Catch SIGINT to terminal!\n");
    else if (signo == SIGQUIT)
        printf("\n%s\n", "Catch SIGQUIT to terminal!\n");
    else
        printf("\n%s\n", "Catch an unkown signal to terminal!\n");
    exit(1);
}
