/*//////////////////////////////////////////////
//
//              Operating Systems
//               Mini-project - 2 
//              Esteban Segarra M.
//                  Section 1
//
//  Desc: Grabs the pstat strut and uses it to display
//        active processes and print out the values in
//        a formatted format.
//        
//
*///////////////////////////////////////////////
#include "types.h"
#include "stat.h"
//#include "pstat.h"
#include "user.h"




void printvals(int mul){
    for (int p = 0; p < mul; p++){
        print(1,"#");
    }
    return returnVal;
}

int main (void){
    struct values = sys_return_pstats();
    printf(1,"Displaying the amount of system calls.");
    size_t n = sizeof(values->inuse)/sizeof(values->inuse[0]);
    for (int i = 0; i < n ; i++){
            if (values->inuse[i] == 0){
            print (1,"Process PID: %d", values->pid[n]);
            print (1,"Process ticket amount: %d", values->tickets[n]);
            print (1,"Process runtime: ");
            printvals(values->ticks[n]);
            print (1, "\n");
        }
    } 
    exit();
}
