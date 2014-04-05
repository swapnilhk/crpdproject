#include<stdio.h>
#include<math.h>
#include"baseConfig.h"
#include"nnp.h"
#include"global.h"
#include"costFunctions.h"
#include"tda.h"

/* Generates set of tasks, corresponding to taskSetNo, in the array taskSet*/
void getTaskSet(const int thisTask, const int taskSetNo, int * taskSet){
	int i, lpTask = 1, offset = 0, hpTask;	
	for(i = 1; i < taskSetNo; i += pow(2,lpTask+1)-1, lpTask++)
		offset = i;
	// Genereting hp task set
	i = 0;
	for(hpTask = 0; hpTask < thisTask; hpTask++){
		int jump = pow(2, hpTask);
		if(((taskSetNo - offset) / jump) % 2 == 1){
			taskSet[i] = hpTask;
			i++;
		}
	}
	taskSet[i] = lpTask;
	taskSet[i+1] = -1;// -1 marks the end
}

/* From the htaskSetNo, find out the the combination of tasks i and j which has the least value of kNNPi_j*/
int getMinNnpMax(int thisTask, int taskSetNo, int nnpMax[NUM_TASKS][NUM_TASKS], int nnpMin[NUM_TASKS][NUM_TASKS]){
	int i, j;
	int taskSet[NUM_TASKS+1];
	
	getTaskSet(thisTask, taskSetNo, taskSet);
	int minNnpMax = nnpMax[taskSet[0]][taskSet[1]];
	for(i = 0; taskSet[i] != -1; i++){
		for(j = i+1; taskSet[j] != -1; j++){
			minNnpMax = nnpMax[taskSet[i]][taskSet[j]] < minNnpMax ? nnpMax[taskSet[i]][taskSet[j]] : minNnpMax; 
		}
	}
	return minNnpMax;
}

// Returns preemption cost for task 'thisTask'
double PC_PRE_MAX_KD3(int thisTask, double Response[], FILE *fp){
	int hpTask;
	int numHpTaskSets;
	int hpTaskSetNo;
	double cost = 0;
	static int nnpMax[NUM_TASKS][NUM_TASKS], nnpMin[NUM_TASKS][NUM_TASKS];

	numHpTaskSets = thisTask * (thisTask + 1) / 2;	
	if (thisTask >= 1){
		getNnp(thisTask, Response, nnpMax, nnpMin);
		for(hpTaskSetNo = 1 ; hpTaskSetNo <= numHpTaskSets; hpTaskSetNo++){			
			int minNnpMax = getMinNnpMax(thisTask, hpTaskSetNo, nnpMax, nnpMin);			
			cost += minNnpMax * varNoToCost(thisTask, hpTaskSetNo);			
		}
		if(MESSAGE_LEVEL > NONE)
			fprintf(fp, "Preemption cost = %g\n", cost);
		return cost;
	}
	else return 0;
}

int ResponseTimePreMaxKd3(){
	int taskNo;
	bool sched = true;
	FILE *fp;
	double Response[NUM_TASKS];
	static int firstCall = 1;

	if(MESSAGE_LEVEL > NONE){
		if(firstCall){			
			fp = fopen("out/kd3.txt", "w");
			firstCall = 0;
		}
		else fp = fopen("out/kd3.txt", "a");
		if(fp == NULL){
			printf("***Unable to open file\n");
			MESSAGE_LEVEL = NONE;
		}		
	}
	if(MESSAGE_LEVEL >= IMP)
		printTaskInfo(fp);
	for(taskNo = 0; taskNo < NUM_TASKS && sched; taskNo++){
		if(MESSAGE_LEVEL > NONE)
			fprintf(fp, "\tT%d\t\n", taskNo);
		wcrt(taskNo, Response, fp, PC_PRE_MAX_KD3);		
		if(Response[taskNo] > D[taskNo])
			sched = false;		
		if(MESSAGE_LEVEL >= IMP)
			fprintf(fp, "[T%d(C=%g,T=%ld,D=%ld) is %s]\n\n", taskNo, C[taskNo], T[taskNo], D[taskNo], sched ? "SCEDLABLE":"NOT SCEDLABLE");		
	}
	if(sched)
		Num_Executed_Tasks[PRE_MAX_KD3]++;

	if(MESSAGE_LEVEL > NONE && fp != NULL)
		fclose(fp);	
	return sched ? 1 : 0;
}
