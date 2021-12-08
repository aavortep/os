#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void analyse_status(int stat_loc)
{
    if (WIFEXITED(stat_loc))
    {
        printf("Child-process finished normally.\n");
        printf("Child-process termination code %d.\n\n", WEXITSTATUS(stat_loc));
        return;
    }
    if (WIFSIGNALED(stat_loc))
    {
        printf("Child-process terminates with un-intercepted signal.\n");
        printf("Signal number %d.\n\n", WTERMSIG(stat_loc));
        return;
    }
    if (WIFSTOPPED(stat_loc))
    {
        printf("Child-process stopped.\n");
        printf("Signal number %d.\n\n", WSTOPSIG(stat_loc));
    }
}

int main (void)
{
    pid_t childpid1, childpid2;
    
    if ((childpid1 = fork()) == -1)
    {
        perror("Can't fork\n");
        exit(EXIT_FAILURE);
    }
    else if (childpid1 == 0)
    {
        printf("\nChild1:    id   |  pgrp  | ppid\n");
        printf("         %d  |  %d | %d\n", getpid(), getppid(), getpgrp());
        
        if (execl("strings.exe", NULL) == -1)
        {
            perror("Child-process can't exec.\n");
            exit(EXIT_FAILURE);
        }
        
        exit(EXIT_SUCCESS);
    }
    if ((childpid2 = fork()) == -1)
    {
        perror("Can't fork\n");
        exit(EXIT_FAILURE);
    }
    else if (childpid2 == 0)
    {
        printf("\nChild2:    id   |  pgrp  | ppid\n");
        printf("         %d  |  %d | %d\n", getpid(), getppid(), getpgrp());
        
        if (execl("stack.exe", NULL) == -1)
        {
            perror("Child-process can't exec.\n");
            exit(EXIT_FAILURE);
        }
        
        exit(EXIT_SUCCESS);
    }
    if (childpid1 && childpid2)
    {
        int stat_loc;
        pid_t childpid;

        childpid = wait(&stat_loc);
        printf("childpid: %d stat_loc: %d\n", childpid, stat_loc);
        analyse_status(stat_loc);

        childpid = wait(&stat_loc);
        printf("childpid: %d stat_loc: %d\n", childpid, stat_loc);
        analyse_status(stat_loc);

        printf("Parent:    id   |  pgrp  | child1 | child2\n");
        printf("         %d  |  %d |  %d |  %d\n", getpid(), getpgrp(), childpid1, childpid2);
        printf("Parent-process finished.\n");
    }

    return EXIT_SUCCESS;
}