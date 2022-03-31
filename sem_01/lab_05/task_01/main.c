#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>

#define TRUE 1
#define N 8
#define MAX_SEMS 3

#define P_COUNT 3
#define C_COUNT 3

#define BIN_SEM 0  // номера семафоров в наборе
#define BUF_FULL 1
#define BUF_EMPTY 2

#define MAX_RAND_P 2
#define MAX_RAND_C 5

struct sembuf P_LOCK[2] = {{BUF_EMPTY, -1, 0}, {BIN_SEM, -1, 0}};
struct sembuf P_RELEASE[2] = {{BUF_FULL, 1, 0}, {BIN_SEM, 1, 0}};

struct sembuf C_LOCK[2] = {{BUF_FULL, -1, 0}, {BIN_SEM, -1, 0}};
struct sembuf C_RELEASE[2] = {{BUF_EMPTY, 1, 0}, {BIN_SEM, 1, 0}};

void p_run(int num, int sem_id, char* buf, int* n, int* letter)
{
    while (TRUE)
    {
        semop(sem_id, P_LOCK, 2);
        buf[*n] = 'a' + *letter;
        printf("Producer №%d wrote to buf[%d] = %c\n", num, *n, buf[*n]);
        *n = *n == N - 1 ? 0 : *n + 1;
        *letter = *letter == 25 ? 0 : *letter + 1;
        semop(sem_id, P_RELEASE, 2);
        sleep(rand() % 4);
    }
}

void c_run(int num, int sem_id, char* buf, int* n)
{
    while (TRUE)
    {
        semop(sem_id, C_LOCK, 2);
        printf("Consumer №%d read  buf[%d] = %c\n", num, *n, buf[*n]);
        *n = *n == N - 1 ? 0 : *n + 1;
        semop(sem_id, C_RELEASE, 2);
        sleep(rand() % 5);
    }
}

int create_proc(int sem_id, char* shm_ptr)
{
    int processes_cnt = 0;
    pid_t pid;
    int* p_addr = shm_ptr + N;
    int* letter = p_addr + 1;
    int* c_addr = p_addr + 2;

    *p_addr = *c_addr = *letter = 0;

    for (int i = 0; i < P_COUNT; i++)
    {
        if ((pid = fork())== -1)
        {
            perror("Can’t fork.\n");
            return 1;
        }
        if (!pid)
            p_run(i + 1, sem_id, shm_ptr, p_addr, letter);
        else
            processes_cnt++;
    }

    for (int i = 0; i < C_COUNT; i++)
    {
        if ((pid = fork())== -1)
        {
            perror("Can’t fork.\n");
            return 1;
        }
        if (!pid)
            c_run(i + 1, sem_id, shm_ptr, c_addr);
        else
            processes_cnt++;
    }
    return processes_cnt;
}

int main()
{
    setbuf(stdout, NULL);
    char *shm_ptr = -1;

    int shm_id = shmget(IPC_PRIVATE, N + 3, IPC_CREAT | S_IRWXU | S_IRWXG | S_IRWXO);
    if (shm_id == -1)
    {
        perror("shmget failed.");
        return EXIT_FAILURE;
    }

    shm_ptr = shmat(shm_id, 0, 0);
    if (shm_ptr == (void *)-1)
    {
        perror("shmat failed.");
        return EXIT_FAILURE;
    }

    int s_id = semget(IPC_PRIVATE, MAX_SEMS, IPC_CREAT | S_IRWXU | S_IRWXG | S_IRWXO);
    if (s_id == -1)
    {
        perror("semget failed.");
        return EXIT_FAILURE;
    }

    semctl(s_id, BIN_SEM, SETVAL, 1);
    semctl(s_id, BUF_EMPTY, SETVAL, N);
    semctl(s_id, BUF_FULL, SETVAL, 0);

    int processes_cnt = create_proc(s_id, shm_ptr);
    int status;
    for (int i = 0; i < processes_cnt; i++)
    {
        wait(&status);
        if (!WIFEXITED(status))
            printf("Error, code = %d\n", status);
    }

    if (shmdt((void *)shm_ptr) == -1 || shmctl(shm_id, IPC_RMID, NULL) == -1 || semctl(s_id, IPC_RMID, 0) == -1)
    {
        perror("Exit error.");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
