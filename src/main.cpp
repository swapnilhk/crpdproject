//#include"uniform_fixed_tasks_ecb_ucb_schemes.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"global.h"
#include"benchmarks.h"
#include"kd.h"
#include"lee.h"
#include"ramaprasad_mueller.h"

struct map{
	char * methodName;
	int (*func)();
}methodsMap[NUM_METHODS];

void init(){	
	methodsMap[NO_PREEMPT] = (struct map){"NO_PREEMPT", NULL/*Response_time_NO_PREEMPT*/};
	methodsMap[ECB_ONLY] = (struct map){"ECB_ONLY", NULL/*Response_time_ECB_Only*/};
	methodsMap[UCB_ONLY] = (struct map){"UCB_ONLY", NULL/*Response_time_UCB_Only*/};
	methodsMap[UCB_UNION] = (struct map){"UCB_UNION", NULL/*Response_time_UCB_Union*/};
	methodsMap[ECB_UNION] = (struct map){"ECB_UNION", NULL/*Response_time_ECB_Union*/};
	methodsMap[ECB_UNION_MULTISET] = (struct map){"ECB_UNION_MULTISET", NULL/*Response_time_ECB_Union_Multiset*/};
	methodsMap[UCB_UNION_MULTISET] = (struct map){"UCB_UNION_MULTISET", NULL/*Response_time_UCB_Union_Multiset*/};
	methodsMap[ECB_UCB_UNION_MULTISET_COMBINED] = (struct map){"ECB_UCB_UNION_MULTISET_COMBINED", NULL/*Response_time_ECB_UCB_Union_Multiset_Combined*/};
	methodsMap[ECB_UNION_MULTISET_PRE] = (struct map){"ECB_UNION_MULTISET_PRE", NULL/*Response_time_ECB_Union_Multiset_PRE*/};
	methodsMap[UCB_UNION_MULTISET_PRE] = (struct map){"UCB_UNION_MULTISET_PRE", NULL/*Response_time_UCB_Union_Multiset_PRE*/};
	methodsMap[ECB_UCB_UNION_MULTISET_COMBINED_PRE] = (struct map){"ECB_UCB_UNION_MULTISET_COMBINED_PRE", NULL/*Response_time_ECB_UCB_Union_Multiset_Combined_PRE*/};
	methodsMap[STASCHULAT] = (struct map){"STASCHULAT", NULL/*Response_time_Staschulat*/};
	methodsMap[STASCHULAT_PRE] = (struct map){"STASCHULAT_PRE", NULL/*Response_time_Staschulat_PRE*/};
	methodsMap[PRE_MAX] = (struct map){"PRE_MAX", NULL/*Response_time_PRE_MAX*/};
	methodsMap[PRE_MAX_KD] = (struct map){"PRE_MAX_KD", /*NULL*/ResponseTimePreMaxKd};
	methodsMap[PRE_MAX_KD2] = (struct map){"PRE_MAX_KD2", /*NULL*/ResponseTimePreMaxKd2};
	methodsMap[LEE_WODC] = (struct map){"LEE_WODC", /*NULL*/ResponseTimeLeeWodc};
	methodsMap[LEE_WDC] = (struct map){"LEE_WDC", /*NULL*/ResponseTimeLeeWdc};
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
	if(VERBOSE) printf("Task execution statistics\n");
	if(heading == 0){
		fprintf(fp, "%-4s\t%-32s\t%s\n", "Util", "Method", "No. of Sched tasks");
	   	heading = 1;
    	}
	if(VERBOSE) fprintf(stdout, "%-4s\t%-32s\t%s\n", "Util", "Method", "No. of Sched tasks");
   	for(int methodIndex = NO_PREEMPT; methodIndex < NUM_METHODS; methodIndex++)
     		if(methodsMap[methodIndex].func != NULL){
			char str[50] = "\0";
			sprintf(str, "%-4.2g\t%-32s\t%d\n", util, methodsMap[methodIndex].methodName , Num_Executed_Tasks[methodIndex]);           	
			fprintf(fp, "%s", str);
			if(VERBOSE)	fprintf(stdout, "%s", str);
		}
	fflush(fp);
}

int CALL_METHODS(){
	int WDC, sched;
	int sched_vector = 0;
	for(int methodIndex = 0; methodIndex < NUM_METHODS; methodIndex++)
		if(methodsMap[methodIndex].func != NULL){
			if(VERBOSE) fprintf(stdout, "%s\n", methodsMap[methodIndex].methodName);
			sched = (methodsMap[methodIndex].func)();
			sched_vector |= sched << methodIndex;
		}
	return sched_vector;
}

void updateDominationMatrix(int schedVector, int dom[]){
	for(int methodIndex = 0; methodIndex < NUM_METHODS; methodIndex++)
		if(schedVector >> methodIndex & 1)// if(sched_vectpr has bit corresponding to the method set)
			dom[methodIndex] |= ~schedVector; // Add to dom[this_method] all methods that this_method domonates, ie are zero
}

void printDominationInfo(int dom[], FILE *fp){
	for(int method1 = 0; method1 < NUM_METHODS - 1; method1++)
		if(methodsMap[method1].func != NULL)
			for(int method2 = method1 + 1; method2 < NUM_METHODS; method2++)
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

void uniformDistributionBenchmark(FILE *fp){
	int dom[NUM_METHODS] = {0};
	fprintf(fp, "Uniform Distribution Benchmark\n");
	if(VERBOSE) fprintf(stdout, "Uniform Distribution Benchmark\n");		
	if(initBenchmark(4/* = NUM_TASKS, TODO: read from config file*/, fp)){
		for(util = UTIL_START; util <= UTIL_END; util += UTIL_INCREMENT){
			if(VERBOSE) printf("The total util is %f\n", util);
			clearTaskExecutionStatistics();
			for(int taskSetNo = 1; taskSetNo <= NUM_TASK_SETS; taskSetNo++){
				int schedVector;//vector of bits with each bit corresponding to each method
				if(VERBOSE) fprintf(stdout, "Task set no: %d Util: %.2f\n",taskSetNo, util);
				createTaskSetUniformDistribution(util, MIN_PERIOD, MAX_PERIOD, fp);
				schedVector = CALL_METHODS();
				updateDominationMatrix(schedVector, dom);
			}
			if(VERBOSE) printf("\n");
			printTaskExecutionStatistics(fp);
		}	
		printDominationInfo(dom, fp);
		if(VERBOSE)printDominationInfo(dom, stdout);
		freeBenchmark();
	}
}

void constantValuesBenchmark(FILE *fp){
	int dom[NUM_METHODS] = {0};	
	fprintf(fp, "Constant Values Banchmark\n");
	if(VERBOSE) fprintf(stdout, "Constant Values Banchmark\n");
	for(util = 0.5; util <= 0.6; util += 0.1){			
		for(int numTasks = 2; numTasks <= (util == 0.8 ? 10 : 2); numTasks+=2){			
			if(initBenchmark(numTasks, fp)){
				int schedVector;//vector of bits with each bit corresponding to each method				
				fprintf(fp, "NUM_TASKS = %d util is %f\n", NUM_TASKS, util);
				if(VERBOSE)fprintf(stdout, "NUM_TASKS = %d util is %f\n", NUM_TASKS, util);
				clearTaskExecutionStatistics();
				createTaskSetConstantValues(numTasks, util);
				schedVector = CALL_METHODS();
				updateDominationMatrix(schedVector, dom);
				printTaskExecutionStatistics(fp);
				printDominationInfo(dom, fp);
				if(VERBOSE)printDominationInfo(dom, stdout);
				freeBenchmark();
			}
		}
		if(VERBOSE) printf("\n");
	}
}

int main(int argc, char * argv[]) {
	FILE *fp = NULL;
	char * filename = "out/statistics.txt";
	for(int i = 1; i < argc; i++){
		if(!strcmp(argv[i], "-v"))
			VERBOSE = 1;
		else if(!strcmp(argv[i], "-a"))
			MESSAGE_LEVEL = ALL;
		else if(!strcmp(argv[i], "-i"))
			MESSAGE_LEVEL = IMP;
		else if(!strcmp(argv[i], "-n"))
			MESSAGE_LEVEL = NONE;
		else{
			fprintf(stderr, "***Invalid parameter %s\nUsage\n-v Verbose\n-a Message Level All\n-i Message Level Imp\n-n Message Level None\n", argv[i]);
			exit(1);
		}		
	}
	init();
	fp = fopen(filename, "w");
	if(fp == NULL){
		fprintf(stderr, "***Unable to open file %s\n", filename);
		exit(1);
	}	
	if(MESSAGE_LEVEL >= IMP) printBaseConfig(fp);
	uniformDistributionBenchmark(fp);
	constantValuesBenchmark(fp);
	fclose(fp);
}
