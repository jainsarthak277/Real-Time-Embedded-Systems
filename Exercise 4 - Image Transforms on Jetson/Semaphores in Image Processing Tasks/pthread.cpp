/************************************************************************************/
/*																					*/
/*File name - Semaphore use for Image Processing Tasks								*/
/*Authors: Adrian Unkeles and Sarthak Jain											*/
/*Date: April 4, 2019															*/
/************************************************************************************/
/*Code has been written for the purpose of Real-Time Embedded Systems				*/
/*under the guidance of Prof. Tim Scherr											*/

/*We have also referred sections from the file written by Prof. Sam Siewert, titled */
/*nanosleep and POSIX 1003.1b RT clock demonstration								*/
/*We have also referred code titled computer-vision/simple-canny-interactive,		*/
/*computer-vision/simple-hough-eliptical-interactive, 								*/
/*computer-vision/simple-hough-interactive and code titled rt_simplethread			*/
/*for the calculation of time difference used below									*/


//#define _GNU_SOURCE
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
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define NUM_THREADS (2)
#define HRES 160//320//640
#define VRES 120//240//480
#define NSEC_PER_SEC (1000000000)

#define THRESHOLD_CANNY 25
#define THRESHOLD_HOUGH 42
#define THRESHOLD_ELLIPTICAL 15

using namespace cv;
using namespace std;

// POSIX thread declarations and scheduling attributes
//
pthread_t thread_1, thread_2, thread_3;
pthread_attr_t att_1,att_2, att_3;
pthread_attr_t main_att;

sem_t sem1,sem2, sem3;

char timg_window_name[] = "Edge Detector Transform";

int lowThreshold=0;
int const max_lowThreshold = 100;
int kernel_size = 3;
int edgeThresh = 1;
int ratio = 3;
int dev=0;
Mat canny_frame, cdst, timg_gray, timg_grad;

CvCapture* capture;
IplImage* frame;

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

void CannyThreshold(int, void*)
{
    Mat mat_frame(frame);

    cvtColor(mat_frame, timg_gray, CV_RGB2GRAY);

    /// Reduce noise with a kernel 3x3
    blur( timg_gray, canny_frame, Size(3,3) );

    /// Canny detector
    Canny( canny_frame, canny_frame, lowThreshold, lowThreshold*ratio, kernel_size );

    /// Using Canny's output as a mask, we display our result
    timg_grad = Scalar::all(0);

    mat_frame.copyTo( timg_grad, canny_frame);

    imshow( timg_window_name, timg_grad );

}

void *canny(void *threadp)
{	
	int cnt_1=0, capture_flag1=0;
	float frames_1;
	struct timespec start_time, stop_time, diff_time;
	    // Create a Trackbar for user to enter threshold
	createTrackbar( "Min Threshold:", timg_window_name, &lowThreshold, max_lowThreshold, CannyThreshold );

	while(1)
	{
		sem_wait(&sem1);
		while(cnt_1 < 50)
		{
			clock_gettime(CLOCK_REALTIME, &start_time);
			cnt_1++;
	   		frame=cvQueryFrame(capture);
	   		if(!frame)
			{
				break;
			}
			CannyThreshold(0, 0);
			char q = cvWaitKey(33);
			if( q == 'q' )
			{
	       		 	printf("got quit\n"); 
				break;
			}
			clock_gettime(CLOCK_REALTIME, &stop_time);
			delta_t(&stop_time, &start_time, &diff_time);
			frames_1 = 1/((float)(((float)diff_time.tv_nsec))/NSEC_PER_SEC);
			printf("Canny Frame rate = %f \n", frames_1);
			printf("Canny Jitter = %f \n", (THRESHOLD_CANNY - frames_1));
			if((THRESHOLD_CANNY - frames_1) < 0)
			{
				printf("\n*****MISSED DEADLINE*****\n");
			}

		}
		cnt_1 = 0;
		sem_post(&sem2);
	}
}


void *hough(void *threadp)
{
	struct timespec start_time, stop_time, diff_time;
	IplImage* frame;
	int cnt_2=0, capture_flag2=0;
	float frames_2;
	Mat gray, canny_frame, cdst;
	vector<Vec4i> lines;

	while(1)
	{
		sem_wait(&sem2);
		while(cnt_2<80)
		{
			clock_gettime(CLOCK_REALTIME, &start_time);
			cnt_2++;
		        frame=cvQueryFrame(capture);

		        Mat mat_frame(frame);
		        Canny(mat_frame, canny_frame, 50, 200, 3);

        		HoughLinesP(canny_frame, lines, 1, CV_PI/180, 50, 50, 10);

		        for( size_t i = 0; i < lines.size(); i++ )
		        {
				Vec4i l = lines[i];
				line(mat_frame, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
			}
     
			if(!frame) break;
			cvShowImage("Capture Example", frame);

			char c = cvWaitKey(10);
			if( c == 27 ) break;
			clock_gettime(CLOCK_REALTIME, &stop_time);
			delta_t(&stop_time, &start_time, &diff_time);
			frames_2 = 1/((float)(((float)diff_time.tv_nsec))/NSEC_PER_SEC);
			printf("Hough Frame rate = %f \n", frames_2);
			printf("Hough Jitter = %f \n", (THRESHOLD_HOUGH - frames_2));
			if((THRESHOLD_HOUGH - frames_2) < 0)
			{
				printf("\n*****MISSED DEADLINE*****\n");
			}

		}
		cnt_2 = 0;
		sem_post(&sem3);
	}
}

void *ellip(void* threadp)
{
	struct timespec start_time, stop_time, diff_time;
	IplImage* frame;
	int cnt_3 = 0, capture_flag3=0;
	float frames_3;
	Mat gray;
	vector<Vec3f> circles;

	while(1)
	{
		sem_wait(&sem3);
		while(cnt_3 < 80)
		{
			cnt_3++;
			frame=cvQueryFrame(capture);

			Mat mat_frame(frame);
			cvtColor(mat_frame, gray, CV_BGR2GRAY);
			GaussianBlur(gray, gray, Size(9,9), 2, 2);

			HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, gray.rows/8, 100, 50, 0, 0);

			printf("circles.size = %d\n", circles.size());

			for( size_t i = 0; i < circles.size(); i++ )
			{
				Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
				int radius = cvRound(circles[i][2]);
		  // circle center
				circle( mat_frame, center, 3, Scalar(0,255,0), -1, 8, 0 );
		  // circle outline
				circle( mat_frame, center, radius, Scalar(0,0,255), 3, 8, 0 );
			}

	     
			if(!frame) break;

			cvShowImage("Capture Example", frame);

			char c = cvWaitKey(10);
			if( c == 27 ) break;
			clock_gettime(CLOCK_REALTIME, &stop_time);
			delta_t(&stop_time, &start_time, &diff_time);
			frames_3 = 1/((float)(((float)diff_time.tv_nsec))/NSEC_PER_SEC);
			printf("Elliptical Frame rate = %f \n", frames_3);
			printf("Elliptical Jitter = %f \n", (THRESHOLD_ELLIPTICAL - frames_3));
			if((THRESHOLD_ELLIPTICAL - frames_3) < 0)
			{
				printf("\n*****MISSED DEADLINE*****\n");
			}

		}
		cnt_3 = 0;
		sem_post(&sem1);
	}
}


int main (int argc, char *argv[])
{
	int rc;
	int i, scope;
	useconds_t T10 = 10000, T20 = 20000;

	sem_init(&sem1,0,1);
	sem_init(&sem2,0,0);
	sem_init(&sem3,0,0);
     
	rc=pthread_attr_init(&att_1);
       
	rc=pthread_attr_init(&att_2);
	
	capture = (CvCapture *)cvCreateCameraCapture(dev);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, HRES);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, VRES);

	pthread_create(&thread_1,   // pointer to thread descriptor
                      &att_1,     // use default attributes
                      canny, // thread function entry point
                      (void *)0 // parameters to pass in
                     );


	pthread_create(&thread_2,   // pointer to thread descriptor
                      &att_2,     // use default attributes
                      hough, // thread function entry point
                      (void *)0 // parameters to pass in
                     );

	pthread_create(&thread_3,   // pointer to thread descriptor
                      &att_3,     // use default attributes
                      ellip, // thread function entry point
                      (void *)0 // parameters to pass in
                     );

	pthread_join(thread_1, NULL);
	pthread_join(thread_2, NULL);
	pthread_join(thread_3, NULL);
	
	pthread_attr_destroy(&att_1);
	pthread_attr_destroy(&att_2);
	pthread_attr_destroy(&att_3);

	sem_destroy(&sem1);
	sem_destroy(&sem2);
	sem_destroy(&sem3);

	printf("\nTEST COMPLETE\n");
}
