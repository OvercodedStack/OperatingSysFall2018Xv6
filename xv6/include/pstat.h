
#include "param.h"
#ifndef _PSTAT_H_
#define _PSTAT_H_


/*
Let's consider this as a formed strut that can store statistics of the processes that have ran
so far on the operating system.
*/
struct pstat {
    int inuse[NPROC];   //whether this slot of the process table is in use (1 or 0)
    int tickets[NPROC]; //the number of tickets this process has
    int pid[NPROC];     //the PID of each process
    int ticks[NPROC];   //the number of ticks each process has accumulated
    int lastwinner[NPROC];
};

#endif // _PSTAT_H_