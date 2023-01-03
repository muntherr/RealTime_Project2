#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include "local.h"

// functions declarations
void signal_rolling_gate();
int generate_random_amount_of_time();

int security_officer_pid;
int female_rolling_gate_pid;
int male_rolling_gate_pid;
int random_number;

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

    signal(SIGUSR1, signal_rolling_gate);
    pid_t ppid = getppid();

    // Access and attach to shared memory
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
    security_officer_pid = getpid();
    female_rolling_gate_pid = atoi(argv[1]);
    male_rolling_gate_pid = atoi(argv[2]);

    // initially, both machines are available
    memptr->f_machine_available = 1;
    memptr->m_machine_available = 1;

    while (1)
        ;

    return 0;
}
void signal_rolling_gate()
{
    // female machine is availablel and male machine is occupied
    if (memptr->f_machine_available && !memptr->m_machine_available)
    {
        kill(female_rolling_gate_pid, SIGUSR1);
        sleep(2);

        memptr->f_machine_available = 0; // machine currently occupied
        sleep(generate_random_amount_of_time());
        memptr->f_machine_available = 1;
    }

    // male machine is availablel and female machine is occupied
    else if (memptr->m_machine_available && !memptr->f_machine_available)
    {
        kill(male_rolling_gate_pid, SIGUSR1);
        sleep(2);

        memptr->m_machine_available = 0; // machine currently occupied
        sleep(generate_random_amount_of_time());
        memptr->m_machine_available = 1;
    }
    // bothe machines are available
    else
    { // generate 0 or 1 randomly to specify which queue to signal
        srand(time(NULL));
        random_number = rand() % 2;

        // signal the head of the females queue
        if (random_number == 0)
        {
            kill(female_rolling_gate_pid, SIGUSR1);
            memptr->f_machine_available = 0; // machine currently occupied
            sleep(generate_random_amount_of_time());
            memptr->f_machine_available = 1;
        }
        // signal the head of the males queue
        else
        {

            kill(male_rolling_gate_pid, SIGUSR1);
            memptr->m_machine_available = 0; // machine currently occupied
            sleep(generate_random_amount_of_time());
            memptr->m_machine_available = 1;
        }
    }
}

int generate_random_amount_of_time()
{
    srand(time(NULL));
    random_number = rand() % 5 + 1;

    return random_number;
}