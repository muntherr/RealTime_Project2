#include "local.h"

// functions declarations
void give_ticket();
int calculate_time_difference(int, int, int, int);
int grouping_area_queue;
int semid, shmid;
char *shmptr;
struct MEMORY *memptr;
int b_displacement = 0;
int i_displacement = 0;
int t_displacement = 0;
int r_displacement = 0;
int passed_max_impatient_people_number;
time_t rawtime;
struct tm *timeinfo2;
int time_difference;
int last_pid;
int ma = 5;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror("Number of arguments must be 2!");
        exit(-1);
    }
    pid_t ppid = getppid();
    grouping_area_queue = atoi(argv[1]);
    passed_max_impatient_people_number = atoi(argv[2]);

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

    while (1)
    {
        give_ticket();
    }
    return 0;
}

void give_ticket()
{

    if (msgrcv(grouping_area_queue, &message, sizeof(message), 1, 0) < 0)
    {
        perror("msgrcv === failed");
        exit(1);
    }

    if (message.gender[0] == 'F' && memptr->people_in_female_gate_counter != 0)
    {
        memptr->people_in_female_gate_counter--;
    }
    if (message.gender[0] == 'M' && memptr->people_in_male_gate_counter != 0)
    {
        memptr->people_in_male_gate_counter--;
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
        printf("GROUPING AREA: killing the leaving person with pid  = %d (time = %d) (limit = %d)\n", message.from_pid, time_difference, message.patience_limit);
        kill(message.from_pid, SIGTERM);
    }
    memptr->entering_people_ids[memptr->tail].pid = message.from_pid;
    memptr->entering_people_ids[memptr->tail].request = message.request_type[0];

    if (memptr->entering_people_ids[memptr->tail].request == 'B')
    {
        memptr->entering_people_ids[memptr->tail].order = memptr->b_serial_number_counter + b_displacement;
        b_displacement++;
    }
    else if (memptr->entering_people_ids[memptr->tail].request == 'I')
    {
        memptr->entering_people_ids[memptr->tail].order = memptr->i_serial_number_counter + i_displacement;
        i_displacement++;
    }
    else if (memptr->entering_people_ids[memptr->tail].request == 'T')
    {
        memptr->entering_people_ids[memptr->tail].order = memptr->t_serial_number_counter + t_displacement;
        t_displacement++;
    }
    else
    {
        memptr->entering_people_ids[memptr->tail].order = memptr->r_serial_number_counter + r_displacement;
        r_displacement++;
    }
    if (memptr->entering_people_ids[memptr->tail].pid != last_pid)
    {
        memptr->groupingAreaCount++;
        last_pid = memptr->entering_people_ids[memptr->tail].pid;
        printf("\033[0;32m"); // set the color to green
        printf("--------------- PERSON %d WITH REQUEST %c AND ORDER %d ENTERED GROUPING AREA  ---------------\n",
               memptr->entering_people_ids[memptr->tail].pid,
               memptr->entering_people_ids[memptr->tail].request,
               memptr->entering_people_ids[memptr->tail].order);
        printf("\033[0m"); // reset the color to the default
        fflush(stdout);
    }
    else
        printf("\n");

    memptr->tail = (memptr->tail + 1) % MAX;
}
int calculate_time_difference(int start_min, int start_sec, int end_min, int end_sec)
{
    return ((end_min - start_min) * 60 + (end_sec - start_sec)) % 10 - 5;
}