#ifndef HEADER_GLOBAL
#define HEADER_GLOBAL

#include<set>
#include<stdio.h>

//#define NUM_TASKS 4
extern int NUM_TASKS;
#define CACHE_SIZE 256
#define Total_ECBs_CU 10
#define RF 0.8
#define NUM_TASK_SETS 2
#define BRT 8e-3
#define UTIL_START 0.70
#define UTIL_INCREMENT 0.05
#define UTIL_END 1
#define MIN_PERIOD 20
#define MAX_PERIOD 500
enum flag {NONE, IMP, ALL};
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
extern int MESSAGE_LEVEL;
extern int VERBOSE;
extern double *C;// Worst case execution time
extern double *B;// Best case execution time
extern long *T;// Period
extern long *D;// Deadline
extern double util;
extern std::set<int> *TASK_ECB;
extern std::set<int> *TASK_UCB;
extern int Num_Executed_Tasks[];

void printBaseConfig(FILE *fp);
void printTaskInfo(FILE *fp);
void print_ecbs(FILE *fp);
void print_ucbs(FILE *fp);
int** Make2DintArrayInt(int arraySizeX, int arraySizeY);
double** Make2DintArrayDouble(int arraySizeX, int arraySizeY);
long** Make2DintArrayLong(int arraySizeX, int arraySizeY);
void free2DintArrayInt(int ** twoDArray, int n);
double sigmaTda(int thisTask, double Response[]);
double wcrt(int thisTask, double Response[], FILE *fp, double (*PC)(int, double[], FILE*));
#endif
