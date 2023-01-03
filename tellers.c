#include "local.h"

char service = 'B';
char *shmptr;
struct MEMORY *memptr;
int shmid;
int semid;
int ppid;
int seat;
void getMemory();

int searchRequest();
void removePerson(int i);
void serve();
void incrementOrder();
void switchService();
void resetOrder();
int order();
void evaluate();
int takeSeat();
void inizializeService();

int main(int argc, char *argv[])
{
    ppid = getppid();
    inizializeService();
    getMemory();

    seat = takeSeat();
    while (1)
    {
        sleep(5);
        serve();
    }
}

void serve()
{

    int i = searchRequest();
    if (i == -1)
    {
        switchService();
        resetOrder();
    }
    if (i != -1)
    {
        printf("PERSON %d GOT SERVED BY TELLER %d\n", memptr->entering_people_ids[i].pid, getpid());
        evaluate();
        removePerson(i);
    }
}

int searchRequest()
{

    int i = 0;
    for (i = 0; i < MAX; i++)
    {
        if (memptr->entering_people_ids[i].request == service && memptr->entering_people_ids[i].order == order())
        {
            // serve person
            incrementOrder();
            return i;
        }
    }
    return -1;
}

void removePerson(int i)
{
    memptr->entering_people_ids[i].order = 0;
    memptr->entering_people_ids[i].request = 0;
    memptr->entering_people_ids[i].pid = 0;
    memptr->groupingAreaCount--;
}

void getMemory()
{
    if ((shmid = shmget((int)getppid(), 0, 0)) != -1)
    {
        if ((shmptr = (char *)shmat(shmid, (char *)0, 0)) == (char *)-1)
        {
            perror("shmat Tellers -- producer -- attach");
            exit(1);
        }
        memptr = (struct MEMORY *)shmptr;
    }
    else
    {
        perror("shmget Tellers-- producer -- access");
        exit(2);
    }
}

void incrementOrder()
{
    switch (service)
    {
    case 'R':
        memptr->r_serial_number_counter++;
        break;
    case 'B':
        memptr->b_serial_number_counter++;
        break;
    case 'I':
        memptr->i_serial_number_counter++;
        break;
    default:
        memptr->t_serial_number_counter++;
        break;
    }
}

int order()
{
    switch (service)
    {
    case 'R':
        return memptr->r_serial_number_counter;
        break;
    case 'B':
        return memptr->b_serial_number_counter;
        break;
    case 'I':
        return memptr->i_serial_number_counter;
        break;
    default:
        return memptr->t_serial_number_counter;
        break;
    }
}

void resetOrder()
{
    switch (service)
    {
    case 'R':
        memptr->r_serial_number_counter = 1;
        break;
    case 'B':
        memptr->b_serial_number_counter = 1;
        break;
    case 'I':
        memptr->i_serial_number_counter = 1;
        break;
    default:
        memptr->t_serial_number_counter = 1;
        break;
    }
}

void switchService()
{
    switch (service)
    {
    case 'R':
        service = 'B';
        memptr->tellers[seat] = 'B';
        break;
    case 'B':
        service = 'I';
        memptr->tellers[seat] = 'I';
        break;
    case 'I':
        service = 'T';
        memptr->tellers[seat] = 'T';
        break;
    default:
        service = 'R';
        memptr->tellers[seat] = 'R';
        break;
    }
}

void evaluate()
{
    int min_range = 0;
    int max_range = 2;
    int value;
    srand(time(NULL));
    value = (int)(min_range + (rand() % (max_range - min_range)));
    if (value > 0)
        memptr->termination_variables[2]++;
    else
        memptr->termination_variables[1]++;
}

int takeSeat()
{
    for (int i = 0; i < 8; i++)
    {
        if (memptr->tellers[i] == 'N')
        {
            memptr->tellers[i] = service;
            return i;
        }
    }
    return -1;
}

void inizializeService()
{
    int min_range = 0;
    int max_range = 5;
    int value;
    srand(getpid());
    value = (int)(min_range + (rand() % (max_range - min_range)));
    switch (value)
    {
    case 0:
        service = 'B';
        break;
    case 1:
        service = 'R';
        break;
    case 2:
        service = 'I';
        break;
    default:
        service = 'T';
        break;
    }
}