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
#include "user.h"
#include "pstat.h"
 



void printvals(int mul){
    for (int p = 0; p < mul; p++){
        printf(1,"#");
    }
}

int main (void){
    struct pstat values;
    values = return_pstats();
    printf(1,"Displaying the amount of system calls.");
    int n = sizeof(values.inuse)/sizeof(values.inuse[0]);
    for (int i = 0; i < n ; i++){
            if (values.inuse[i] == 0){
            printf (1,"Process PID: %d", (int)values.pid[n]);
            printf (1,"Process ticket amount: %d", (int)values.tickets[n]);
            printf (1,"Process runtime: ");
            printvals((int)values.ticks[n]);
            printf (1, "\n");
        }
    } 
    exit();
}
