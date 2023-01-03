#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include "local.h"

// functions declarations
void advance();
void opening_gates();
int calculate_time_difference(int, int, int, int);

int rolling_gate_pid;
char rolling_gate_type[7];
int max_capacity_threshold;
int status;
int min_capacity_threshold;
int queue_id;
int grouping_area_queue;
int passed_max_impatient_people_number;
int semid, shmid;
char *shmptr;
struct MEMORY *memptr;
time_t rawtime;
struct tm *timeinfo2;
int time_difference;

int main(int argc, char *argv[])
{
    if (argc != 7)
    {
        perror("Number of arguments must be 6!");
        exit(-1);
    }

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
    // Access the semaphore set
    if ((semid = semget((int)ppid, 5, 0)) == -1)
    {
        perror("semget -- producer -- access");
        exit(3);
    }

    signal(SIGUSR1, advance);
    signal(SIGUSR2, opening_gates);
    memptr->female_gate_status = 1;
    memptr->male_gate_status = 1;
    rolling_gate_pid = getpid();
    strcpy(rolling_gate_type, argv[1]);
    max_capacity_threshold = atoi(argv[2]);
    status = atoi(argv[3]);
    queue_id = atoi(argv[4]);
    grouping_area_queue = atoi(argv[5]);
    passed_max_impatient_people_number = atoi(argv[6]);
    min_capacity_threshold = max_capacity_threshold / 2;

    while (1)
        ;
    return 0;
}

void advance()
{
    if (rolling_gate_type[0] == 'M')
    {

        if (memptr->male_gate_status && memptr->menOutside != 0)
        {
            if (msgrcv(queue_id, &message, sizeof(message), 1, 0) < 0)
            {
                perror("msgrcv === failed");
                exit(1);
            }
            time(&rawtime);
            timeinfo2 = localtime(&rawtime);
            time_difference = calculate_time_difference(message.start_min, message.start_sec, timeinfo2->tm_min, timeinfo2->tm_sec);
            if (time_difference > message.patience_limit)
            {
                memptr->termination_variables[0]++;
                if (memptr->termination_variables[0] > passed_max_impatient_people_number)
                {
                    printf("\033[5;31m"); // set the color to red
                    printf("Terminating Due to Exceeding MAX_IMPATIENT_PASSENGERS_NUMBER\n");
                    printf("\033[0m"); // reset the color to the default
                    kill(getppid(), SIGINT);
                }
                printf("\033[5;31m"); // set the color to red
                printf("GATE: leaving person with pid  = %d (time = %d) (limit = %d)\n", message.from_pid, time_difference, message.patience_limit);
                printf("\033[0m"); // reset the color to the default

                kill(message.from_pid, SIGTERM);
            }
            acquire.sem_num = GRANTED_ACCESS;
            if (semop(semid, &acquire, 1) == -1)
            {
                perror("semop -- producer -- acquire4");
                exit(4);
            }
            memptr->people_in_male_gate_counter++;
            memptr->forked_people_number--;
            memptr->menOutside--;

            release.sem_num = GRANTED_ACCESS;

            if (semop(semid, &release, 1) == -1)
            {
                perror("semop -- producer -- release");
                exit(5);
            }
            // sleep(2);
            fflush(stdout);
            printf("\033[0;33m"); // set the color to yellow
            printf("ROLLING GATE RECEIVED SIGUSR1: %d, %c, %c \n", message.from_pid, message.gender[0], message.request_type[0]);
            printf("\033[0m"); // reset the color to the default
            fflush(stdout);

            if (memptr->people_in_male_gate_counter >= max_capacity_threshold)
            {
                memptr->male_gate_status = 0; // disable male gate
                printf("\033[32m");
                printf("Male gate has been closed");
                printf("\033[0m\n");
                fflush(stdout);
            }
            else if (memptr->people_in_male_gate_counter <= min_capacity_threshold)
            {                                 // To continue receive the people
                memptr->male_gate_status = 1; // enable male gate
            }
        }
    }

    if (rolling_gate_type[0] == 'F')
    {

        if (memptr->female_gate_status && memptr->womenOutside != 0)
        {

            if (msgrcv(queue_id, &message, sizeof(message), 1, 0) < 0)

            {
                perror("msgrcv --> failed");
                exit(1);
            }

            time(&rawtime);
            timeinfo2 = localtime(&rawtime);
            time_difference = calculate_time_difference(message.start_min, message.start_sec, timeinfo2->tm_min, timeinfo2->tm_sec);
            if (time_difference > message.patience_limit)
            {
                memptr->termination_variables[0]++;
                if (memptr->termination_variables[0] > passed_max_impatient_people_number)
                {
                    printf("\033[5;31m"); // set the color to red
                    printf("Terminating Due to Exceeding MAX_IMPATIENT_PASSENGERS_NUMBER\n");
                    printf("\033[0m"); // reset the color to the default
                    kill(getppid(), SIGINT);
                }
                printf("\033[5;31m"); // set the color to red
                printf("GATE: leaving person with pid  = %d (time = %d) (limit = %d)\n", message.from_pid, time_difference, message.patience_limit);
                printf("\033[0m"); // reset the color to the default
                
                kill(message.from_pid, SIGTERM);
            }
            acquire.sem_num = GRANTED_ACCESS;
            if (semop(semid, &acquire, 1) == -1)
            {
                perror("semop -- producer -- acquire4");
                exit(4);
            }
            memptr->people_in_female_gate_counter++;
            memptr->forked_people_number--;
            memptr->womenOutside--;

            release.sem_num = GRANTED_ACCESS;

            if (semop(semid, &release, 1) == -1)
            {
                perror("semop -- producer -- release");
                exit(5);
            }

            fflush(stdout);
            printf("\033[0;33m"); // set the color to yellow
            printf("ROLLING GATE RECEIVED SIGUSR1: %d, %c, %c\n", message.from_pid, message.gender[0], message.request_type[0]);
            printf("\033[0m"); // reset the color to the default
            fflush(stdout);

            if (memptr->people_in_female_gate_counter >= max_capacity_threshold)
            {
                memptr->female_gate_status = 0;

                printf("\033[32m");
                printf("Female gate has been closed");
                printf("\033[0m\n");
                fflush(stdout);
            }

            else if (memptr->people_in_female_gate_counter <= min_capacity_threshold)
            { // To continue receive the people
                memptr->female_gate_status = 1;
            }
        }
    }

    sleep(2);
    if (msgsnd(grouping_area_queue, &message, sizeof(message), 0) == -1)
    {
        perror("msgsnd");
        exit(1);
    }
}

void opening_gates()
{
    memptr->male_gate_status = 1;
    memptr->female_gate_status = 1;
}
int calculate_time_difference(int start_min, int start_sec, int end_min, int end_sec)
{
    return ((end_min - start_min) * 60 + (end_sec - start_sec)) % 10;
}