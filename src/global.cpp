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

void printBaseConfig(FILE *fp){
	fprintf(fp, "\nBase Config\n");	
	fprintf(fp, "Cache size = %d\n", CACHE_SIZE);
	fprintf(fp, "BRT = %g\n", BRT);
	fprintf(fp, "RF = %g\n", RF);
	fprintf(fp, "Num Tasks = %d\n", NUM_TASKS);
	fprintf(fp, "Num Task Sets = %d\n\n", NUM_TASK_SETS);
	fflush(fp);
}

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

void print_SET(std::set<int> & Set1, FILE *fp){
    std::set<int>::iterator it;
    for(it=Set1.begin(); it!= Set1.end(); ++it)       
        fprintf(fp, "%d ", *it);
    fprintf(fp, "\n");
}

void print_ecbs(FILE *fp){
	int i;
	fprintf(fp, "\nECB Sets\n");	
	for(i = 0; i < NUM_TASKS; i++){
		fprintf(fp, "ECB Set %d : \n", i);
		print_SET(TASK_ECB[i], fp);
		fprintf(fp, "\n");
	}
}

void print_ucbs(FILE *fp){
	int i;
	fprintf(fp, "\nUCB Sets\n");	
	for(i = 0; i < NUM_TASKS; i++){
		fprintf(fp, "UCB Set %d : \n", i);
		print_SET(TASK_UCB[i], fp);
		fprintf(fp, "\n");
	}
}

int** Make2DintArrayInt(int arraySizeX, int arraySizeY){	
    int i;
    int** theArray;
    theArray = (int**) malloc(arraySizeX*sizeof(int*));
    for (i = 0; i < arraySizeX; i++)
        theArray[i] = (int*) malloc(arraySizeY*sizeof(int));
    return theArray;
}

double** Make2DintArrayDouble(int arraySizeX, int arraySizeY){	
    int i;
    double** theArray;
    theArray = (double**) malloc(arraySizeX*sizeof(double*));
    for (i = 0; i < arraySizeX; i++)
        theArray[i] = (double*) malloc(arraySizeY*sizeof(double));
    return theArray;
}

long** Make2DintArrayLong(int arraySizeX, int arraySizeY){	
    int i;
    long** theArray;
    theArray = (long**) malloc(arraySizeX*sizeof(long*));
    for (i = 0; i < arraySizeX; i++)
        theArray[i] = (long*) malloc(arraySizeY*sizeof(long));
    return theArray;
}

void free2DintArrayInt(int ** twoDArray, int n){
	while(n--)
		free(twoDArray[n]);
	free(twoDArray);
}

double sigmaTda(int thisTask, double Response[]){
	double RNew = 0;
	int hpTask = thisTask - 1;
	while(hpTask >= 0){
		RNew += ceil(Response[thisTask]/T[hpTask]) * C[hpTask];
		hpTask = hpTask - 1;
	}
	return RNew;
}

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
