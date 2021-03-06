#include<set>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>
#include"base_config.h"
#include"kd.cpp"

using namespace std;

int SIZE_ECB_TASK[NUM_TASKS];
int SIZE_UCB_TASK[NUM_TASKS];
int ECB_TASK_ARRAY[NUM_TASKS][CACHE_SIZE];
std::set<int> TASK_ECB[NUM_TASKS], TASK_UCB[NUM_TASKS];

void print_base_config(FILE *fp){
	fprintf(fp, "\t\t CACHE SIZE = %d\n", CACHE_SIZE);
	fprintf(fp, "\t\t BRT = %g\n", BRT);
	fprintf(fp, "\t\t RF = %g\n", RF);
	fprintf(fp, "\t\t NUM_TASKS = %d\n\n", NUM_TASKS);
}

void print_ecbs(FILE *fp){
	printf("Printing ECBs\n");
	for(int i = 0; i < NUM_TASKS; i++)
	{
		fprintf(fp, "ECB Set %d : \n", i);
		print_SET(TASK_ECB[i], fp);
		fprintf(fp, "\n");
	}
	printf("Printing ECBs DONE\n");
}

void print_ucbs(FILE *fp){
	printf("Printing UCBs\n");

	for(int i = 0; i < NUM_TASKS; i++)
	{
		fprintf(fp, "UCB Set %d : \n", i);
		print_SET(TASK_UCB[i], fp);
		fprintf(fp, "\n");
	}

	printf("Printing UCBs Done\n");
}

double printTaskInfo(FILE *fp)
{
	double util = 0;

	printf("**************************************************************************************************\n\n");
	printf("                                    PRINTING TASK INFO                                            \n\n");
	printf("**************************************************************************************************\n");

	fprintf(fp, "**************************************************************************************************\n\n");
	fprintf(fp, "                                    PRINTING TASK INFO                                            \n\n");
	fprintf(fp, "**************************************************************************************************\n");

	for(int i = 0; i < NUM_TASKS; i++)
	{
		printf("TASK = %d   EXEC_TIME = %g  PERIOD = %ld  DEADLINE = %ld \n\n", i, C[i], T[i], D[i]);
		fprintf(fp, "TASK = %d   EXEC_TIME = %g  PERIOD = %ld  DEADLINE = %ld \n\n", i, C[i], T[i], D[i]);

		util += ( C[i] / T[i]);
	}

	printf("The Utilization of TASK_SET is %f \n", util);
	fprintf(fp, "The Utilization of TASK_SET is %f \n", util);

	printf("________________________________PRINTING TASK INFO ENDS___________________________________________\n\n");
	fprintf(fp, "________________________________PRINTING TASK INFO ENDS___________________________________________\n\n");

	return util;
}
void Print_Task_Execution_Statistics()
{
     int i;
     
     for(i = 0; i < NUM_METHODS; i++)
     {
           switch(i)
           {
                    case PRE_MAX_KD : 
				printf("The number of schedulable tasks for PRE_MAX_KD is %d \n", Num_Executed_Tasks[PRE_MAX_KD]);
                       		fprintf(fp, "The number of schedulable tasks for PRE_MAX_KD is %d \n", Num_Executed_Tasks[PRE_MAX_KD]);
                             	break;
                    default : break;
                             
           }
     }
}

void Read_ECBs(void)
{
	int i, j;
	std::set<int>::iterator it;

	for(j=0; j < NUM_TASKS; j++)
	{
		int count = SIZE_ECB_TASK[j];
		int newPos = 0;
		int blockToInsert = rand() % CACHE_SIZE;

		for(i = 0; i < count; i++)
		{
			it = TASK_ECB[j].find(blockToInsert);

			if (it == TASK_ECB[j].end())
			{
				TASK_ECB[j].insert(blockToInsert);
				ECB_TASK_ARRAY[j][newPos++] = blockToInsert;
			}

			blockToInsert = (blockToInsert + 1) % CACHE_SIZE;
		}
	}
}

void Read_UCBs(void)
{
	int j;
	std::set<int>::iterator it;

	for(j=0; j < NUM_TASKS; j++)
	{
		int newUCBlock, newPos, count, numBlocks, numUCBs;

		count = 0;

		if (SIZE_ECB_TASK[j] <= CACHE_SIZE)
		{
			numBlocks = SIZE_ECB_TASK[j];
		}
		else
		{
			numBlocks = CACHE_SIZE;
		}

		if (SIZE_UCB_TASK[j] <= CACHE_SIZE)
		{
			numUCBs = SIZE_UCB_TASK[j];
		}
		else
		{
			numUCBs = CACHE_SIZE;
		}

		while(count < numUCBs)
		{
			newPos = rand() % numBlocks;

			newUCBlock = ECB_TASK_ARRAY[j][newPos];
			it = TASK_UCB[j].find(newUCBlock);

			if (it == TASK_UCB[j].end())
			{
				TASK_UCB[j].insert(newUCBlock);
				count++;
			}
		}

		printf("The UCBs for task %d is done\n", j);

	}
}

void UUniSort(float A[], int n, float sumUtil)
{
	int i, j; float random_val; float temp;

	printf("The total util is %f\n", sumUtil);

	srand((unsigned)time(NULL));
	A[0] = 0.0;

	for (i=1; i < n; i++)
	{
		random_val = ((double) rand() / (RAND_MAX));
		A[i] = sumUtil * random_val;
	}

	for (i = 0; i < n-1; i++)
	{
		for (j=0; j < n-i-1; j++)
		{
			if (A[j] > A[j+1])
			{
				temp = A[j];
				A[j] = A[j+1];
				A[j+1] = temp;
			}
		}
	}

	for (i = 1; i < n ; i++)
	{
		A[i-1] = A[i] - A[i-1];
	}

	A[n-1] = sumUtil - A[n-1];

	for(i=0; i < n; i++)
	{
		printf("The util for element %d is %f\n", i, A[i]);
	}
}

// 5 - 500 => 1 - 100 => 0 => log (100)
void LogUniformPeriods(float A[], int n, int minP, int maxP)
{
	int i, j; float temp;
	float logScaleFactor;

	logScaleFactor = log(maxP/minP);

	srand((unsigned)time(NULL));

	for(i=0; i < n; i++)
	{
		// generate random numbers between 0 and 1
		A[i] = (double) rand()/RAND_MAX;
		// Now A[i] is between 0 to log(maxP/minP) to make it uniform log distribution
		A[i] = logScaleFactor * A[i];
		// Now map it to the scale 1 to maxP/minP
		A[i] = exp(A[i]);
		// Finally map it to minP to maxP
		A[i] = A[i] * minP;
	}

	// Sort the periods
	for (i = 0; i < n-1; i++)
	{
		for (j=0; j < n-i-1; j++)
		{
			if (A[j] > A[j+1])
			{
				temp = A[j];
				A[j] = A[j+1];
				A[j+1] = temp;
			}
		}
	}

	for(i= 0; i < n; i++)
	{
		printf("The generated random number is %f\n", A[i]);
	}
}

void UUniFast(float A[], int n, float sumUtil)
{
	int i; float nextSumUtil; float random_val;

	printf("The total util is %f\n", sumUtil);

	srand((unsigned)time(NULL));
	for (i=1; i < n; i++)
	{
		random_val = ((double) rand() / (RAND_MAX));
		nextSumUtil = sumUtil * random_val;
		A[i-1] = sumUtil - nextSumUtil;
		sumUtil = nextSumUtil;
	}

	A[i-1] = sumUtil;

	for (i = 0; i < n; i++)
	{
		printf("The util for element %d is %f\n", i, A[i]);
	}
}

void CreateTask_Uniform_Distribution(float totalUtil, int minPeriod, int maxPeriod)
{
	int i;

	float utilsArray[NUM_TASKS], periodsArray[NUM_TASKS];

	UUniFast(utilsArray, NUM_TASKS, totalUtil);
	LogUniformPeriods(periodsArray, NUM_TASKS, minPeriod, maxPeriod);

	for(i=0; i < NUM_TASKS; i++)
	{
		T[i] = (long) periodsArray[i];
		C[i] = utilsArray[i] * T[i];
		D[i] = T[i];
	}

	for(i=0; i < NUM_TASKS; i++)
	{
		printf("The utilization of task %d is %f\n", i, utilsArray[i]);
	}

	for(i=0; i < NUM_TASKS; i++)
	{
		printf("The period of task %d is %ld\n", i, T[i]);
	}

	for(i=0; i < NUM_TASKS; i++)
	{
		printf("The execution time for task %d is %f\n", i, C[i]);
	}
}

void Set_SizeUCBs_Uniform(void)
{
	int i;

	srand((unsigned)time(NULL));
	for(i=0; i < NUM_TASKS; i++)
	{
		SIZE_UCB_TASK[i] = (int) ( RF * SIZE_ECB_TASK[i] * ( (double) rand()/ RAND_MAX )) ;
		printf("The number of UCBs for task %d is %d \n", i, SIZE_UCB_TASK[i]);
	}
}

void Set_SizeECBs_UUniFast(void)
{
	int i;
	float values_uunifast[NUM_TASKS];

	UUniSort(values_uunifast, NUM_TASKS, 1.0);

	for(i=0; i < NUM_TASKS; i++)
	{
		SIZE_ECB_TASK[i] = (int) (values_uunifast[i] * Total_ECBs_CU * CACHE_SIZE);
		printf("The number of ECBs for task %d is %d \n", i, SIZE_ECB_TASK[i]);
	}
}

void CALL_METHODS(FILE *fp){
	Response_time_PRE_MAX_KD(fp);
}

void Uniform_Distribution_Benchmark()
{
	float totalUtil = taskSetUtil = 0.05;
	int minPeriod = 20;
	int maxPeriod = 500;

	FILE *fp = fopen("Results_Malardalen.txt", "w");
	print_base_config(fp);

	Set_SizeECBs_UUniFast();
	Set_SizeUCBs_Uniform();

	Read_ECBs();
	Read_UCBs();

	print_ecbs(fp);
	print_ucbs(fp);

	for (; totalUtil <= 1.0; totalUtil += 0.05)
	{
		taskSetUtil = totalUtil;

		Clear_Task_Execution_Statistics();

		for(int i=1; i <= NUM_TASK_SETS; i++)
		{
			CreateTask_Uniform_Distribution(totalUtil, minPeriod, maxPeriod);
			printTaskInfo(fp);
			CALL_METHODS(fp);
		}
		printf("##################################################################################################\n");
		fprintf(fp, "##################################################################################################\n");
		printf("Printing statistics for %f utilization and %d NUM_TASK_SETS \n", totalUtil, NUM_TASK_SETS);
	        fprintf(fp, "Printing statistics for %f utilization and %d NUM_TASK_SETS \n", totalUtil, NUM_TASK_SETS);  
	        Print_Task_Execution_Statistics();
	        printf("Printing statistics for %f utilization and %d NUM_TASK_SETS complete\n", totalUtil, NUM_TASK_SETS);
	        fprintf(fp, "Printing statistics for %f utilization and %d NUM_TASK_SETS complete\n", totalUtil, NUM_TASK_SETS);
		printf("##################################################################################################\n\n");
		fprintf(fp, "##################################################################################################\n\n");
	}		
}
