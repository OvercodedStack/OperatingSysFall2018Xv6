/*//////////////////////////////////////////////
//
//              Operating Systems
//               Mini-project - 1 
//              Esteban Segarra M.
//                  Section 1
//
//  Desc: Program that passes an array of integers
//        and then perform the summation of all 
//        integers in the child process. Then
//        the sum is sent to the parent process.
//
*///////////////////////////////////////////////
#include "types.h"
#include "stat.h"
#include "user.h"

//Start running system call howmanysys();

int main (void){
    printf(1,"Displaying the amount of system calls.");
    printf(1,"There are %d system calls.\n", howmanysys());
    exit();
}



