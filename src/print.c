#include<stdio.h>
#include"global.h"

void printTaskInfo(FILE *fp){
	int i;
	double util = 0;
	fprintf(fp, "\nTASK INFO\nTask\tExectime\tPeriod\tDeadline\n");	
	for(i = 0; i < NUM_TASKS; i++){
		fprintf(fp, "%d\t%8.4g\t%ld\t%ld\n", i, C[i], T[i], D[i]);
		util += ( C[i] / T[i]);
	}
	fprintf(fp, "Util = %f\n\n", util);
	fflush(fp);
}

