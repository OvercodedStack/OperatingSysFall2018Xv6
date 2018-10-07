#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "sysfunc.h"

#include string
#include 


//int sfs = 0; 

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;
  
  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since boot.
int
sys_uptime(void)
{
  uint xticks;
  
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


//Function to seek literal amount of tickets assigned to proc
//Returns 0 if successful, -1 if not
int
sys_settickets(int tick_set){
  int value;
  if(argint(0,&value) <0)
    return -1;
  proc->ticket_amt = tick_set;
  return 0;
}

string printvals(int mul){
    string returnVal = "";
    for (int p = 0; p < mul; p++){
        returnVal +=  "#"
    }
    return returnVal;
}

//Prints out a list of data processing. Will print out a graph when prompted. 
//Returns 0 if successful, -1 if not
int 
sys_getinfo(struct pstat *){
  int returnVal;
  if (argint(0,&returnVal) <0)
    return -1
  
  for (int i = 0; i < n ; i++){
          if (values->inuse[i] == 0){
          print (1,"Process PID: %d", values->pid[n]);
          print (1,"Process ticket amount: %d", values->tickets[n]);
          print (1,"Process runtime: %s \n", printvals(values->ticks[n]));
      }
  }
  return 0; 
}

// Return how many system calls have been made since the start of the program. Since boot. 
int
sys_howmanysys(void)
{
int call_amt = 0;
call_amt = proc->syscall_num;
return call_amt; 
}

void 