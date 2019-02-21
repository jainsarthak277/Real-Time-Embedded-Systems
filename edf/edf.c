/**************************************************************************************************/
/*	This code was written by Sarthak Jain for the course of Real-Time Embedded Systems	  */
/*	as an algorithm for plotting out the Earliest Deadline First scheduling scheme		  */
/*	Parts of the code (calculation of LCM) have been referred from GeeksForGeeks, link	  */
/*	provided here: https://www.geeksforgeeks.org/lcm-of-given-array-elements/ 		  */
/*	Reference given to Dr. Sam Siewert as well, link for which is provided below		  */
/*	http://mercury.pr.erau.edu/~siewerts/cec450/code/Feasibility/ 				  */
/**************************************************************************************************/


#include <math.h>
#include <stdio.h>

#define TRUE 1
#define FALSE 0
#define U32_T unsigned int

// U=0.7333
U32_T ex0_period[] = {2, 10, 15};
U32_T ex0_wcet[] = {1, 1, 2};

// U=0.9857
U32_T ex1_period[] = {2, 5, 7};
U32_T ex1_wcet[] = {1, 1, 2};

// U=0.9967
U32_T ex2_period[] = {2, 5, 7, 13};
U32_T ex2_wcet[] = {1, 1, 1, 2};

// U=0.93
U32_T ex3_period[] = {3, 5, 15};
U32_T ex3_wcet[] = {1, 2, 3};

// U=1.0
U32_T ex4_period[] = {2, 4, 16};
U32_T ex4_wcet[] = {1, 1, 4};

U32_T ex5_period[] = {2, 5, 10};
U32_T ex5_wcet[] = {1, 2, 1};

U32_T ex7_period[] = {3, 5, 15};
U32_T ex7_wcet[] = {1, 2, 4};

U32_T gcd(U32_T a, U32_T b)		//Find HCF of 2 given numbers
{
	if (b==0)
	return a;
	return gcd(b, a%b);
}

U32_T lcm(U32_T a[], U32_T n)		//Find LCM of given array of numbers having size n
{
	U32_T res = 1, i;
	for (i=0;i<n;i++)
	{
		res = res*a[i]/gcd(res, a[i]);	
	}
	return res;
}

U32_T minimum(U32_T arr[], U32_T size)		//Find minimum of given array of numbers
{
	U32_T min = arr[0], tally = 0, k;
	for(k=0; k<size; k++)
	{
		if(min>arr[k])
		{
			min = arr[k];
			tally=k;
		}
	}	
	return tally;
}

int EDF_feasibility(U32_T numServices,U32_T period[], U32_T wcet[], U32_T deadline[])
{
	U32_T flag = 0;
	U32_T deadline_temp[sizeof(ex0_period)/sizeof(U32_T)];
	U32_T wcet_temp[sizeof(ex0_period)/sizeof(U32_T)];
	for(int m = 0;m<numServices;m++)		//Creating temporary variables for deadine and WCET
	{
		deadline_temp[m] = deadline[m];
		wcet_temp[m] = wcet[m];
	}
	U32_T tally;
	U32_T lcm_p = lcm(period, numServices);
	for(int i=0; i<lcm_p; i++)			//i represents time in seconds
	{
		for(int j=0; j<numServices; j++)	//j represents service number
		{	
			if((i!=0) && ((i%deadline[j]) == 0))		//At every deadline for respective task
			{
				if(wcet_temp[j] != 0)		//If, given above condition, WCET != 0, return infeasible
				{
					return FALSE;
				}
				else				//reset WCETs and deadlines
				{
					wcet_temp[j] = wcet[j];	
					deadline_temp[j] = deadline[j];
				}
			}
			deadline_temp[j]--;
			if(wcet_temp[j] == 0)			//assign a very high value to deadline_temp, if WCET = 0 before deadline is reached
			{
				deadline_temp[j] = 100;
			}
		}
		flag=0;
		for(int l=0;l<numServices;l++)
		{
			if(wcet_temp[l] != 0)
			{
				flag=1;				//flag to make sure at least 1 task is yet to be executed. If not, program does nothing and wastes that seconds
			}
		}
		if(flag != 0)
		{
			tally = minimum(deadline_temp, numServices);		//Find task with minimum deadline and decrement WCET of that task
			wcet_temp[tally]--;
			printf("\nTask %d was executed at instant %d", tally+1, i);
		}
		else 
		{
			printf("\nNo task executed at instant %d", i);
		}
	}
	return TRUE;

}

int main(void)
{ 
	int i;
	U32_T numServices;
	printf("Ex-0 U=%4.2f (C1=1, C2=1, C3=2; T1=2, T2=10, T3=15; T=D): ", ((1.0/2.0) + (1.0/10.0) + (2.0/15.0)));
	numServices = sizeof(ex4_period)/sizeof(U32_T);
	if(EDF_feasibility(numServices, ex4_period, ex4_wcet, ex4_period) == TRUE)
        	printf("\nGIVEN TASK SET IS FEASIBLE\n");
	else
        	printf("\nGIVEN TASK SET IS INFEASIBLE\n");
}
    


