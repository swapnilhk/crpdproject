#include"benchmarks.h"
#include<time.h>
#include<stdlib.h>
#include<math.h>
#include"global.h"
#include<new>
#include<set>

#define CNT 28

static int *SIZE_ECB_TASK;
static int *SIZE_UCB_TASK;
static int **ECB_TASK_ARRAY;

/**
 * @brief Creates the ECB array for each task using
 * ramnom function
 */
static void Read_ECBs(void){
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

/**
 * @brief Creates the ECB array for each task using
 * ramnom function
 */
static void Read_UCBs(void){
	int j;
	std::set<int>::iterator it;
	for(j=0; j < NUM_TASKS; j++){
		int newUCBlock, newPos, count, numBlocks, numUCBs;
		count = 0;
		if(SIZE_ECB_TASK[j] <= CACHE_SIZE)
			numBlocks = SIZE_ECB_TASK[j];
		else
			numBlocks = CACHE_SIZE;
		if(SIZE_UCB_TASK[j] <= CACHE_SIZE)
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

static void UUniSort(float A[], int n, float sumUtil){
	int i, j; float random_val; float temp;
	srand((unsigned)time(NULL));
	A[0] = 0.0;
	for (i=1; i < n; i++){
		random_val = ((double) rand() / (RAND_MAX));
		A[i] = sumUtil * random_val;
	}
	for (i = 0; i < n-1; i++)
		for (j=0; j < n-i-1; j++)
			if (A[j] > A[j+1]){
				temp = A[j];
				A[j] = A[j+1];
				A[j+1] = temp;
			}
	for (i = 1; i < n ; i++)
		A[i-1] = A[i] - A[i-1];
	A[n-1] = sumUtil - A[n-1];
}

// 5 - 500 => 1 - 100 => 0 => log (100)
static void LogUniformPeriods(float A[], int n, int minP, int maxP){
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
	for (i = 0; i < n-1; i++)
		for (j=0; j < n-i-1; j++)
			if (A[j] > A[j+1]){
				temp = A[j];
				A[j] = A[j+1];
				A[j+1] = temp;
			}
}

static void UUniFast(float A[], int n, float sumUtil){
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

static void Set_SizeUCBs_Uniform(){
	int i;
	srand((unsigned)time(NULL));
	for(i=0; i < NUM_TASKS; i++)
		SIZE_UCB_TASK[i] = (int) ( RF * SIZE_ECB_TASK[i] * ( (double) rand()/ RAND_MAX ));
}

static void Set_SizeECBs_UUniFast(){
	int i;
	float values_uunifast[NUM_TASKS];	
	UUniSort(values_uunifast, NUM_TASKS, 1.0);
	for(i=0; i < NUM_TASKS; i++)
		SIZE_ECB_TASK[i] = (int) (values_uunifast[i] * Total_ECBs_CU * CACHE_SIZE);
}

/**
 * @brief Frees the benchmark
 */
void freeBenchmark(void){
	free(C);
	free(B);	
	free(T);	
	free(D);
	delete [] TASK_ECB;
	delete [] TASK_UCB;
	free(SIZE_ECB_TASK);
	free(SIZE_UCB_TASK);
	free2DintArrayInt(ECB_TASK_ARRAY, NUM_TASKS);	
}

/**
 * @brief Initializes the benchmark the number of
 * tasks in a set and the cache size
 * @param numTasks :- Number of task in a set
 * @param fp :- Pointer to the file where the
 * intermediate logs are to be written
 * @return 1 if successful, 0 if faliure
 */
int initBenchmark(int numTasks, FILE *fp){
	NUM_TASKS = numTasks;
	C = (double*)malloc(sizeof(*C) * NUM_TASKS);
	B = (double*)malloc(sizeof(*B) * NUM_TASKS);	
	T = (long*)malloc(sizeof(*T) * NUM_TASKS);	
	D = (long*)malloc(sizeof(*D) * NUM_TASKS);
	TASK_ECB = new std::set<int>[NUM_TASKS];
	TASK_UCB = new std::set<int>[NUM_TASKS];	
	SIZE_ECB_TASK = (int*)malloc(sizeof(*SIZE_ECB_TASK) * NUM_TASKS);
	SIZE_UCB_TASK = (int*)malloc(sizeof(*SIZE_UCB_TASK) * NUM_TASKS);
	ECB_TASK_ARRAY = Make2DintArrayInt(NUM_TASKS, CACHE_SIZE);
	if(C==NULL||B==NULL||T==NULL||D==NULL||TASK_ECB==NULL||TASK_UCB==NULL||SIZE_ECB_TASK==NULL||SIZE_UCB_TASK==NULL||ECB_TASK_ARRAY==NULL){
		fprintf(stderr, "Memory allocation error\n");
		return 0;
	}	
	Set_SizeECBs_UUniFast();
	Set_SizeUCBs_Uniform();
	Read_ECBs();
	Read_UCBs();
	return 1;
}

/**
 * @brief Creates a task set
 * @param totalUtil :- Total utilisation of the system
 * @param minPeriod :- minimum vlue of period
 * @param maxPeriod :- maximum vlue of period
 * @param fp :- Pointer to the file where the
 * intermediate logs are to be written
 * @return 1 if successful, 0 if faliure
 */
void createTaskSetUniformDistribution(float totalUtil, int minPeriod, int maxPeriod, FILE *fp){
	int i;
	float utilsArray[NUM_TASKS], periodsArray[NUM_TASKS];	
	UUniFast(utilsArray, NUM_TASKS, totalUtil);
	LogUniformPeriods(periodsArray, NUM_TASKS, minPeriod, maxPeriod);
	for(i=0; i < NUM_TASKS; i++){
		T[i] = (long) periodsArray[i];
		C[i] = utilsArray[i] * T[i];
		B[i] = 0.6 * C[i];// TODO: Replace it with comething else
		D[i] = T[i];
	}
}
