/*//////////////////////////////////////////////
//
//              Operating Systems
//               Mini-project - 3
//              Esteban Segarra M.
//                  Section 1
//
//  Desc:  Creates a case where a dereference occurs
//         and an exception is created. 
//        
//
*///////////////////////////////////////////////
#include "types.h"
#include "stat.h"
#include "user.h"


int main (void){
    int *x = 0;
    printf(1,"Attemping to read value: %d\n",x);
    printf(1,"Pointer setting pointer *x to a value: 0.\n");
    *x = NULL;
    printf(1,"Attemping to read value: %d\n",x);
   exit();
}