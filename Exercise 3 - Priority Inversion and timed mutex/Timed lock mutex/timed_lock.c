/*Thread Safety and timeouts using mutexes								*/
/*Author: Sarthak Jain											*/
/*Date: 3/8/19												*/
/*This code was written for the course of Real-Time Embedded Systems, under the guidance of Prof. Tim   */
/*Scherr. 												*/
/*The code demonstrates thread safety, of one thread updating a structure, another reading from it and	*/
/*and a third thread reading the structure in such a fashion that it always tried to access a locked 	*/
/*mutex. The concept of a watchdog timer is demonstrated using timedlock  mutex			 	*/
/*The three are synchronized using a mutex								*/

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>

typedef struct
{
	struct timespec TimeSpec;
	double accX, accY, accZ, roll, pitch, yaw;
} threadParams;

struct timespec Func_Time, timeout;

pthread_t thread_1, thread_2, thread_3;
pthread_attr_t att_1, att_2, att_3;
pthread_mutex_t lock;

double generate_rand()
{
	double x = (float)rand();
	x += x/RAND_MAX;
	return x;
}

void* updates(void* threadp)
{
	while(1)
	{	
		threadParams* threadParameter = (threadParams*) threadp;
		pthread_mutex_lock(&lock);
		sleep(12);
		threadParameter->accX = generate_rand();
		threadParameter->accY = generate_rand();
		threadParameter->accZ = generate_rand();
		threadParameter->roll = generate_rand();
		threadParameter->pitch = generate_rand();
		threadParameter->yaw = generate_rand();
		clock_gettime(CLOCK_REALTIME, &(threadParameter->TimeSpec));
//		pthread_mutex_unlock(&lock);
		printf("\nUpdating Thread\nUpdated Timestamp %lu secs, %lu nsecs", (threadParameter->TimeSpec).tv_sec, (threadParameter->TimeSpec).tv_nsec);
		printf("\nUpdated acceleration is %f in x, %f in y, %f in z", threadParameter->accX, threadParameter->accY, threadParameter->accZ);
		printf("\nUpdated attitude is roll of %f, pitch of %f, yaw of %f\n", threadParameter->roll, threadParameter->pitch, threadParameter->yaw);
		printf("\n------------------------------------------------------------------------------------------------------------------------\n");
		pthread_mutex_unlock(&lock);
	}
}

void* reads(void* threadp)
{
	while(1)
	{
		sleep(13);
		threadParams* threadParameter = (threadParams*) threadp;
		clock_gettime(CLOCK_REALTIME, &Func_Time);
		printf("\nReading Thread Time instant is %lu secs, %lu nsecs", Func_Time.tv_sec, Func_Time.tv_nsec);
		printf("\nTimestamp obtained from above is %lu secs, %lu nsecs", (threadParameter->TimeSpec).tv_sec, (threadParameter->TimeSpec).tv_nsec);
		printf("\nAcceleration obtained is %f in x, %f in y, %f in z", threadParameter->accX, threadParameter->accY, threadParameter->accZ);
		printf("\nAttitude obtained is roll of %f, pitch of %f, yaw of %f\n", threadParameter->roll, threadParameter->pitch, threadParameter->yaw);
		printf("\n------------------------------------------------------------------------------------------------------------------------\n");
	}	
}

void* locks(void* threadp)
{
	int check;
	sleep(1);
	while(1)
	{
		clock_gettime(CLOCK_REALTIME, &timeout);
		timeout.tv_sec += 10;
		check = pthread_mutex_timedlock(&lock, &timeout);
		if(check == 0)
		{
			pthread_mutex_unlock(&lock);
			printf("\nLocked thread can read data!!!\n");
		}
		else if(check == ETIMEDOUT)
		{
			clock_gettime(CLOCK_REALTIME, &Func_Time);
			printf("\nNo new data available at time %lu secs, %lu nsecs\n", Func_Time.tv_sec, Func_Time.tv_nsec);
			printf("\n------------------------------------------------------------------------------------------------------------------------\n");
		}
	}
}

int main (int argc, char *argv[])
{
	threadParams threadParameter;
	int check;
	srand(time(NULL));

	if(pthread_mutex_init(&lock, NULL) != 0)
	{
		printf("\nMutex initialization failed\n");
	}

	check=pthread_attr_init(&att_1);       	//thread_1 initialization
	check=pthread_attr_init(&att_2);	//thread_2 initialization
	check=pthread_attr_init(&att_3);       	//thread_1 initialization

	pthread_create(&thread_1,   // pointer to thread descriptor
			&att_1,     // use default attributes
			updates, // thread function entry point
			(void *)&(threadParameter) // parameters to pass in
			);

	pthread_create(&thread_2,   // pointer to thread descriptor
			&att_2,     // use default attributes
			reads, // thread function entry point
			(void *)&(threadParameter) // parameters to pass in
			);

	pthread_create(&thread_3,   // pointer to thread descriptor
			&att_3,     // use default attributes
			locks, // thread function entry point
			(void *)&(threadParameter) // parameters to pass in
			);

	pthread_join(thread_1, NULL);
	pthread_join(thread_2, NULL);
	pthread_join(thread_3, NULL);

	pthread_mutex_destroy(&lock);
}

