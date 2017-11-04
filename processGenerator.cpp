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
key_t upq;
key_t downq;
int main() 
{
    int choice = 5;
    int quantium = 0;
   // signal(SIGINT,ClearResources);
    //TODO: 
    // 1-Ask the user about the chosen scheduling Algorithm and its parameters if exists.
    // 2-Initiate and create Scheduler and Clock processes.
    while (choice > 3 || choice < 1)
    {       
        cout << "\nChoose algorithm";
        cout << "\n1)Non-Preemptive Highest Priority First";
        cout << "\n2)Shortest Remaining Time Next";
        cout << "\n3)Round Robin";
        cout << endl;
        cin >> choice;
        if (choice == 3)
        {
            cout << "\nEnter Quantium Time"<<endl;
            cin >> quantium;
        }
    }

    int pid;
    int Child_PID = 1;

    upq = msgget(12643,IPC_CREAT | 0644);
    downq = msgget(12645,IPC_CREAT | 0644);
    if (upq==-1)
    {
        perror("Error in Up Queue");
        exit(-1);
    }
    printf("UP Queue ID = %d\n",upq);
        
    if (downq==-1)
    {
        perror("Error in Down Queue");
        exit(-1);
    }
    printf("Down Queue ID = %d\n",downq);

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
    printf("Current Time is %d\n",x);
    
    /////////////////////////////////////////////////////////////////
     
    char *sprintf_buffer;
    char send,squant;
      
    send = '0'+ choice;
    char *argv[] = {"./sch.out",&send,NULL};
    //sprintf (sprintf_buffer, "%d", choice);
    if(choice==3)
    { char para[16];/////////////////////change//////////////////////
        sprintf (para, "%d", quantium);
      squant='0'+quantium;
      argv[2]=&squant;         //&squant;
       argv[3]=NULL;
    }
         
    int pid2=-1,Child_PID2;
    cout<<"Scheduler Forked";
    pid2 = fork(); //Schduler forking
    if (pid2 == 0)
    {
        Child_PID2 = execv ("./sch.out",argv);      
    }

    if(Child_PID2 == -1)
    {
        cout<<"Error in Forking Scheduler";
    }
 
    //Toget time use the following function 
    std::vector<process> processes;
    cout<<"read"<<endl;
    ifstream inFile("./processes");
    if(inFile.fail())
    {
        cout<<"Failed to read from file";
    }
    else
    {
        cout<<"Reading Success";
        processes = read_process(inFile);
        cout<<"arrsend:"<<endl;
        cout<<processes[0].arrival_time<<endl;
        cout<<"runsend:"<<endl;
        cout<<processes[0].run_time<<endl;
    }

    // 3-use this function after creating clock process to initialize clock
    cout<<"size:"<<endl;
    cout<<processes.size()<<endl;
    int time_now=0;
    int count=0;
    while(1)
    {
        time_now=getClk();
        //cout<<"time_now:"<<time_now<<"  next arr :"<<processes[count].arrival_time;

        while(time_now==processes[count].arrival_time)
        {
            int send_val;
            time_now=getClk();
            send_val = msgsnd(downq, &processes[count], sizeof(process)-sizeof(long), !IPC_NOWAIT);
       
            if(send_val == -1)
            {
                //perror("Error in send");
                cout<<"Error in Sending"<<endl;
            }
            else
            {
                cout<<"Entered"<<endl;
                count++;
            }      
        }
    }
   destroyClk(true);
}  

void ClearResources(int)
{
    //TODO: it clears all resources in case of interruption
    msgctl(upq,IPC_RMID, (struct msqid_ds *) 0);
	msgctl(downq,IPC_RMID, (struct msqid_ds *) 0);
	exit(0);
}