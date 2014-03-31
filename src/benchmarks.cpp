#include<set>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>
#include"base_config.h"
#include"set_operations.cpp"
#include"common.h"
#include"kd.cpp"
#include"kd2.cpp"
#include"lee_wdc.cpp"
#include"uniform_fixed_tasks_ecb_ucb_schemes.cpp"

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
	     	fprintf(fp, "%-4s\t%-32s\t%s\n", "Util", "Method", "No. of Sched tasks");
	     	heading = 1;
     	}
	else
		fprintf(fp, "\n");

    	fprintf(fp, "%-4.2g", taskSetUtil);
     	for(int method_name = NO_PREEMPT; method_name < NUM_METHODS; method_name++)
     	{
		char str[50] = "\0";
		sprintf(str, "\t%-32s\t%d\n", method_names_map[method_name] , Num_Executed_Tasks[method_name]);           	
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

int CALL_METHODS(){
	int WDC, sched;
	int sched_vector = 0;

	if(VERBOSE)
		printf("\t%s", "NO_PREEMPT\n");		
	sched = Response_time_NO_PREEMPT();
	sched_vector |= sched << NO_PREEMPT;

	if(VERBOSE)		
		printf("\t%s", "ECB_Only\n");
        sched = Response_time_ECB_Only();    
        sched_vector |= sched << ECB_ONLY;

	if(VERBOSE)		
		printf("\t%s", "UCB_Only\n");    
        sched = Response_time_UCB_Only(); 
        sched_vector |= sched << UCB_ONLY;

	if(VERBOSE)		
		printf("\t%s", "UCB_Union\n");
        sched = Response_time_UCB_Union();        
        sched_vector |= sched << UCB_UNION;

	if(VERBOSE)		
		printf("\t%s", "ECB_Union\n");
        sched = Response_time_ECB_Union();        
        sched_vector |= sched << ECB_UNION;

	if(VERBOSE)		
		printf("\t%s", "ECB_Union_Multiset_PRE\n");
        sched = Response_time_ECB_Union_Multiset_PRE();
        sched_vector |= sched << ECB_UNION_MULTISET_PRE;

	if(VERBOSE)		
		printf("\t%s", "UCB_Union_Multiset_PRE\n");
        sched = Response_time_UCB_Union_Multiset_PRE();
        sched_vector |= sched << UCB_UNION_MULTISET_PRE;

	if(VERBOSE)		
		printf("\t%s", "ECB_UCB_Union_Multiset_Combined_PRE\n");
        sched = Response_time_ECB_UCB_Union_Multiset_Combined_PRE();          
        sched_vector |= sched << ECB_UCB_UNION_MULTISET_COMBINED_PRE;

	if(VERBOSE)		
		printf("\t%s", "ECB_Union_Multiset\n");
        sched = Response_time_ECB_Union_Multiset();
        sched_vector |= sched << ECB_UNION_MULTISET;

	if(VERBOSE)		
		printf("\t%s", "UCB_Union_Multiset\n");
        sched = Response_time_UCB_Union_Multiset();
        sched_vector |= sched << UCB_UNION_MULTISET;

	if(VERBOSE)		
		printf("\t%s", "Staschulat\n");
	sched = Response_time_Staschulat();
	sched_vector |= sched << STASCHULAT;

	if(VERBOSE)		
		printf("\t%s", "ECB_UCB_Union_Multiset_Combined\n");
        sched = Response_time_ECB_UCB_Union_Multiset_Combined();        
        sched_vector |= sched << ECB_UCB_UNION_MULTISET_COMBINED;

	if(VERBOSE)		
		printf("\t%s", "PRE_MAX\n");
        sched = Response_time_PRE_MAX();        
        sched_vector |= sched << PRE_MAX;

	if(VERBOSE)		
		printf("\t%s", "Staschulat_PRE\n");
        sched = Response_time_Staschulat_PRE();
        sched_vector |= sched << STASCHULAT_PRE;

	if(VERBOSE)		
		printf("\t%s", "PRE_MAX_KD\n");
	sched = Response_time_PRE_MAX_KD();
	sched_vector |= sched << PRE_MAX_KD;	
	
	




	if(VERBOSE)		
		printf("\t%s", "PRE_MAX_KD2\n");
	sched = Response_time_PRE_MAX_KD2();	
	sched_vector |= sched << PRE_MAX_KD2;	
	
	



	if(VERBOSE)		
		printf("\t%s", "lee_wdoc\n");
	sched = Response_time_lee_wdc(WDC = 0);	
	sched_vector |= sched << LEE_WODC;		
	


	if(VERBOSE)		
		printf("\t%s", "lee_wdc\n");
	sched = Response_time_lee_wdc(WDC = 1);
	sched_vector |= sched << LEE_WDC;		
	
	
	return sched_vector;
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
	float totalUtil = taskSetUtil = UTIL_START;
	int minPeriod = MIN_PERIOD;
	int maxPeriod = MAX_PERIOD;
	int dom[NUM_METHODS] = {0};

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

	for (; totalUtil <= UTIL_END; totalUtil += UTIL_INCREMENT)
	{
		taskSetUtil = totalUtil;

		if(VERBOSE)
			printf("The total util is %f\n", totalUtil);

		Clear_Task_Execution_Statistics();

		for(int task_set_no = 1; task_set_no <= NUM_TASK_SETS; task_set_no++)
		{
			int sched_vector;//vector of bits with each bit corresponding to each method
			if(VERBOSE)
				printf("Task set no: %d Util: %.2f\n",task_set_no, taskSetUtil);

			CreateTask_Uniform_Distribution(totalUtil, minPeriod, maxPeriod);
			sched_vector = CALL_METHODS();
			for(int method_name = NO_PREEMPT; method_name < NUM_METHODS; method_name++){
				if(sched_vector >> method_name & 1)// if(sched_vectpr has bit corresponding to the method set)
					dom[method_name] |= ~sched_vector; // Add to dom[this_method] all methods that this_method domonates, ie are zero
			}			
		}		
		
		if(VERBOSE) printf("\n");

	        Print_Task_Execution_Statistics(fp);
	}	
	
	for(int method1 = NO_PREEMPT; method1 < NUM_METHODS - 1; method1++){
		for(int method2 = method1 + 1; method2 < NUM_METHODS; method2++){
			//Checking if method1 dominates method2
			if((dom[method1] >> method2 & 1) && !(dom[method2] >> method1 & 1))
				fprintf(fp, "%s dominates %s\n", method_names_map[method1], method_names_map[method2]);
			//Checking if method2 dominates method1
			else if((dom[method2] >> method1 & 1) && !(dom[method1] >> method2 & 1))
				fprintf(fp, "%s is dominated by %s\n", method_names_map[method1], method_names_map[method2]);
			//Checking if method1 and method2 are equal
			else if(!(dom[method2] >> method1 & 1) && !(dom[method1] >> method2 & 1))
				fprintf(fp, "%s and %s are equal\n", method_names_map[method1], method_names_map[method2]);
			//If all above conditions fail, method1 and method2 are not comparable
			else fprintf(fp, "%s and %s are not comparable\n", method_names_map[method1], method_names_map[method2]);
		}
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
