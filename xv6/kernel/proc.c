#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "pstat.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;
struct pstat process_statuses; 

int MINTICKETS = 15; 
int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

struct spinlock lgp;

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

//================Process locks for tickets================
//Make sure to lock ptable before using some of the ticket commands. 
int ticket_total; 

//Function to fill out the pstats data structure. 
void
get_pstats(void){
  struct proc *p;
  int var = 0;
  p = proc;
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
        process_statuses.inuse[var]    = p->inuse;
        process_statuses.tickets[var]  = p->ticket_amt;
        process_statuses.pid[var]      = p->pid;
        process_statuses.ticks[var]    = p->ticks;
      process_statuses.lastwinner[var] = p->winnerNum;
      var+=1;
    } 
}

//Calculate ticket total
int 
get_lottoTotal(void){
struct proc *p;
int total_tickets_output = 0;
for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
  if(p->state == RUNNABLE){
    //cprintf("val: %d",p->ticket_amt);
    total_tickets_output+=p->ticket_amt;
  }
}

return total_tickets_output ;
}

//Set the process's tickets
void
set_process_tickets(struct proc* p, int t){
    p->ticket_amt = t;
}

//Generate a random "seed" utilizing howmanysys values
int 
genRand(struct proc* p)
{
  int next_val = p->syscall_num;
  next_val = ((next_val * next_val)/100)%10000;
  return next_val;
}

//Random number generator for random generation purposes
int 
rand_gen(int min, int max,struct proc* p){
  int genVal = (genRand(p) % (max + 1 - min)) + min;
  return genVal;
}
//===========================================================

// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;
  release(&ptable.lock);
  return 0;


found:
  p->state = EMBRYO;
  p->pid = nextpid++;

  
  release(&ptable.lock);


  // Allocate kernel stack if possible.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;
  
  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;
  
  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];
  
  p = allocproc();
  acquire(&ptable.lock);
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  //Process must initialize with a number of tickets inside of it. 
  set_process_tickets(p,MINTICKETS);
  
  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  p->state = RUNNABLE;
  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *p;
  //pde_t *temp = proc->pgdir;

  sz = proc->sz;
  if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  }

  acquire(&ptable.lock);
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if (p->pgdir != proc->pgdir)
      continue;
    
    p->sz = sz;
    /*
    acquire(&lgp);
    switchuvm(p);
    release(&lgp);    */
  }
  
  proc->sz = sz;
  release(&ptable.lock);
  switchuvm(proc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;

  // Allocate process.
  if((np = allocproc()) == 0)
    return -1;

  // Copy process state from p.
  if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }

  

  np->sz = proc->sz;
  np->parent = proc;
  *np->tf = *proc->tf;
  
  //Copy the amount of tickets over from parent to child
  //Formally processed by a method
  if (proc->ticket_amt <=0){
    np->ticket_amt = MINTICKETS;
    proc->ticket_amt = MINTICKETS;
  } else{
    set_process_tickets(np, proc->ticket_amt);
  }  

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);
 
  pid = np->pid;

  np->state = RUNNABLE;
  safestrcpy(np->name, proc->name, sizeof(proc->name));
  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *p;
  int fd;

  if(proc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }


  iput(proc->cwd);

  proc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(proc->parent);



  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == proc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    } 
  }

  //Since process died, remove from ticket total. 
  set_process_tickets(p,0);
  get_pstats();
  // Jump into the scheduler, never to return.
  proc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;

  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for zombie children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->state = UNUSED;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->ticks =0; 
        set_process_tickets(p,0); //We halted the process and desire no more activity.
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
/*
void
scheduler(void)
{
  struct proc *p;
  int tick_counter = 0; 

  acquire(&ptable.lock);
  set_process_tickets(ptable.proc,15);
  release(&ptable.lock);
  for(;;){   
    // Enable interrupts on this processor.
    sti();    
    tick_counter = 0; //Current ticket counter

    acquire(&ptable.lock);
    ticket_total = get_lottoTotal(); //Total amount of tickets in system
    int winner_ticket = rand_gen(0, ticket_total,p);
    if (ticket_total < winner_ticket){
        winner_ticket %= ticket_total;
    }
    release(&ptable.lock);

    // Loop over process table looking for process to run.
    // Switch to chosen process.  It is the process's job
    // to release ptable.lock and then reacquire it
    // before jumping back to us.
    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state == RUNNABLE){
        tick_counter += p->ticket_amt;
      } 
      if(p->state != RUNNABLE || winner_ticket >=0){
        continue; 
      }   
      //We cannot have a counter less than the winner, if so skip processing. 
      int printable = 0;
      if (tick_counter < winner_ticket)
      {
        continue;
      }else if (tick_counter > winner_ticket && printable == 1){
        cprintf("Total tickets: %d\n", ticket_total);
        cprintf("Ticket counter: %d\n", tick_counter);
        cprintf("Winner ticket: %d\n", winner_ticket);
        cprintf("Process PID: %d  in %d  with %d\n",p->pid,p->state,p->ticket_amt);
        
      }
      proc = p; 
      p->inuse = 1;
      switchuvm(p);
      p->state = RUNNING;
      p->winnerNum = p->state;
      //Start the process.
      const int start_ticks = ticks; 
      swtch(&cpu->scheduler, proc->context);
      
      // Process is done running for now.
      // It should have changed its p->inuse before coming back.
      p->ticks += ticks - start_ticks;
      p->inuse = 0;
      switchkvm();
      proc = 0;
      break; 
    }
    
    release(&ptable.lock);
    ticket_total = 0; 
    tick_counter = 0; 
  }
}
*/

void
scheduler(void)
{
  struct proc *p;

  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      proc = p;
      switchuvm(p);
      p->state = RUNNING;
      swtch(&cpu->scheduler, proc->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      proc = 0;
    }
    release(&ptable.lock);

  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state.
void
sched(void)
{
  int intena;

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(cpu->ncli != 1)
    panic("sched locks");
  if(proc->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = cpu->intena;
  swtch(&proc->context, cpu->scheduler);
  cpu->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  proc->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);
  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  if(proc == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  proc->chan = chan;
  proc->state = SLEEPING;
  sched();

  // Tidy up.
  proc->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
        set_process_tickets(p,0); //Again, remove the tickets from proc
      release(&ptable.lock);
      
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  cprintf("SCREAMS OF AGONY");
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];
  
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }

}

//This function allows the processes to wait for each other and "join" once a process
//state changes from zombie to running/ready. 
int join(void **stack){
  cprintf("SCREAMS OF TYRANNY");
  struct proc *p;         //Process struct to manipulate
  int childThreads,pid;   //How many threads we have and its pid
  acquire(&ptable.lock);  //Ptable lock
  
  for(;;){
    childThreads = 0;
    for (p=ptable.proc; p< &ptable.proc[NPROC];p++){
      if (p->pgdir != proc->pgdir || p->parent != proc|| proc->pid == p->pid){
        continue;
      }
      //We've identified this to be a child.
      childThreads = 1;
    
      if(p->state == ZOMBIE){
        //We're going to adquire and set some values at this point
        int *tmp = (int*) 0x1FD8;
        pid  = p->pid;
        void *stackAddr = (void *)p->parent->tf->esp + 7*sizeof(void *);
        *(uint *)stackAddr = p->tf->ebp;
        *(uint *)stackAddr += 3 * sizeof(void *) - PGSIZE;

        kfree(p->kstack);
        p->parent = 0;
        p->name[0] = 0;
        p->kstack = 0;
        p->pid = 0;
        p->killed = 0;
        p->state = UNUSED;
        
        // Get stack of the zombie child thread to return
        *tmp = pid;
         *((int*)((int*)stack))=p->stack;
        release(&ptable.lock);
        return pid;
      }
    }
  //No Children conditional
  if(!childThreads || proc->killed){
    release(&ptable.lock);
    return -1;
  }
  //The actual wait function for the children.
  sleep(proc, &ptable.lock);
  }
  return 0;
}
  
int clone(void(*fcn)(void*), void *arg, void *stack){
  int j,pid;
  uint ustack[2];
  struct proc *np;
  cprintf("SCREAMS OF CLONING");

  //Check if process can be allocated, else drop.
  if((np = allocproc())==0){
    return -1;
  }

  //Copy and allocate process data from one process to the next
  np->stack = (uint)stack;
  np->pgdir = proc->pgdir;
  np->sz = proc->sz;
  np->parent = proc; 
  *np->tf = *proc->tf;

  //init stack - Remember that the size of one page is 4096 bytes
  void *argStack, *rectStack;

  np->tf->esp = (uint) stack + PGSIZE;
  argStack = stack + 4096 - sizeof(void*);
  *(uint *)argStack = (uint)arg;
  ustack[1] = (uint)arg;  
  ustack[0] = 0xFFFFFFFF;
  rectStack = stack + 4096 - 2 * sizeof(void*);
  *(uint *)rectStack = 0xFFFFFFFF;

  np->tf->esp -=(2)*4;
  copyout(np->pgdir, np->tf->esp, ustack, (2)*4);


  np->tf->ebp = np->tf->esp;
  np->tf->eip = (int)fcn;
  //Create the location where the stack pointer will be. 
  //np->tf->esp = (int)stack;
  //memmove((void *)np->tf->esp,stack,PGSIZE);
  //np->tf->esp += PGSIZE - 2 * sizeof(void *);
  
  

  	
  //Copy over the ofile from one process to another
  for (j =0; j < NOFILE; j++){
    if (proc->ofile[j]){
      np->ofile[j] = filedup(proc->ofile[j]);
    }
  }
  np->cwd = idup(proc->cwd);
  np->tf->eax = 0;
  np->state = RUNNABLE;
  safestrcpy(np->name,proc->name,sizeof(proc->name));
  //Return pid of process 
  pid = np->pid;
  return pid;
}



