
#include "headers.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>

/* Modify this file as needed*/
int remainingtime = 0;


int main(int agrc, char* argv[]) 
{   
    //if you need to use the emulated clock uncomment the following line
    //initClk();
    remainingtime=stoi(argv[1]);
   // remainingtime=*argv[1] -'0';
    while(remainingtime > 0) 
    {    
       sleep(1);
       remainingtime--;
    }
    exit(0);

    //if you need to use the emulated clock uncomment the following line
    //destroyClk(false);

    return 0;
}
