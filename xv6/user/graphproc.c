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

void timewaster(void){
  int timer;
  int very_large_num =19035763;
  for(timer = 0; timer <very_large_num ; timer++){     
  }; 
}

//Generate a random "seed"
int 
genRand()
{
  int next_val = howmanysys();
  next_val = ((next_val * next_val)/100)%10000;
  return next_val;
}

//Random number generator for random generation purposes
int 
rand_gen(int min, int max){
  int genVal = (genRand() % (max + 1 - min)) + min;
  return genVal;
}


int main (void){
    //Total runnable proc
    int n;
    int childs = 2; 

    printf(1,"Starting the printout.\n");
    for(n = 0; n < childs; n++){
        if(n>1){
            //printf(1,"In parent.\n");
            settickets(30);  
            timewaster(); 
            //getpinfo();
        }
        if(n==0){
            //printf(1,"In child.\n");
            settickets(30);
            timewaster();
            //getpinfo();
        }
        if(fork()==0){
            //printf(1,"Forking\n");
            int randNum = rand_gen(0,3) + 1;
            if (randNum == 1){
                settickets(10);
            } 
            else if(randNum == 2){
                settickets(20);
            }else{
                settickets(30);
            }
            timewaster();
            //getpinfo();
            exit();
        }
        
    }
    
    for (int n = 0; n < childs; n++ ){
        getpinfo();
        wait();
        
    }
    exit();
}
