#include "headers.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <queue>
using namespace std;

//TODO: pausing,clk,killing,sending,get id of buffer
key_t upq;
key_t downq;
bool fork_flag=false;
bool pause_flag=false;
int check=-1;
std::vector<process> processes;

void handler(int number)
{
   fork_flag=false;
   pause_flag=false;      
   //get req stat
   cout << "Process ID " << processes.begin()->id << " Is Killed" <<endl;
   processes.erase(processes.begin());
   cout<<"You Made It "<<endl;
   int timefin=getClk();
   cout<<timefin<<endl;
   cout<<processes.size()<<endl;
   //delete from vector
}


void prohandler(int number)
{
   cout<<"Remaining Time: "<<endl;
   processes[0].run_time--;
   cout<<processes[0].run_time;
   //delete from vector
}

struct by_run
{
  bool operator()(process const &a,process const &b) const 
  {
     return a.run_time < b.run_time;
  }
};

struct by_runRR
{
  bool operator()(process const &a,process const &b) const 
  {
    if (a.run_time == 0 || b.run_time == 0)
    {
     return a.run_time < b.run_time;
    }
  }
};


int main(int argc, char* argv[]) 
{
  signal (SIGCHLD, handler);
  signal (SIGUSR1, prohandler);
  struct sigaction act;
  
  act.sa_handler = handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_NOCLDSTOP;  
  if (sigaction(SIGCHLD, &act, 0) == -1)
  {
    perror("Sigaction");
    exit(1);
  }
  
  initClk();
  int pid2,Child_PID2=1;
  upq = msgget(12643,IPC_CREAT | 0644);
  downq = msgget(12645,IPC_CREAT | 0644);
  int timestep=0;
  int count=0;
  int id_run=-1;
  int remaining=-1;
  int alog;
  int msgq_arr_pr;
  int rec_val;
  char send;
  int Quant;
   
  //TODO: implement the scheduler :)
  timestep=getClk();
  cout<<*argv[1]<<endl;        
  alog=*argv[1]-'0'; 
  if(alog==3)
  {
    //q=strtol(argv[2],argv,10);////////////change//////////
    Quant=*argv[2]-'0'; 
    cout<<"Quantam:"<< Quant <<endl;
  }

  struct process *rec_pro=new process(0,0,0,0);
  count=0;
    
  //Scheduler Algorithms
  // 1 => Non-Preemptive Highest Priority First      => Aya Mohamed
  // 2 => Shortest Remaining Time Next               => Mohamed Sameh
  // 3 => Round Robin                                => Fady Nasser

  if (alog == 1) // Highest Priority First
  {
    //FILE *fp = freopen("Test.out","w",stdout); // to output results on test file
    cout << "\nBegin of Non-Preemptive Highest Priority First"<<endl;

    while (1) // Write the code inside while
    {




    }

    //upon termination release clock
    cout << "Processes are Done" << endl;
    //fclose(fp); //Close the output file
    cout << "Terminating" << endl;
    destroyClk(true);  
  }
  else if (alog == 2) 
  {
    FILE *fp = freopen("scheduler.log","w",stdout);
    cout << "\nBegin of Shortest Remaining Time Next"<<endl;
    while(1)
    {
      int counter=processes.size();
      rec_val = msgrcv(downq, rec_pro,sizeof(process)-sizeof(long), 0,IPC_NOWAIT);  
      if(rec_val != -1)
      {
          cout<<"I Recieved Process at Clock: "<<endl;
          processes.emplace_back(rec_pro->id, rec_pro->arrival_time, rec_pro->run_time, rec_pro->proirty);
          count++;
          std::sort(processes.begin(),processes.end(),by_run());
      }
     int t=1;
      if(fork_flag==true)
      {
        if(processes[0].id!=id_run )
        {
          kill(pid2,SIGSTOP);
          for( t=1;t<processes.size();t++)
          {
           if (id_run ==processes[t].id)
           break;
          }
          processes[t].mtype=pid2;  //// you should make sure of [1],loop untill 
          pause_flag=true;             // id_run =processes[i]=id_run;
          cout<<endl<<"At time "<<timestep<<" process "<<processes[t].id<<" stopped ";
          cout<<" arr "<<processes[t].arrival_time;
          cout<<" total "<<processes[t].run<<" remain "<<processes[t].run_time;
          cout<<" wait "<<processes[t].waited;
        }
      }
      bool choose=false;
      if(fork_flag==false || pause_flag==true)
      {
        if(processes.size()>0)
         {
          if(pause_flag==true)
          choose=true;
         // send= '0'+processes[0].run_time;  
          char para[16];/////////////////////change//////////////////////
          sprintf (para, "%d", processes[0].run_time);
          //char *argv1[]={"./pro1.out",&send,NULL};
          char *argv[]={"./pro.out",para,NULL};
          fork_flag=true;
          pause_flag=false;
          id_run=processes[0].id;
          remaining=processes[0].run_time;
          if(processes[0].mtype==1)
          {
            cout<<"Process Fork"<<endl;
            timestep=getClk();
            cout<<endl<<"At time "<<timestep<<" process "<<processes[0].id<<" startted ";
            cout<<" arr "<<processes[0].arrival_time;
            cout<<" total "<<processes[0].run<<" remain "<<processes[0].run_time;
            cout<<" wait "<<processes[0].waited;
            cout<<processes[0].id<<endl;
            cout<<processes[0].run_time<<endl;
            pid2 = fork(); //Schduler forking
            if (pid2 == 0)
            {  
              Child_PID2 = execv (argv[0],argv);    
            }
            if(Child_PID2 == -1)
            {
               cout<<"Error in Forking Process"<<endl;
            }
            cout<<pid2<<endl;
          }
          else 
          {cout<<endl<<"At time "<<timestep<<" process "<<processes[0].id<<" resumed ";
          cout<<" arr "<<processes[0].arrival_time;
          cout<<" total "<<processes[0].run<<" remain "<<processes[0].run_time;
          cout<<" wait "<<processes[0].waited;
          cout<<processes[0].id<<endl;
          cout<<processes[0].run_time<<endl;
            int pidtorun=processes[0].mtype;
            cout<<pidtorun;
            processes[0].mtype=1;
            kill(pidtorun,SIGCONT);
            cout<<"Should Continue"<<endl;
          }
        }
      }  
      if(timestep+1==getClk()&& fork_flag==true)
      { //At​ ​ time​ ​ 1 ​ ​ process​ ​ 1 ​ ​ started​ ​ arr​ ​ 1 ​ ​ total​ ​ 6 ​ ​ ​ remain​ ​ 6 ​ ​ wait​ ​ 0
        processes[0].run_time--;
        timestep=getClk();
         ///////fff/////////////////
      for(int g=1;g<counter;g++)
      {
        processes[g].waited++;
      }
      //////////////////////////

        cout<<endl<<"At time "<<timestep<<" process "<<processes[0].id;
        cout<<" arr "<<processes[0].arrival_time;
        cout<<" total "<<processes[0].run<<" remain "<<processes[0].run_time;
        cout<<" wait "<<processes[0].waited;
        if(processes[0].run_time==0)
        {processes[0].ta=timestep-processes[0].arrival_time;
          processes[0].wta=float(processes[0].ta)/float(processes[0].run);
          cout<<" TA "<<processes[0].ta<<" WTA "<<processes[0].wta;
        }
      }
    }
    //upon termination release clock
    cout << "Processes are Done" << endl;
    fclose(fp);
    cout << "Terminating" << endl;
    destroyClk(true);  
  }


  // Round Robin Algorithm
  if (alog == 3)
  {
    int temp = Quant;
    bool Way = false;
    //FILE *fp = freopen("Test.out","w",stdout);
    cout << "\nBegin of Round Robin"<<endl;
    queue <process> current;
    while(1)
    {
      rec_val = msgrcv(downq, rec_pro,sizeof(process)-sizeof(long), 0,IPC_NOWAIT);  

      if(rec_val != -1)
      {
        cout<<"I received process at clock :- " << getClk() <<endl;
        processes.emplace_back(rec_pro->id, rec_pro->arrival_time, rec_pro->run_time, rec_pro->proirty);
        count++;
        current.push(*rec_pro);
        cout << "Recieved Process Details " << current.back().id << current.back().arrival_time << current.back().run_time << current.back().proirty << endl;
        current.back().mtype = 0;
        std::sort(processes.begin(),processes.end(),by_runRR());
      }

      if (processes.size() > 0)
      {
        cout << "Current Time is " << getClk() << endl; 
        cout << "Processor vector Size is " << processes.size() <<endl;

        if (fork_flag == false && current.front().mtype == 0)
        {
          cout << "Process will be forking first  " << current.front().id << endl;
          send= '0'+current.front().run_time;
          char *argv[]={"./pro.out",&send,NULL};
          fork_flag = true;
          cout<<"Fork Process is done for "<< current.front().id << endl;
          pid2 = fork(); //Schduler forking
          if (pid2 == 0)
          {  
             Child_PID2 = execv (argv[0],argv);    
          }
          if(Child_PID2 == -1)
          {
             cout<<"Error in Forking Process"<<endl;
          }
          cout<<pid2<<endl;
          current.front().mtype = pid2;
        }
        else if (fork_flag == false && current.front().mtype > 0)
        {
          cout << "Process "<< current.front().id << " Already Forked Before" << endl;
          fork_flag = true;
        }
        
        // check if process runtime greater than Quantium 
        // subtract quantium then stop
        
        if (current.front().run_time > Quant || Way == true) 
        {
          Way = true;
          cout << "Process " << current.front().id;
          cout << " has run time = " << current.front().run_time << " Seconds" <<endl;
          if (temp > 0) // --temp till it reach 0 and sleep for 1 sec
          {
            int pidtorun = current.front().mtype;
            kill(pidtorun,SIGCONT);
            cout <<"Process ID (won't stop)" << current.front().id ;
            cout <<" Should Continue for " << temp << " Seconds ";
            cout << "out of " << current.front().run_time <<endl;
            temp--;
            sleep(1);
          }
          else // stop the task then pop and push it in the queue
          {
            int pidtostop=current.front().mtype;
            kill(pidtostop,SIGSTOP);
            cout << "Process " << current.front().id << " Stopped" <<endl;
            current.front().run_time = current.front().run_time - Quant;
            process traverse = current.front();
            current.pop();
            current.push(traverse);
            temp = Quant;
            fork_flag = false;
            Way = false;
            //sleep(1);
          }
        }
        else //if process runtime <= 0
        {
          if (current.front().run_time > 0) // subtract till it be =0
          {
            int pidtorun=current.front().mtype;
            kill(pidtorun,SIGCONT);
            cout <<"Process ID " <<current.front().id ;
            cout<<" Should Continue for " << current.front().run_time << " Seconds"<<endl; 
            current.front().run_time --;
            sleep(1);
          }
          else //kill the task and pop it
          { 
            int pidtostop=current.front().mtype;
            kill(pidtostop,SIGCHLD);
            current.front().run_time = 0;
            current.pop();
            temp = Quant;
            fork_flag = false;
            sleep(1);
          }
        }
      }
    }
    cout << "Processes are Done" << endl;
    //fclose(fp);
    cout << "Terminating" << endl;
    destroyClk(true); 
 
  }
}