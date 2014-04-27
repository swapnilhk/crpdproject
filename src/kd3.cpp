#include<stdio.h>
#include<math.h>
#include"baseConfig.h"
#include"nnp.h"
#include"global.h"
#include"costFunctions.h"
#include"tda.h"
#include"set_operations.cpp"

struct kd4{
	int * taskSet;
	int size;
	double cost;
};

/* Generates set of tasks, corresponding to varNo, in the array taskSet*/
int * getTaskSetFormVarNo(const int thisTask, const int varNo){
	int i, lpTask = 1, offset = 0, hpTask;	
	int * taskSet = (int*)malloc((thisTask+1) * sizeof(*taskSet));
	
	for(i = 1; i < varNo; i += pow(2,lpTask+1)-1, lpTask++)
		offset = i;
	// Genereting task set corresponding to varNo
	i = 0;
	for(hpTask = 0; hpTask < thisTask; hpTask++){
		int jump = pow(2, hpTask);
		if(((varNo - offset) / jump) % 2 == 1){
			taskSet[i] = hpTask;
			i++;
		}
	}
	taskSet[i] = thisTask;// thisTask marks the end
	return taskSet;
}

/* From the taskSet, find out the the combination of tasks i and j which has the least value of thisTask_NNPi_j*/
int getMinNnpMax(int * taskSet, int thisTask, double  Response[], int nnpMax[NUM_TASKS][NUM_TASKS], int nnpMin[NUM_TASKS][NUM_TASKS]){
	int i, j;
	
	int minNnpMax = nnpMax[taskSet[0]][taskSet[1]] * ceil(Response[thisTask]/T[taskSet[1]]);
	for(i = 0; taskSet[i] < thisTask; i++){
		for(j = i; taskSet[j] < thisTask; j++){
			minNnpMax = 
				nnpMax[taskSet[i]][taskSet[j+1]] * ceil(Response[thisTask]/T[taskSet[j+1]])
				< minNnpMax 
				? nnpMax[taskSet[i]][taskSet[j+1]] * ceil(Response[thisTask]/T[taskSet[j+1]])
				: minNnpMax; 
		}
	}	
	return minNnpMax;
}

// Returns preemption cost for task 'thisTask'
double PC_PRE_MAX_KD3(int thisTask, double Response[], FILE *fp){
	int i, numVar = 0;
	int varNo;
	double cost = 0;
	int *taskSet;
	static int nnpMax[NUM_TASKS][NUM_TASKS], nnpMin[NUM_TASKS][NUM_TASKS];

	if (thisTask >= 1){	
		getNnp(thisTask, Response, nnpMax, nnpMin);
		for(i = 1;i <= thisTask; i++)// Counting the no. of variables for this_task
			numVar+=pow(2,i)-1;	
		for(varNo = 1 ; varNo <= numVar; varNo++){// For each task sets involving higher priority tasks corresponding to varNo, do the following
			taskSet = getTaskSetFormVarNo(thisTask, varNo);// Find the the actual tasks in the task set corresponding to the 'varNo'
			int minNnpMax = getMinNnpMax(taskSet, thisTask, Response, nnpMax, nnpMin);// Find minimum nnp_max among each pair of tasks in 'tsakSet'
			cost += minNnpMax * varNoToCost(thisTask, varNo);
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

void * fillStruct(int taskSetNo, int thisTask, struct kd4 *t){
	int i, j, hpTask;	
	t->taskSet = (int *)malloc((thisTask+1) * sizeof(int));
	double cost = 0;
	extern std::set<int> TASK_ECB[NUM_TASKS], TASK_UCB[NUM_TASKS];
	std::set<int> workingSet1;

	// Genereting hp task set
	i = 0;
	for(hpTask = 0; hpTask < thisTask; hpTask++){
		int jump = pow(2, hpTask);
		if((taskSetNo / jump) % 2 == 1){
			t->taskSet[i] = hpTask;
			i++;
		}
	}
	t->taskSet[i] = thisTask;// thisTask marks the end
	t->size = i+1;
	
	for(j = 1; j < t->size; j++){
		workingSet1.clear();
		for(i = 0; i < j; i++)
			Set_Union(workingSet1, TASK_ECB[t->taskSet[i]], workingSet1);
		Set_Intersect(workingSet1, TASK_UCB[t->taskSet[j]], workingSet1);
		cost += SET_MOD(workingSet1);
	}	
	cost *= BRT;
	
	t->cost = cost;
}

int compareKd4(const void * arg1, const void * arg2){	
	struct kd4 * a1 = (struct kd4*)arg1;
	struct kd4 * a2 = (struct kd4*)arg2;
	if(a1->cost > a2->cost)
		return -1;
	else if(a1->cost < a2->cost)
		return 1;
	else return 0;
}

/* From the taskSet, find out the the combination of tasks i and j which has the least value of thisTask_NNPi_j
   and also, iand j are adjacent ie hve a difference of 1 ie |i-j| = 1
*/
int getMinNnpMax2(int * taskSet, int thisTask, double  Response[], int nnpMax[NUM_TASKS][NUM_TASKS], int nnpMin[NUM_TASKS][NUM_TASKS], int consumedNnpMax[NUM_TASKS][NUM_TASKS]){
	int i, j;	
	int minNnpMax = nnpMax[taskSet[0]][taskSet[1]] * ceil(Response[thisTask]/T[taskSet[1]]) - consumedNnpMax[taskSet[0]][taskSet[1]];
	
	for(i = 0, j = i + 1; taskSet[i] < thisTask; i++, j++){	
		int temp = nnpMax[taskSet[i]][taskSet[j]] * ceil(Response[thisTask]/T[taskSet[j]]) - consumedNnpMax[taskSet[i]][taskSet[j]];
		minNnpMax = 
			temp
			< minNnpMax 
			? temp
			: minNnpMax; 
	}
	return minNnpMax;
}

// Returns preemption cost for task 'thisTask'
double PC_PRE_MAX_KD4(int thisTask, double Response[], FILE *fp){
	int i;
	int hpTask;
	int numHpTaskSets;
	int hpTaskSetNo;
	double cost = 0;
	static int nnpMax[NUM_TASKS][NUM_TASKS], nnpMin[NUM_TASKS][NUM_TASKS];	
	int consumedNnpMax[NUM_TASKS][NUM_TASKS];
	
	memset(consumedNnpMax, 0, NUM_TASKS * NUM_TASKS * sizeof(int));
	if (thisTask >= 1){			
		getNnp(thisTask, Response, nnpMax, nnpMin);
		numHpTaskSets = pow(2, thisTask) - 1;
		struct kd4 t[numHpTaskSets];
		for(hpTaskSetNo = 1 ; hpTaskSetNo <= numHpTaskSets; hpTaskSetNo++)
			fillStruct(hpTaskSetNo, thisTask, &t[hpTaskSetNo-1]);			
		if(MESSAGE_LEVEL >= ALL){
			fprintf(fp,"\nCosts before sorting:\n");
			for(i = 0 ; i < numHpTaskSets; i++){
				int j;
				fprintf(fp, "Task Set {");			
				for(j = 0; j < t[i].size ; j++)
					fprintf(fp, "T%d%c", t[i].taskSet[j], (j == t[i].size - 1) ? '}' : ',');
				fprintf(fp, " Cost = %g\n", t[i].cost);			
			}
			fprintf(fp,"\nCosts after sorting:\n");
		}
		qsort(t, numHpTaskSets, sizeof(struct kd4), compareKd4);
		if(MESSAGE_LEVEL >= ALL){
			for(i = 0 ; i < numHpTaskSets; i++){
				int j;
				fprintf(fp, "Task Set {");			
				for(j = 0; j < t[i].size ; j++)
					fprintf(fp, "T%d%c", t[i].taskSet[j], (j == t[i].size - 1) ? '}' : ',');				
				fprintf(fp, " Cost = %g\n", t[i].cost);			
			}			
		}
		for(i = 0 ; i < numHpTaskSets; i++){
			int j, k;
			int minNnpMax = getMinNnpMax2(t[i].taskSet, thisTask, Response, nnpMax, nnpMin, consumedNnpMax);
			cost += minNnpMax * t[i].cost;			
			for(j = 0, k = j + 1; k < t[i].size; j++, k++)
				consumedNnpMax[t[i].taskSet[j]][t[i].taskSet[k]] += minNnpMax;
		}
		if(MESSAGE_LEVEL > NONE)
			fprintf(fp, "Preemption cost = %g\n", cost);
		for(i = 0 ; i < numHpTaskSets; i++){			
			free(t[i].taskSet);
		}
		return cost;
	}
	else return 0;
}

int ResponseTimePreMaxKd4(){
	int taskNo;
	bool sched = true;
	FILE *fp;
	double Response[NUM_TASKS];
	static int firstCall = 1;

	if(MESSAGE_LEVEL > NONE){
		if(firstCall){			
			fp = fopen("out/kd4.txt", "w");
			firstCall = 0;
		}
		else fp = fopen("out/kd4.txt", "a");
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
		wcrt(taskNo, Response, fp, PC_PRE_MAX_KD4);		
		if(Response[taskNo] > D[taskNo])
			sched = false;		
		if(MESSAGE_LEVEL >= IMP)
			fprintf(fp, "[T%d(C=%g,T=%ld,D=%ld) is %s]\n\n", taskNo, C[taskNo], T[taskNo], D[taskNo], sched ? "SCEDLABLE":"NOT SCEDLABLE");		
	}
	if(sched)
		Num_Executed_Tasks[PRE_MAX_KD4]++;

	if(MESSAGE_LEVEL > NONE && fp != NULL)
		fclose(fp);
	return sched ? 1 : 0;
}
