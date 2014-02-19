#include<stdio.h>
#include<math.h>
#include"../lib/lp_solve_ux64/lp_lib.h"
#include"common.h"

// Array to store the worst case response times of tasks
static FILE *fp;

double min(double a, double b);
double solve_constraints(int this_task);
double PC(int this_task);
double sigma_tda(int this_task);
double wcrt(int this_task);
void var_no_to_name(const int this_task, const int var_no, char * var_name);
void itoa(int i, char s[10]);
void strrev(char * s);
double var_on_to_cost(const int this_task, const int var_no);


void Response_time_lee_wodc(FILE *fp1){
	int task_no;
	fp = fp1;
	bool sched = true;
	clear_Response();

	fprintf(fp, "                              *******************                                     \n");
	fprintf(fp, "******************************  LEE WODC BEGINS  **************************************\n");
	fprintf(fp, "                              *******************                                     \n");

	for(task_no = 0; task_no < NUM_TASKS && sched; task_no++){		
		wcrt(task_no);
	fprintf(fp, "======================================================================================\n");
		if(Response[task_no] > D[task_no]){
			sched = false;
			//printf("TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", task_no, taskSetUtil, Response[task_no], D[task_no]);
			fprintf(fp, "TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", task_no, taskSetUtil, Response[task_no], D[task_no]);
		}
		else{
			//printf("TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", task_no, taskSetUtil, Response[task_no], D[task_no]);
			fprintf(fp, "TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", task_no, taskSetUtil, Response[task_no], D[task_no]);			
		}
		fprintf(fp, "======================================================================================\n\n");
	}
	fprintf(fp, "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	if(sched == false)
	{
		//printf("TASKSET NOT schedulable under LEE-WODC at TASKSET_UTIL = %f \n", taskSetUtil);
		fprintf(fp, "TASKSET is NOT schedulable under LEE-WODC at TASKSET_UTIL = %f \n", taskSetUtil);
	}
	else
	{
		//printf("TASKSET IS schedulable under LEE-WODC at TASKSET_UTIL = %f \n", taskSetUtil);
		fprintf(fp, "TASKSET IS schedulable under LEE-WODC at TASKSET_UTIL = %f \n", taskSetUtil);
		Num_Executed_Tasks[LEE_WODC]++;
	}
	fprintf(fp, "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
	fprintf(fp, "_________________________________LEE WODC ENDS________________________________________\n\n\n\n");
}

// Returns worst case response time of task 'task_no'
double wcrt(int this_task){	
	double R_new;
	R_new = C[this_task];
	Response[this_task] = 0;
	while(R_new != Response[this_task] && (Response[this_task] = R_new) <= D[this_task]){
		R_new = C[this_task] + sigma_tda(this_task) + PC(this_task);// Time demand equation
		fprintf(fp, "--------------------------------------------------------------------------------------\n");
		fprintf(fp, "TASK_NO =  %d Old response time = %g New response time = %g Deadline = %ld\n", this_task, Response[this_task], R_new, D[this_task]);
		fprintf(fp, "--------------------------------------------------------------------------------------\n\n");
	}
	return R_new;
}

double sigma_tda(int this_task){
	double R_new = 0;
	int hp_task = this_task - 1;
	while(hp_task >= 0){
		R_new += ceil(Response[this_task]/T[hp_task]) * C[hp_task];
		hp_task = hp_task - 1;
	}
	return R_new;
}

// Returns preemption cost for task 'this_task'
double PC(int this_task){
	int hp_task;
	if (this_task >= 1){
		return solve_constraints(this_task);
	}
	else return 0;
}

double solve_constraints(int this_task)
{
	lprec *lp;
	int numVar = 0, *var = NULL, ret = 0, i, j, k, var_count, H, var_no;
	double *coeff = NULL, r, obj;
	char col_name[20];

	/* Creating a model */	
	for(i = 1;i <= this_task; i++)/* Counting the no. of variables for this_task*/
		numVar+=pow(2,i)-1;	
	lp = make_lp(0, numVar);
	
	if(lp == NULL)
		ret = 1; /* Couldn't construct a new model */

	if(ret == 0) {
		var_count = 1;
		for(i = 1 ; i <= numVar; i++){
			var_no_to_name(this_task, i, col_name);
			set_col_name(lp, var_count, col_name);
			var_count++;
		}
		/* create space large enough for one row(i.e. equation) */
		var = (int *) malloc(numVar * sizeof(*var));
		coeff = (double *) malloc(numVar * sizeof(*coeff));
		if((var == NULL) || (coeff == NULL))
			ret = 2;
	}
	
	/* add the equations to lpsolve */
	if(ret == 0) {
		set_add_rowmode(lp, TRUE);
		/* --------------------adding first constraint-------------------- */
		for(j = 1; j <= this_task; j++){
			var_count = 0;
			var_no = 1;
			for(i = 1; i <= j; i++){
				int end = var_no + pow(2,i)-1;
				for(; var_no < end; var_no++){
					var[var_count] = var_no;
					coeff[var_count] = 1;
					var_count++;
				}
			}
			r = 0;
			for(i = 0; i <= j ; i++)
				r += ceil(Response[this_task]/T[i]);
			if(!add_constraintex(lp, var_count, coeff, var, LE, r))
				ret = 3;
		}
	}
	if(ret == 0) {
		/* --------------------adding second constraint------------------- */
		/* Example: For this task = T4, variables have been nubmered from 1 to 11 as follows:
		 * g2({T1}),g3({T1}),g3({T2}),g3({T1,T2}),g4({T1}),g4({T2}),g4({T1,T2}),g4({T3}),g4({T1,T3}),g4({T2,T3}),g4({T1,T2,T3})
		 */
		for(j = 1; j <= this_task; j++){
			int offset = 0;// offset => offset i.e. starting place
			double end;
			for(i = 1; i < j; i++){
				offset += pow(2,i) - 1;
			}
			end = offset + pow(2,j);// end => value we want ot reach to
			for(i = 0; i < j; i++){// All higher priority jobs
				int jump = pow(2,i);// jump => value we want ot jump to				

				var_count = 0;
				for(var_no = offset + jump; var_no < end; var_no += jump){

					int end2 = var_no + jump;
					
					for(; var_no < end2; var_no++){//var_no reaching to end2
						
						var[var_count] = var_no;
						coeff[var_count] = 1;
						var_count++;
					}

				}
				
				r = min(ceil(Response[this_task]/T[j])*ceil(Response[j]/T[i]),ceil(Response[this_task]/T[i]));
				if(!add_constraintex(lp, var_count, coeff, var, LE, r))
					ret = 3;
			}
		}
	}
	
	set_add_rowmode(lp, FALSE);
	if(ret == 0) {	
		/* -------------------adding objective function------------------- */
		var_count = 0;
		var_no = 1;
		for(j = 1; j <= this_task; j++){
			for(H = 1; H < pow(2,j); H++){
				var[var_count] = var_no;
				coeff[var_count] = var_on_to_cost(this_task, var_no);
				var_count++;
				var_no++;
			}
		}
		if(!set_obj_fnex(lp, var_count, coeff, var))
			ret = 4;
		set_maxim(lp);
		//write_LP(lp, fp);
		set_verbose(lp, IMPORTANT);
		ret = solve(lp);
		if(ret == OPTIMAL)
			ret = 0;
		else
			ret = 5;
	}
	if(ret == 0) {
		obj = get_objective(lp);
		/* Displaying calculated values */		
		/* variable values */
		//fprintf(fp, "\n/* Variable values */\n");
		//get_variables(lp, coeff);
		//for(j = 0; j < numVar; j++)
		//	fprintf(fp, "%s: %0.2f\n", get_col_name(lp, j + 1), coeff[j]);		
		/* objective value */
		//fprintf(fp, "\n/* Objective value */\n%0.2f\n", obj);
	}
	if(ret != 0)
		fprintf(fp, "\nLP ERROR = %d\n", ret);
	
	/* free allocated memory */
	if(coeff != NULL)
		free(coeff);
	if(var != NULL)
		free(var);
	if(lp != NULL) 
		delete_lp(lp);

	return ret == 0 ? obj : 0;
}

double min(double a, double b){
	return a < b ? a : b;
}

/* Example: For this task = T4, variables have been nubmered from 1 to 11 as follows:
* g2({T1}),g3({T1}),g3({T2}),g3({T1,T2}),g4({T1}),g4({T2}),g4({T1,T2}),g4({T3}),g4({T1,T3}),g4({T2,T3}),g4({T1,T2,T3})
* Given a var_no, the following function creates variable accoeding to the above order
*/
void var_no_to_name(const int this_task, const int var_no, char * var_name){
	int i, lp_task = 1, offset = 0, hp_task;
	// Append this task nmae to var_name
	itoa(this_task, var_name);
	// Append Letter 'g' to var_name
	strcat(var_name,"g");	
	for(i = 1; i < var_no; i += pow(2,lp_task+1)-1, lp_task++)
		offset = i;
	// Append lp_task to var_name
	itoa(lp_task, &var_name[strlen(var_name)]);
	// Append opening brackets '({' to var_name
	strcat(var_name,"({");	
	// Genereting hp task set
	for(hp_task = 0; hp_task < this_task; hp_task++)
	{
		int jump = pow(2, hp_task);
		if(((var_no - offset) / jump) % 2 == 1){
			char temp[10];
			if(var_name[strlen(var_name)-1] != '{'){
				strcat(var_name, ",");
			}
			sprintf(temp, "T%d", hp_task);
			strcat(var_name, temp);
		}
	}	
	// Append closing brackets '})' to var_name
	strcat(var_name,"})");
}

void itoa(int i, char s[10]){
	int j = 0;
	do{
		s[j++] = '0' + i % 10;
	}while((i /= 10) != 0);
	s[j] = '\0';
	strrev(s);
}

void strrev(char * s){
	int len = strlen(s), j = 0;
	while(j < len/2){
		char temp = s[j];
		s[j] = s[len-1-j];
		s[len-1-j] = temp;
		j++;
	}
}

/* Returns the cost of cache perrmption as:
 * UCB[this_task] INTERSECT (UNION(T | T is a set of tasks that execute during this_task's preemption))
*/
double var_on_to_cost(const int this_task, const int var_no){
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

