#ifndef HEADER_COMMON
#define HEADER_COMMON

#include"base_config.h"

double C[NUM_TASKS];
long   D[NUM_TASKS];
long   T[NUM_TASKS];
double taskSetUtil;

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
	LEE_WODC,
	LEE_WDC,
	NUM_METHODS
};

int Num_Executed_Tasks[NUM_METHODS];

double printTaskInfo(FILE *fp)
{
	double util = 0;
	int i;

	fprintf(fp, "\n\nPRINTING TASK INFO\nTask\tExectime\tPeriod\tDeadline\n");
	
	for(i = 0; i < NUM_TASKS; i++)
	{
		fprintf(fp, "%d\t%8.4g\t%ld\t%ld\n", i, C[i], T[i], D[i]);
		util += ( C[i] / T[i]);
	}

	fprintf(fp, "Util = %f\n\n", util);

	return util;
}
#endif
