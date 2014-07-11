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
/**
 * @brief Sreucture describing job
 * @element inTime :- Arrival time of the job
 * @element remTime:- Remaining time of the job
 * @element priority:- Priority of the job. It also corresponds to the task name
 * @element hpTasks:- Vector (of bits) indicating the set of tasks that executed
 * during the preemption of this task
 * @element firstExec :- Value 1 indiactes that  the job is executing for the
 * first time and 0 indicates that the job is remuming from a previous
 * preemption
 */
typedef struct{
	long inTime;
	double remTime;
	int priority; // Priority <=> Task Name
	int hpTasks;
	int firstExec;
}JOB;

/**
 * @brief Structure for priority queue
 * @element rear :- index refering to the rear of the queue
 * @element size
 */
typedef struct{
	int rear;
	int size;
	JOB** data;
	int (*compare)(JOB*, JOB*);	
}PRIORITY_QUEUE;

/**
 * @brief This function prints the priority queue, which is in
 * the form of a heap, levelwise.
 * @param q :- The priority queue to be printed levelwise
 */
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

/**
 * @brief Compares two jobs on the basis of their in time
 * @param j1 :- The first job
 * @param j2 :- The second job
 * @return negative value if j1.inTime < j2.intime,
 * 0 if j1.inTime == j2.intime,
 * positive value if j1.inTime > j2.intime
 */
int compareByInTime(JOB* j1, JOB* j2){
	return j1->inTime - j2->inTime;
}

/**
 * @brief Compares two jobs on the basis of their priorities.
 * If priorities are equal then compares the jobs by their in
 * times.
 * @param j1 :- The first job
 * @param j2 :- The second job
 * @return negative value if j1 < j2,
 * 0 if j1 == j2,
 * positive value if j1 > j2
 */
int compareByPriorityAndInTime(JOB* j1, JOB* j2){	
	if(j1->priority - j2->priority)
		return j1->priority - j2->priority;
	else return compareByInTime(j1, j2);
}

/**
 * @brief Min-Heapify the priority 
 * @param q :- Queue to be heapified
 * @param i :- adjust the element at this index 
 * by following a downword direction in the tree so that the heap
 * property is maintained
 */
void minHeapifyUpwards(PRIORITY_QUEUE* q, int i){
	int p = parentOf(i);	
	if(p >= 1 && q->compare(q->data[i], q->data[p]) < 0){// q->data[i] is larger than its parent's data
		JOB* temp = q->data[i];
		q->data[i] = q->data[p];
		q->data[p] = temp;
		minHeapifyUpwards(q, p);
	}
}

/**
 * @brief Min-Heapify the priority 
 * @param q :- Queue to be heapified
 * @param i :- Adjust the element at this index by following
 * a upword direction in the tree so that the heap property
 * is maintained
 */
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

/**
 * @brief Initializes the priority queue
 * @param maxElements :- Size of the queue
 * @param compare :- Pointer to the function to be used to
 * compare the elements of the queue
 * @return PRIORITY_QUEUE :- Initiaized priority queue
 */
PRIORITY_QUEUE * initQueue(int maxElements, int (*compare)(JOB*, JOB*)){
	PRIORITY_QUEUE * q;
	q = (PRIORITY_QUEUE *)malloc(sizeof(PRIORITY_QUEUE));
	q->rear = 0;
	/**
	 * maxElements+1 because we ignore 0th index in a queue
	 * implemented as heap, so we require one more array element
	 */
	q->size = maxElements+1;
	q->data = (JOB**)malloc(q->size * sizeof(JOB*));
	q->compare = compare;
	return q;
}

/**
 * @brief Frees the space allocated to the queue
 * @param q :- Queue to be freed
 */
void freeQueue(PRIORITY_QUEUE * q){
	free(q->data);
	free(q);
}

/**
 * @brief Inserts an element into the priority queue
 * @param q :- Queue in which element is to be inserted
 * @param j :- Job to be inseted into quque
 */
void insertIntoQueue(PRIORITY_QUEUE* q, JOB* j){
	if(q->rear == q->size)
		fprintf(stderr, "Queue full\n");
	else{	
		q->data[++q->rear] = j;
		minHeapifyUpwards(q, q->rear);
	}
}

/**
 * @brief Delete top element from the priority queue
 * @param q :- Queue from which element is to be deleted
 * @return Deleted job is returned
 */
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

/**
 * @brief Returns top element from the priority queue.
 * This element is not deleted from the queue
 * @param q :- Queue from which element is to be deleted
 * @return Top element is returned
 */
JOB* topOfQueue(PRIORITY_QUEUE* q){
	if(q->rear == 0)
		fprintf(stderr, "Queue empty\n");
	else return q->data[1];
}

/**
 * @brief Finds lcm of two numbers
 * @param n1 :- The first number
 * @param n2 :- The second number
 * @return lcm of n1 and n2
 */
long lcm(long n1, long n2){
	long temp1 = n1, temp2 = n2;
	while(temp1 != temp2)
		if(temp1 > temp2)
			temp1 = temp1 - temp2;
		else
			temp2 = temp2 - temp1;
	return n1 * n2 / temp1;
}

/**
 * @brief Finds hyperperiod of the task set under consideration
 * @param numTasks :- The number of tasks in the set
 * @return The hyperperiod of the taskset
 */
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

/**
 * @brief This function tests the schedulability of the task set under
 * consideration using the method by Ramaprasad and Mueller
 * @return 1 if the task set is schedulable, 0 if not
 */
int ramaprasadMueller(void){
		JOB *j;	
	int i, qsize = 0, thisTask, sched = 1, numPreemptionPoints = 0;
	long hyperperiod = getHyperperiod(NUM_TASKS);
	double time, timeBc, timeWc, preemptionPoint = 0, prevPreemptionPoint, idleBeforePreemptionPoint, finTime;
	PRIORITY_QUEUE *inQBcet, *inQWcet, *readyQBcet, *readyQWcet;
	static int first_call = 1;
	static FILE *fp1, *fp2, *fp3;
	
	if(first_call){
		fp1 = fopen("out/timelinebect.txt", "w");
		fp2 = fopen("out/timelinewect.txt", "w");
		fp3 = fopen("out/ramaprasad_mueller.txt", "w");
		first_call = 0;
	}
	
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
				j = new JOB;//(JOB*)malloc(sizeof(JOB));
				j->inTime = inTime;
				j->remTime = B[i];
				j->priority = i;				
				j->hpTasks = 0;
				j->firstExec = 1;
				insertIntoQueue(inQBcet, j);					
				inTime += T[j->priority];				
			}while(inTime < hyperperiod);
		}
		
		for(i = 0; i <= thisTask; i++){
			long inTime = 0;
			do{
				j = new JOB;//(JOB*)malloc(sizeof(JOB));
				j->inTime = inTime;			
				j->remTime = C[i];			
				j->priority = i;
				j->hpTasks = 0;
				j->firstExec = 1;
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

					//---------------------------Cache Related Computations------------------------------------------------					
					/*if(j->firstExec){ // j is executing for the first time
						// Load the cache blocks
						Set_Union(j->cacheBlocks, TASK_ECB[j->priority], j->cacheBlocks);
						j->firstExec = 0;
					}
					else{ // j is not executing for the first time
						std::set<int> temp;
						Set_Difference(TASK_UCB[j->priority], j->cacheBlocks, temp);
						j->remTime += BRT * SET_MOD(temp);
						Set_Union(j->cacheBlocks, temp, j->cacheBlocks);
					}
					// In either case, for all jobs in the ready queue, remove the cache blocks belonging to this task
					for(int i = 1; i <= readyQBcet->rear; i++)
						Set_Difference(readyQBcet->data[i]->cacheBlocks, j->cacheBlocks, readyQBcet->data[i]->cacheBlocks);
					*/
					if(j->firstExec)// j is executing for the first time
						j->firstExec = 0;// Do nothong
					else{
						std::set<int> temp;
						temp.clear();
						// Union of ECBs of all HP tasks that have jth bit in j->hpTasks set
						for(int i = 0; i < j->priority; i++)
							if((j->hpTasks >> i) & 1)
								Set_Union(TASK_ECB[i], temp, temp);
						j->hpTasks = 0;
						// Intersection of UCB of this task
						Set_Intersect(TASK_UCB[j->priority], temp, temp);
						j->remTime += BRT * SET_MOD(temp);
					}
					for(int i = 1; i <= readyQBcet->rear; i++)// For all jobs in the ready queue do
						if(!readyQBcet->data[i]->firstExec)// Do nothing for new jobs ie those who have not executed even once
							readyQBcet->data[i]->hpTasks |= (1<<j->priority);
					//-----------------------------------------------------------------------------------------------------
					
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
						
						//---------------------------Cache Related Computations------------------------------------------------					
						/*if(j->firstExec){ // j is executing for the first time
							// Load the cache blocks
							Set_Union(j->cacheBlocks, TASK_ECB[j->priority], j->cacheBlocks);
							j->firstExec = 0;
						}
						else{ // j is not executing for the first time
							std::set<int> temp;
							Set_Difference(TASK_UCB[j->priority], j->cacheBlocks, temp);
							j->remTime += BRT * SET_MOD(temp);
							Set_Union(j->cacheBlocks, temp, j->cacheBlocks);
						}
						// In either case, for all jobs in the ready queue, remove the cache blocks belonging to this task
						for(int i = 1; i <= readyQWcet->rear; i++)
							Set_Difference(readyQWcet->data[i]->cacheBlocks, j->cacheBlocks, readyQWcet->data[i]->cacheBlocks);
						*/
						if(j->firstExec)// j is executing for the first time
							j->firstExec = 0;// Do nothong
						else{
							std::set<int> temp;
							temp.clear();
							// Union of ECBs of all HP tasks that have jth bit in j->hpTasks set
							for(int i = 0; i < j->priority; i++)
								if((j->hpTasks >> i) & 1)
									Set_Union(TASK_ECB[i], temp, temp);
							j->hpTasks = 0;
							// Intersection of UCB of this task
							Set_Intersect(TASK_UCB[j->priority], temp, temp);
							j->remTime += BRT * SET_MOD(temp);
						}
						for(int i = 1; i <= readyQWcet->rear; i++)// For all jobs in the ready queue do
							if(!readyQWcet->data[i]->firstExec)// Do nothing for new jobs ie those who have not executed even once
								readyQWcet->data[i]->hpTasks |= (1<<j->priority);
						//-----------------------------------------------------------------------------------------------------
									
						if(j->remTime <= nextInTime - timeWc){//ie. j will finish in this interval
							if(MESSAGE_LEVEL >= ALL)
								fprintf(fp2, "->T%d(%g,%g)", j->priority, timeWc, timeWc + j->remTime);						
							timeWc = timeWc + j->remTime;
							if(j->priority == thisTask){
								finTime = timeWc;
								if(finTime > j->inTime + D[thisTask])
									sched = 0;
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
	if(sched)
		Num_Executed_Tasks[RAMAPRASAD_MUELLER]++;		
	return sched;
}
