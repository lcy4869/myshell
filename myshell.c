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

static void signal_handler(int);
static char *** cmdParser(char *);

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

    char cmd[1024]; /* max length of command is 1024 */    
    int num_char;   /* length of command */
    char c, *ptr, *** cmdList;
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
        while (num_char < 1024 && ((c = getchar()) != EOF && c != '\n'))
        {
            *ptr++ = c;
            num_char++;
        }
        *ptr = 0;
    
        cmdList = cmdParser(cmd);

        if (backend)
        {
            if ((pid = vfork()) < 0)   
            {
                write(2, "ERROR: fork error!\n", 19);
            } 
            else if (pid == 0)    /* child */
            {
                if (execvp(cmdList[0][0], cmdList[0]) < 0)
                {
                    write(2, "ERROR: execvp error!\n", 31);
                    break;
                }
                break;
            }
            else    /* parent */
            {
                backend = 0;
                break;
            }
        }
    }
}

static char *** 
cmdParser(char * cmd_line)
{
    cmd_line += strlen(cmd_line)-1;
    if (*cmd_line == '&')
    {
        backend = 1;
    }
    char *** cmd;
    **cmd = "matmult_p";
    return cmd;
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
