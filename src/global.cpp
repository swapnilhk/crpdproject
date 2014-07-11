#include"global.h"
#include<math.h>
#include<set>
#include<stdlib.h>

int NUM_TASKS;
int CACHE_SIZE;
int NUM_TASK_SETS;
double UTIL_START;
double UTIL_INCREMENT;
double UTIL_END;
int MIN_PERIOD;
int MAX_PERIOD;
double *C;//Worst case execution time
double *B;//Best case execution time
long *T;// Period
long *D;// Deadline
double util;
std::set<int> *TASK_ECB, *TASK_UCB;
int Num_Executed_Tasks[NUM_METHODS];
int MESSAGE_LEVEL = NONE;
int VERBOSE = 0;

/**
 * @brief Prints the variables that from the base config
 * @param fp :- Pointer to the file where values are to be
 * written
 */
void printBaseConfig(FILE *fp){
	fprintf(fp, "\nBase Config\n");	
	fprintf(fp, "Cache size = %d\n", CACHE_SIZE);
	fprintf(fp, "BRT = %g\n", BRT);
	fprintf(fp, "RF = %g\n", RF);
	fprintf(fp, "Num Tasks = %d\n", NUM_TASKS);
	fprintf(fp, "Num Task Sets = %d\n", NUM_TASK_SETS);
	fprintf(fp, "MIN_PERIOD = %d\n", MIN_PERIOD);
	fprintf(fp, "MAX_PERIOD = %d\n\n", MAX_PERIOD);
	fflush(fp);
}

/**
 * @brief Prints the parameters on the task in the set
 * @param fp :- Pointer to the file where values are to be
 * written
 */
void printTaskInfo(FILE *fp){
	int i;
	double util = 0;
	fprintf(fp, "\nTask Info\nTask\tExectime\tPeriod\tDeadline\n");	
	for(i = 0; i < NUM_TASKS; i++){
		fprintf(fp, "%d\t%8.4g\t%ld\t%ld\n", i, C[i], T[i], D[i]);
		util += ( C[i] / T[i]);
	}
	fprintf(fp, "Util = %f\n\n", util);
	fflush(fp);
}

/**
 * @brief Prints all the elements of a set
 * @param Set1 :- The set whose element are to be printed
 * @param fp :- Pointer to the file where values are to be
 * written
 */
void print_SET(std::set<int> & Set1, FILE *fp){
    std::set<int>::iterator it;
    for(it=Set1.begin(); it!= Set1.end(); ++it)       
        fprintf(fp, "%d ", *it);
    fprintf(fp, "\n");
}

/**
 * @brief Prints ECBs of all tasks in the set
 * @param fp :- Pointer to the file where values are to be
 * written
 */
void print_ecbs(FILE *fp){
	int i;
	fprintf(fp, "\nECB Sets\n");	
	for(i = 0; i < NUM_TASKS; i++){
		fprintf(fp, "ECB Set %d : \n", i);
		print_SET(TASK_ECB[i], fp);
		fprintf(fp, "\n");
	}
}

/**
 * @brief Prints UCBs of all tasks in the set
 * @param fp :- Pointer to the file where values are to be
 * written
 */
void print_ucbs(FILE *fp){
	int i;
	fprintf(fp, "\nUCB Sets\n");	
	for(i = 0; i < NUM_TASKS; i++){
		fprintf(fp, "UCB Set %d : \n", i);
		print_SET(TASK_UCB[i], fp);
		fprintf(fp, "\n");
	}
}

/**
 * @brief Creates a two dimentional array of integers
 * @param arraySizeX :- Number of rows in the array
 * @param arraySizeY :- Number of columns in the array
 * @retuen pointes to the two dimentional array created
 */
int** Make2DintArrayInt(int arraySizeX, int arraySizeY){	
    int i;
    int** theArray;
    theArray = (int**) malloc(arraySizeX*sizeof(int*));
    for (i = 0; i < arraySizeX; i++)
        theArray[i] = (int*) malloc(arraySizeY*sizeof(int));
    return theArray;
}

/**
 * @brief Frees the memory assigned to a two dimentional array
 * of integers
 * @param twoDArray :- The two dimentional array to be freed
 * @param n :- Number of columns in the matrix
 */
void free2DintArrayInt(int ** twoDArray, int n){
	while(n--)
		free(twoDArray[n]);
	free(twoDArray);
}

/**
 * @brief Sumation function of the Time Demand Analysis equation
 * @param thisTask :- The task under consideration
 * @param Response:- Array containing the response times of all
 * the tasks
 * @return Value of the new response time calculated
 */
double sigmaTda(int thisTask, double Response[]){
	double RNew = 0;
	int hpTask = thisTask - 1;
	while(hpTask >= 0){
		RNew += ceil(Response[thisTask]/T[hpTask]) * C[hpTask];
		hpTask = hpTask - 1;
	}
	return RNew;
}

/**
 * @brief This function implements the complete time demand
 * equation and the iteartions involved in its solution
 * @param thisTask :- The task under consideration
 * @param Response:- Array containing the response times of
 * all the tasks
 * @param fp :- Pointer to the file where intermediate logs
 * are to be written
 * @param PC :- Pointer to the function to calculate the
 * preemption cost in the Time Demand Analysis equation
 * @return The response time for this task.
 */
double wcrt(int thisTask, double Response[], FILE *fp, double (*PC)(int, double[], FILE*)){
	double RNew;
	RNew = C[thisTask];
	Response[thisTask] = 0;
	while(RNew != Response[thisTask] && (Response[thisTask] = RNew) <= D[thisTask]){
		RNew = C[thisTask] 
			+ sigmaTda(thisTask, Response) 
			+ PC(thisTask, Response, fp);// Time demand equation
		if(MESSAGE_LEVEL >= IMP)
			fprintf(fp, "T%d(D=%ld) Response time: Old = %g, New = %g\n\n", thisTask, D[thisTask], Response[thisTask], RNew);
	}
	return RNew;
}
