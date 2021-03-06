#include<iostream>
#include<set>
#include<stdio.h>
#include<math.h>
#include"../lib/lp_solve_ux64/lp_lib.h"
#include"common.h"
#include"set_operations.cpp"

double Response_PRE_MAX_KD[NUM_TASKS];
int nnp_max[NUM_TASKS][NUM_TASKS];
int nnp_min[NUM_TASKS][NUM_TASKS];
static FILE * fp;

double wcrt(int);
double sigma_tda(int);
double min3(double, double, double);
int get_f(int, int, int);
double PC(int);
double inv_max(int hp_task, int lp_task);
double inv_min(int hp_task, int lp_task);
double calc_nnp_max(int hp_task, int lp_task);
double calc_nnp_min(int hp_task, int lp_task);
double solve_constraints(int this_task);

void Response_time_PRE_MAX_KD(FILE *fp1){
	int task_no;
	fp = fp1;
	bool sched = true;

	fprintf(fp, "                              *******************                                     \n");
	fprintf(fp, "****************************** PRE MAX KD BEGINS **************************************\n");
	fprintf(fp, "                              *******************                                     \n");

	for(task_no = 0; task_no < NUM_TASKS && sched; task_no++){
		wcrt(task_no);
		fprintf(fp, "======================================================================================\n");
		if(Response[task_no] > D[task_no]){
			sched = false;
			printf("TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", task_no, taskSetUtil, Response[task_no], D[task_no]);
			fprintf(fp, "TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", task_no, taskSetUtil, Response[task_no], D[task_no]);
		}
		else{
			printf("TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", task_no, taskSetUtil, Response[task_no], D[task_no]);
			fprintf(fp, "TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", task_no, taskSetUtil, Response[task_no], D[task_no]);			
		}
		fprintf(fp, "======================================================================================\n\n");
	}
	fprintf(fp, "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	if(sched == false)
	{
		printf("TASKSET NOT schedulable under PRE-MAX-KD at TASKSET_UTIL = %f \n", taskSetUtil);
		fprintf(fp, "TASKSET is NOT schedulable under PRE-MAX-KD at TASKSET_UTIL = %f \n", taskSetUtil);
	}
	else
	{
		printf("TASKSET IS schedulable under PRE-MAX-KD at TASKSET_UTIL = %f \n", taskSetUtil);
		fprintf(fp, "TASKSET IS schedulable under PRE-MAX-KD at TASKSET_UTIL = %f \n", taskSetUtil);
		Num_Executed_Tasks[PRE_MAX_KD]++;
	}
	fprintf(fp, "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
	fprintf(fp, "________________________________PRE MAX KD ENDS_______________________________________\n\n\n\n");
}

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
		for(hp_task = this_task-1; hp_task >= 0; hp_task--){
			nnp_max[hp_task][this_task] = calc_nnp_max(hp_task, this_task);
			nnp_min[hp_task][this_task] = calc_nnp_min(hp_task, this_task);
		}
		// Define constraints
		return solve_constraints(this_task);
	}
	else return 0;
}

double solve_constraints(int this_task)
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
				coeff[var_count] = BRT * 1e-1 * get_f(this_task, i, j);
				var_count++;
			}
		}
		if(!set_obj_fnex(lp, var_count, coeff, var))
			ret = 4;
		set_maxim(lp);
		write_LP(lp, fp);
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
		fprintf(fp, "\nVariable values:\n");
		get_variables(lp, coeff);
		fprintf(fp, "\n");
		for(j = 0; j < numVar; j++)
			fprintf(fp, "%s: %f\n", get_col_name(lp, j+1), coeff[j]);
		/* objective value */
		fprintf(fp, "\nObjective value: %f\n\n", obj);
	}
	fprintf(fp, "LP ERROR = %d\n\n", ret);

	/* free allocated memory */
	if(coeff != NULL)
		free(coeff);
	if(var != NULL)
		free(var);
	if(lp != NULL);
		delete_lp(lp);

	return ret == 0 ? obj : 0;
}

int get_f(int this_task, int hp_task, int ip_task){
	int Num_Blocks = 0;
	extern std::set<int> TASK_ECB[NUM_TASKS], TASK_UCB[NUM_TASKS];

	std::set<int> workingSet1, workingSet2;
	workingSet1.clear();
	workingSet2.clear();
	Set_Union(TASK_ECB[hp_task], TASK_ECB[ip_task], workingSet1);
	Set_Intersect(workingSet1, TASK_UCB[this_task], workingSet2);
	Num_Blocks += nnp_max[hp_task][ip_task] * inv_max(ip_task, this_task) * SET_MOD(workingSet2);
	return Num_Blocks;
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

//void PC(FILE *fp){
//	lprec *lp;
//	int numVar = 0, *var = NULL, ret = 0, i, j, k, var_count, this_task;
//	double *coeff = NULL, lhs, rhs, response_time;
//	char col_name[10];
//	int hp_task, lp_task;
//	int flag;
//	bool sched = true;
//	int LAST_TASK = NUM_TASKS - 1;
//
//
//	clear_Response();
//	fprintf(fp, "************** PRE MAX KD ********************* \n\n");
//	printf     ("************** PRE MAX KD ********************* \n\n");
//	Response_PRE_MAX_KD[0] = Response[0] = C[0];
//	Reset_PRE_min();
//	Reset_PRE_max();
//
//	// Select a task 'this_task'
//	for (this_task = 0; this_task < NUM_TASKS && sched == true; this_task++){
//		flag = 1;
//		// Find NNP Values for all pairs {hp_task, this_task}
//		if (this_task >= 1){
//			for(hp_task = this_task-1; hp_task >= 0; hp_task--){
//				PRE_ij_max[hp_task][this_task] = calculate_pre_max_ij(hp_task, this_task);
//				PRE_ij_min[hp_task][this_task] = calculate_pre_max_ij(hp_task, this_task);
//			}
//		}
//		while(flag){
//			/* Creating a model */
//			for(i = 1; i < this_task; i++)
//				numVar+=i;
//			lp = make_lp(0, numVar);
//			if(lp == NULL)
//				ret = 1; /* Couldn't construct a new model */
//
//			if(ret == 0) {
//				var_count = 1;
//				for(i = 0 ; i < this_task; i++){
//					for(j = i+1 ; j <= this_task; j++)
//					{
//						sprintf(col_name, "%dNNP%d_%d", this_task, i, j);
//						set_col_name(lp, var_count, col_name);
//						var_count++;
//					}
//				}
//				/* create space large enough for one row(i.e. equation) */
//				var = (int *) malloc(numVar * sizeof(*var));
//				coeff = (double *) malloc(numVar * sizeof(*coeff));
//				if((var == NULL) || (coeff == NULL))
//					ret = 2;
//			}
//			// Create equations using NNP values for {hp_task, this_task}
//			/* add the equations to lpsolve */
//			if(ret == 0) {
//				set_add_rowmode(lp, TRUE);
//				/* --------------------adding EQN-D-------------------- */
//				for(j = 1; j <= this_task; j++){
//					var_count = 0;
//					for(i = 1; i < j; i++){
//						sprintf(col_name,"%dNNP%d_%d",this_task, i, j);
//						var[var_count] = get_nameindex(lp, col_name, FALSE);
//						coeff[var_count] = 1;
//						var_count++;
//					}
//
//					lhs= 0;
//					for(i = 0; i < j; i++)
//						lhs+= PRE_ij_min[i][j];
//					lhs*= floor(Response[this_task]/T[j]);
//
//					rhs = 0;
//					for(i = 0; i < j; i++)
//						rhs += PRE_ij_max[i][j];
//					rhs *= ceil(Response[this_task]/T[j]);
//
//					if(!add_constraintex(lp, var_count, coeff, var, GE, lhs))
//						ret = 3;
//					if(!add_constraintex(lp, var_count, coeff, var, LE, rhs))
//						ret = 3;
//				}
//			}
//
//			if(ret == 0) {
//				/* --------------------adding EQN-E-------------------- */
//				for(k = 1;k <= this_task;k++)
//				{
//					var_count = 0;
//					for(j = 1; j <= k; j++){
//						for(i = 0; i < j; i++){
//							sprintf(col_name,"%dNNP%d_%d",this_task, i, j);
//							var[var_count] = get_nameindex(lp, col_name, FALSE);
//							coeff[var_count] = 1;
//							var_count++;
//						}
//					}
//
//					rhs = 0;
//					for(i = 0; i < k; i++)
//						rhs += ceil(Response[this_task]/T[i]);
//					if(!add_constraintex(lp, var_count, coeff, var, LE,rhs))
//						ret = 3;
//				}
//			}
//
//			if(ret == 0) {
//				/* ------------------adding EQN-G & H------------------ */
//				for(j = 1; j <= this_task ; j++){
//					for(i = 0; i < j; i++){
//						lhs= floor(Response[this_task]/T[j]) * PRE_ij_min[i][j];
//						sprintf(col_name,"%dNNP%d_%d",this_task, i, j);
//						var[0] = get_nameindex(lp, col_name, FALSE);
//						coeff[0] = 1;
//						if(!add_constraintex(lp, 1, coeff, var, GE, lhs))
//							ret = 3;
//
//						rhs = min3(ceil(Response[this_task]/T[i]), ceil(Response[this_task]/T[j]) * ceil(Response[j]/T[i]), ceil(Response[this_task]/T[j]) * PRE_ij_max[i][j]);
//						if(!add_constraintex(lp, 1, coeff, var, LE,rhs))
//							ret = 3;
//					}
//				}
//			}
//
//			if(ret == 0) {
//				/* --------------------adding EQN-I-------------------- */
//				for(i = 0; i < this_task; i++){
//					var_count = 0;
//					for(j = i+1; j <= this_task; j++){
//						sprintf(col_name,"%dNNP%d_%d",this_task, i, j);
//						var[var_count] = get_nameindex(lp, col_name, FALSE);
//						coeff[var_count] = 1;
//						var_count++;
//					}
//					rhs = ceil(Response[this_task]/T[i]);
//					if(!add_constraintex(lp, var_count, coeff, var, LE,rhs))
//						ret = 3;
//				}
//			}
//			// Create objective function
//			set_add_rowmode(lp, FALSE);
//			if(ret == 0) {
//				/* -----------------set the objective----------------- */
//				var_count = 0;
//				for(hp_task = 0 ; hp_task < this_task; hp_task++){
//					int ip_task;
//					for(ip_task = hp_task + 1 ; ip_task <= this_task; ip_task++){
//						fprintf(fp, "***hp_task = %d ip_task = %d this_task = %d\n", hp_task, ip_task, this_task);
//						sprintf(col_name,"%dNNP%d_%d",this_task, hp_task, ip_task);
//						var[var_count] = get_nameindex(lp, col_name, FALSE);
//						coeff[var_count] =  BRT * get_f(this_task, hp_task, ip_task);
//						var_count++;
//					}
//				}
//				if(!set_obj_fnex(lp, var_count, coeff, var))
//					ret = 4;
//				set_maxim(lp);
//				write_LP(lp, fp);
//				set_verbose(lp, IMPORTANT);
//				ret = solve(lp);
//				if(ret == OPTIMAL)
//					ret = 0;
//				else
//					ret = 5;
//			}
//			printf("***ret = %d\n", ret);
//			fprintf(fp, "***ret = %d\n", ret);
//			if(ret == 0) {
//				response_time = get_objective(lp);
//				printf("***response_time = %g Response[i] = %g\n", response_time, Response[i]);
//				fprintf(fp, "***response_time = %g Response[i] = %g\n", response_time, Response[i]);
//				if (response_time <=  Response[i])
//					flag = 0;
//				else{
//					Response_PRE_MAX_KD[i] = Response[i] = response_time;
//					if (Response[i] > D[i]){
//						flag = 0;
//						sched = false;
//						LAST_TASK = i;
//					}
//					// printf("acc =%f, response = %f, going again \n\n", acc, Response[j]);
//				}
//			}
//			flag = 0;
//		}
//	}
//	//flag = 1;
//	for(i = 0; i < NUM_TASKS; i++)
//	{
//		if ( Response[i] > D[i])
//		{
//			flag = 0;
//			sched = false;
//			printf("TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
//			fprintf(fp, "TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
//		}
//		else
//		{
//			printf("TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
//			fprintf(fp, "TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
//		}
//	}
//	if(sched == false)
//	{
//		printf("\nTASKSET NOT schedulable under PRE-MAX-KD at TASKSET_UTIL = %f \n", taskSetUtil);
//		fprintf(fp, "\nTASKSET is NOT schedulable under PRE-MAX-KD at TASKSET_UTIL = %f \n", taskSetUtil);
//	}
//	else
//	{
//		printf("\nTASKSET IS schedulable under PRE-MAX-KD at TASKSET_UTIL = %f \n", taskSetUtil);
//		fprintf(fp, "\nTASKSET IS schedulable under PRE-MAX-KD at TASKSET_UTIL = %f \n", taskSetUtil);
//
//		Num_Executed_Tasks[PRE_MAX_KD]++;
//
//		/*if(BDU_found == false)
//	             {
//	                 BDU_found = true;
//	                 fprintf(BDU_fp, "\t\t PRE MAX  \t\t\t\t %f \t\t %f \n\n", taskSetUtil, multFactor);
//	             }*/
//	}
//	/* free allocated memory */
//	if(coeff != NULL)
//		free(coeff);
//	if(var != NULL)
//		free(var);
//	if(lp != NULL)
//		delete_lp(lp);
//	fprintf(fp, "************** PRE MAX KD Ends ********************* \n\n" );
//	std::cout<< "************** PRE MAX KD Ends *********************" << std::endl << std::endl;
//}
