#ifndef HEADER_GLOBAL
#define HEADER_GLOBAL

#include<set>
#include<stdio.h>

extern int NUM_TASKS;
extern int CACHE_SIZE;
extern int NUM_TASK_SETS;
extern double UTIL_START;
extern double UTIL_INCREMENT;
extern double UTIL_END;
extern int MIN_PERIOD;
extern int MAX_PERIOD;
#define Total_ECBs_CU 10
#define RF 0.8
#define BRT 8e-3

enum flag {NONE, IMP, ALL};
/**
 * @brief The following enum define an integer constant
 * corrsponding to each of the methods.
 */
enum METHOD_NAMES {
	PRE_MAX_KD,
	PRE_MAX_KD2,
	LEE_WODC,
	LEE_WDC,
	RAMAPRASAD_MUELLER,
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
