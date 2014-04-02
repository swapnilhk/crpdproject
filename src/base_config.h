#ifndef HEADER_BASE_CONFIG
#define HEADER_BASE_CONFIG

// BASE CONFIG
#define NUM_TASKS 4
#define CACHE_SIZE 256
#define RF 0.8 			// Some form of Reuse Factor for Cache
#define Total_ECBs_CU 10
#define NUM_TASK_SETS 500

double BRT = 8e-3;		// In MicroSecs
double NIPMS = 0.04;		// Number of Insns per Micro Second

#define UTIL_START 0.5
#define UTIL_INCREMENT 0.05
#define UTIL_END 1

#define MIN_PERIOD 20
#define MAX_PERIOD 500

enum level {NONE, IMP, ALL};
int MESSAGE_LEVEL = NONE;
int VERBOSE = 1;

#endif

