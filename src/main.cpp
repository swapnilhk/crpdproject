#include<set>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>
#include"baseConfig.h"
#include"set_operations.cpp"
#include"global.h"
#include"lee.cpp"
#include"kd.cpp"
//#include"kd2.cpp"
#include"uniformDistribution.cpp"
#include"kd3.cpp"
//#include"uniform_fixed_tasks_ecb_ucb_schemes.cpp"

struct map{
	char * methodName;
	int (*func)();
}methodsMap[NUM_METHODS];


void init(){
	methodsMap[NO_PREEMPT] = (struct map){"NO_PREEMPT", NULL};
	methodsMap[ECB_ONLY] = (struct map){"ECB_ONLY", NULL};
	methodsMap[UCB_ONLY] = (struct map){"ECB_ONLY", NULL};
	methodsMap[UCB_UNION] = (struct map){"UCB_UNION", NULL};
	methodsMap[ECB_UNION] = (struct map){"ECB_UNION", NULL};
	methodsMap[ECB_UNION_MULTISET] = (struct map){"ECB_UNION_MULTISET", NULL};
	methodsMap[UCB_UNION_MULTISET] = (struct map){"UCB_UNION_MULTISET", NULL};
	methodsMap[ECB_UCB_UNION_MULTISET_COMBINED] = (struct map){"ECB_UCB_UNION_MULTISET_COMBINED", NULL};
	methodsMap[ECB_UNION_MULTISET_PRE] = (struct map){"ECB_UNION_MULTISET_PRE", NULL};
	methodsMap[UCB_UNION_MULTISET_PRE] = (struct map){"UCB_UNION_MULTISET_PRE", NULL};
	methodsMap[ECB_UCB_UNION_MULTISET_COMBINED_PRE] = (struct map){"ECB_UCB_UNION_MULTISET_COMBINED_PRE", NULL};
	methodsMap[STASCHULAT] = (struct map){"STASCHULAT", NULL};
	methodsMap[STASCHULAT_PRE] = (struct map){"STASCHULAT_PRE", NULL};
	methodsMap[PRE_MAX] = (struct map){"PRE_MAX", NULL};
	methodsMap[PRE_MAX_KD] = (struct map){"PRE_MAX_KD", ResponseTimePreMaxKd};
	methodsMap[PRE_MAX_KD2] = (struct map){"PRE_MAX_KD2", NULL};
	methodsMap[PRE_MAX_KD3] = (struct map){"PRE_MAX_KD3", ResponseTimePreMaxKd3};
	methodsMap[LEE_WODC] = (struct map){"LEE_WODC", ResponseTimeLeeWodc};
	methodsMap[LEE_WDC] = (struct map){"LEE_WDC", ResponseTimeLeeWdc};
}

using namespace std;

void clearTaskExecutionStatistics(){
     int i;     
     for(i = 0; i < NUM_METHODS; i++)
          Num_Executed_Tasks[i] = 0;
}

void printTaskExecutionStatistics(FILE *fp){
	int i;
	static int heading = 0;
	if(VERBOSE) printf("Printing task execution statistics\n");
     	if(heading == 0){
	     	fprintf(fp, "%-4s\t%-32s\t%s\n", "Util", "Method", "No. of Sched tasks");
	     	heading = 1;
     	}
	else fprintf(fp, "\n");
    	fprintf(fp, "%-4.2g", util);
     	for(int methodIndex = NO_PREEMPT; methodIndex < NUM_METHODS; methodIndex++){
	     	if(methodsMap[methodIndex].func != NULL){
			char str[50] = "\0";
			sprintf(str, "\t%-32s\t%d\n", methodsMap[methodIndex].methodName , Num_Executed_Tasks[methodIndex]);           	
			fprintf(fp, "%s", str);
			if(VERBOSE)
				printf("%s", str);
		}
     	}
	fflush(fp);
}

int CALL_METHODS(){
	int WDC, sched;
	int sched_vector = 0;

	for(int methodIndex = 0; methodIndex < NUM_METHODS; methodIndex++){
		if(methodsMap[methodIndex].func != NULL){
			if(VERBOSE)
				printf("\t%s\n", methodsMap[methodIndex].methodName);		
			sched = (methodsMap[methodIndex].func)();
			sched_vector |= sched << methodIndex;
		}
	}
	return sched_vector;
}

void updateDominationMatrix(int schedVector, int dom[]){
	for(int methodIndex = 0; methodIndex < NUM_METHODS; methodIndex++)
		if(schedVector >> methodIndex & 1)// if(sched_vectpr has bit corresponding to the method set)
			dom[methodIndex] |= ~schedVector; // Add to dom[this_method] all methods that this_method domonates, ie are zero
}

void printDominationInfo(int dom[], FILE *fp){
	for(int method1 = 0; method1 < NUM_METHODS - 1; method1++){
		if(methodsMap[method1].func != NULL){
			for(int method2 = method1 + 1; method2 < NUM_METHODS; method2++){
				if(methodsMap[method2].func != NULL){
					//Checking if method1 dominates method2
					if((dom[method1] >> method2 & 1) && !(dom[method2] >> method1 & 1))
						fprintf(fp, "%s dominates %s\n", methodsMap[method1].methodName, methodsMap[method2].methodName);
					//Checking if method2 dominates method1
					else if((dom[method2] >> method1 & 1) && !(dom[method1] >> method2 & 1))
						fprintf(fp, "%s is dominated by %s\n", methodsMap[method1].methodName, methodsMap[method2].methodName);
					//Checking if method1 and method2 are equal
					else if(!(dom[method2] >> method1 & 1) && !(dom[method1] >> method2 & 1))
						fprintf(fp, "%s and %s are equal\n", methodsMap[method1].methodName, methodsMap[method2].methodName);
					//If all above conditions fail, method1 and method2 are not comparable
					else fprintf(fp, "%s and %s are not comparable\n", methodsMap[method1].methodName, methodsMap[method2].methodName);
				}
			}		
		}
	}
}

void uniformDistributionBenchmark(FILE *fp)
{
	int dom[NUM_METHODS] = {0};
	fprintf(fp, "Uniform Distribution Benchmark\n");
	initUniformDistributionBenchmark(fp);
	for(util = UTIL_START; util <= UTIL_END; util += UTIL_INCREMENT){
		if(VERBOSE) printf("The total util is %f\n", util);
		clearTaskExecutionStatistics();		
		for(int taskSetNo = 1; taskSetNo <= NUM_TASK_SETS; taskSetNo++){
			int schedVector;//vector of bits with each bit corresponding to each method
			VERBOSE = 1;
			if(NUM_TASK_SETS >= 10 && taskSetNo % (NUM_TASK_SETS/10) != 0)//To reduce the output messages on stdout to to 1/101h
				VERBOSE = 0;
			if(VERBOSE)
				printf("Task set no: %d Util: %.2f\n",taskSetNo, util);
			createTaskSetUniformDistribution(util, MIN_PERIOD, MAX_PERIOD);
			schedVector = CALL_METHODS();
			updateDominationMatrix(schedVector, dom);
		}
		if(VERBOSE) printf("\n");
	        printTaskExecutionStatistics(fp);
	}	
	printDominationInfo(dom, fp);
}

int main() {
	FILE *fp = NULL;
	init();
	fp = fopen("out/statistics.txt", "w");
	if(fp == NULL){
		printf("***Unable to open file\n");
		exit(0);
	}	
	if(MESSAGE_LEVEL >= IMP) printBaseConfig(fp);
	uniformDistributionBenchmark(fp);
	fclose(fp);
}