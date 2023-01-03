/*
 * Common header file: parent, producer and consumer
 */

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
#include <errno.h>
#include <time.h>
#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#define TELMAX 8

#define MAX 1000

// structure for message queue
struct mesg_buffer
{
  long mesg_type;
  int from_pid; /* sender pid */
  char gender[1];
  char request_type[2];
  int patience_limit;
  int start_sec;
  int start_min;
  char mesg_text[100];
  int serial_number;
} message;

union semun
{
  int val;
  struct semid_ds *buf;
  ushort *array;
};

struct person
{
  int pid;
  int request;
  int order;
} person;

struct MEMORY
{
  struct person entering_people_ids[MAX]; // people entering the grouping area waiting to tellers' services
  int head, tail;
  int forked_people_number;
  // int number_of_people_term;
  int people_in_male_gate_counter;
  int people_in_female_gate_counter;
  int b_serial_number_counter;
  int i_serial_number_counter;
  int t_serial_number_counter;
  int r_serial_number_counter;
  int female_gate_status; // females rolling gate status: 0 --> disabled, 1 --> enabled
  int male_gate_status;
  int f_machine_available; // females metal detector machine availability: 0 --> occupied, 1 --> available
  int m_machine_available;
  int termination_variables[3]; // termination_varibale[0]: counter of people leavig unserved
                                // termination_varibale[1]: counter of people leavig unhappy
                                // termination_varibale[2]: counter of people leavig satisfied
  int p;

  char tellers[8];
  int groupingAreaCount;
  int menOutside;
  int womenOutside;
};

struct sembuf acquire = {0, -1, SEM_UNDO},
              release = {0, 1, SEM_UNDO};

enum
{
  GRANTED_ACCESS,
  DENIED_ACCESS,
  TIME_LIMIT_EXCEEDED,
  CURRENT_HALL_CAPACITY,
  BUS_TURN
};

