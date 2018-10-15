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
 
int main (void){
    //Total runnable proc
    int n;
    int children = 2; 
    settickets(10);
    printf(1,"Starting the printout.\n");
    for(n = 0; n < children; n++){
        if(n==1)
            settickets(20);   
        if(n==0)
            settickets(30);
        if(fork()==0)
            getpinfo();
            exit();
    }
    for (int n = 0; n < children; n++ ){
        getpinfo();
        wait();
    }
    exit();
}
