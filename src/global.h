#ifndef HEADER_COMMON
#define HEADER_COMMON

#include"baseConfig.h"
#include<set>

double C[NUM_TASKS];
long   D[NUM_TASKS];
long   T[NUM_TASKS];
double util;

enum METHOD_NAMES {
	NO_PREEMPT,
	ECB_ONLY,
	UCB_ONLY,
	UCB_UNION,
	ECB_UNION,
	ECB_UNION_MULTISET,
	UCB_UNION_MULTISET,
	ECB_UCB_UNION_MULTISET_COMBINED,
	ECB_UNION_MULTISET_PRE,
	UCB_UNION_MULTISET_PRE,
	ECB_UCB_UNION_MULTISET_COMBINED_PRE,
	STASCHULAT,
	STASCHULAT_PRE,
	PRE_MAX,
	PRE_MAX_KD,
	PRE_MAX_KD2,
	PRE_MAX_KD3,
	PRE_MAX_KD4,
	LEE_WODC,
	LEE_WDC,
	NUM_METHODS
};

std::set<int> TASK_ECB[NUM_TASKS], TASK_UCB[NUM_TASKS];

int Num_Executed_Tasks[NUM_METHODS];

void printBaseConfig(FILE *fp){
	fprintf(fp, "\nBase Config\n");	
	fprintf(fp, "CACHE SIZE = %d\n", CACHE_SIZE);
	fprintf(fp, "BRT = %g\n", BRT);
	fprintf(fp, "BRT = %g\n", RF);
	fprintf(fp, "NUM_TASKS = %d\n\n", NUM_TASKS);
	fflush(fp);
}

void printTaskInfo(FILE *fp){
	int i;
	double util = 0;
	fprintf(fp, "\nTASK INFO\nTask\tExectime\tPeriod\tDeadline\n");	
	for(i = 0; i < NUM_TASKS; i++){
		fprintf(fp, "%d\t%8.4g\t%ld\t%ld\n", i, C[i], T[i], D[i]);
		util += ( C[i] / T[i]);
	}
	fprintf(fp, "Util = %f\n\n", util);
	fflush(fp);
}

void print_ecbs(FILE *fp){
	int i;
	fprintf(fp, "\nECB Sets\n");	
	for(i = 0; i < NUM_TASKS; i++){
		fprintf(fp, "ECB Set %d : \n", i);
		print_SET(TASK_ECB[i], fp);
		fprintf(fp, "\n");
	}
	fflush(fp);
}

void print_ucbs(FILE *fp){
	int i;
	fprintf(fp, "\nUCB Sets\n");	
	for(i = 0; i < NUM_TASKS; i++){
		fprintf(fp, "UCB Set %d : \n", i);
		print_SET(TASK_UCB[i], fp);
		fprintf(fp, "\n");
	}	
	fflush(fp);
}
#endif
