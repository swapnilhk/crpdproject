#ifndef HEADER_GLOBAL
#define HEADER_GLOBAL

#include<set>

#define NUM_TASKS 4
#define CACHE_SIZE 256
#define Total_ECBs_CU 10
#define RF 0.8
#define NUM_TASK_SETS 1
#define BRT 8e-3
#define UTIL_START 0.70
#define UTIL_INCREMENT 0.05
#define UTIL_END 1
#define MIN_PERIOD 20
#define MAX_PERIOD 500
extern int MESSAGE_LEVEL;
extern int VERBOSE;
extern double C[NUM_TASKS];
extern long D[];
extern long T[];
extern double util;
extern std::set<int> TASK_ECB[];
extern std::set<int> TASK_UCB[];
extern int Num_Executed_Tasks[];

void printTaskInfo(FILE *fp);
void initUniformDistributionBenchmark(FILE* fp);
void createTaskSetUniformDistribution(float totalUtil, int minPeriod, int maxPeriod);
double sigmaTda(int thisTask, double Response[]);
double wcrt(int thisTask, double Response[], FILE *fp, double (*PC)(int, double[], FILE*));
void Set_Union(std::set<int> & Set1, std::set<int> & Set2, std::set<int> & Set3);
void Set_Intersect(std::set<int> & Set1, std::set<int> & Set2, std::set<int> & Set3);
void MultiSet_Intersect(std::multiset<int> & Set1, std::multiset<int> & Set2, std::multiset<int> & Set3);
int MultiSet_MOD(std::multiset<int> & Set1);
int SET_MOD(std::set<int> & Set1);
void print_SET(std::set<int> & Set1, FILE *fp);
int TestMultiSet();
#endif
