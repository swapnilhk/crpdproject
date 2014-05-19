#ifndef UNIFORN_FIXED_TASKS_ECB_UCB_SCHEMES
#define UNIFORN_FIXED_TASKS_ECB_UCB_SCHEMES

#include<stdio.h>

extern float multFactor;

extern double **Gamma_i_j_ECB_Only;
extern double **Gamma_i_j_UCB_Only;
extern double **Gamma_i_j_UCB_Union;
extern double **Gamma_i_j_ECB_Union;
extern double **Gamma_i_j_Staschulat;
extern double **Gamma_i_j_ECB_Union_Multiset;
extern double **Gamma_i_j_UCB_Union_Multiset;
extern double **Gamma_i_j_Staschulat_PRE;
extern double **Gamma_i_j_ECB_Union_Multiset_PRE;
extern double **Gamma_i_j_UCB_Union_Multiset_PRE;
extern double **Gamma_i_j_PRE_MAX;

extern long *Num_Displaced_Blocks_ECB_Only;
extern long *Num_Displaced_Blocks_UCB_Only;
extern long *Num_Displaced_Blocks_UCB_Union;
extern long *Num_Displaced_Blocks_ECB_Union;
extern long *Num_Displaced_Blocks_Staschulat;
extern long *Num_Displaced_Blocks_ECB_Union_Multiset;
extern long *Num_Displaced_Blocks_UCB_Union_Multiset;
extern long *Num_Displaced_Blocks_ECB_UCB_Union_Multiset_Combined;
extern long *Num_Displaced_Blocks_Staschulat_PRE;
extern long *Num_Displaced_Blocks_ECB_Union_Multiset_PRE;
extern long *Num_Displaced_Blocks_UCB_Union_Multiset_PRE;
extern long *Num_Displaced_Blocks_ECB_UCB_Union_Multiset_Combined_PRE;
extern long *Num_Displaced_Blocks_PRE_MAX;

extern double *Response;
extern double *Response_NO_PREEMPT;
extern double *Response_ECB_Only;
extern double *Response_UCB_Only;
extern double *Response_UCB_Union;
extern double *Response_ECB_Union;
extern double *Response_Staschulat;
extern double *Response_ECB_Union_Multiset;
extern double *Response_UCB_Union_Multiset;
extern double *Response_ECB_UCB_Union_Multiset_Combined;
extern double *Response_Staschulat_PRE;
extern double *Response_ECB_Union_Multiset_PRE;
extern double *Response_UCB_Union_Multiset_PRE;
extern double *Response_ECB_UCB_Union_Multiset_Combined_PRE;
extern double *Response_PRE_MAX;

extern int *Cost;
extern int *Useful_cost;

extern long **PRE_ij_min;
extern long **PRE_ij_max;
    
extern long *PRE_min;
extern long *PRE_max;

extern double Cycles[15];
extern bool BDU_ONLY;
extern FILE* BDU_fp;

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
void Clear_ALL(void);
#endif
