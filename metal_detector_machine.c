#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include "local.h"

int pid;
char type[7];
int security_officer_pid;

int semid, shmid;
char *shmptr;
struct MEMORY *memptr;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror("Number of argumentss must be 2!");
        exit(-1);
    }
    pid_t ppid = getppid();
    pid = getpid();
    strcpy(type, argv[2]);
    security_officer_pid = atoi(argv[1]);
    // Access and attach to shared memory
    if ((shmid = shmget((int)ppid, 0, 0)) != -1)
    {
        if ((shmptr = (char *)shmat(shmid, (char *)0, 0)) == (char *)-1)
        {
            perror("shmat MACHINE-- producer -- attach");
            exit(1);
        }
        memptr = (struct MEMORY *)shmptr;
    }
    else
    {
        perror("shmget MACHINE -- producer -- access");
        exit(2);
    }
  
    while (1)
    {
        if (memptr->m_machine_available || memptr->f_machine_available)
        {
            kill(security_officer_pid, SIGUSR1);
            sleep(1);
        }
    }
    while (1)
        ;
    return 0;
}