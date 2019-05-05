/**************************************************************************************************/
/*	This code was written by Sarthak Jain for the course of Real-Time Embedded Systems	  */
/*	as an algorithm for plotting out the Least Laxity First scheduling scheme		  */
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

U32_T ex6_period[] = {2, 5, 7, 13};
U32_T ex6_wcet[] = {1, 1, 1, 2};
U32_T ex6_deadline[] = {2, 3, 7, 15};

U32_T ex7_period[] = {3, 5, 15};
U32_T ex7_wcet[] = {1, 2, 4};

U32_T gcd(U32_T a, U32_T b)
{
	if (b==0)
	return a;
	return gcd(b, a%b);
}

U32_T lcm(U32_T a[], U32_T n)
{
	U32_T res = 1, i;
	for (i=0;i<n;i++)
	{
		res = res*a[i]/gcd(res, a[i]);	
	}
	return res;
}

U32_T minimum(U32_T arr1[], U32_T size, U32_T arr2[])
{
	U32_T min = arr1[0]-arr2[0], tally = 0, k;
	for(k=0; k<size; k++)
	{
		if(min>(arr1[k]-arr2[k]))
		{
			min = arr1[k]-arr2[k];
			tally=k;
		}
	}	
	return tally;
}

int LLF_feasibility(U32_T numServices,U32_T period[], U32_T wcet[], U32_T deadline[])
{
	U32_T flag = 0;
	U32_T deadline_temp[numServices];
	U32_T wcet_temp[numServices];
	U32_T dead[numServices];
	U32_T barrier[numServices];
	for(int m = 0;m<numServices;m++)		//Creating temporary variables for deadine and WCET
	{
		deadline_temp[m] = deadline[m];
		wcet_temp[m] = wcet[m];
		barrier[m] = 0;
	}
	U32_T tally;
	U32_T lcm_p = lcm(period, numServices);
	for(int i=0; i<lcm_p; i++)			//i represents time in seconds
	{
		for(int j=0; j<numServices; j++)	//j represents service number
		{
			if(barrier[j] == 1)
			{
				dead[j]--;
				if(wcet_temp[j] == 0)
				{
					deadline_temp[j] = dead[j];
					wcet_temp[j] = wcet[j];
					barrier[j] = 0;
				}
				else if(deadline_temp[j] == 0)
				{
					return FALSE;
				}
				else 
				{
					dead[j] = deadline[j];
					barrier[j] = 1;					
				}
			}	
			if((i!=0) && ((i%period[j]) == 0))		//At every deadline for respective task
			{
				if(wcet_temp[j] != 0)		//If, given above condition, WCET != 0, return infeasible
				{
					if(deadline_temp[j]==0)
					{
						return FALSE;
					}
					else
					{
						dead[j] = deadline[j];
						barrier[j] = 1;
					}
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
				deadline_temp[j] = lcm_p+100;
			}
		}
		flag=0;
		for(int l=0;l<numServices;l++)
		{
			if(wcet_temp[l] != 0)
			{
				flag=1;				//flag to make sure at least 1 task is yet to be executed. If not, program does nothing and wastes that second
			}
		}
		if(flag != 0)
		{
			tally = minimum(deadline_temp, numServices, wcet_temp);		//Find task with minimum deadline and decrement WCET of that task
			wcet_temp[tally]--;
//			printf("\nTask %d was executed at instant %d", tally+1, i);
		}
		else 
		{
//			printf("\nNo task executed at instant %d", i);
		}
	}
	return TRUE;
}

int main(void)
{ 
	int i;
	U32_T numServices;
	printf("Ex-0 U=%4.2f (C1=1, C2=1, C3=2; T1=2, T2=10, T3=15; T=D): ", ((1.0/2.0) + (1.0/10.0) + (2.0/15.0)));
	numServices = sizeof(ex0_period)/sizeof(U32_T);
	if(LLF_feasibility(numServices, ex0_period, ex0_wcet, ex0_period) == TRUE)
        	printf(" FEASIBLE\n");
	else
        	printf(" INFEASIBLE\n");

	printf("Ex-1 U=%4.2f (C1=1, C2=1, C3=2; T1=2, T2=5, T3=7; T=D): ", ((1.0/2.0) + (1.0/5.0) + (2.0/7.0)));
	numServices = sizeof(ex1_period)/sizeof(U32_T);
	if(LLF_feasibility(numServices, ex1_period, ex1_wcet, ex1_period) == TRUE)
        	printf(" FEASIBLE\n");
	else
        	printf(" INFEASIBLE\n");

	printf("Ex-2 U=%4.2f (C1=1, C2=1, C3=1, C4=2; T1=2, T2=5, T3=7, T4=13; T=D): ", ((1.0/2.0) + (1.0/5.0) + (1.0/7.0) + (2.0/13.0)));
	numServices = sizeof(ex2_period)/sizeof(U32_T);
	if(LLF_feasibility(numServices, ex2_period, ex2_wcet, ex2_period) == TRUE)
	      	printf(" FEASIBLE\n");
	else
	       	printf(" INFEASIBLE\n");

	printf("Ex-3 U=%4.2f (C1=1, C2=2, C3=3; T1=3, T2=5, T3=15; T=D): ", ((1.0/3.0) + (2.0/5.0) + (3.0/15.0)));
	numServices = sizeof(ex3_period)/sizeof(U32_T);
	if(LLF_feasibility(numServices, ex3_period, ex3_wcet, ex3_period) == TRUE)
        	printf(" FEASIBLE\n");
	else
        	printf(" INFEASIBLE\n");

	printf("Ex-4 U=%4.2f (C1=1, C2=1, C3=4; T1=2, T2=4, T3=16; T=D): ", ((1.0/2.0) + (1.0/4.0) + (2.0/16.0)));
	numServices = sizeof(ex4_period)/sizeof(U32_T);
	if(LLF_feasibility(numServices, ex4_period, ex4_wcet, ex4_period) == TRUE)
        	printf(" FEASIBLE\n");
	else
        	printf(" INFEASIBLE\n");

	printf("Ex-5 U=%4.2f (C1=1, C2=2, C3=1; T1=2, T2=5, T3=10; T=D): ", ((1.0/2.0) + (2.0/5.0) + (1.0/10.0)));
	numServices = sizeof(ex5_period)/sizeof(U32_T);
	if(LLF_feasibility(numServices, ex5_period, ex5_wcet, ex5_period) == TRUE)
        	printf(" FEASIBLE\n");
	else
        	printf(" INFEASIBLE\n");

	printf("Ex-6 U=%4.2f (C1=1, C2=1, C3=1, C4=2; T1=2, T2=5, T3=7, T4=13; D1=2, D2=3, D3=7, D4=15): ", ((1.0/2.0) + (1.0/3.0) + (1.0/7.0) + (2.0/15.0)));
	numServices = sizeof(ex6_period)/sizeof(U32_T);
	if(LLF_feasibility(numServices, ex6_period, ex6_wcet, ex6_deadline) == TRUE)
        	printf(" FEASIBLE\n");
	else
        	printf(" INFEASIBLE\n");

	printf("Ex-7 U=%4.2f (C1=1, C2=2, C3=4; T1=3, T2=5, T3=15; T=D): ", ((1.0/3.0) + (2.0/5.0) + (4.0/15.0)));
	numServices = sizeof(ex7_period)/sizeof(U32_T);
	if(LLF_feasibility(numServices, ex7_period, ex7_wcet, ex7_period) == TRUE)
        	printf(" FEASIBLE\n");
	else
        	printf(" INFEASIBLE\n");
}
