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
#include <deque>
#include <cmath>
#include <iomanip>
using namespace std;



key_t downq;  //key for the down queue
deque <process> current;
bool fork_flag=false;
bool pause_flag=false;
int check=-1;
int alog;
float avg_wta=0;
std::vector<process> processes;
int timestep=0;
int avg_count=0;
int avg_w=0;
int last_time=0;
int utlization=0;
std::vector<float> st_wta;
void handler(int number) //sigchild handler
{ 
   last_time=getClk();
   fork_flag=false;
   pause_flag=false;      
 
   avg_count++;
  
  if(alog!=3)  // if hpf or sjrt
  {  
  
    utlization+=processes[0].run;   //adding all the run time
    cout<<"At time "<<timestep<<" process "<<processes[0].id<<" finished";
    cout<<" arr "<<processes[0].arrival_time;
    cout<<" total "<<processes[0].run<<" remain "<<processes[0].run_time;
    cout<<" wait "<<processes[0].waited;
    processes[0].ta=timestep-processes[0].arrival_time;
    cout << std::setprecision(2);                  //setting 2 decimal spaces
    processes[0].wta=float(processes[0].ta)/float(processes[0].run);
    cout<<" TA "<<processes[0].ta<<" WTA "<<processes[0].wta<<endl;  
    avg_wta+=processes[0].wta;
    st_wta.push_back(processes[0].wta);
    avg_w+=processes[0].waited;
    processes.erase(processes.begin());  //removing from the vector the first process
  }
  else //if round robin
  {
    current.front().run_time = 0;
    utlization+=current.front().run;
    cout << "At time " << getClk() << " process " << current.front().id << " finished";
    cout << " arr " << current.front().arrival_time;
    cout << " total " << current.front().run << " remain " << current.front().run_time;
    cout << " wait " << current.front().waited ;
    current.front().ta = getClk() - current.front().arrival_time;
    cout << std::setprecision(2);
    current.front().wta = float(current.front().ta) / float(current.front().run);
    cout << " TA " << current.front().ta << " WTA " << current.front().wta <<endl;
    avg_wta+=current.front().wta;
    st_wta.push_back(current.front().wta);
    avg_w+=current.front().waited;
    current.pop_front();     //removing the first process
  }
}

void Clear(int)  // sigint handler
{

  FILE *fp2 = freopen("scheduler.perf","w",stdout);
  float av_wta=0.0;
  float stand=0.0;
  float diff=0.0;
  utlization++;
  av_wta=avg_wta/float(avg_count);// writing the output of scheduler.perf
  cout << std::setprecision(2);
  cout<<"CPU​ ​utilization="<<int((float(utlization)/float(last_time))*100)<<"%"<<endl;
  cout<<"Avg WTA="<<av_wta<<endl;
  cout<<"Avg waiting="<<float(avg_w)/float(avg_count)<<endl;
  for(int u=0;u<st_wta.size();u++)  //calc the std
  {  
    diff=st_wta[u]-av_wta;
    diff=diff*diff;
    diff=diff/float(avg_count);
    stand+=diff;
  }
  stand=pow(stand,0.5);
  cout<<"Std​ ​WTA="<<stand<<endl;
  fclose(fp2);
	msgctl(downq,IPC_RMID, (struct msqid_ds *) 0);
	exit(0);
}



struct by_run  // sorting by run time
{
  bool operator()(process const &a,process const &b) const 
  {
     return a.run_time < b.run_time;
  }
};

struct by_priorty  // sorting by priorty
{
  bool operator()(process const &a,process const &b) const 
  {
     return ((a.proirty > b.proirty) && (a.arrival_time == b.arrival_time)) || (a.proirty > b.proirty) ;
  }
};

int main(int argc, char* argv[])  // scheduler main
{ 
  signal(SIGINT,Clear);
  signal (SIGCHLD, handler);
  struct sigaction act;  //masking the sigchild to work only with exiting
  
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
  
  downq = msgget(12645,IPC_CREAT | 0644); //getting the id of the queue
  
  int count=0;
  int id_run=-1;
  int remaining=-1;

  int msgq_arr_pr;
  int rec_val;
  char send;
  int Quant;
   
  
  timestep=getClk();
          
  alog=*argv[1]-'0'; 
  if(alog==3)
  {
    Quant=stoi(argv[2]);
    
  }

  struct process *rec_pro=new process(0,0,0,0);
  count=0;
  
  //Scheduler Algorithms
  // 1 => Non-Preemptive Highest Priority First      => Aya Mohamed
  // 2 => Shortest Remaining Time Next               => Mohamed Sameh
  // 3 => Round Robin                                => Fady Nasser

  if (alog == 1) // Highest Priority First
  {  
    int size=0;
    int Num_pro;
    int rem_time=0;
    bool forked=false;

  	cout << "\nBegin of Non-Preemptive Highest Priority First"<<endl;
    FILE *fp = freopen("scheduler.log","w",stdout); // to output results on test file
    cout << "#At​ time​​ x​ process​​ y​  state​​ arr​ w​ total​​ z​ remain​ ​ y wait​ k"<<endl;
     
    while (1) // Write the code inside while
    {
      Num_pro=-1;
      rec_val=0;

      while (rec_val != -1)   //keep reciving untill empty
      { 
        Num_pro++;            //to know how many processes are recieved in a timestep
        rec_val = msgrcv(downq, rec_pro,sizeof(process)-sizeof(long), 0,IPC_NOWAIT);   
       
        if(rec_val != -1)  //append the recieved processes to the vector
        {
          processes.emplace_back(rec_pro->id, rec_pro->arrival_time, rec_pro->run_time, rec_pro->proirty);
          size++;  
        }
      }

      if( size > 0 && processes[0].run_time > 0 && rem_time==0 ) // check whether there is a process to run or no
      {    

        std::sort(processes.begin(),processes.end(),by_priorty()); // sort by priorty
        cout<<"At time "<<timestep<<" process "<<processes[0].id<<" started";   //print arrival time and more detalied info 
        cout<<" arr "<<processes[0].arrival_time;
        cout<<" total "<<processes[0].run<<" remain "<<processes[0].run_time;
        processes[0].waited = timestep - processes[0].arrival_time;
        cout<<" wait "<<processes[0].waited<<endl;  
        send= '0' + processes[0].run_time; 
        char para2[16];
        sprintf (para2, "%d", processes[0].run_time);
        char *argv[]={"./pro.out",para2,NULL};

        rem_time= processes[0].run_time;
        pid2 = fork(); //Scheduler forking
        if (pid2 == 0)
        {  
          Child_PID2 = execv (argv[0],argv);         
        }
        if(Child_PID2 == -1)
        {
          cout<<"Error in Forking Process"<<endl;
        }
      }
            
      if(timestep+1==getClk()) // increase the timestep
      { 
        timestep=getClk();
        if(rem_time > 0 && processes[0].run_time !=0)  //check if a process is exist and run time is greater than 0
        {
          processes[0].run_time--;
          rem_time--;
        }

        if(rem_time == 0  && forked) //decrease the size when a process is about to die 
        {
          size--;
          forked=false;
        }
      }
    }
    //upon termination release clock
    fclose(fp); //Close the output file
    cout << "Processes are Done" << endl;
    cout << "Terminating" << endl;
    destroyClk(true);  
  }

  else if (alog == 2) 
  {
    cout << "\nBegin of Shortest Remaining Time Next"<<endl;
    FILE *fp = freopen("scheduler.log","w",stdout);      //opening file scheduler.log to write the output
    cout << "#At​ time​​ x​ process​​ y​  state​​ arr​ w​ total​​ z​ remain​ ​ y wait​ k"<<endl;

    while(1)
    {
      int counter=processes.size();
      rec_val = msgrcv(downq, rec_pro,sizeof(process)-sizeof(long), 0,IPC_NOWAIT);  
      if(rec_val != -1)  //if process arrived and recived 
      {
          
          processes.emplace_back(rec_pro->id, rec_pro->arrival_time, rec_pro->run_time, rec_pro->proirty);
          count++; //adding it to the vector
          std::sort(processes.begin(),processes.end(),by_run()); //sort the vector by remaining 
      }                                                          // runtime
      int t=1;
      if(fork_flag==true)
      {
        if(processes[0].id!=id_run )// if 1st process 
        {
          kill(pid2,SIGSTOP);   //stopping the running process as it is not the shortest
          for( t=1;t<processes.size();t++)
          {
            if (id_run ==processes[t].id)//get its new location on vector
            break;
          }
          processes[t].mtype=pid2;  //puting its pid on the mtype 
          pause_flag=true;          
          cout<<"At time "<<timestep<<" process "<<processes[t].id<<" stopped";
          cout<<" arr "<<processes[t].arrival_time;
          cout<<" total "<<processes[t].run<<" remain "<<processes[t].run_time;
          cout<<" wait "<<processes[t].waited<<endl;
        }
      }
      bool choose=false;
      if(fork_flag==false || pause_flag==true)
      {
        if(processes.size()>0)
        {
          if(pause_flag==true)
          choose=true;
          
          char para[16];  //sending the run_time to the process 
          sprintf (para, "%d", processes[0].run_time);
          char *argv[]={"./pro.out",para,NULL};
          fork_flag=true;
          pause_flag=false;
          id_run=processes[0].id;
          remaining=processes[0].run_time;
          if(processes[0].mtype==1)   // if it wasnt paused befor fork it
          {
            
            timestep=getClk();
            cout<<"At time "<<timestep<<" process "<<processes[0].id<<" started";
            cout<<" arr "<<processes[0].arrival_time;
            cout<<" total "<<processes[0].run<<" remain "<<processes[0].run_time;
            cout<<" wait "<<processes[0].waited<<endl;
            pid2 = fork(); //Scheduler forking a new process
            if (pid2 == 0)
            {  
              Child_PID2 = execv (argv[0],argv);    
            }
            if(Child_PID2 == -1)
            {
               cout<<"Error in Forking Process"<<endl;
            }
            
          }
          else 
          {  // process should resume
            cout<<"At time "<<timestep<<" process "<<processes[0].id<<" resumed";
            cout<<" arr "<<processes[0].arrival_time;
            cout<<" total "<<processes[0].run<<" remain "<<processes[0].run_time;
            cout<<" wait "<<processes[0].waited<<endl;
            int pidtorun=processes[0].mtype;
            processes[0].mtype=1;
            kill(pidtorun,SIGCONT); //let the process to resume
            
          }
        }
      }  
      if(timestep+1==getClk()&& fork_flag==true)//if new timestep
      { 
        
        processes[0].run_time--;
        timestep=getClk();
        
        for(int g=1;g<counter;g++)
        {
          processes[g].waited++;   //incrementing the waiting time for all processes
                                  // not running
        }
      }
      
    }
    //upon termination release clock
    fclose(fp);
    cout << "Processes are Done" << endl;
    cout << "Terminating" << endl;
    destroyClk(true);  
  }


  // Round Robin Algorithm
  if (alog == 3)
  {
    //deque <process> current;
    int temp = Quant; //story quant in temp variable to decrement it
    bool Way = false; //(If the process is more than quantium number or not)
    bool end = false; //(to break when queue is empty)
    int timesteps = getClk();
    cout << "\nBegin of Round Robin"<<endl;
    FILE *fp = freopen("scheduler.log","w",stdout);
    cout << "#At​ time​​ x​ process​​ y​  state​​ arr​ w​ total​​ z​ remain​ ​ y wait​ k" <<endl;
    while(1)
    {
      int counter = current.size();
      rec_val = msgrcv(downq, rec_pro,sizeof(process)-sizeof(long), 0,IPC_NOWAIT);  

      if(rec_val != -1)
      {
        
        current.emplace_back(rec_pro->id, rec_pro->arrival_time, rec_pro->run_time, rec_pro->proirty);
        count++;
        current.back().mtype = 0;
       
      }

      if (current.size() > 0)
      {
        int timestep = getClk();
        
        if (fork_flag == false && current.front().mtype == 0) 
        {
          char para3[16];
          sprintf (para3, "%d", current.front().run_time);
          char *argv[]={"./pro.out",para3,NULL};
          fork_flag = true;
          cout << "At time " << timestep << " process " << current.front().id << " started";
          cout << " arr " << current.front().arrival_time;
          cout << " total " << current.front().run << " remain " << current.front().run_time;
          cout << " wait " << current.front().waited << endl;
          
          pid2 = fork(); //Scheduler forking
          if (pid2 == 0)
          {  
             Child_PID2 = execv (argv[0],argv);    
          }
          if(Child_PID2 == -1)
          {
             cout<<"Error in Forking Process"<< endl;
          }
          
          current.front().mtype = pid2;
        }
        else if (fork_flag == false && current.front().mtype > 0)
        {
          //cout << "Process "<< current.front().id << " Already Forked Before" << endl;
          cout << "At time " << timestep << " process " << current.front().id << " resumed";
          cout << " arr " << current.front().arrival_time;
          cout << " total " << current.front().run << " remain " << current.front().run_time;
          cout << " wait " << current.front().waited << endl;
          fork_flag = true;
        }
        
        // check if process runtime greater than Quantium 
        // subtract quantium then stop
        
        if (current.front().run_time > Quant || Way == true) 
        {
          Way = true; //to stay in this part of code not shift to other code
          if (temp > 0) // --temp till it reach 0 and sleep for 1 sec
          {
            int pidtorun = current.front().mtype;
            kill(pidtorun,SIGCONT);
            temp--;
            sleep(1);
          }
          else // stop the task then pop and push it in the queue
          {
            int pidtostop=current.front().mtype;
            kill(pidtostop,SIGSTOP);
            current.front().run_time = current.front().run_time - Quant;
            cout << "At time " << timestep << " process " << current.front().id << " stopped";
            cout << " arr " << current.front().arrival_time;
            cout << " total " << current.front().run << " remain " << current.front().run_time;
            cout << " wait " << current.front().waited << endl;
            process traverse = current.front();
            current.pop_front();
            current.push_back(traverse);
            temp = Quant;
            fork_flag = false;
            Way = false;
            //sleep(1);
          }
        }
        else //if process runtime <= 0
        {
          if (current.front().run_time > 0) // subtract till it be = 0
          {
            int pidtorun=current.front().mtype;
            kill(pidtorun,SIGCONT);
            current.front().run_time --;
            sleep(1);
          }
          else //kill the task and pop it
          { 
            int pidtostop=current.front().mtype;
            kill(pidtostop,SIGCHLD);
            temp = Quant;
            fork_flag = false;
            sleep(1);
          }
        }
      }
      
      if(timesteps != getClk()) //increment waited time for all processes except first as it is the running one
      {
        timesteps = getClk();
        for(int i = 1 ; i < current.size() ; i++)
        {
          current[i].waited++; 
        }
      }
    }
    fclose(fp);
    cout << "Processes are Done" << endl;
    cout << "Terminating" << endl;
    destroyClk(true);
  }
}