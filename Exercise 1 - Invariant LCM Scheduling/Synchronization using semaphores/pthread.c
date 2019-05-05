/************************************************************************************/
/*																					*/
/*File name - Semaphore use for synchronization										*/
/*Authors: Siddhant Jajoo and Sarthak Jain											*/
/*Date: February 2, 2019															*/
/************************************************************************************/
/*																					*/
/*For the purpose of this code, we have referred sections of it from the Independent*/
/*Study on RM Scheduling feasibility tests conducted by Nisheeth Bhat, link of which*/
/*is provided here:http://ecee.colorado.edu/~ecen5623/ecen/labs/Linux/IS/Report.pdf */
/*																					*/
/*We have also referred sections from the file written by Prof. Sam Siewert, titled */
/*nanosleep and POSIX 1003.1b RT clock demonstration								*/


#define _GNU_SOURCE
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/param.h>
#include <syslog.h>

#define NUM_THREADS (2)
#define NUM_CPUS (1)

#define NSEC_PER_SEC (1000000000)
#define NSEC_PER_MSEC (1000000)
#define NSEC_PER_MICROSEC (1000)
#define DELAY_TICKS (1)
#define ERROR (-1)
#define OK (0)


unsigned int idx = 0, jdx = 1;
unsigned int seqIterations = 47;
unsigned int reqIterations[2] = {1231940,2463880};
volatile unsigned int fib = 0, fib0 = 0, fib1 = 1;

#define FIB_TEST(seqCnt, iterCnt)      \
   for(idx=0; idx < iterCnt; idx++)    \
   {                                   \
      fib = fib0 + fib1;               \
      while(jdx < seqCnt)              \
      {                                \
         fib0 = fib1;                  \
         fib1 = fib;                   \
         fib = fib0 + fib1;            \
         jdx++;                        \
      }                                \
   }                                   \

typedef struct
{
    int threadIdx;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
//
pthread_t thread_fib10, thread_fib20;
pthread_attr_t att_fib10,att_fib20;
pthread_attr_t main_att;
threadParams_t threadParams_fib10, threadParams_fib20;

struct sched_param rt_param_fib10, rt_param_fib20;
struct sched_param main_param;
struct timespec start_time = {0, 0};
struct timespec stop_time = {0, 0};
struct timespec diff_final_time = {0, 0};

int rt_max_prio, rt_min_prio;
pid_t mainpid;
sem_t sem1,sem2;
int ctrl1 = 1,ctrl2 = 1;

int delta_t(struct timespec *stop, struct timespec *start, struct timespec *delta_t)
{
  int dt_sec=stop->tv_sec - start->tv_sec;
  int dt_nsec=stop->tv_nsec - start->tv_nsec;

  if(dt_sec >= 0)
  {
    if(dt_nsec >= 0)
    {
      delta_t->tv_sec=dt_sec;
      delta_t->tv_nsec=dt_nsec;
    }
    else
    {
      delta_t->tv_sec=dt_sec-1;
      delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
    }
  }
  else
  {
    if(dt_nsec >= 0)
    {
      delta_t->tv_sec=dt_sec;
      delta_t->tv_nsec=dt_nsec;
    }
    else
    {
      delta_t->tv_sec=dt_sec-1;
      delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
    }
  }

  return(1);
}


void *delay_fib10(void *threadp)
{	
    int i;
    pthread_t thread;
    cpu_set_t cpuset;
    struct timespec finish_time = {0, 0};
    struct timespec thread_dt = {0, 0};
    threadParams_t *threadParams = (threadParams_t *)threadp;


    while(ctrl1)
    {
    sem_wait(&sem1);
    thread=pthread_self();
    CPU_ZERO(&cpuset);

    FIB_TEST(seqIterations, reqIterations[0]);
    // END COMPUTE SECTION
 
    pthread_getaffinity_np(thread, sizeof(cpu_set_t), &cpuset);

    
    clock_gettime(CLOCK_REALTIME, &finish_time);
    delta_t(&finish_time, &start_time, &thread_dt);
    printf("\nThread idx=%d priority = %d timestamp: %ld msec (%ld microsec)", threadParams->threadIdx, rt_param_fib10.sched_priority, (thread_dt.tv_nsec / NSEC_PER_MSEC), (thread_dt.tv_nsec / NSEC_PER_MICROSEC));
    for(i=0; i<NUM_CPUS; i++)
        if(CPU_ISSET(i, &cpuset))  printf("utilizing CPU-%d\n ", i);

    }
    
}


void *delay_fib20(void *threadp)
{
    int i;
    pthread_t thread;
    cpu_set_t cpuset;
    
    struct timespec finish_time = {0, 0};
    struct timespec thread_dt = {0, 0};
    threadParams_t *threadParams = (threadParams_t *)threadp;


    while(ctrl2)
    {
    sem_wait(&sem2);
    thread=pthread_self();
    CPU_ZERO(&cpuset);

    FIB_TEST(seqIterations, reqIterations[1]);
    // END COMPUTE SECTION
 
    pthread_getaffinity_np(thread, sizeof(cpu_set_t), &cpuset);

    clock_gettime(CLOCK_REALTIME, &finish_time);
    delta_t(&finish_time, &start_time, &thread_dt);
    printf("\nThread idx=%d priority = %d timestamp: %ld msec (%ld microsec)", threadParams->threadIdx, rt_param_fib20.sched_priority, (thread_dt.tv_nsec / NSEC_PER_MSEC), (thread_dt.tv_nsec /        NSEC_PER_MICROSEC));
    for(i=0; i<NUM_CPUS; i++)
        if(CPU_ISSET(i, &cpuset))  printf("utilizing CPU-%d\n ", i);

    //syslog(LOG_KERN |LOG_CRIT,"\nThread idx=%d priority = %d timestamp: %ld msec (%ld microsec)", threadParams->threadIdx, rt_param_fib20.sched_priority, (thread_dt.tv_nsec / NSEC_PER_MSEC),   ////(thread_dt.tv_nsec / NSEC_PER_MICROSEC));

    }
    
}


void print_scheduler(void)
{
   int schedType;

   schedType = sched_getscheduler(getpid());

   switch(schedType)
   {
     case SCHED_FIFO:
           printf("Pthread Policy is SCHED_FIFO\n");
           break;
     case SCHED_OTHER:
           printf("Pthread Policy is SCHED_OTHER\n");
       break;
     case SCHED_RR:
           printf("Pthread Policy is SCHED_OTHER\n");
           break;
     default:
       printf("Pthread Policy is UNKNOWN\n");
   }

}



int main (int argc, char *argv[])
{
   int rc;
   int i, scope;
   useconds_t T10 = 10000, T20 = 20000;
   cpu_set_t cpuset;

   sem_init(&sem1,0,1);
   sem_init(&sem2,0,1);
   
   CPU_ZERO(&cpuset);
   for(i=0; i < NUM_CPUS; i++)
       CPU_SET(i, &cpuset);

   mainpid=getpid();

   rt_max_prio = sched_get_priority_max(SCHED_FIFO);
   rt_min_prio = sched_get_priority_min(SCHED_FIFO);

   print_scheduler();  //Prints SCHED_OTHER

   rc=sched_getparam(mainpid, &main_param);
   main_param.sched_priority=rt_max_prio;	//gives highest priority 99 to main process
  
   rc=sched_setscheduler(getpid(), SCHED_FIFO, &main_param);
   if(rc < 0) perror("main_param");
   print_scheduler();		//Prints SCHED_FIFO as schedule policy

   pthread_attr_getscope(&main_att, &scope);

   if(scope == PTHREAD_SCOPE_SYSTEM)
     printf("PTHREAD SCOPE SYSTEM\n");
   else if (scope == PTHREAD_SCOPE_PROCESS)
     printf("PTHREAD SCOPE PROCESS\n");
   else
     printf("PTHREAD SCOPE UNKNOWN\n");

   printf("rt_max_prio=%d\n", rt_max_prio);
   printf("rt_min_prio=%d\n", rt_min_prio);


	//thread_fib10 initialization
       rc=pthread_attr_init(&att_fib10);
       rc=pthread_attr_setinheritsched(&att_fib10, PTHREAD_EXPLICIT_SCHED);
       rc=pthread_attr_setschedpolicy(&att_fib10, SCHED_FIFO);
       rc=pthread_attr_setaffinity_np(&att_fib10, sizeof(cpu_set_t), &cpuset);
       rt_param_fib10.sched_priority=rt_max_prio-1;
       pthread_attr_setschedparam(&att_fib10, &rt_param_fib10);
       threadParams_fib10.threadIdx=0;
       

	//thread_fib20 initialization
       rc=pthread_attr_init(&att_fib20);
       rc=pthread_attr_setinheritsched(&att_fib20, PTHREAD_EXPLICIT_SCHED);
       rc=pthread_attr_setschedpolicy(&att_fib20, SCHED_FIFO);
       rc=pthread_attr_setaffinity_np(&att_fib20, sizeof(cpu_set_t), &cpuset);
       rt_param_fib20.sched_priority=rt_max_prio-2;
       pthread_attr_setschedparam(&att_fib20, &rt_param_fib20);
       threadParams_fib20.threadIdx=1;


       clock_gettime(CLOCK_REALTIME, &start_time);

       pthread_create(&thread_fib10,   // pointer to thread descriptor
                      &att_fib10,     // use default attributes
                      delay_fib10, // thread function entry point
                      (void *)&(threadParams_fib10) // parameters to pass in
                     );


       pthread_create(&thread_fib20,   // pointer to thread descriptor
                      &att_fib20,     // use default attributes
                      delay_fib20, // thread function entry point
                      (void *)&(threadParams_fib20) // parameters to pass in
                     );


	//Routine After C.I
	usleep(T20);
	sem_post(&sem1);
	usleep(T20);
	sem_post(&sem1);
	usleep(T10);
	ctrl2 = 0;
	sem_post(&sem2);
	usleep(T10);
	sem_post(&sem1);
	usleep(T20);
	ctrl1 = 0;
	sem_post(&sem1);
	usleep(T20);


       clock_gettime(CLOCK_REALTIME, &stop_time);
       delta_t(&stop_time, &start_time, &diff_final_time);

	printf("\nThe test took %ld msec (%ld microsec)\n", (diff_final_time.tv_nsec / NSEC_PER_MSEC), (diff_final_time.tv_nsec / NSEC_PER_MICROSEC));

       pthread_join(thread_fib10, NULL);
       pthread_join(thread_fib20, NULL);

       pthread_attr_destroy(&att_fib10);
       pthread_attr_destroy(&att_fib20);

	sem_destroy(&sem1);
	sem_destroy(&sem2);

   printf("\nTEST COMPLETE\n");
}
