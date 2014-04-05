#ifndef HEADER_COST_FUNCTION
#define HEADER_COST_FUNCTION

#include"set_operations.cpp"

/* Returns the cost of cache perrmption as:
 * UCB[this_task] INTERSECT (UNION( ECB[T] | T is a set of tasks that execute during this_task's preemption))
 * Example for var number: For this task = T4, variables have been nubmered from 1 to 11 as follows:
 * g2({T1}),g3({T1}),g3({T2}),g3({T1,T2}),g4({T1}),g4({T2}),g4({T1,T2}),g4({T3}),g4({T1,T3}),g4({T2,T3}),g4({T1,T2,T3})
 * Corresponding to this var bumber preemption cost is generated
 */
double varNoToCost(const int this_task, const int var_no){
	int i, lp_task = 1, offset = 0, hp_task;
	extern std::set<int> TASK_ECB[NUM_TASKS], TASK_UCB[NUM_TASKS];

	for(i = 1; i < var_no; i += pow(2,lp_task+1)-1, lp_task++)
		offset = i;

	std::set<int> workingSet1, workingSet2;
	workingSet1.clear();
	workingSet2.clear();

	for(hp_task = 0; hp_task < this_task; hp_task++)
	{
		int jump = pow(2, hp_task);
		if(((var_no - offset) / jump) % 2 == 1){
			Set_Union(workingSet1, TASK_ECB[hp_task], workingSet1);
		}
	}
	Set_Intersect(workingSet1, TASK_UCB[this_task], workingSet2);
	return BRT * SET_MOD(workingSet2);
}

double costEcbUnion(int hpTask, int lpTask, double Response[])
{
	std::set<int> workingSet1, workingSet2, workingSet3;
	extern std::set<int> TASK_ECB[NUM_TASKS], TASK_UCB[NUM_TASKS];
	int i;
	
	workingSet1.clear();
	workingSet2.clear();

	for(i = 0; i <= hpTask; i++){
		Set_Union(workingSet1, TASK_ECB[i], workingSet1);
	}

	Set_Intersect(workingSet1, TASK_UCB[lpTask], workingSet2);

	return BRT * SET_MOD(workingSet2);
}


#endif
