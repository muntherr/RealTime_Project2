#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include "local.h"

int person_pid;
int patience_limit;
time_t time_arrive; // ** To give priority to whom arrive early
int rolling_gate_number;
int semid, shmid;
char *shmptr;
struct MEMORY *memptr;
char gender[2];
char request_type[2];
int main(int argc, char *argv[])
{
    time_t rawtime;

    struct tm *timeinfo1;

    if (argc != 5)
    {
        perror("Number of argumentss must be 4!");
        exit(-1);
    }

    person_pid = getpid();
    pid_t ppid = getppid();

    // // Access and attach to shared memory
    if ((shmid = shmget((int)ppid, 0, 0)) != -1)
    {
        if ((shmptr = (char *)shmat(shmid, (char *)0, 0)) == (char *)-1)
        {
            perror("shmat -- producer -- attach");
            exit(1);
        }
        memptr = (struct MEMORY *)shmptr;
    }
    else
    {
        perror("shmget -- producer -- access");
        exit(2);
    }

    strcpy(gender, argv[1]);

    strcpy(request_type, argv[2]);
    patience_limit = atoi(argv[3]);

    rolling_gate_number = atoi(argv[4]);

    printf("\033[0;36m"); // set the color to cyan
                          // display the message
    printf("person with %d and gender %c with request_type %c my pl %d my gn %d\n", person_pid, gender[0], request_type[0], patience_limit, rolling_gate_number);

    printf("\033[0m"); // reset the color to the default
    message.mesg_type = 1;

    message.from_pid = person_pid;

    strcpy(message.gender, &gender[0]);

    message.patience_limit = patience_limit;

    strcpy(message.request_type, &request_type[0]);

    time(&rawtime);
    timeinfo1 = localtime(&rawtime);
    message.start_sec = timeinfo1->tm_sec;
    message.start_min = timeinfo1->tm_min;

    // sending message to rolling_gate
    if (msgsnd(rolling_gate_number, &message, sizeof(message), 0) == -1)
    {
        perror("msgsnd");
        return 1;
    }
    sleep(4);

    while (1)
        ;
    return 0;
}
