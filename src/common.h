#ifndef NUM_TASKS
#include"base_config.h"
#endif

double C[NUM_TASKS];
long   D[NUM_TASKS];
long   T[NUM_TASKS];
double taskSetUtil;
long PRE_ij_min[NUM_TASKS][NUM_TASKS];
long PRE_ij_max[NUM_TASKS][NUM_TASKS];
long PRE_min[NUM_TASKS];
long PRE_max[NUM_TASKS];

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
	NUM_METHODS
};

int Num_Executed_Tasks[NUM_METHODS];
double Response[NUM_TASKS] = {0};

void Clear_Task_Execution_Statistics()
{
     int i;
     
     for( i = 0; i < NUM_METHODS; i++)
     {
          Num_Executed_Tasks[i] = 0;
     }
}

void clear_Response()
{
	int i;
	for(i = 0; i < NUM_TASKS; i++)
	{
		Response[i] = 0;
	}
}

long INTR_max(int i, int j)
{
	return (long) ceil( Response[j] / T[i]);
}

long INTR_min(int i, int j)
{
	return (long) floor( Response[j] / T[i]);
}

void Reset_PRE_min()
{
	int i, j;

	for(i=0; i < NUM_TASKS; i++)
	{
		for(j=0; j < NUM_TASKS; j++)
		{
			PRE_ij_min[i][j] = 0;
		}
		PRE_min[i] = 0;
	}

}


void Reset_PRE_max()
{
	int i, j;

	for(i=0; i < NUM_TASKS; i++)
	{
		for(j=0; j < NUM_TASKS; j++)
		{
			PRE_ij_max[i][j] = 0;
		}
		PRE_max[i] = 0;
	}

}


void SetPRE_min(int i)
{
	int j;


	long acc = 0;

	for(j=0; j < i; j++)
	{
		acc += PRE_ij_min[j][i];
	}

	PRE_min[i] = acc;

}


void  SetPRE_max(int i)
{
	int j;


	long acc = 0;

	for(j=0; j < i; j++)
	{
		acc += PRE_ij_max[j][i];
	}

	PRE_max[i] = acc;

}


long calculate_pre_max_ij( int i, int j)
{
	int k;
	long acc, ret_val;

	acc = 0;

	for ( k = i+1; k < j; k++)
	{
		acc += PRE_ij_min[i][k] * INTR_min( k, j);
	}

	ret_val = INTR_max(i, j) - acc;

	if (ret_val < 0)
		ret_val = 0;

	return ret_val;

}


long calculate_pre_min_ij( int i, int j)
{
	int k;
	long acc, ret_val;

	acc = 0;

	for ( k = i+1; k < j; k++)
	{
		acc += PRE_ij_max[i][k] * INTR_max( k, j);
	}

	ret_val = INTR_min(i, j) - acc;

	if (ret_val < 0)
		ret_val = 0;

	return ret_val;

}
