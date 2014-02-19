#include<iostream>
#include<set>
#include<stdio.h>
#include<math.h>
#include"../lib/lp_solve_ux64/lp_lib.h"
#include"common.h"

double Response_PRE_MAX_KD[NUM_TASKS];
int nnp_max[NUM_TASKS][NUM_TASKS];
int nnp_min[NUM_TASKS][NUM_TASKS];
static FILE *fp2;

double wcrt_PRE_MAX_KD(int);
double sigma_tda_PRE_MAX_KD(int);
double min3(double, double, double);
double get_f_PRE_MAX_KD(int, int, int);
double PC_PRE_MAX_KD(int);
double inv_max(int hp_task, int lp_task);
double inv_min(int hp_task, int lp_task);
double calc_nnp_max(int hp_task, int lp_task);
double calc_nnp_min(int hp_task, int lp_task);
double solve_constraints_PRE_MAX_KD(int this_task);

void Response_time_PRE_MAX_KD(FILE *fp1){
	int task_no;
	fp2 = fp1;
	bool sched = true;
	clear_Response();

	fprintf(fp2, "                              *******************                                     \n");
	fprintf(fp2, "****************************** PRE MAX KD BEGINS **************************************\n");
	fprintf(fp2, "                              *******************                                     \n");

	for(task_no = 0; task_no < NUM_TASKS && sched; task_no++){
		wcrt_PRE_MAX_KD(task_no);
		fprintf(fp2, "======================================================================================\n");
		if(Response[task_no] > D[task_no]){
			sched = false;
			//printf("TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", task_no, taskSetUtil, Response[task_no], D[task_no]);
			fprintf(fp2, "TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", task_no, taskSetUtil, Response[task_no], D[task_no]);
		}
		else{
			//printf("TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", task_no, taskSetUtil, Response[task_no], D[task_no]);
			fprintf(fp2, "TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", task_no, taskSetUtil, Response[task_no], D[task_no]);			
		}
		fprintf(fp2, "======================================================================================\n\n");
	}
	fprintf(fp2, "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	if(sched == false)
	{
		//printf("TASKSET NOT schedulable under PRE-MAX-KD at TASKSET_UTIL = %f \n", taskSetUtil);
		fprintf(fp2, "TASKSET is NOT schedulable under PRE-MAX-KD at TASKSET_UTIL = %f \n", taskSetUtil);
	}
	else
	{
		//printf("TASKSET IS schedulable under PRE-MAX-KD at TASKSET_UTIL = %f \n", taskSetUtil);
		fprintf(fp2, "TASKSET IS schedulable under PRE-MAX-KD at TASKSET_UTIL = %f \n", taskSetUtil);
		Num_Executed_Tasks[PRE_MAX_KD]++;
	}
	fprintf(fp2, "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
	fprintf(fp2, "________________________________PRE MAX KD ENDS_______________________________________\n\n\n\n");
}

double wcrt_PRE_MAX_KD(int this_task){
	double R_new;
	R_new = C[this_task];
	while(R_new != Response[this_task] && (Response[this_task] = R_new) <= D[this_task]){
		R_new = C[this_task] + sigma_tda_PRE_MAX_KD(this_task) + PC_PRE_MAX_KD(this_task);// Time demand equation
		fprintf(fp2, "--------------------------------------------------------------------------------------\n");
		fprintf(fp2, "TASK_NO =  %d Old response time = %g New response time = %g Deadline = %ld\n", this_task, Response[this_task], R_new, D[this_task]);
		fprintf(fp2, "--------------------------------------------------------------------------------------\n\n");
	}
	return R_new;
}

double sigma_tda_PRE_MAX_KD(int this_task){
	double R_new = 0;
	int hp_task = this_task - 1;
	while(hp_task >= 0){
		R_new += ceil(Response[this_task]/T[hp_task]) * C[hp_task];
		hp_task = hp_task - 1;
	}
	return R_new;
}

// Returns preemption cost for task 'this_task'
double PC_PRE_MAX_KD(int this_task){
	int hp_task;
	if (this_task >= 1){
		for(hp_task = this_task-1; hp_task >= 0; hp_task--){
			nnp_max[hp_task][this_task] = calc_nnp_max(hp_task, this_task);
			nnp_min[hp_task][this_task] = calc_nnp_min(hp_task, this_task);
		}
		// Define constraints
		return solve_constraints_PRE_MAX_KD(this_task);
	}
	else return 0;
}

double solve_constraints_PRE_MAX_KD(int this_task)
{
	lprec *lp;
	int numVar = 0, *var = NULL, ret = 0, i, j, k, var_count;
	double *coeff = NULL, lhs,rhs, obj;
	char col_name[10];
	
	/* Creating a model */
	for(i = 1;i <= this_task; i++)
		numVar+=i;
	lp = make_lp(0, numVar);
	if(lp == NULL)
		ret = 1; /* Couldn't construct a new model */

	if(ret == 0) {
		var_count = 1;
		for(i = 0 ; i < this_task; i++){
			for(j = i+1 ; j <= this_task; j++)
			{
				sprintf(col_name, "%dNNP%d_%d", this_task, i, j);
				set_col_name(lp, var_count, col_name);
				var_count++;
			}
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
		/* --------------------adding EQN-D-------------------- */
		for(j = 1;j <= this_task;j++){
			var_count = 0;
			for(i = 0; i < j; i++){
				sprintf(col_name,"%dNNP%d_%d",this_task, i, j);
				var[var_count] = get_nameindex(lp, col_name, FALSE);
				coeff[var_count] = 1;
				var_count++;
			}

			lhs= 0;
			for(i = 0; i < j; i++)
				lhs+= nnp_min[i][j];
			lhs*= floor(Response[this_task]/T[j]);

			rhs = 0;
			for(i = 0; i < j; i++)
				rhs += nnp_max[i][j];
			rhs *= ceil(Response[this_task]/T[j]);

			if(!add_constraintex(lp, var_count, coeff, var, GE, lhs))
				ret = 3;
			if(!add_constraintex(lp, var_count, coeff, var, LE, rhs))
				ret = 3;
		}
	}

	if(ret == 0) {
		/* --------------------adding EQN-E-------------------- */
		for(k = 1;k <= this_task;k++)
		{
			var_count = 0;
			for(j = 1; j <= k; j++){
				for(i = 1; i < j; i++){
					sprintf(col_name,"%dNNP%d_%d",this_task, i, j);
					var[var_count] = get_nameindex(lp, col_name, FALSE);
					coeff[var_count] = 1;
					var_count++;
				}
			}

			rhs = 0;
			for(i = 0; i < k; i++)
				rhs += ceil(Response[this_task]/T[i]);
			if(!add_constraintex(lp, var_count, coeff, var, LE,rhs))
				ret = 3;
		}
	}

	if(ret == 0) {
		/* ------------------adding EQN-G & H------------------ */
		for(j = 1; j <= this_task ; j++){
			for(i = 0; i < j; i++){
				lhs= floor(Response[this_task]/T[j]) * nnp_min[i][j];
				sprintf(col_name,"%dNNP%d_%d",this_task, i, j);
				var[0] = get_nameindex(lp, col_name, FALSE);
				coeff[0] = 1;
				if(!add_constraintex(lp, 1, coeff, var, GE, lhs))
					ret = 3;

				rhs = min3(
						ceil(Response[this_task]/T[i]),
						ceil(Response[this_task]/T[j]) * ceil(Response[j]/T[i]),
						ceil(Response[this_task]/T[j]) * nnp_max[i][j]
				);
				if(!add_constraintex(lp, 1, coeff, var, LE,rhs))
					ret = 3;
			}
		}
	}

	if(ret == 0) {
		/* --------------------adding EQN-I-------------------- */
		for(i = 0; i < this_task; i++){
			var_count = 0;
			for(j = i+1; j <= this_task; j++){
				sprintf(col_name,"%dNNP%d_%d",this_task, i, j);
				var[var_count] = get_nameindex(lp, col_name, FALSE);
				coeff[var_count] = 1;
				var_count++;
			}
			rhs = ceil(Response[this_task]/T[i]);
			if(!add_constraintex(lp, var_count, coeff, var, LE,rhs))
				ret = 3;
		}
	}

	set_add_rowmode(lp, FALSE);
	if(ret == 0) {
		/* -----------------set the objective----------------- */
		var_count = 0;
		for(i = 0 ; i < this_task; i++){
			for(j = i+1 ; j<= this_task; j++){
				sprintf(col_name,"%dNNP%d_%d",this_task, i, j);
				var[var_count] = get_nameindex(lp, col_name, FALSE);
				coeff[var_count] = get_f_PRE_MAX_KD(this_task, i, j);
				var_count++;
			}
		}
		if(!set_obj_fnex(lp, var_count, coeff, var))
			ret = 4;
		set_maxim(lp);
		//write_LP(lp,fp2);
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
		//fprintf(fp2, "\nVariable values:\n");
		//get_variables(lp, coeff);
		//fprintf(fp2, "\n");
		//for(j = 0; j < numVar; j++)
		//	fprintf(fp2, "%s: %f\n", get_col_name(lp, j+1), coeff[j]);
		/* objective value */
		//fprintf(fp2, "\nObjective value: %f\n\n", obj);
	}
	if(ret != 0)
		fprintf(fp2, "LP ERROR = %d\n\n", ret);

	/* free allocated memory */
	if(coeff != NULL)
		free(coeff);
	if(var != NULL)
		free(var);
	if(lp != NULL);
		delete_lp(lp);

	return ret == 0 ? obj : 0;
}

/*
 * Procedure to get cost function:
 * 1. if ip_task != lp_task then cost = f(this,task, hp_task) UNION f(this,task, ip_task)
 *    if ip_task == lp_task then cost = f(this,task, hp_task)
*/
double get_f_PRE_MAX_KD(int this_task, int hp_task, int ip_task){
	std::set<int> workingSet1, workingSet2, workingSet3;

	workingSet1.clear();
	workingSet2.clear();
	workingSet3.clear();

	if(ip_task != this_task){
		f(this_task, hp_task, workingSet1);
		f(this_task, ip_task, workingSet2);
		Set_Union(workingSet1, workingSet2, workingSet3);
		return BRT * SET_MOD(workingSet3);
	}
	else{
		f(this_task, hp_task, workingSet1);
		return BRT * SET_MOD(workingSet1);
	}
}

/*
 * Returns minimum of three numbers
 * */
double min3(double a, double b, double c){
	return a < b ? (a < c ? a : c) : (b < c ? b : c);
}

double inv_max(int hp_task, int lp_task){
	return ceil(Response[lp_task] / T[hp_task]);
}

double inv_min(int hp_task, int lp_task){
	return floor(Response[lp_task] / T[hp_task]);
}

double calc_nnp_max(int hp_task, int lp_task){
	int i;
	double ret_val = inv_max(hp_task, lp_task);
	for(i = hp_task + 1;i < lp_task; i++)
		ret_val -= nnp_min[hp_task][i] * inv_min(i, lp_task);
	return ret_val > 0 ? ret_val : 0;
}

double calc_nnp_min(int hp_task, int lp_task){
	int i;
	double ret_val = inv_min(hp_task, lp_task);
	for(i = hp_task + 1;i < lp_task; i++)
		ret_val -= nnp_max[hp_task][i] * inv_max(i, lp_task);
	return ret_val > 0 ? ret_val : 0;
}

