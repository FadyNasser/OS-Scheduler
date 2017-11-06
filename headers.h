
#include <stdio.h>      //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "vector"
#include "string"
#include <sstream>
using namespace std;
#include<iostream>
#define SHKEY 300


///==============================
//don't mess with this variable//
int* shmaddr;                  //
//===============================

// process
struct process
{
  long mtype;
  int id ;
  int arrival_time;
  int run_time;
  int proirty;
  int waited;
  int run;
  int ta;
  float wta;
  char status;
  process(int i, int s ,int n, int p):mtype(1),id(i),arrival_time(s),run_time(n),proirty(p),waited(0),run(n) {}
  process(){}
};

vector<process> read_process(istream & is)//reading the input file
{
  vector<process> result;

  string ids;
  int id,arr,run,p;
   
  while (is >> ids)
  {
    if(ids.length()>2)
    {      
      is >> ids>>ids>>ids;   //comment line
      
    }
    else
    {
      is >> arr >>run >>p;
      stringstream(ids) >> id;     
      result.emplace_back(id, arr, run, p); //put it in vector
    }
        
  }
  return result;
}


int getClk()
{
  int clk=*shmaddr;
  return clk;		
}


/* All process call this function at the begining to establish communication
between them and the clock module 

Again, Remember that the clock is only emulation
*/
void initClk()
{
  int shmid = shmget(SHKEY, 4, 0444);
  while((int)shmid == -1)
  {
    //Make sure that the Clock exists
    
    sleep(1);
    shmid = shmget(SHKEY, 4, 0444);
  }
  shmaddr = (int*) shmat(shmid, (void *)0, 0);
}


/* All process call this function at the end to release the  communication
resources between them and the clock module 

Again, Remember that the clock is only emulation 

input: terminateAll : is a flag to indicate whether that 
this is the end of simulation it terminates all the system and release resources
*/

void destroyClk(bool terminateAll)
{
  shmdt(shmaddr);
  if(terminateAll)
  {
    killpg(getpgrp(),SIGINT);
  }
}