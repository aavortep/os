#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define OK 0
#define FORK_FAILURE 1
#define EXEC_FAILURE 2
#define PIPE_FAILURE 3

#define GET 1
#define N 2
#define SLP_INTV 2
#define BUFFSIZE 128

int mode = 0;

void dummy(int sigint) {  }

void change_mode(int sigint) 
{
   mode = GET; 
}

int main() 
{
    int child[N];
    int fd[N];
    int pid;

    const char *const messages[N] = { "mnbvcdrtyujnb\n", "qwerty\n" };
    char str1[BUFFSIZE] = { 0 };
	char str2[BUFFSIZE] = { 0 };

    if (pipe(fd) == -1)
    {
        perror("Cant pipe.");
        return PIPE_FAILURE;
    }

    fprintf(stdout, "Parent process. PID: %d, GROUP: %d\n", getpid(), getpgrp());
    signal(SIGINT, dummy);

    for (size_t i = 0; i < N; i++) 
    {
        pid = fork();

        if (pid == -1) 
        {
            perror("Cant fork.");
            return FORK_FAILURE;
        } 
        else if (pid == 0) 
        {
            signal(SIGINT, change_mode);
            sleep(SLP_INTV);

            if (mode) 
            {
                close(fd[0]);
                write(fd[1], messages[i], strlen(messages[i]));
                fprintf(stdout, "Message #%d sent to parent!\n", i + 1);
            }
            else
                fprintf(stdout, "No signal sent!\n");

            return OK;
        }
		else 
            child[i] = pid;
    }

    for (size_t i = 0; i < N; i++) 
    {
        int status, statval;

        pid_t childpid = wait(&status);
        fprintf(stdout, "Child process (PID %d) finished. Status: %d\n", childpid, status);

        if (WIFEXITED(statval)) 
        {
            fprintf(stdout, "Child process #%d finished with code: %d\n", i + 1, WEXITSTATUS(statval));
        }
        else if (WIFSIGNALED(statval))
        {
            fprintf(stdout, "Child process #%d finished from signal with code: %d\n", i + 1, WTERMSIG(statval));
        }
        else if (WIFSTOPPED(statval))
        {
            fprintf(stdout, "Child process #%d finished stopped with code: %d\n", i + 1, WSTOPSIG(statval));
        }

    }

    close(fd[1]);
	read(fd[0], str1, strlen(messages[0]));
	read(fd[0], str2, strlen(messages[1]));

    fprintf(stdout, "First message: %s", str1);
	fprintf(stdout, "Second message: %s\n", str2);
    fprintf(stdout, "Parent process. Children ID: %d, %d.\nParent process is dead.\n", child[0], child[1]);

    return OK;
}