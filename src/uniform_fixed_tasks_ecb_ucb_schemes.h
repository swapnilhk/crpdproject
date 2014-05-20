#ifndef UNIFORN_FIXED_TASKS_ECB_UCB_SCHEMES
#define UNIFORN_FIXED_TASKS_ECB_UCB_SCHEMES

#include<stdio.h>

void init_uniform_fixed_tasks_ecb_ucb_schemes(void);
void clear_Response(void);
long INTR_max(int i, int j);
long INTR_min(int i, int j);
void Reset_PRE_min(void);
void Reset_PRE_max(void);
void SetPRE_min(int i);
void  SetPRE_max(int i);
long calculate_pre_max_ij( int i, int j);
long calculate_pre_min_ij( int i, int j);
void ResponseMethod1(void);
void ResponseMethod2(void);
void quickCalculate(void);
int Response_time_NO_PREEMPT(void);
int Response_time_ECB_Only(void);
int Response_time_UCB_Only(void);
int Response_time_UCB_Union(void);
int Response_time_ECB_Union(void);
int Response_time_Staschulat(void);
int Response_time_ECB_Union_Multiset(void);
int Response_time_UCB_Union_Multiset(void);
int Response_time_ECB_UCB_Union_Multiset_Combined(void);
int Response_time_Staschulat_PRE(void);
int Response_time_ECB_Union_Multiset_PRE(void);
int Response_time_UCB_Union_Multiset_PRE(void);
int Response_time_ECB_UCB_Union_Multiset_Combined_PRE(void);
int Response_time_PRE_MAX(void);
#endif
