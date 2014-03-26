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
static void cmdParser(char *);
static void tokenize(char * , char * []);

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
        while (num_char < 1024 && ((c = getchar()) != EOF && c != '\n'))
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
    printf("%s\n", cmd_line);
    tokenize(cmd_line, buf);
    /*
    int i = 0;
    for (i = 0; i < 10; i++)
    {
        printf("%s\n", buf[i]);
    }*/
    
}

static void
tokenize(char * s, char * buf[])
{
    char c, token[1024], *ptr_t;
    char * ptr = s;
    int i = 0;
    
    while ((c = *ptr++) != 0)
    {
        //printf("%c\n", c);
        ptr_t = token;
        while (c != ' ' && c != '\n' && c != 0)
        {
            printf("%c\n", c);
            *ptr_t++ = c;
            c = *ptr++;
        }
        *ptr_t = 0;
        buf[i] = token;
        printf("%s\n", buf[i]);
        i++;
    }
    
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
