#include"global.h"
#include<time.h>

static int SIZE_ECB_TASK[NUM_TASKS];
static int SIZE_UCB_TASK[NUM_TASKS];
static int ECB_TASK_ARRAY[NUM_TASKS][CACHE_SIZE];

void Read_ECBs(void)
{
	int i, j;
	std::set<int>::iterator it;

	for(j=0; j < NUM_TASKS; j++){
		int count = SIZE_ECB_TASK[j];
		int newPos = 0;
		int blockToInsert = rand() % CACHE_SIZE;
		for(i = 0; i < count; i++){
			it = TASK_ECB[j].find(blockToInsert);
			if (it == TASK_ECB[j].end()){
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
			numBlocks = SIZE_ECB_TASK[j];
		else
			numBlocks = CACHE_SIZE;
		if (SIZE_UCB_TASK[j] <= CACHE_SIZE)
			numUCBs = SIZE_UCB_TASK[j];
		else
			numUCBs = CACHE_SIZE;
		while(count < numUCBs){
			newPos = rand() % numBlocks;
			newUCBlock = ECB_TASK_ARRAY[j][newPos];
			it = TASK_UCB[j].find(newUCBlock);
			if (it == TASK_UCB[j].end()){
				TASK_UCB[j].insert(newUCBlock);
				count++;
			}
		}
	}
}

void UUniSort(float A[], int n, float sumUtil)
{
	int i, j; float random_val; float temp;

	srand((unsigned)time(NULL));
	A[0] = 0.0;
	for (i=1; i < n; i++){
		random_val = ((double) rand() / (RAND_MAX));
		A[i] = sumUtil * random_val;
	}
	for (i = 0; i < n-1; i++){
		for (j=0; j < n-i-1; j++){
			if (A[j] > A[j+1]){
				temp = A[j];
				A[j] = A[j+1];
				A[j+1] = temp;
			}
		}
	}
	for (i = 1; i < n ; i++){
		A[i-1] = A[i] - A[i-1];
	}
	A[n-1] = sumUtil - A[n-1];
}

// 5 - 500 => 1 - 100 => 0 => log (100)
void LogUniformPeriods(float A[], int n, int minP, int maxP)
{
	int i, j; float temp;
	float logScaleFactor;

	logScaleFactor = log(maxP/minP);
	srand((unsigned)time(NULL));
	for(i=0; i < n; i++){
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
	for (i = 0; i < n-1; i++){
		for (j=0; j < n-i-1; j++){
			if (A[j] > A[j+1]){
				temp = A[j];
				A[j] = A[j+1];
				A[j+1] = temp;
			}
		}
	}
}

void UUniFast(float A[], int n, float sumUtil)
{
	int i;
	float nextSumUtil;
	float random_val;	

	srand((unsigned)time(NULL));
	for (i=1; i < n; i++){
		random_val = ((double) rand() / (RAND_MAX));
		nextSumUtil = sumUtil * random_val;
		A[i-1] = sumUtil - nextSumUtil;
		sumUtil = nextSumUtil;
	}
	A[i-1] = sumUtil;

}

void createTaskSetUniformDistribution(float totalUtil, int minPeriod, int maxPeriod)
{
	int i;
	float utilsArray[NUM_TASKS], periodsArray[NUM_TASKS];
	
	UUniFast(utilsArray, NUM_TASKS, totalUtil);
	LogUniformPeriods(periodsArray, NUM_TASKS, minPeriod, maxPeriod);
	for(i=0; i < NUM_TASKS; i++){
		T[i] = (long) periodsArray[i];
		C[i] = utilsArray[i] * T[i];
		D[i] = T[i];
	}
}

void Set_SizeUCBs_Uniform()
{
	int i;
	
	srand((unsigned)time(NULL));
	for(i=0; i < NUM_TASKS; i++){
		SIZE_UCB_TASK[i] = (int) ( RF * SIZE_ECB_TASK[i] * ( (double) rand()/ RAND_MAX )) ;
		if(VERBOSE) printf("The number of UCBs for task %d is %d \n", i, SIZE_UCB_TASK[i]);
	}
}

void Set_SizeECBs_UUniFast()
{
	int i;
	float values_uunifast[NUM_TASKS];
	
	UUniSort(values_uunifast, NUM_TASKS, 1.0);
	for(i=0; i < NUM_TASKS; i++){
		SIZE_ECB_TASK[i] = (int) (values_uunifast[i] * Total_ECBs_CU * CACHE_SIZE);
		if(VERBOSE) printf("The number of ECBs for task %d is %d \n", i, SIZE_ECB_TASK[i]);
	}
}

void initUniformDistributionBenchmark(FILE* fp){	
	Set_SizeECBs_UUniFast();
	Set_SizeUCBs_Uniform();
	Read_ECBs();
	Read_UCBs();
	if(MESSAGE_LEVEL >= ALL){
		print_ecbs(fp);
		print_ucbs(fp);
	}
}
