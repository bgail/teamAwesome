/* ------------------------------------------------------------------------
   Team Check Your Privilege
   Members:
   Joshua Redpath
   Abigail Arias
   Skeleton file for Phase 1. These routines are very incomplete and are
   intended to give you a starting point. Feel free to use thiTeams or not.


   ------------------------------------------------------------------------ */

#include <stddef.h>
#include <stdlib.h>
#include "usloss.h"
#include "phase1.h"
#include <string.h>

#define DEFAULT -99;
/* -------------------------- Globals ------------------------------------- */

typedef struct PCB {
    USLOSS_Context      context;
    int                 (*startFunc)(void *);   /* Starting function */
    void                 *startArg;             /* Arg to starting function */
    int PID;
    int cpuTime;
    int isOrphan;
    int state;//0=running, 1=ready,2=killed,3=quit,4=waiting
    int status;
    int numChildren;
    int parent;
    int priority;
    char* name;
    void* stack;
    int notFreed;
    struct PCB* nextPCB;
    struct PCB* prevPCB;    
} PCB;

typedef struct 
{
  int value;
  struct PCB *queue;
}Semaphore;

int dispatcherTimeTracker=-1;
/* the process table */
PCB procTable[P1_MAXPROC];

PCB readyHead;
PCB blockedHead;

/* current process ID */
int currPid = -1;

/* number of processes */

int numProcs = 0;

static int sentinel(void *arg);
static void launch(void);
static void removeProc(int PID);
static void Check_Your_Privilege();
static void free_Procs();
//static int P1_ReadTime(void);

P1_Semaphore P1_SemCreate(unsigned int value);
int P1_SemFree(P1_Semaphore sem);
int P1_P(P1_Semaphore sem);
int P1_V(P1_Semaphore sem);


/* -------------------------- Functions ----------------------------------- */
/* ------------------------------------------------------------------------
   Name - dispatcher
   Purpose - runs the highest priority runnable process
   Parameters - none
   Returns - nothing
   Side Effects - runs a process
   ----------------------------------------------------------------------- */

void free_Procs(){
  
  for(int i = 0; i <P1_MAXPROC; i++){
    if(procTable[i].state == 3 && procTable[i].notFreed) {
      free(procTable[i].stack); // free
      //USLOSS_Console("Freed %s\n",procTable[i].name);
      free(procTable[i].name);
      procTable[i].notFreed = 0;
      if(procTable[i].isOrphan){
        procTable[i].priority=-1;
      }
    }
  }
}


void dispatcher()
{
  Check_Your_Privilege();
  // USLOSS_Console("dispatcher Called\n");
  /*Adjust Runttime for current process*/
  int timeRun;
  if (dispatcherTimeTracker==-1) {
    timeRun=0;
  }else{
      timeRun=USLOSS_Clock()-dispatcherTimeTracker;
  }
  procTable[currPid].cpuTime+=timeRun;
  /*
   * Run the highest priority runnable process. There is guaranteed to be one
   * because the sentinel is always runnable.
   */
  // USLOSS_Console("In dispatcher PID -- before:  %d\n", currPid);
  int oldpid = currPid;
  PCB* readyListPos=&readyHead;
  while(readyListPos->nextPCB){//List is in order of priority
    /*Breaks and runs this process*/
    if(readyListPos->nextPCB->state==1||readyListPos->nextPCB->state==2){
        break;
    }
    // USLOSS_Console("Moving Foward in dispatcher\n");
    readyListPos=readyListPos->nextPCB;
  }

  dispatcherTimeTracker=USLOSS_Clock();
  currPid=readyListPos->nextPCB->PID;
  readyListPos->nextPCB->state=0;//set state to running
  /*Set Proc state to ready unless it has quit or been killed*/
  if(procTable[oldpid].state!=3||procTable[oldpid].state==2){
    procTable[oldpid].state=1;
  }
  currPid = readyListPos->nextPCB->PID;
 // USLOSS_Console("In dispatcher PID -- after:  %d\n", currPid); 
  USLOSS_ContextSwitch(&(procTable[oldpid]).context, &(readyListPos->nextPCB->context));

}

/* ------------------------------------------------------------------------
   Name - startup
   Purpose - Initializes semaphores, process lists and interrupt vector.
             Start up sentinel process and the P2_Startup process.
   Parameters - none, called by USLOSS
   Returns - nothing
   Side Effects - lots, starts the whole thing
   ----------------------------------------------------------------------- */
void startup()
{
  Check_Your_Privilege();
  int i;
  /* initialize the process table here */
  for(i = 0; i < P1_MAXPROC; i++){
      PCB dummy;
      procTable[i]=dummy;
      //USLOSS_Context DummyCon;
      procTable[i].priority = -1;
      //procTable[i].context=DummyCon;
  } 

  /* Initialize the Ready list, Blocked list, etc. here */
  readyHead.prevPCB=NULL;
  readyHead.nextPCB=NULL;
  blockedHead.prevPCB=NULL;
  blockedHead.nextPCB=NULL;
  /* Initialize the interrupt vector here */

  /* Initialize the semaphores here */

  /* startup a sentinel process */
  /* HINT: you don't want any forked processes to run until startup is finished.
   * You'll need to do something in your dispatcher to prevent that from happening.
   * Otherwise your sentinel will start running right now and it will call P1_Halt. 
   */
  P1_Fork("sentinel", sentinel, NULL, USLOSS_MIN_STACK, 6);

  /* start the P2_Startup process */
  P1_Fork("P2_Startup", P2_Startup, NULL, 4 * USLOSS_MIN_STACK, 1);

  dispatcher();

  /* Should never get here (sentinel will call USLOSS_Halt) */

  return;
} /* End of startup */

/* ------------------------------------------------------------------------
   Name - finish
   Purpose - Required by USLOSS
   Parameters - none
   Returns - nothing
   Side Effects - none
   ----------------------------------------------------------------------- */
void finish()
{
  Check_Your_Privilege();
  // free_Procs();
  USLOSS_Console("Goodbye.\n");
} /* End of finish */


int P1_GetPID(){
  Check_Your_Privilege();
  return currPid;
}

int P1_Join(int *status){
   Check_Your_Privilege();
   if(procTable[currPid].numChildren == 0){
    return -1;
   }

   // get the PID of the child that quit
   for(int i = 0; i < P1_MAXPROC; i++){

   }
    return 0;
}

int P1_GetState(int PID){
  Check_Your_Privilege();
  if(PID<0||PID>P1_MAXPROC-1){
    return -1;
  }else if(PID==currPid){
    return 0;
  }
 // USLOSS_Console("In P1_GetState PID -- after:  %d,State=%d\n", PID,procTable[PID].state);
  return procTable[PID].state;//1=ready,2=killed,3=quit,4=waiting
}


void P1_DumpProcesses(){// Do CPU Time Part
  Check_Your_Privilege();
  int i;
    for(i=0;i<P1_MAXPROC;i++){
        if(procTable[i].priority==-1){
          continue;
        }
        int state=procTable[i].state;
        char* statePhrase;
        switch(state){
          case 1:
            statePhrase="Ready";
            break;
          case 2:
            statePhrase="Killed";
            break;
          case 3:
            statePhrase="Quit";
            break;
          case 4:
            statePhrase="Waiting";
            break;
        }
        
        USLOSS_Console("Name:%s\t PID:%-5d\tParent:%d\tPriority:%d\tState:%s\tKids%d\tCPUTime:%d\n",
                procTable[i].name,i,procTable[i].parent,procTable[i].priority,
                statePhrase,procTable[i].numChildren,procTable[i].cpuTime); 
    }
}

/*Checks Whether or not thecurrent process is in Kernel Mode*/
void Check_Your_Privilege(){
  if((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE)==0){
    USLOSS_Console("Error: Access Denied to User Mode");
    USLOSS_Halt(1);
  }
}


P1_Semaphore P1_SemCreate(unsigned int value){

  P1_Semaphore semPointer; 
  Semaphore* semi= malloc(sizeof(Semaphore));
  semi->value = value;
  semPointer = &semi;
  return semPointer;

}

int P1_SemFree(P1_Semaphore sem){
  Check_Your_Privilege();
  //if sem is invalid return -1, sem is invalid if it is not created using SemCreate method
  free(sem);
  return 0;
}

int P1_P(P1_Semaphore sem){
  Check_Your_Privilege();
  Semaphore* semP=(Semaphore*)sem;
  while(1){
    // interrupt disable HERE;

    if(semP->value > 0){
      semP->value--;
      break;
    }
  }
  //interrupt enable
  return 0;
}

int P1_V(P1_Semaphore sem){
  Check_Your_Privilege();
  // interrupt disable HERE!
  Semaphore* semP=(Semaphore*)sem;
  semP->value++;
  if(semP->queue != NULL){
    //addToReady
    dispatcher();
  }
  // interrupt enable HERE!
  return 0;
}





/* ------------------------------------------------------------------------
   Name - P1_Fork
   Purpose - Gets a new process from the process table and initializes
             information of the process.  Updates information in the
             parent process to reflect this child process creation.
   Parameters - the process procedure address, the size of the stack and
                the priority to be assigned to the child process.
   Returns - the process id of the created child or an error code.
   Side Effects - ReadyList is changed, procTable is changed, Current
                  process information changed
   ------------------------------------------------------------------------ */
int P1_Fork(char *name, int (*f)(void *), void *arg, int stacksize, int priority)
{
    /*Check current Mode. If not Kernel Mode return error*/
    Check_Your_Privilege();

    //free the available spots
    free_Procs();

    /*Check Priority and Stack Size*/
    if(priority<1||priority>6){//is priority # valid
      return -3;
    }
    if(stacksize<USLOSS_MIN_STACK){//is stacksize valid
      return -2;
    }

    //find PID
    int newPid = 0;
    while(procTable[newPid].priority!=-1){
      newPid++;
      if(newPid>=P1_MAXPROC){
        return -1;
      }
    }

    /* stack = allocated stack here */
    // void* newStack=malloc(stacksize*sizeof(char));
    procTable[newPid].stack=malloc(stacksize*sizeof(char));
    procTable[newPid].notFreed=1;

    /*set PCB fields*/
    procTable[newPid].PID=newPid;
    procTable[newPid].cpuTime=0;
    procTable[newPid].state=1;//0=running 1=ready,2=killed,3=quit,4=waiting

    procTable[newPid].status=DEFAULT;

    if(currPid==-1){
      procTable[newPid].parent=-1;
      procTable[newPid].isOrphan=1;
    }
    procTable[currPid].numChildren++;//increment parents numChildren
    procTable[newPid].numChildren=0;
    procTable[newPid].priority=priority;
    procTable[newPid].name=strdup(name);
    procTable[newPid].startFunc = f;
    procTable[newPid].startArg = arg;
    procTable[newPid].isOrphan= 0;
    /*PCB Fields are set*/


    /*add to ready list*/
    PCB* pos = &readyHead;
    while(pos->nextPCB!=NULL&&pos->nextPCB->priority<priority){
      pos=pos->nextPCB;
    }
    if(pos->nextPCB==NULL){
      pos->nextPCB=&(procTable[newPid]);
      procTable[newPid].prevPCB=pos->nextPCB;
      procTable[newPid].nextPCB=NULL;
    }else{
      procTable[newPid].prevPCB=pos;
      procTable[newPid].nextPCB=pos->nextPCB;
      pos->nextPCB->prevPCB=&(procTable[newPid]);
      pos->nextPCB=&(procTable[newPid]);
    }
    

    /*increment numProcs*/
    numProcs++;
    
    /*initialize context*/
    USLOSS_ContextInit(&(procTable[newPid].context), USLOSS_PsrGet(), procTable[newPid].stack, 
        stacksize, launch);


    if(currPid != -1&&priority<procTable[currPid].priority){
      dispatcher();
    }
    //USLOSS_Console("In Fork PID -- after:  %d\n", currPid);
    return newPid;
} /* End of fork */

/* ------------------------------------------------------------------------
   Name - launch
   Purpose - Dummy function to enable interrupts and launch a given process
             upon startup.
   Parameters - none
   Returns - nothing
   Side Effects - enable interrupts
   ------------------------------------------------------------------------ */
void launch(void){
  Check_Your_Privilege();
  int  rc;
  USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);
  rc = procTable[currPid].startFunc(procTable[currPid].startArg);
  // USLOSS_Console("Laung Ending for %s\n",procTable[currPid].name);
  /* quit if we ever come back */
  P1_Quit(rc);
} /* End of launch */

/* ------------------------------------------------------------------------
   Name - P1_Quit
   Purpose - Causes the process to quit and wait for its parent to call P1_Join.
   Parameters - quit status
   Returns - nothing
   Side Effects - the currently running process quits
   ------------------------------------------------------------------------ */
void P1_Quit(int status) {
  Check_Your_Privilege();
//  USLOSS_Console("In quit PID -- before:  %d\n", currPid);
  
  int i;
  for (i = 0; i < P1_MAXPROC; i++) {
      if(procTable[i].PID==currPid){
          procTable[i].isOrphan = 1; // orphanize the children
      }
  }

 // USLOSS_Console("Process: %s Quitting\n",procTable[currPid].name);
  
  procTable[currPid].state = 3;
  procTable[currPid].status = status;
  numProcs--;
 // USLOSS_Console("Process state: %d\n", procTable[currPid].state);
 // USLOSS_Console("PID :  %d\n", currPid);
  


  /*Remove from Ready List*/
  if(procTable[currPid].nextPCB!=NULL){
    procTable[currPid].nextPCB->prevPCB=procTable[currPid].prevPCB;
  }
  procTable[currPid].prevPCB->nextPCB=procTable[currPid].nextPCB;

  /*Add to blocked List*/
  PCB* pos=&blockedHead;
  while(pos->nextPCB&&pos->nextPCB->priority<procTable[currPid].priority){
    // USLOSS_Console("Looping on %s\n",pos->nextPCB->name);
    pos=pos->nextPCB;
  }
  procTable[currPid].nextPCB=NULL;
  pos->nextPCB=&(procTable[currPid]);


  
  /*remove if orphan*/
  if(procTable[currPid].isOrphan){//||procTable[currPid].parent==-1){
      removeProc(currPid);
  }
  
  //USLOSS_Console("In quit PID -- after:  %d\n", currPid);
  
  // USLOSS_Console("Number of processes: %d\n", numProcs);
  dispatcher();
  
  
}
/*Removes a pcb from procTable*/
void removeProc(int PID){   
    procTable[PID].priority = -1;
}

int P1_Kill(int PID,int status){
  Check_Your_Privilege();
  if(PID==currPid){
    return -2;
  }if(PID < 0 ||PID >= P1_MAXPROC){
    return -1;
  }
  procTable[PID].state=2;
  return 0;
}

/* ------------------------------------------------------------------------
   Name - sentinel
   Purpose - The purpose of the sentinel routine is two-fold.  One
             responsibility is to keep the system going when all other
             processes are blocked.  The other is to detect and report
             simple deadlock states.
   Parameters - none
   Returns - nothing
   Side Effects -  if system is in deadlock, print appropriate error
                   and halt.
   ----------------------------------------------------------------------- */
int sentinel (void *notused)
{
 // USLOSS_Console("in sentinel\n");
  Check_Your_Privilege();
  /*No Interupts within Part 1 so commented out*/
  while (numProcs > 1)
  {
    //Check for deadlock here 
    USLOSS_WaitInt();
  }
  USLOSS_Halt(0);
  /* Never gets here. */
  return 0;
} /* End of sentinel */