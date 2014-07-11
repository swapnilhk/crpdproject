#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"global.h"
#include"benchmarks.h"
#include"kd.h"
#include"lee.h"
#include"ramaprasad_mueller.h"

using namespace std;

/**
 * @brief This structure maps the indexes in enum 'METHOD_NAMES'
 * to strings containing corresponding method name and pointer
 * to corresponding function.
 */
struct map{
	char * methodName;
	int (*func)();
}methodsMap[NUM_METHODS];

/**
 * @brief This function reads base configuration from file 'config'
 * @return 1 if successful, 0 if unable to open file
 */
static int readConfig(void){
	FILE *fp = fopen("config", "r");
	if(fp == NULL){
		fprintf(stderr, "Unable to open config file\n");
		return 0;
	}
	char *line = (char*)malloc(15);
	int len;
	size_t n;
	n = 50;
	while((len = getline(&line, &n, fp)) != -1){
		if(line[len-1] == '\n')
			line[len-1] = '\0';
		char *paramValue = strchr(line ,'=');
		*paramValue = '\0';
		paramValue++;
		char *paramName = line;
		if(!strcmp(paramName,"NUM_TASKS"))
			NUM_TASKS = atoi(paramValue);
		if(!strcmp(paramName,"CACHE_SIZE"))
			CACHE_SIZE = atoi(paramValue);
		if(!strcmp(paramName,"NUM_TASK_SETS"))
			NUM_TASK_SETS = atoi(paramValue);
		if(!strcmp(paramName,"UTIL_START"))
			UTIL_START = atof(paramValue);
		if(!strcmp(paramName,"UTIL_INCREMENT"))
			UTIL_INCREMENT = atof(paramValue);
		if(!strcmp(paramName,"UTIL_END"))
			UTIL_END = atof(paramValue);
		if(!strcmp(paramName,"MIN_PERIOD"))
			MIN_PERIOD = atof(paramValue);
		if(!strcmp(paramName,"MAX_PERIOD"))
			MAX_PERIOD = atof(paramValue);
	}
	return 1;
}

/**
 * @brief This function sets the structure 'struct map' with 
 * proper values.
 * @return 1 if reading base config is successful, 0 if
 * failure
 */
static int init(void){
	methodsMap[PRE_MAX_KD] = (struct map){"PRE_MAX_KD", ResponseTimePreMaxKd};
	methodsMap[PRE_MAX_KD2] = (struct map){"PRE_MAX_KD2", ResponseTimePreMaxKd2};
	methodsMap[LEE_WODC] = (struct map){"LEE_WODC", ResponseTimeLeeWodc};
	methodsMap[LEE_WDC] = (struct map){"LEE_WDC", ResponseTimeLeeWdc};
	methodsMap[RAMAPRASAD_MUELLER] = (struct map){"RAMAPRASAD_MUELLER", ramaprasadMueller};
	return readConfig();	
}

/**
 * @brief Clears the array 'Num_Executed_Tasks'
 */
static void clearTaskExecutionStatistics(){
     int i;     
     for(i = 0; i < NUM_METHODS; i++)
          Num_Executed_Tasks[i] = 0;
}

/**
 * @brief Prints task execution statistics, i.e. number of executable
 * task sets by each method, into output file in proper format.
 * @param fp :- file pointer to the output file
 */
static void printTaskExecutionStatistics(FILE *fp){
	int i;
	static int heading = 0;
	if(VERBOSE) printf("Task execution statistics\n");
	if(heading == 0){
		fprintf(fp, "%-4s\t%-32s\t%s\n", "Util", "Method", "No. of Sched tasks");
	   	heading = 1;
    }
	if(VERBOSE) fprintf(stdout, "%-4s\t%-32s\t%s\n", "Util", "Method", "No. of Sched tasks");
	fprintf(fp, "\n");
   	for(int methodIndex = 0; methodIndex < NUM_METHODS; methodIndex++)
    	if(methodsMap[methodIndex].func != NULL){
			char str[50] = "\0";
			sprintf(str, "%-4.2g\t%-32s\t%d\n", util, methodsMap[methodIndex].methodName , Num_Executed_Tasks[methodIndex]);           	
			fprintf(fp, "%s", str);
			if(VERBOSE)	fprintf(stdout, "%s", str);
		}
	fflush(fp);
}

/**
 * @brief This method calls each method and collects their status into
 * variable 'sched_vector'. Each method sets/resets corresponding bits
 * of the variable 'sched_vector' according to the method's schedulab-
 * ility status.
 * @return vector (of bits) containing the schedulability status of all
 * the methods.
*/
static int CALL_METHODS(){
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

/**
 * @brief This function updated the domination relation between the
 * methods. It recieves the schedulability status of all the methods
 * It uses this information to update the domination relation.
 * @param schedVector :- Vector (of bits) containing schedulability
 * status of all the methods
 * @param dom :- Matrix (of bits) containing the domination relation
 * which needs to be updated.
*/
static void updateDominationMatrix(int schedVector, int dom[]){
	for(int methodIndex = 0; methodIndex < NUM_METHODS; methodIndex++)
		if(schedVector >> methodIndex & 1)// if(sched_vectpr has bit corresponding to the method set)
			dom[methodIndex] |= ~schedVector; // Add to dom[this_method] all methods that this_method domonates, ie are zero
}

/**
 * @brief This function prints the domination relation from the matrix
 * (of bits) and writes it into a file.
 * @param dom :- matrix (of bits) containing the domination relation
 * @param fp :- File pointed to the file where domination relation is
 * to be printed
 */
static void printDominationInfo(int dom[], FILE *fp){
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

/**
 * @brief This function generates uniformly distributed task sets.
 * Each generated task set is tested againt every method and the
 * domination elation between the methods is updated.
 * @param fp :- File pointer to the file in which the final output
 * is to be written.
 */
static void uniformDistributionBenchmark(FILE *fp){
	int dom[NUM_METHODS] = {0};
	fprintf(fp, "Uniform Distribution Benchmark\n");
	if(VERBOSE) fprintf(stdout, "Uniform Distribution Benchmark\n");		
	readConfig();
	if(initBenchmark(NUM_TASKS, fp)){
		printBaseConfig(fp);
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

int main(int argc, char * argv[]) {
	FILE *fp = NULL;
	char * filename = "out/statistics.txt";
	
	for(int i = 1; i < argc; i++){
		if(!strcmp(argv[i], "-v"))VERBOSE = 1;
		else if(!strcmp(argv[i], "-a"))MESSAGE_LEVEL = ALL;
		else if(!strcmp(argv[i], "-i"))MESSAGE_LEVEL = IMP;
		else if(!strcmp(argv[i], "-n"))MESSAGE_LEVEL = NONE;
		else{
			fprintf(stderr, "***Invalid parameter %s\nUsage\n-v Verbose\n-a Message Level All\n-i Message Level Imp\n-n Message Level None\n", argv[i]);
			exit(1);
		}
	}
	if(init()){
		fp = fopen(filename, "w");
		if(fp == NULL){
			fprintf(stderr, "***Unable to open file %s\n", filename);
			exit(1);
		}
		if(MESSAGE_LEVEL >= IMP) printBaseConfig(fp);
		uniformDistributionBenchmark(fp);
		fclose(fp);
	}
}

