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
	for(i=0; i < NUM_TASKS; i++){
		SIZE_UCB_TASK[i] = (int) ( RF * SIZE_ECB_TASK[i] * ( (double) rand()/ RAND_MAX )) ;
		if(VERBOSE) printf("The number of UCBs for task %d is %d \n", i, SIZE_UCB_TASK[i]);
	}
}

static void Set_SizeECBs_UUniFast(){
	int i;
	float values_uunifast[NUM_TASKS];	
	UUniSort(values_uunifast, NUM_TASKS, 1.0);
	for(i=0; i < NUM_TASKS; i++){
		SIZE_ECB_TASK[i] = (int) (values_uunifast[i] * Total_ECBs_CU * CACHE_SIZE);
		if(VERBOSE) printf("The number of ECBs for task %d is %d \n", i, SIZE_ECB_TASK[i]);
	}
}

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

static int initBenchmark(FILE *fp){
	static int benchmarkInitialized = 0;
	static int numTasks = 0;
	if(numTasks != NUM_TASKS){
	/*NUM_TASKS was changed, hence variables need to be allocated once more*/
		if(benchmarkInitialized){
			freeBenchmark();
			numTasks = NUM_TASKS;
		}
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
			benchmarkInitialized = 0;
		}
		else benchmarkInitialized = 1;
	}
	if(benchmarkInitialized){
		Set_SizeECBs_UUniFast();
		Set_SizeUCBs_Uniform();
		Read_ECBs();
		Read_UCBs();
		if(MESSAGE_LEVEL >= ALL){
			print_ecbs(fp);
			print_ucbs(fp);
		}
	}
	return benchmarkInitialized;
}

void createTaskSetUniformDistribution(float totalUtil, int minPeriod, int maxPeriod, FILE *fp){
	int i;
	float utilsArray[NUM_TASKS], periodsArray[NUM_TASKS];	
	static int first_call = 1;
	static int numTasks;
	if(!first_call && numTasks != NUM_TASKS){
		/* The uniform distribution benchmark had been initialized for NUM_TASKS. Since its value has changed, we have to
		 * initialize the benchmark again*/
		fprintf(fp, "NUM_TASKS got changed from previous value %d to new value %d. Initializing uniform benchmark again\n", numTasks, NUM_TASKS);
		fprintf(stderr, "NUM_TASKS got changed from previous value %d to new value %d. Initializing uniform benchmark again\n", numTasks, NUM_TASKS);
		first_call = 1;
	}
	if(first_call){
		NUM_TASKS = 4;//TODO: read from config file
		numTasks = NUM_TASKS;
		printBaseConfig(fp);
		initBenchmark(fp);
		first_call = 0;
	}
	UUniFast(utilsArray, NUM_TASKS, totalUtil);
	LogUniformPeriods(periodsArray, NUM_TASKS, minPeriod, maxPeriod);
	for(i=0; i < NUM_TASKS; i++){
		T[i] = (long) periodsArray[i];
		C[i] = utilsArray[i] * T[i];
		D[i] = T[i];
	}
}

//-----------------Constant Values---------------------
struct{
	long BCET;
	long WCET;
}ET[CNT]={
	{7491,7491},{14191,14191},{20891,20891},{34291,34291},
	{40991,45291},{47691,55491},{54391,66191},{61091,76391},
	{67791,87091},{16738,16738},{47338,56538},{62638,92238},
	{77938,127538},{750,750},{54015,59896},{9537,9537},
	{43937,43937},{52537,54837},{61137,65937},{69737,77037},
	{78337,88137},{86937,99237},{14536,14536},{79536,89636},
	{92536,112636},{105536,135636},{118536,158636},{131536, 181636}
};

void createTaskSetConstantValues(void){
	if(NUM_TASKS == 2 && util == 0.5){

		C[0] = ET[15].WCET;
		B[0] = ET[15].BCET;
		D[0] = 50000;	
		T[0] = 50000;

		C[1] = ET[18].WCET;
		B[1] = ET[18].BCET;
		D[1] = 200000;	
		T[1] = 200000;		

	}
	else if(NUM_TASKS == 4 && util == 0.5){

		C[0] = ET[0].WCET;
		B[0] = ET[0].BCET;
		D[0] = 50000;	
		T[0] = 50000;

		C[1] = ET[14].WCET;
		B[1] = ET[14].BCET;
		D[1] = 400000;	
		T[1] = 400000;

		C[2] = ET[17].WCET;
		B[2] = ET[17].BCET;
		D[2] = 500000;	
		T[2] = 500000;

		C[3] = ET[21].WCET;
		B[3] = ET[21].BCET;
		D[3] = 1000000;	
		T[3] = 1000000;

	}
	else if(NUM_TASKS == 6 && util == 0.5){

		C[0] = ET[22].WCET;
		B[0] = ET[22].BCET;
		D[0] = 100000;	
		T[0] = 100000;

		C[1] = ET[2].WCET;
		B[1] = ET[2].BCET;
		D[1] = 400000;	
		T[1] = 400000;

		C[2] = ET[5].WCET;
		B[2] = ET[5].BCET;
		D[2] = 500000;	
		T[2] = 500000;

		C[3] = ET[10].WCET;
		B[3] = ET[10].BCET;
		D[3] = 1000000;	
		T[3] = 1000000;

		C[4] = ET[18].WCET;
		B[4] = ET[18].BCET;
		D[4] = 1000000;	
		T[4] = 1000000;

		C[5] = ET[25].WCET;
		B[5] = ET[25].BCET;
		D[5] = 2000000;	
		T[5] = 2000000;

	}
	else if(NUM_TASKS == 8 && util == 0.5){

		C[0] = ET[1].WCET;
		B[0] = ET[1].BCET;
		D[0] = 100000;	
		T[0] = 100000;

		C[1] = ET[2].WCET;
		B[1] = ET[2].BCET;
		D[1] = 400000;	
		T[1] = 400000;

		C[2] = ET[3].WCET;
		B[2] = ET[3].BCET;
		D[2] = 500000;	
		T[2] = 500000;

		C[3] = ET[10].WCET;
		B[3] = ET[10].BCET;
		D[3] = 800000;	
		T[3] = 800000;

		C[4] = ET[14].WCET;
		B[4] = ET[14].BCET;
		D[4] = 1000000;	
		T[4] = 1000000;

		C[5] = ET[17].WCET;
		B[5] = ET[17].BCET;
		D[5] = 2000000;	
		T[5] = 2000000;

		C[6] = ET[6].WCET;
		B[6] = ET[6].BCET;
		D[6] = 2000000;	
		T[6] = 2000000;

		C[7] = ET[26].WCET;
		B[7] = ET[26].BCET;
		D[7] = 4000000;	
		T[7] = 4000000;

	}
	else if(NUM_TASKS == 2 && util == 0.6){

		C[0] = ET[20].WCET;
		B[0] = ET[20].BCET;
		D[0] = 300000;	
		T[0] = 300000;

		C[1] = ET[26].WCET;
		B[1] = ET[26].BCET;
		D[1] = 500000;	
		T[1] = 500000;		

	}
	else if(NUM_TASKS == 4 && util == 0.6){

		C[0] = ET[0].WCET;
		B[0] = ET[0].BCET;
		D[0] = 50000;	
		T[0] = 50000;

		C[1] = ET[14].WCET;
		B[1] = ET[14].BCET;
		D[1] = 400000;	
		T[1] = 400000;

		C[2] = ET[7].WCET;
		B[2] = ET[7].BCET;
		D[2] = 500000;	
		T[2] = 500000;

		C[3] = ET[26].WCET;
		B[3] = ET[26].BCET;
		D[3] = 1000000;	
		T[3] = 1000000;

	}
	else if(NUM_TASKS == 6 && util == 0.6){

		C[0] = ET[2].WCET;
		B[0] = ET[2].BCET;
		D[0] = 100000;	
		T[0] = 100000;

		C[1] = ET[3].WCET;
		B[1] = ET[3].BCET;
		D[1] = 400000;	
		T[1] = 400000;

		C[2] = ET[5].WCET;
		B[2] = ET[5].BCET;
		D[2] = 500000;	
		T[2] = 100000;

		C[3] = ET[10].WCET;
		B[3] = ET[10].BCET;
		D[3] = 1000000;	
		T[3] = 1000000;

		C[4] = ET[18].WCET;
		B[4] = ET[18].BCET;
		D[4] = 1000000;	
		T[4] = 1000000;

		C[5] = ET[25].WCET;
		B[5] = ET[25].BCET;
		D[5] = 2000000;	
		T[5] = 2000000;

	}
	else if(NUM_TASKS == 8 && util == 0.6){

		C[0] = ET[1].WCET;
		B[0] = ET[1].BCET;
		D[0] = 100000;	
		T[0] = 100000;

		C[1] = ET[4].WCET;
		B[1] = ET[4].BCET;
		D[1] = 400000;	
		T[1] = 400000;

		C[2] = ET[5].WCET;
		B[2] = ET[5].BCET;
		D[2] = 500000;	
		T[2] = 500000;

		C[3] = ET[10].WCET;
		B[3] = ET[10].BCET;
		D[3] = 800000;	
		T[3] = 800000;

		C[4] = ET[14].WCET;
		B[4] = ET[14].BCET;
		D[4] = 1000000;	
		T[4] = 1000000;

		C[5] = ET[17].WCET;
		B[5] = ET[17].BCET;
		D[5] = 2000000;	
		T[5] = 2000000;

		C[6] = ET[6].WCET;
		B[6] = ET[6].BCET;
		D[6] = 2000000;	
		T[6] = 2000000;

		C[7] = ET[26].WCET;
		B[7] = ET[26].BCET;
		D[7] = 4000000;	
		T[7] = 4000000;

	}
	if(NUM_TASKS == 2 && util == 0.7){
	
		C[0] = ET[26].WCET;
		B[0] = ET[26].BCET;
		D[0] = 300000;	
		T[0] = 300000;
		
		C[1] = ET[20].WCET;
		B[1] = ET[20].BCET;
		D[1] = 500000;	
		T[1] = 500000;		
				
	}
	if(NUM_TASKS == 4 && util == 0.7){
	
		C[0] = ET[15].WCET;
		B[0] = ET[15].BCET;
		D[0] = 50000;	
		T[0] = 50000;
		
		C[1] = ET[8].WCET;
		B[1] = ET[8].BCET;
		D[1] = 400000;	
		T[1] = 400000;
		
		C[2] = ET[6].WCET;
		B[2] = ET[6].BCET;
		D[2] = 500000;	
		T[2] = 500000;
		
		C[3] = ET[26].WCET;
		B[3] = ET[26].BCET;
		D[3] = 1000000;	
		T[3] = 1000000;
		
	}
	if(NUM_TASKS == 6 && util == 0.7){
	
		C[0] = ET[2].WCET;
		B[0] = ET[2].BCET;
		D[0] = 100000;	
		T[0] = 100000;
		
		C[1] = ET[16].WCET;
		B[1] = ET[16].BCET;
		D[1] = 400000;	
		T[1] = 400000;
		
		C[2] = ET[7].WCET;
		B[2] = ET[7].BCET;
		D[2] = 500000;	
		T[2] = 500000;
		
		C[3] = ET[6].WCET;
		B[3] = ET[6].BCET;
		D[3] = 1000000;	
		T[3] = 1000000;
		
		C[4] = ET[19].WCET;
		B[4] = ET[19].BCET;
		D[4] = 1000000;	
		T[4] = 1000000;
		
		C[5] = ET[26].WCET;
		B[5] = ET[26].BCET;
		D[5] = 2000000;	
		T[5] = 2000000;
			
	}
	if(NUM_TASKS == 8 && util == 0.7){
	
		C[0] = ET[2].WCET;
		B[0] = ET[2].BCET;
		D[0] = 100000;	
		T[0] = 100000;
		
		C[1] = ET[4].WCET;
		B[1] = ET[4].BCET;
		D[1] = 400000;	
		T[1] = 400000;
		
		C[2] = ET[19].WCET;
		B[2] = ET[19].BCET;
		D[2] = 500000;	
		T[2] = 500000;
		
		C[3] = ET[10].WCET;
		B[3] = ET[10].BCET;
		D[3] = 800000;	
		T[3] = 800000;
		
		C[4] = ET[14].WCET;
		B[4] = ET[14].BCET;
		D[4] = 1000000;	
		T[4] = 1000000;
		
		C[5] = ET[18].WCET;
		B[5] = ET[18].BCET;
		D[5] = 2000000;	
		T[5] = 2000000;
				
		C[6] = ET[7].WCET;
		B[6] = ET[7].BCET;
		D[6] = 2000000;	
		T[6] = 2000000;
		
		C[7] = ET[25].WCET;
		B[7] = ET[25].BCET;
		D[7] = 4000000;	
		T[7] = 4000000;
		
	}	
	if(NUM_TASKS == 2 && util == 0.8){
	
		C[0] = ET[26].WCET;
		B[0] = ET[26].BCET;
		D[0] = 300000;	
		T[0] = 300000;
		
		C[1] = ET[27].WCET;
		B[1] = ET[27].BCET;
		D[1] = 500000;	
		T[1] = 500000;		
				
	}
	if(NUM_TASKS == 4 && util == 0.8){
	
		C[0] = ET[27].WCET;
		B[0] = ET[27].BCET;
		D[0] = 500000;	
		T[0] = 500000;
		
		C[1] = ET[12].WCET;
		B[1] = ET[12].BCET;
		D[1] = 500000;	
		T[1] = 500000;
		
		C[2] = ET[26].WCET;
		B[2] = ET[26].BCET;
		D[2] = 1000000;	
		T[2] = 1000000;
		
		C[3] = ET[18].WCET;
		B[3] = ET[18].BCET;
		D[3] = 2000000;	
		T[3] = 2000000;
		
	}
	if(NUM_TASKS == 6 && util == 0.8){
	
		C[0] = ET[20].WCET;
		B[0] = ET[20].BCET;
		D[0] = 400000;	
		T[0] = 400000;
		
		C[1] = ET[7].WCET;
		B[1] = ET[7].BCET;
		D[1] = 500000;	
		T[1] = 500000;
		
		C[2] = ET[19].WCET;
		B[2] = ET[19].BCET;
		D[2] = 500000;	
		T[2] = 500000;
		
		C[3] = ET[12].WCET;
		B[3] = ET[12].BCET;
		D[3] = 1000000;	
		T[3] = 1000000;
		
		C[4] = ET[24].WCET;
		B[4] = ET[24].BCET;
		D[4] = 1000000;	
		T[4] = 1000000;
		
		C[5] = ET[18].WCET;
		B[5] = ET[18].BCET;
		D[5] = 2000000;	
		T[5] = 2000000;
			
	}
	if(NUM_TASKS == 8 && util == 0.8){
	
		C[0] = ET[7].WCET;
		B[0] = ET[7].BCET;
		D[0] = 400000;
		T[0] = 400000;
		
		C[1] = ET[25].WCET;
		B[1] = ET[25].BCET;
		D[1] = 500000;	
		T[1] = 500000;
		
		C[2] = ET[19].WCET;
		B[2] = ET[19].BCET;
		D[2] = 800000;	
		T[2] = 800000;
		
		C[3] = ET[14].WCET;
		B[3] = ET[14].BCET;
		D[3] = 800000;	
		T[3] = 800000;
		
		C[4] = ET[8].WCET;
		B[4] = ET[8].BCET;
		D[4] = 1000000;	
		T[4] = 1000000;
		
		C[5] = ET[10].WCET;
		B[5] = ET[10].BCET;
		D[5] = 2000000;	
		T[5] = 2000000;
				
		C[6] = ET[7].WCET;
		B[6] = ET[7].BCET;
		D[6] = 2000000;	
		T[6] = 2000000;
		
		C[7] = ET[20].WCET;
		B[7] = ET[20].BCET;
		D[7] = 4000000;	
		T[7] = 4000000;
		
	}	
	if(NUM_TASKS == 10 && util == 0.8){
	
		C[0] = ET[9].WCET;
		B[0] = ET[9].BCET;
		D[0] = 100000;	
		T[0] = 100000;
		
		C[1] = ET[7].WCET;
		B[1] = ET[7].BCET;
		D[1] = 625000;	
		T[1] = 625000;
		
		C[2] = ET[14].WCET;
		B[2] = ET[14].BCET;
		D[2] = 625000;	
		T[2] = 625000;
		
		C[3] = ET[8].WCET;
		B[3] = ET[8].BCET;
		D[3] = 625000;	
		T[3] = 625000;
		
		C[4] = ET[4].WCET;
		B[4] = ET[4].BCET;
		D[4] = 1000000;	
		T[4] = 1000000;
		
		C[5] = ET[10].WCET;
		B[5] = ET[10].BCET;
		D[5] = 1000000;	
		T[5] = 1000000;
				
		C[6] = ET[19].WCET;
		B[6] = ET[19].BCET;
		D[6] = 1250000;	
		T[6] = 1250000;
		
		C[7] = ET[26].WCET;
		B[7] = ET[26].BCET;
		D[7] = 1250000;	
		T[7] = 1250000;
		
		C[8] = ET[21].WCET;
		B[8] = ET[21].BCET;
		D[8] = 2500000;	
		T[8] = 2500000;
		
		C[9] = ET[16].WCET;
		B[9] = ET[16].BCET;
		D[9] = 5000000;	
		T[9] = 5000000;
		
	}
}
