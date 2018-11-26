/*//////////////////////////////////////////////
//
//              Operating Systems
//               Mini-project - 4
//              Esteban Segarra M.
//                  Section 1
//
//  Desc:  Test program  - Check global workspace. 
//        
//
*///////////////////////////////////////////////
#include "types.h"
#include "stat.h"
#include "user.h"
volatile int global = 0;

void worker (void *arg_ptr){
    //assert(global == 1);
    printf(1,"I WORK\n");
    if (global == 0){
        global = 5;
    }
    exit();
}

int main(int argc, char *argv[]){
    //int ppid = getpid();
    int PGSIZE = 4096;
    void *stack = malloc(PGSIZE*2);
    //assert(stack != NULL);

    if((uint)stack %PGSIZE && stack != NULL)
        stack = stack + (4096 - (uint)stack % PGSIZE);
    int clone_pid = clone(worker,0, stack);
    printf(1,"Num:%d\n",clone_pid);
    //assert(clone_pid > 0);
    if (clone_pid > 0){
        printf(1,"I scream %d\n",global);
        while (global != 5);
        printf(1,"I scream %d\n",global);
        printf(1,"TEST PASSED\n");
    }
    else{
        printf(1,"FAIL");
    }



    exit();
}