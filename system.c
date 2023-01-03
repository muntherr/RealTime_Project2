#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <wait.h>
#include <signal.h>
#include "local.h"

// functions declarations
void rolling_gates_forking();
void security_officer_forking();
void people_forking();
void grouping_area_manager_forking();
void metal_detector_machines_forking();
void kill_all();
void signal_to_open_gates();
void forkTellers();
void forkPainter();
void quit();
int tellersCount();
void initialize();
void read_configuration(char *);

// used variables
int number_of_rolling_gates;
int number_of_people;
int ROLLING_GATE_CAPACITY_THRESHOLD;
int tellersNo;
int tellers[TELMAX];
int painterPid;
int max_impatient_people_number;
int *rolling_gates_pids;
int security_officer_pid;
int *people_pids;
int generate_random_request_type;
int random_number;
int *QUEUE_ID;
int MALE_QUEUE_ID;
int rolling_gate_number;
int grouping_area_manager_pid;
int *machines_pids;
int grouping_area_queue;
int quitOnSatisfied;
int quitOnUnsatisfied;
int quitOnLeft;
static struct MEMORY memory;
struct MEMORY *memptr;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Error No file name specified.\n");
        return 1;
    }

    read_configuration(argv[1]); // reading configuration.txt file to get the required variables
    static ushort start_val[5] = {1, 1, 1, 1, 1};
    int semid, croaker;
    union semun arg;
    int shmid;
    char *shmptr;
    pid_t p_id, c_id, pid = getpid();
    memory.head = memory.tail = 0;
    tellersNo = tellersCount();
    initialize();

    grouping_area_queue = msgget(0, 0666 | IPC_CREAT);
    if (grouping_area_queue == -1)
    {
        perror("msgget");
        exit(1);
    }

    // Creating and attaching the shared memory
    if ((shmid = shmget((int)pid, sizeof(memory),
                        IPC_CREAT | 0600)) != -1)
    {

        if ((shmptr = (char *)shmat(shmid, 0, 0)) == (char *)-1)
        {
            perror("shmptr -- parent -- attach");
            exit(1);
        }
        memcpy(shmptr, (char *)&memory, sizeof(memory));
        memptr = (struct MEMORY *)shmptr;
    }
    else
    {
        perror("shmid -- parent -- creation");
        exit(2);
    }

    // Creating and initiallizing the semaphores
    if ((semid = semget((int)pid, 5,
                        IPC_CREAT | 0666)) != -1)
    {
        arg.array = start_val;

        if (semctl(semid, 0, SETALL, arg) == -1)
        {
            perror("semctl -- parent -- initialization");
            exit(3);
        }
    }

    rolling_gates_forking();
    grouping_area_manager_forking();
    people_forking();
    sleep(4);
    security_officer_forking();
    metal_detector_machines_forking();
    forkTellers();
    forkPainter();

    if (sigset(SIGINT, kill_all) == SIG_ERR)
    {
        printf("\033[0;31m"); // set the color to red
        perror("Sigset can not set SIGQUIT");
        printf("\033[0m"); // reset the color to the default
        exit(-2);
    }
    while (1)
    {
        quit();
    }
    return (0);
}

void initialize()
{
    // all shared memory variables are initially cleared
    for (int i = 0; i < MAX; i++)
    {
        memory.entering_people_ids[i].order = 0;
        memory.entering_people_ids[i].pid = 0;
        memory.entering_people_ids[i].request = 0;
    }
    memory.head = 0;
    memory.tail = 0;
    memory.people_in_male_gate_counter = 0;
    memory.people_in_female_gate_counter = 0;
    memory.b_serial_number_counter = 1;
    memory.i_serial_number_counter = 1;
    memory.t_serial_number_counter = 1;
    memory.r_serial_number_counter = 1;
    memory.female_gate_status = 0;
    memory.male_gate_status = 0;
    memory.f_machine_available = 0;
    memory.m_machine_available = 0;
    memory.termination_variables[0] = 0;
    memory.termination_variables[1] = 0;
    memory.termination_variables[2] = 0;
    memory.p = 0;
    for (int i = 0; i < 8; i++)
    {
        memory.tellers[i] = 'N';
    }
    memory.groupingAreaCount = 0;
    memory.menOutside = 0;
    memory.womenOutside = 0;
}

void forkPainter()
{
    // this process is responsible for rendering openGL window
    int pid;
    pid = fork();
    if (pid != 0)
        painterPid = pid;
    if (pid == 0)
        execl("./openGL", "./openGL", (char *)NULL);
}

void grouping_area_manager_forking()
{
    int pid;
    char pass_grouping_area_queue[10];
    char pass_max_impatient_people_number[5];

    if ((pid = fork()) == -1)
    {
        printf("fork failure ... getting out\n");
        exit(-1);
    }
    if (pid != 0)
    {                                    // parent
        grouping_area_manager_pid = pid; // save pid
    }
    else
    {
        sprintf(pass_max_impatient_people_number, "%d", max_impatient_people_number);
        sprintf(pass_grouping_area_queue, "%d", grouping_area_queue);
        execlp("./grouping_area", "./grouping_area", pass_grouping_area_queue, pass_max_impatient_people_number, (char *)NULL);
    }
}

void rolling_gates_forking()
{
    char pass_rolling_gate_capacity_threshold[10], pass_rolling_gate_status[5], pass_queue_id[10];
    int pid;
    int queue_id = 0;
    char pass_max_impatient_people_number[5];

    char pass_grouping_area_queue[10];
    int rolling_gate_capacity_threshold = ROLLING_GATE_CAPACITY_THRESHOLD;
    int rolling_gate_status = 0; // 0 -> Disabled, 1 -> Enabled (initially, gates are disabled)
    rolling_gates_pids = malloc((number_of_rolling_gates + 1) * sizeof(int));
    QUEUE_ID = malloc((number_of_rolling_gates + 1) * sizeof(int));
    int key = 5;

    for (int i = 0; i < number_of_rolling_gates; i++)
    {
        // message queues creation, one for females and another one for males
        queue_id = msgget(key, 0666 | IPC_CREAT);
        if (queue_id == -1)
        {
            perror("msgget");
            exit(1);
        }
        *(QUEUE_ID + i) = queue_id;
        key++;

        // forking rolling gates
        if ((pid = fork()) == -1)
        {
            printf("Error in Forking\n");
            exit(-1);
        }
        if (pid != 0)
        { // parent
            *(rolling_gates_pids + i) = pid;
        }
        else
        {
            char type[7];
            if (i == 0)
            { // females rolling gate

                strcpy(type, "Female");
            }
            else
            { // males rolling gate
                strcpy(type, "Male");
            }

            // converting passed aruments in exec to strings
            sprintf(pass_rolling_gate_capacity_threshold, "%d", rolling_gate_capacity_threshold);
            sprintf(pass_rolling_gate_status, "%d", rolling_gate_status);
            sprintf(pass_queue_id, "%d", queue_id);
            sprintf(pass_grouping_area_queue, "%d", grouping_area_queue);
            sprintf(pass_max_impatient_people_number, "%d", max_impatient_people_number);

            execlp("./rolling_gate", "./rolling_gate", type, pass_rolling_gate_capacity_threshold,
                   pass_rolling_gate_status, pass_queue_id, pass_grouping_area_queue, pass_max_impatient_people_number, (char *)NULL);
        }
    }
}
int tellersCount()
{
    int min_range = 2;
    int max_range = 9;
    int value;
    srand(time(NULL));
    value = (int)(min_range + (rand() % (max_range - min_range)));
    return value;
}
void forkTellers()
{
    int pid;
    for (int i = 0; i < tellersNo; i++)
    {
        if ((pid = fork()) == -1)
        {
            printf("Error in Forking\n");
            exit(-1);
        }

        if (pid != 0)
            tellers[i] = pid;
        if (pid == 0)
        {
            execl("./tellers", "./tellers", (char *)NULL);
        }
    }
}
void security_officer_forking()
{
    int pid;
    char pass_female_rolling_gate_pid[10], pass_male_rolling_gate_pid[10];
    if ((pid = fork()) == -1)
    {
        printf("Error in Forking\n");
        exit(-1);
    }
    if (pid != 0)
    { // parent
        security_officer_pid = pid;
    }
    else
    {
        int female_rolling_gate_pid = rolling_gates_pids[0];
        int male_rolling_gate_pid = rolling_gates_pids[1];
        sprintf(pass_female_rolling_gate_pid, "%d", female_rolling_gate_pid);
        sprintf(pass_male_rolling_gate_pid, "%d", male_rolling_gate_pid);
        execlp("./security_officer", "./security_officer", pass_female_rolling_gate_pid, pass_male_rolling_gate_pid, (char *)NULL);
    }
}

void people_forking()
{

    int pid;
    time_t time_arrive; // *** to give priority to whom arrive early
    char pass_patience_limit[3], pass_rolling_gate_number[10];
    people_pids = malloc((number_of_people + 1) * sizeof(int));

    for (int i = 0; i < number_of_people; i++)
    {
        if ((pid = fork()) == -1)
        {
            printf("fork failure ... getting out\n");
            exit(-1);
        }
        if (pid != 0)
        {                             // parent
            *(people_pids + i) = pid; // save pids
        }
        else
        { // forked child
          // give a random gender for each person
            srand(getpid());
            random_number = rand() % 9 + 1;
            char gender[2];
            if ((random_number % 2) == 0)
            {
                strcpy(gender, "F");
                rolling_gate_number = *(QUEUE_ID + 0);
                memptr->womenOutside++;
            }
            else
            {
                strcpy(gender, "M");
                rolling_gate_number = *(QUEUE_ID + 1);
                memptr->menOutside++;
            }

            // generate a random request type for each person
            srand(getpid());
            random_number = rand() % 4 + 1;
            char request_type[2]; // T, R, B, I
            if (random_number == 1)
            {
                strcpy(request_type, "B");
            }
            if (random_number == 2)
            {
                strcpy(request_type, "I");
            }
            if (random_number == 3)
            {
                strcpy(request_type, "T");
            }
            if (random_number == 4)
            {
                strcpy(request_type, "R");
            }

            // generate a random patience level for each person
            srand(getpid());
            random_number = rand() % 5 + 5; // minimum patience level is 5 for example
            sprintf(pass_patience_limit, "%d", random_number);
            sprintf(pass_rolling_gate_number, "%d", rolling_gate_number);
            memptr->forked_people_number++;

            execlp("./people", "./people", gender, request_type, pass_patience_limit, pass_rolling_gate_number, (char *)NULL);
        }
    }
}

void metal_detector_machines_forking()
{
    int pid;
    machines_pids = malloc((2 + 1) * sizeof(int));
    char pass_security_officer_pid[10];
    for (int i = 0; i < 2; i++)
    {

        if ((pid = fork()) == -1)
        {
            printf("fork failure ... getting out\n");
            exit(-1);
        }
        if (pid != 0)
        {                               // parent
            *(machines_pids + i) = pid; // save pid
        }
        else
        {
            char type[7];
            if (i == 0)
            { // females rolling gate

                strcpy(type, "Female");
            }
            else
            { // males rolling gate
                strcpy(type, "Male");
            }
            sprintf(pass_security_officer_pid, "%d", security_officer_pid);
            execlp("./metal_detector_machine", "./metal_detector_machine", pass_security_officer_pid, type, (char *)NULL);
        }
    }
}

void kill_all(int sig)
{
    int queue_id;
    for (int p = 0; p < number_of_rolling_gates; p++)
    {

        queue_id = *(QUEUE_ID + p);
        if (msgctl(queue_id, IPC_RMID, NULL) < 0)
        {
            // perror("msgctl  999 failed");
            exit(1);
        }
    }
    for (int i = 0; i <= number_of_people; i++)
    {
        kill((*people_pids + i), SIGTERM);
    }

    for (int i = 0; i <= number_of_rolling_gates; i++)
    {
        kill((*rolling_gates_pids + i), SIGTERM);
    }

    if (msgctl(grouping_area_queue, IPC_RMID, NULL) < 0)
    {
        perror("msgctl  999 failed");
        exit(1);
    }
    for (int i; i < 2; i++)
    {
        kill(*(machines_pids + i), SIGTERM);
    }
    for (int i = 0; i < 8; i++)
        kill(tellers[i], SIGINT);
    kill(painterPid, SIGINT);

    kill(security_officer_pid, SIGTERM);

    exit(0);
}

void signal_to_open_gates()
{
    for (int i = 0; i < number_of_rolling_gates; i++) // Signal sent from system.c to both rolling gates:
                                                      // Update the gate_status to be enabled (1)
    {
        kill(*(rolling_gates_pids + i), SIGUSR2);
        sleep(2);
    }
}
void read_configuration(char *file_name)
{
    char read[50];
    FILE *fptr;
    fptr = fopen(file_name, "r");
    if (fptr == NULL)
    {
        printf("\033[0;31m"); // set the color to red
        perror("Error while opening the file.\n");
        printf("\033[0m"); // reset the color to the default
        exit(-1);
    }
    while (fgets(read, sizeof(read), fptr))
    {
        char *token = strtok(read, " ");
        if (strcmp(token, "NUMBER_OF_ROLLING_GATE") == 0)
        {
            number_of_rolling_gates = atoi(strtok(NULL, "\n"));
        }
        else if (strcmp(token, "NUMBER_OF_PEOPLE") == 0)
        {
            number_of_people = atoi(strtok(NULL, "\n"));
        }
        else if (strcmp(token, "ROLLING_GATE_CAPACITY_THRESHOLD") == 0)
        {
            ROLLING_GATE_CAPACITY_THRESHOLD = atoi(strtok(NULL, "\n"));
        }
        else if (strcmp(token, "max_impatient_people_number") == 0)
        {
            max_impatient_people_number  = atoi(strtok(NULL, "\n"));
        }
        else if (strcmp(token, "quitOnSatisfied") == 0)
        {
            quitOnSatisfied= atoi(strtok(NULL, "\n"));
        }
        else if (strcmp(token, "quitOnUnsatisfied") == 0)
        {
            quitOnUnsatisfied = atoi(strtok(NULL, "\n"));
        }
        else if (strcmp(token, "quitOnLeft") == 0)
        {
            quitOnLeft = atoi(strtok(NULL, "\n"));
        }
        
    }
    if (fclose(fptr))
    {
        exit(-1);
    }
}
void quit()
{
    if (memptr->termination_variables[0] >= quitOnLeft)
    {
        printf("Terminted exceded max number of left people\n");
        kill_all(0);
    }

    if (memptr->termination_variables[1] >= quitOnUnsatisfied)
    {
        printf("Terminted exceded max number of Unsatisfied people\n");
        kill_all(0);
    }
    if (memptr->termination_variables[2] >= quitOnSatisfied)
    {
        printf("Terminted exceded max number of Unsatisfied people\n");
        kill_all(0);
    }
}