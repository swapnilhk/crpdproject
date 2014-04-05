#ifndef HEADER_BASE_CONFIG
#define HEADER_BASE_CONFIG

// BASE CONFIG
#define NUM_TASKS 4
#define CACHE_SIZE 256	// Some form of Reuse Factor for Cache
#define Total_ECBs_CU 10
#define RF 0.8
#define NUM_TASK_SETS 1

double BRT = 8e-3; // In MicroSecs

#define UTIL_START 0.70
#define UTIL_INCREMENT 0.05
#define UTIL_END 1

#define MIN_PERIOD 20
#define MAX_PERIOD 500

enum level {NONE, IMP, ALL};
int MESSAGE_LEVEL = ALL;
int VERBOSE = 1;
#endif

