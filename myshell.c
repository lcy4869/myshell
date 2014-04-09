/* File:    myshell.c
 * Author:  Tianyou Luo, piping added by Joel Mough
 * Date:    03/24/14
 */
 
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_LEN_CMD 1024

typedef struct Command
{
    char * std_in;
    char * std_out;
    char * std_err;
    char * program;
    char ** arg;
} Command;

static void signal_handler(int);
static int cmdParser(char *, Command []);
static int tokenize(char * , char * []);
void change_std(Command *, int, int);

static int backend = 0; /* whether execute program in backend */
static int foreground_pid;
static int background_pid_list[1024];
static int num_background_p;
static int num_pipe;


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
    num_background_p = 0;
    Command cmd_list[1024];
    int num_program = 0;

    while (1)
    {
        if (isatty(0) == 1) /* from terminal */
        {
        printf("%s", "myshell>");
        }
    
    	cmd[0] = 0;
        ptr = cmd;
        num_char = 0;

        while (num_char < MAX_LEN_CMD && ((c = getchar()) != EOF && c != '\n'))
        {
            *ptr++ = c;
            num_char++;
        }
        *ptr = 0;
    	
        // check whether it's backend program
        if (ptr[-1] == '&')
        {
        	backend = 1;
        	ptr[-1] = 0;
        }

        int ii;
        for (ii = 0; ii < num_program; ii++)
        {	
        	//printf("here1\n");
        	cmd_list[ii].program = NULL;
        	cmd_list[ii].std_in = NULL;
        	cmd_list[ii].std_out = NULL;
        	cmd_list[ii].std_err = NULL;
        	cmd_list[ii].arg = NULL;
        	//printf("here2\n");
        }

        num_program = cmdParser(cmd, cmd_list) + 1;

        //printf("%d\n", num_program );
        
        int pipe_index = 0;
        int pfd[num_pipe][2];


        int i;
        for (i = 0; i < num_program; i++)
        {
			int a = 1;
			int b = 1;
			if (cmd_list[i].std_in != NULL) {
        		a = strcmp(cmd_list[i].std_in, "|");	// pipe stdin
													//printf("%d\n", a);
			}
			if (cmd_list[i].std_out != NULL) {
        		b = strcmp(cmd_list[i].std_out, "|");	// pipe stdout
			}

        	if (b == 0) {
				pipe_index++;
        		if (pipe(pfd[pipe_index]) < 0)
        		{
        			write(2, "ERROR: can NOT create pipe!\n", 27);
        			return;
        		}


        	}

        	if ((pid = fork()) < 0)   
        	{
            	write(2, "ERROR: fork error!\n", 19);
        	} 
        	else if (pid == 0)    /* child */
        	{

        		
        		if (a == 0)
        		{
					int workingIndex;
					if (b != 0) {
						workingIndex = pipe_index;
					} else {
						workingIndex = pipe_index - 1;
					}

					close(pfd[workingIndex][1]);
        			if (dup2(pfd[workingIndex][0], STDIN_FILENO) < 0) {
						printf("dup2 error\n");
						perror("duplicating:");
						return -1;
					}

        		}

        		if (b == 0)
        		{
        			if (dup2(pfd[pipe_index][1], STDOUT_FILENO) < 0) {
						printf("dup2 error\n");
						perror("duplicating:");
						return -1;
					}
					close(pfd[pipe_index][0]);
        			//pipe_index++;
        		}
				change_std(&cmd_list[i], a, b);

            	if (execvp(cmd_list[i].program, cmd_list[i].arg) < 0)
            	{
                	write(2, "ERROR: execvp error!\n", 31);
                	continue;
            	}
        	}
        	else    /* parent */
        	{
				// Dealing with some pipe issues.
				if (a == 0) {	
					close(pfd[pipe_index-1][0]);
				}
				if (b == 0) {
					close(pfd[pipe_index][1]);
				}
            	if (backend)
            	{
                	if (signal(SIGCHLD, signal_handler))
                	{
        
       	         	}
                	backend = 0;
                	continue;
            	}
            	else
            	{
                	foreground_pid = pid;
                	// printf("foreground: %d\n", foreground_pid);
                	waitpid(foreground_pid, &status, 0);
                	foreground_pid = -1;
            	}
        	}
    	}

    	//dup2(0, STDIN_FILENO);

    }
}

static int
cmdParser(char * cmd_line, Command cmd_list[])
{
	num_pipe = 0;
    int num_program = 0;
    char * buf[1024];
    int i = tokenize(cmd_line, buf);
    //printf("%s\n", cmd_line);
    //printf("%s\n", buf[0]);

    if (i < 0)
    {
        return;
    }

    int j;
    int programEnds = 0;
    int programBegin = 1;
    int num_arg = 0;
    for (j = 0; j < i; j++)
    {
        //printf("%s\n", buf[j]);

        if (num_program >= 1024) 
        {
            write(2, "ERROR: too many programs!\n", 26);
            return;
        }
        
        char * s = buf[j];
        
        // printf("num_program:\t%d\n", num_program);
        if (programBegin)
        {
            //printf("pars:%s\n", s);

  	        if (check_semicomma(s))
  	      	{
        		programEnds = 1;
        	}
            cmd_list[num_program].program = s;
            programBegin = 0;

            char * tmp_arg[1024];
            tmp_arg[0] = s;
            cmd_list[num_program].arg = tmp_arg;
            num_arg = 1;
			//cmd_list[num_program].std_in = "stdin";
			//cmd_list[num_program].std_out = "stdout";
        }
        else if (strcmp(s, ">") == 0 || strcmp(s, "1>") == 0)
        {
        	j++;
        	s = buf[j];
        	//printf("parsing:%s\n", s);
  	        if (check_semicomma(s))
  	      	{
        		programEnds = 1;
        	}
            cmd_list[num_program].std_out = s;
        } 
        else if (strcmp(s, "<") == 0)
        {
        	//printf("parsing:%s\n", s);

        	j++;
            s = buf[j];
  	        if (check_semicomma(s))
  	      	{
        		programEnds = 1;
        	}
            cmd_list[num_program].std_in = s;
        }
        else if (strcmp(s, "2>") == 0)
        {

        	j++;
            s = buf[j];
  	        if (check_semicomma(s))
  	      	{
        		programEnds = 1;
        	}
            cmd_list[num_program].std_err = s;
        }
        else if (strcmp(s, "&>") == 0)
        {
        	j++;
        	s = buf[j];
  	        if (check_semicomma(s))
  	      	{
        		programEnds = 1;
        	}
            cmd_list[num_program].std_out = s;
            cmd_list[num_program].std_err = s;
        }
        else if (strcmp(s, "|") == 0)
        {
			//printf("num_program %d\n", num_program);
        	cmd_list[num_program].std_out = s;
        	cmd_list[num_program+1].std_in = s;
        	num_pipe++;
        	programEnds = 1;
        }
        else
        {
        	cmd_list[num_program].arg[num_arg] = s;
        	num_arg++;
        }
        

        if (programEnds)
        {
        	cmd_list[num_program].arg[num_arg] = NULL;
        	num_program++;
        	programBegin = 1;
        	programEnds = 0;
        }
    }

    // print programs
    /*
    for (i = 0; i < num_program+1; i++)
    {
    	Command cmd = cmd_list[i];
    	printf("program: \t%s\n", cmd.program);
    	printf("standin: \t%s\n", cmd.std_in);
    	printf("standout:\t%s\n", cmd.std_out);
    	printf("standerr:\t%s\n\n", cmd.std_err);
    }
    */
    return num_program;
    
}

void
change_std(Command * cmd, int a, int b)
{
	int fd;
	if (cmd->std_in != NULL && (a != 0))
	{

		if (( fd = open ( cmd->std_in, O_RDONLY )) == -1 )
		{
    		fprintf( stderr, "mysh error: can't open %s\n", cmd->std_in);
    		exit(1);
		}
		dup2(fd, STDIN_FILENO);
		close(fd);
		
	}
	if (cmd->std_out != NULL && (b != 0))
        {
            if (( fd = open ( cmd->std_out, O_WRONLY | O_CREAT)) == -1 )
                {
                fprintf( stderr, "mysh error: can't open %s\n", cmd->std_out);
                exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
        } 
        if (cmd->std_err != NULL)
        {
            if (( fd = open ( cmd->std_err, O_WRONLY )) == -1 )
                {
                fprintf( stderr, "mysh error: can't open %s\n", cmd->std_out);
                exit(1);
                }
                dup2(fd, STDERR_FILENO);
                close(fd);
        }
}

/* check ';' in the end of given token */
int
check_semicomma(char * token)
{
	char * ptr = token + strlen(token) - 1;
	if (*ptr == ';')
	{
		*ptr = 0;
		return 1;
	}
	return 0;
}

/* store tokens in buf
 * and return number of tokens
 */
static int
tokenize(char * s, char * buf[])
{
    int i = 0;
    char * t;
	
    char * tmp = (char *)malloc(sizeof(char)*1024);
    if (strlen(s) > 1023)
    {
        write(2, "ERROR: token length is too long!\n", 33);
        return -1;
    }

    strcpy(tmp, s);
	while ((t = strtok(tmp, " ")) != NULL)
    {
    	tmp = NULL;
        buf[i++] = t;
    }

    /*
    printf("%s\n", "tokenize");
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
    int status;
    pid_t pid;

    if (signo == SIGINT)
    {
        printf("\n%s\n", "Catch SIGINT to terminal foreground process!\n");
        kill(foreground_pid, SIGINT);
        return;
    }
    else if (signo == SIGCHLD)
    {
        //if (wait(&status) == foreground_pid)
        //    return;
        // printf("handler:\n");
        pid = wait(&status);
        // printf("%d\n", pid);
        // printf("%d\n", foreground_pid);
        // printf("get SIGCHLD\n");
        if (foreground_pid != -1 && pid == foreground_pid)
        	return;
        
        return;
    }   

 
    else if (signo == SIGQUIT)
        printf("\n%s\n", "Catch SIGQUIT to terminal!\n");
    else
        printf("\n%s\n", "Catch an unkown signal to terminal!\n");
    exit(1);

}
