#include<set>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>
#include"base_config.h"
#include"set_operations.cpp"
#include"common.h"
//#include"kd.cpp"
#include"lee_wdc.cpp"

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
	int i;

	if(VERBOSE)
		printf("Printing ECB sets\n");

	for(i = 0; i < NUM_TASKS; i++)
	{
		fprintf(fp, "ECB Set %d : \n", i);
		print_SET(TASK_ECB[i], fp);
		fprintf(fp, "\n");
	}

	if(VERBOSE)
		printf("Printing ECB sets done\n");
}

void print_ucbs(FILE *fp){
	int i;

	if(VERBOSE)
		printf("Printing UCB sets\n");

	for(i = 0; i < NUM_TASKS; i++)
	{
		fprintf(fp, "UCB Set %d : \n", i);
		print_SET(TASK_UCB[i], fp);
		fprintf(fp, "\n");
	}

	if(VERBOSE)
		printf("Printing UCB sets done\n");
}

void Print_Task_Execution_Statistics(FILE *fp)
{
	int i;
	static int heading = 0;

	if(VERBOSE)
		printf("Printing task execution statistics\n");

     	if(heading == 0){
	     	fprintf(fp, "Utilization\tMethod\t\tNumber of schedulable tasks\n");
	     	heading = 1;
     	}
    	fprintf(fp, "%g", taskSetUtil);
     	for(i = 0; i < NUM_METHODS; i++)
     	{
		char str[20] = "\0";
           	switch(i)
           	{
			case PRE_MAX_KD : 
                       		sprintf(str, "\t\tPRE_MAX_KD\t%d\n", Num_Executed_Tasks[PRE_MAX_KD]);
                             	break;
			case LEE_WODC : 
                       		sprintf(str, "\t\tLEE_WODC  \t%d\n", Num_Executed_Tasks[LEE_WODC]);
                             	break;
			case LEE_WDC : 
                       		sprintf(str, "\t\tLEE_WDC   \t%d\n", Num_Executed_Tasks[LEE_WDC]);
                             	break;
			default : break;                             
           	}
		fprintf(fp, "%s", str);
		if(VERBOSE)
			printf("%s", str);
     	}

	if(VERBOSE)
		printf("Printing task execution statistics done\n");
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
	}
}

void UUniSort(float A[], int n, float sumUtil)
{
	int i, j; float random_val; float temp;

	//printf("The total util is %f\n", sumUtil);

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

	/*if(VERBOSE){
		for(i=0; i < n; i++)
		{
			printf("The util for element %d is %f\n", i, A[i]);
		}
	}*/
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

	/*if(VERBOSE){	
		for(i= 0; i < n; i++)
		{
			printf("The generated random number is %f\n", A[i]);
		}
	}*/
}

void UUniFast(float A[], int n, float sumUtil)
{
	int i; float nextSumUtil; float random_val;	

	srand((unsigned)time(NULL));
	for (i=1; i < n; i++)
	{
		random_val = ((double) rand() / (RAND_MAX));
		nextSumUtil = sumUtil * random_val;
		A[i-1] = sumUtil - nextSumUtil;
		sumUtil = nextSumUtil;
	}

	A[i-1] = sumUtil;

	/*if(VERBOSE){
		for (i = 0; i < n; i++)
		{
			printf("The util for element %d is %f\n", i, A[i]);
		}
	}*/
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
	/*if(VERBOSE)
	{
		for(i=0; i < NUM_TASKS; i++)
			printf("The utilization of task %d is %f\n", i, utilsArray[i]);
	
		for(i=0; i < NUM_TASKS; i++)
			printf("The period of task %d is %ld\n", i, T[i]);
	
		for(i=0; i < NUM_TASKS; i++)
			printf("The execution time for task %d is %f\n", i, C[i]);
	}*/
}

void Set_SizeUCBs_Uniform()
{
	int i;

	srand((unsigned)time(NULL));
	for(i=0; i < NUM_TASKS; i++)
	{
		SIZE_UCB_TASK[i] = (int) ( RF * SIZE_ECB_TASK[i] * ( (double) rand()/ RAND_MAX )) ;

		if(VERBOSE)
			printf("The number of UCBs for task %d is %d \n", i, SIZE_UCB_TASK[i]);
	}
}

void Set_SizeECBs_UUniFast()
{
	int i;
	float values_uunifast[NUM_TASKS];

	UUniSort(values_uunifast, NUM_TASKS, 1.0);

	for(i=0; i < NUM_TASKS; i++)
	{
		SIZE_ECB_TASK[i] = (int) (values_uunifast[i] * Total_ECBs_CU * CACHE_SIZE);

		if(VERBOSE)
			printf("The number of ECBs for task %d is %d \n", i, SIZE_ECB_TASK[i]);
	}
}

void CALL_METHODS(){
	//Response_time_PRE_MAX_KD();
	int WDC;
	Response_time_lee_wdc(WDC = 0);
	Response_time_lee_wdc(WDC = 1);
}

void Clear_Task_Execution_Statistics()
{
     int i;
     
     for( i = 0; i < NUM_METHODS; i++)
     {
          Num_Executed_Tasks[i] = 0;
     }
}

void Uniform_Distribution_Benchmark(FILE *fp)
{
	float totalUtil = taskSetUtil = 0.05;
	int minPeriod = 20;
	int maxPeriod = 500;

	if(MESSAGE_LEVEL >= IMP)
		print_base_config(fp);
	
	Set_SizeECBs_UUniFast();
	Set_SizeUCBs_Uniform();

	Read_ECBs();
	Read_UCBs();

	if(MESSAGE_LEVEL >= ALL){
		print_ecbs(fp);
		print_ucbs(fp);
	}

	for (; totalUtil <= 1.0; totalUtil += 0.05)
	{
		taskSetUtil = totalUtil;

		if(VERBOSE)
			printf("The total util is %f\n", totalUtil);

		Clear_Task_Execution_Statistics();

		for(int i=1; i <= NUM_TASK_SETS; i++)
		{
			CreateTask_Uniform_Distribution(totalUtil, minPeriod, maxPeriod);
			CALL_METHODS();
		}

	        Print_Task_Execution_Statistics(fp);
	}		
}

int main() {
	FILE *fp = NULL;

	//Malardaren_benchmark();
	//Geometric_execution_times_benchmark();	

	fp = fopen("out/Uniform_Distribution_Benchmark.txt", "w");

	if(fp == NULL){
		printf("***Unable to open file\n");
		exit(0);
	}
	
	Uniform_Distribution_Benchmark(fp);

	fclose(fp);
}
