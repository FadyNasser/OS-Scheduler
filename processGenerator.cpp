#include "headers.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
using namespace std;

void ClearResources(int);
key_t downq;
int main() 
{
    int choice = 5;
    int quantium = 0;
    signal(SIGINT,ClearResources);
    
    while (choice > 3 || choice < 1) //keep asking the user if he entered wrong input
    {       
        cout << "\nChoose algorithm";
        cout << "\n1)Non-Preemptive Highest Priority First";
        cout << "\n2)Shortest Remaining Time Next";
        cout << "\n3)Round Robin";
        cout << endl;
        cin >> choice;
        if (choice == 3)
        {
            cout << "Enter Quantium Time"<<endl;
            cin >> quantium;
        }
    }

    int pid;
    int Child_PID = 1;

    
    downq = msgget(12645,IPC_CREAT | 0644);// creating the queue
 
        
    if (downq==-1)
    {
        perror("Error in Down Queue");
        exit(-1);
    }
    

    pid = fork(); //clock forking
    
    if (pid == 0)
    {
        Child_PID = execv("./clock.out",NULL);//(path,array of * chars to pass arg)
    }
    if(Child_PID == -1)
    {
        cout<< "Error in Forking clock" ;
    }

    initClk();
    int x= getClk();
    
    
    /////////////////////////////////////////////////////////////////
     
    char *sprintf_buffer;
    char send,squant; 
    send = '0'+ choice;
    char *argv[] = {"./sch.out",&send,NULL};

    if(choice==3)
    { 
        char para4[16];
        sprintf (para4, "%d", quantium);
        argv[2]=para4;         //&squant;
        argv[3]=NULL;
    }
         
    int pid2=-1,Child_PID2;
  
    pid2 = fork(); //Schduler forking
    if (pid2 == 0)
    {
        Child_PID2 = execv ("./sch.out",argv);      
    }

    if(Child_PID2 == -1)
    {
        cout<<"Error in Forking Scheduler";
    }
 
   
    std::vector<process> processes; //reciving all processes from input file
    ifstream inFile("./processes");
    if(inFile.fail())
    {
        cout<<"Failed to read from file";
    }
    else
    {
        processes = read_process(inFile); //return vector of all processes
    }


    int time_now=0;
    int count=0;
    while(1)
    {
        time_now=getClk();
        //cout<<"time_now:"<<time_now<<"  next arr :"<<processes[count].arrival_time;

        while(time_now==processes[count].arrival_time)
        {
            int send_val;
            time_now=getClk();  //send the process to scheduler if it is arrived
            send_val = msgsnd(downq, &processes[count], sizeof(process)-sizeof(long), !IPC_NOWAIT);

            if(send_val == -1)
            {
                cout<<"Error in Sending"<<endl;
            }
            else
            {
                count++;
            }      
        }
    }
   destroyClk(true);
}  

void ClearResources(int)
{
	msgctl(downq,IPC_RMID, (struct msqid_ds *) 0);
	exit(0);
}