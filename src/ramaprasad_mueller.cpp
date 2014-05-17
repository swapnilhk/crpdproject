#include"ramaprasad_mueller.h"
#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<set>
#include"global.h"
#include"set_operations.h" 

#define parentOf(i) ceil((i)/2);
#define leftChildOf(i) ((i)*2);
#define rightChildOf(i) ((i)*2+1);

#define isFull(q) ((q)->rear == (q)->size)) ? 1 : 0
#define isEmpty(q) ((q)->rear == 0) ? 1 : 0

typedef struct{
	long inTime;
	double remTime;
	int priority; // Priority <=> Task Name
}JOB;

typedef struct{
	int rear;
	int size;
	JOB** data;
	int (*compare)(JOB*, JOB*);
}PRIORITY_QUEUE;

void levelwisePrint(PRIORITY_QUEUE* q){
	int index = 1;
	int level = 0;
	int levelLimit = 0;
	while(index <= q->rear){
		levelLimit += pow(2, level);
		while(index <= q->rear && index <= levelLimit){
			printf("(%d,%ld,%0.2g) ", q->data[index]->priority, q->data[index]->inTime, q->data[index]->remTime);
			index++;
		}
		level++;
		printf("\n");
	}
}

double cost(void){
	return 1;
}

int compareByInTime(JOB* j1, JOB* j2){
	if(j1->inTime < j2->inTime)
		return -1;
	else if(j1->inTime > j2->inTime)
		return 1;
	else return 0;
}

int compareByPriorityAndInTime(JOB* j1, JOB* j2){
	if(j1->priority < j2->priority)
		return -1;
	else if(j1->priority > j2->priority)
		return 1;
	else return compareByInTime(j1, j2);
}

void minHeapifyUpwards(PRIORITY_QUEUE* q, int i){
	int p = parentOf(i);	
	if(p >= 1 && q->compare(q->data[i], q->data[p]) < 0){// q->data[i] is larger than its parent's data
		JOB* temp = q->data[i];
		q->data[i] = q->data[p];
		q->data[p] = temp;
		minHeapifyUpwards(q, p);
	}
}

void minHeapifyDownwards(PRIORITY_QUEUE* q, int i){
	int smallest;
	int l = leftChildOf(i);
	int r = rightChildOf(i);
	if(l <= q->rear && q->compare(q->data[l], q->data[i]) < 0)
		smallest = l;
	else smallest = i;
	if(r <= q->rear && q->compare(q->data[r], q->data[smallest]) < 0)
		smallest = r;
	if(smallest != i){
		JOB* temp = q->data[i];
		q->data[i] = q->data[smallest];
		q->data[smallest] = temp;
		minHeapifyDownwards(q, smallest);
	}
}

PRIORITY_QUEUE * initQueue(int maxElements, int (*compare)(JOB*, JOB*)){
	PRIORITY_QUEUE * q;
	q = (PRIORITY_QUEUE *)malloc(sizeof(PRIORITY_QUEUE));
	q->rear = 0;
	q->size = maxElements+1;// maxElements+1 because we ignore 0th index in a queue implemented as heap, so we require one more array element
	q->data = (JOB**)malloc(q->size * sizeof(JOB*));
	q->compare = compare;
	return q;
}

void freeQueue(PRIORITY_QUEUE * q){
	free(q->data);
	free(q);
}

void insertIntoQueue(PRIORITY_QUEUE* q, JOB* j){
	if(q->rear == q->size)
		fprintf(stderr, "Queue full\n");
	else{	
		q->data[++q->rear] = j;
		minHeapifyUpwards(q, q->rear);
	}
}

JOB* deleteFromQueue(PRIORITY_QUEUE* q){
	JOB* ret;
	if(q->rear == 0)
		fprintf(stderr, "Queue empty\n");
	else{		
		ret = q->data[1];
		q->data[1] = q->data[q->rear];
		q->rear--;
		minHeapifyDownwards(q, 1);
		return ret;
	}
}

JOB* topOfQueue(PRIORITY_QUEUE* q){
	if(q->rear == 0)
		fprintf(stderr, "Queue empty\n");
	else return q->data[1];
}

long lcm(long n1, long n2){
	long temp1 = n1, temp2 = n2;
	while(temp1 != temp2)
		if(temp1 > temp2)
			temp1 = temp1 - temp2;
		else
			temp2 = temp2 - temp1;
	return n1 * n2 / temp1;
}

long getHyperperiod(int numTasks){
	int i;
	long hyperperiod;	
	if(numTasks >= 2){
		hyperperiod = lcm(T[0], T[1]);
		for(i = 1; i < numTasks; i++)
			hyperperiod = lcm(hyperperiod, T[i]);
		return hyperperiod;
	}
}

int ramaprasadMueller(void){
	JOB *j;	
	int i, qsize = 0, thisTask, sched = 1, numPreemptionPoints = 0;
	long hyperperiod = getHyperperiod(NUM_TASKS);
	double time, timeBc, timeWc, preemptionPoint = 0, prevPreemptionPoint, idleBeforePreemptionPoint, finTime;
	PRIORITY_QUEUE *inQBcet, *inQWcet, *readyQBcet, *readyQWcet;
	std::set<int> cache;
	FILE *fp1 = fopen("out/timelinebect.txt", "w"), *fp2 = fopen("out/timelinewect.txt", "w"), *fp3 = fopen("out/ramaprasad_mueller.txt", "w");
	
	cache.clear();	
	if(MESSAGE_LEVEL >= ALL)
		fprintf(fp3, "Current Task,Previous Preemption Point,Current Preemption Point,Idle Time Before Preemption Point,Finish Time,Valid/Invalid Preemption Point\n");
	for(thisTask = 1; thisTask < NUM_TASKS && sched; thisTask++){				
		if(MESSAGE_LEVEL >= ALL){
			fprintf(fp1, "\nThis task = %d\n", thisTask);
			fprintf(fp2, "\nThis task = %d\n", thisTask);
		}
		time = timeBc = timeWc = 0;
		idleBeforePreemptionPoint = finTime = -INFINITY;
		
		/* Priority queue inQBcet is used to keep the jobs of task, that have higher
		   priority than this_tsak, during the hyperperiod ordered according to their
		   arrival time. They will be removed from the queue as time progresses.
		*/	
		for(i = 0; i <= thisTask; i++)
			qsize += ceil(hyperperiod/T[i]);//Approximation of maximum queue size that is sorted according to inTime. TODO: Find more accurate value
		
		inQBcet = initQueue(qsize, compareByInTime);
		inQWcet = initQueue(qsize, compareByInTime);
	
		/* For each task, generate jobs that can arrive during the hyperperiod and
		   insert them into the priority queue	
		*/
		for(i = 0; i < thisTask; i++){
			long inTime = 0;
			do{
				j = (JOB*)malloc(sizeof(JOB));
				j->inTime = inTime;
				j->remTime = BC[i];
				j->priority = i;				
				insertIntoQueue(inQBcet, j);	
				inTime += T[j->priority];
			}while(inTime < hyperperiod);
		}
		
		for(i = 0; i <= thisTask; i++){
			long inTime = 0;
			do{
				j = (JOB*)malloc(sizeof(JOB));
				j->inTime = inTime;			
				j->remTime = C[i];			
				j->priority = i;				
				insertIntoQueue(inQWcet, j);	
				inTime += T[j->priority];
			}while(inTime < hyperperiod);
		}
	
		readyQBcet = initQueue(qsize, compareByPriorityAndInTime);//TODO: find tighter bound for qbcet->size		
		readyQWcet = initQueue(qsize, compareByPriorityAndInTime);//TODO: find tighter bound for qbcet->size
		
		preemptionPoint = 0;
		
		while(time < hyperperiod && sched){
			while((!isEmpty(inQBcet)) && topOfQueue(inQBcet)->inTime == time){
				j = deleteFromQueue(inQBcet);//j is the new job that arrives in best case. Hence we put it into queue readyQBcet			
				insertIntoQueue(readyQBcet, j);
			}
			prevPreemptionPoint	= preemptionPoint;
			preemptionPoint = (!isEmpty(inQBcet)) ? topOfQueue(inQBcet)->inTime : hyperperiod;//newTime is the arrival time for next job.
			idleBeforePreemptionPoint = -INFINITY;// -INFINITY indicates the point where idle time begind in this interval has not been encountered yet
			finTime = (finTime == INFINITY) ? finTime : -INFINITY;
			while(timeBc < preemptionPoint && sched){
				if(!isEmpty(readyQBcet)){				
					j = deleteFromQueue(readyQBcet);//j is the job that needs to be executed now									
					if(j->remTime <= preemptionPoint - timeBc){//ie. j will finish in this interval
						if(MESSAGE_LEVEL >= ALL)
							fprintf(fp1, "->T%d(%g,%g)", j->priority, timeBc, timeBc + j->remTime);
						timeBc = timeBc + j->remTime;
						free(j);//j finishes, hence deallocating memory
					}
					else{//ie. j will not finish in this interval
						if(MESSAGE_LEVEL >= ALL)
							fprintf(fp1, "->T%d(%g,%g)", j->priority, timeBc, preemptionPoint);						
						j->remTime -= (preemptionPoint - timeBc);
						insertIntoQueue(readyQBcet, j);//j does not finish, hence reinserting j into the queue						
						timeBc = preemptionPoint;
					}					
				}
				else{
					if(MESSAGE_LEVEL >= ALL)
						fprintf(fp1, "->I(%g,%g)", timeBc, preemptionPoint);
					idleBeforePreemptionPoint = timeBc;
					timeBc = preemptionPoint;					
				}
			}
			
			while(timeWc < preemptionPoint && sched){
				double nextInTime;
				while((!isEmpty(inQWcet)) && topOfQueue(inQWcet)->inTime == timeWc){
					j = deleteFromQueue(inQWcet);//j is the new job that arrives in worst case. Hence we put it into queue readyQWcet
					insertIntoQueue(readyQWcet, j);
				}
				nextInTime = (!isEmpty(inQWcet)) ? topOfQueue(inQWcet)->inTime : hyperperiod;
				while(timeWc < nextInTime){
					if(!isEmpty(readyQWcet)){					
						j = deleteFromQueue(readyQWcet);//j is the job that needs to be executed now						
						if(j->remTime <= nextInTime - timeWc){//ie. j will finish in this interval
							if(MESSAGE_LEVEL >= ALL)
								fprintf(fp2, "->T%d(%g,%g)", j->priority, timeWc, timeWc + j->remTime);						
							timeWc = timeWc + j->remTime;
							if(j->priority == thisTask){
								finTime = timeWc;
								if(finTime > j->inTime + D[thisTask])
									sched = 0;
								cache.clear();
								if(MESSAGE_LEVEL >= IMP)
									fprintf(fp3, "Task:%d,Intime:%ld,FinishTime=%g,ResponseTime=%g,NumPreemptionPts=%d,Deadline=%ld,%sSchedulable\n",thisTask, j->inTime, finTime, finTime - j->inTime, numPreemptionPoints, j->inTime + D[thisTask],sched?"":"NOT ");
								numPreemptionPoints = 0;
							}					
							free(j);//j finishes, hence deallocating memory
						}
						else{//ie. j will not finish in this interval				
							if(MESSAGE_LEVEL >= ALL)
								fprintf(fp2, "->T%d(%g,%g)", j->priority, timeWc, nextInTime);
							j->remTime -= (nextInTime - timeWc);						
							insertIntoQueue(readyQWcet, j);//j does not finish, hence reinserting j into the queue
							if(j->priority == thisTask){
								finTime = INFINITY;// INFINITY indicates j does not finish in this interval													
								if(timeWc >= j->inTime + D[thisTask])
									sched = 0;
							}
							else
								Set_Difference(cache, TASK_ECB[thisTask], cache);
							timeWc = nextInTime;
						}
					}
					else{
						if(MESSAGE_LEVEL >= ALL)
							fprintf(fp2, "->I(%g,%g)", timeWc, nextInTime);
						timeWc = nextInTime;
					}				
				}
			}
			if(MESSAGE_LEVEL >= ALL)
				fprintf(fp3, "%d,%g,%g,%g,%g,",thisTask, prevPreemptionPoint,preemptionPoint,idleBeforePreemptionPoint,finTime);
			time = preemptionPoint;
			if(idleBeforePreemptionPoint != -INFINITY && finTime == INFINITY){
				if(MESSAGE_LEVEL >= ALL){
					fprintf(fp3, "V\n");
					numPreemptionPoints++;
				}
				// Adding preemption cost for this task
				std::set<int> temp;
				Set_Difference(TASK_UCB[thisTask], cache, temp);
				j->remTime += BRT * SET_MOD(temp);
				Set_Union(cache, TASK_UCB[thisTask], cache);//Restore cache
			}
			else
				if(MESSAGE_LEVEL >= ALL)
					fprintf(fp3, "I\n");
		}		
		if(!isEmpty(readyQBcet)){
			if(MESSAGE_LEVEL >= IMP)
				fprintf(fp1,"\nNot Schedulable");
			while(!isEmpty(readyQBcet))
				free(deleteFromQueue(readyQBcet));
			sched = 0;
		}
	 	if(!isEmpty(readyQWcet)){
	 		if(MESSAGE_LEVEL >= IMP)
			 	fprintf(fp2,"\nNot Schedulable");
			 while(!isEmpty(readyQWcet))
				free(deleteFromQueue(readyQWcet));
		 	sched = 0;
		}	
		freeQueue(inQBcet);
		freeQueue(inQWcet);
		freeQueue(readyQBcet);
		freeQueue(readyQWcet);
	}
	fclose(fp1);
	fclose(fp2);
	fclose(fp3);		
	return sched;
}

