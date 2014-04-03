#include<iostream>
#include<set>
#include<stdio.h>
#include<math.h>
#include"../lib/lp_solve_ux64/lp_lib.h"
#include"common.h"

int nnp_max[NUM_TASKS][NUM_TASKS];
int nnp_min[NUM_TASKS][NUM_TASKS];

/*
 * Returns minimum of three numbers
 * */
double min3(double a, double b, double c){
	return a < b ? (a < c ? a : c) : (b < c ? b : c);
}

double inv_max(int hp_task, int lp_task, double Response[]){
	return ceil(Response[lp_task] / T[hp_task]);
}

double inv_min(int hp_task, int lp_task, double Response[]){
	return floor(Response[lp_task] / T[hp_task]);
}

double calc_nnp_max(int hp_task, int lp_task, double Response[]){
	int i;
	double ret_val = inv_max(hp_task, lp_task, Response);
	for(i = hp_task + 1;i < lp_task; i++)
		ret_val -= nnp_min[hp_task][i] * inv_min(i, lp_task, Response);
	return ret_val > 0 ? ret_val : 0;
}

double calc_nnp_min(int hp_task, int lp_task, double Response[]){
	int i;
	double ret_val = inv_min(hp_task, lp_task, Response);
	for(i = hp_task + 1;i < lp_task; i++)
		ret_val -= nnp_max[hp_task][i] * inv_max(i, lp_task, Response);
	return ret_val > 0 ? ret_val : 0;
}

/*

 * Function to find set of blocks of this_task that are affected by the execution of hp_task.
 * Method used is ucb union
 * TODO: Change method to combined method
*/
/*void f(int this_task, int hp_task, std::set<int> & ret_set){
	int num_blocks = 0, aff;
	extern std::set<int> TASK_ECB[NUM_TASKS], TASK_UCB[NUM_TASKS];

	std::set<int> workingSet1, workingSet2;
	workingSet1.clear();
	workingSet2.clear();

	for(aff = this_task; aff > hp_task; aff--){
		Set_Union(workingSet1, TASK_UCB[aff], workingSet1);
	}
	
	Set_Intersect(workingSet1, TASK_ECB[hp_task], ret_set);

}*/

/*
 * Procedure to get cost function:
 * 1. if ip_task != lp_task then cost = f(this,task, hp_task) UNION f(this,task, ip_task)
 *    if ip_task == lp_task then cost = f(this,task, hp_task)
*/
/*double get_f(int this_task, int hp_task, int ip_task){
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
}*/





double get_f(int this_task, int hp_task, int lp_task, double Response[])
{
	std::set<int> workingSet1, workingSet2, workingSet3;
	extern std::set<int> TASK_ECB[NUM_TASKS], TASK_UCB[NUM_TASKS];
	int i;
	
	workingSet1.clear();
	workingSet2.clear();

	for(i = 0; i <= hp_task; i++){
		Set_Union(workingSet1, TASK_ECB[i], workingSet1);
	}

	Set_Intersect(workingSet1, TASK_UCB[lp_task], workingSet2);

	return BRT * ceil(Response[this_task]/T[lp_task]) * SET_MOD(workingSet2);
}







void get_hp_task_set(const int this_task, const int hp_task_set_no, char * hp_task_set){
	int i, lp_task = 1, offset = 0, hp_task;
	char temp[10];

	strcpy(hp_task_set, "");
	
	for(i = 1; i < hp_task_set_no; i += pow(2,lp_task+1)-1, lp_task++)
		offset = i;	

	// Genereting hp task set
	for(hp_task = 0; hp_task < this_task; hp_task++)
	{
		int jump = pow(2, hp_task);
		if(((hp_task_set_no - offset) / jump) % 2 == 1){			
			sprintf(temp, "%d", hp_task);
			strcat(hp_task_set, temp);
		}
	}
	sprintf(temp, "%d", lp_task);
	strcat(hp_task_set, temp);
}










double solve_constraints_PRE_MAX_KD(int this_task, double Response[], FILE *fp)
{
	lprec *lp;
	int numVar = 0, *var = NULL, ret = 0, i, j, k, var_count;
	double *coeff = NULL, lhs,rhs, obj;
	char col_name[10];
	int hp_task_set_no;
	char hp_task_set[50];
	
	/* Creating a model */
	numVar = this_task * (this_task + 1) / 2;

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
				sprintf(col_name,"%dNNP%d_%d", this_task, i, j);
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
				for(i = 0; i < j; i++){
					sprintf(col_name,"%dNNP%d_%d", this_task, i, j);
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
				sprintf(col_name,"%dNNP%d_%d", this_task, i, j);
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
				sprintf(col_name,"%dNNP%d_%d", this_task, i, j);
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
		/*var_count = 0;
		for(i = 0 ; i < this_task; i++){
			for(j = i+1 ; j<= this_task; j++){
				sprintf(col_name,"NNP%d_%d", i, j);
				var[var_count] = get_nameindex(lp, col_name, FALSE);
				coeff[var_count] = get_f(this_task, i, j, Response) * ceil(Response[this_task] / T[j]);
				var_count++;
			}
		}*/
				
		int var1[500];
		double coeff1[500];
		var_count = 0;
		for(hp_task_set_no = 1 ; hp_task_set_no <= numVar; hp_task_set_no++){
			get_hp_task_set(this_task, hp_task_set_no, hp_task_set);
			for(i = 0; i < strlen(hp_task_set)-1; i++){
				for(j = i+1; j < strlen(hp_task_set); j++){
					sprintf(col_name, "%dNNP%c_%c", this_task, hp_task_set[i], hp_task_set[j]);
					var1[var_count] = get_nameindex(lp, col_name, FALSE);
					coeff1[var_count] = var_no_to_cost(this_task, hp_task_set_no) /*/ (strlen(hp_task_set)-1)*/;
					var_count++;
				}
			}
			//sprintf(col_name, "%dNNP%c_%c", this_task, hp_task_set[j-1], hp_task_set[strlen(hp_task_set)-1]);
			//var1[var_count] = get_nameindex(lp, col_name, FALSE);
			//coeff1[var_count] = var_no_to_cost(this_task, hp_task_set_no)/* / (strlen(hp_task_set)-1)*/;
			//var_count++;
		}
		
		
		if(!set_obj_fnex(lp, var_count, coeff1, var1))
			ret = 4;
		set_maxim(lp);

		if(MESSAGE_LEVEL >= ALL)			
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

		if(MESSAGE_LEVEL >= ALL){
			/* Displaying calculated values */		
			/* variable values */
			fprintf(fp, "\n/* Variable values */\n");
			get_variables(lp, coeff);
			for(j = 0; j < numVar; j++)
				fprintf(fp, "%s: %0.2f\n", get_col_name(lp, j + 1), coeff[j]);		
			/* objective value */
			fprintf(fp, "\n/* Objective value */\n%0.2f\n", obj);
		}
	}
	if(ret != 0){
		if(MESSAGE_LEVEL >= IMP)
			fprintf(fp, "\nLP ERROR = %d\n", ret);		
		printf("\nLP ERROR = %d\n", ret);
	}
	/* free allocated memory */
	if(coeff != NULL)
		free(coeff);
	if(var != NULL)
		free(var);
	if(lp != NULL);
		delete_lp(lp);

	return ret == 0 ? obj : INFINITY;
}




// Returns preemption cost for task 'this_task'
double PC_PRE_MAX_KD(int this_task, double Response[], FILE *fp){
	int hp_task;
	if (this_task >= 1){
		for(hp_task = this_task-1; hp_task >= 0; hp_task--){
			nnp_max[hp_task][this_task] = calc_nnp_max(hp_task, this_task, Response);
			nnp_min[hp_task][this_task] = calc_nnp_min(hp_task, this_task, Response);
		}
		// Define constraints
		return solve_constraints_PRE_MAX_KD(this_task, Response, fp);
	}
	else return 0;
}




double sigma_tda_PRE_MAX_KD(int this_task, double Response[]){
	double R_new = 0;
	int hp_task = this_task - 1;
	while(hp_task >= 0){
		R_new += ceil(Response[this_task]/T[hp_task]) * C[hp_task];
		hp_task = hp_task - 1;
	}
	return R_new;
}




double wcrt_PRE_MAX_KD(int this_task, double Response[], FILE *fp){
	double R_new;
	R_new = C[this_task];
	Response[this_task] = 0;
	while(R_new != Response[this_task] && (Response[this_task] = R_new) <= D[this_task]){
		R_new = C[this_task] 
			+ sigma_tda_PRE_MAX_KD(this_task, Response) 
			+ PC_PRE_MAX_KD(this_task, Response, fp);// Time demand equation
		if(MESSAGE_LEVEL >= IMP)
			fprintf(fp, "T%d(D=%ld) Response time: Old = %g, New = %g\n\n", this_task, D[this_task], Response[this_task], R_new);
	}
	return R_new;
}




int Response_time_PRE_MAX_KD(){
	int task_no;
	bool sched = true;
	FILE *fp;
	double Response[NUM_TASKS];
	static int first_call = 1;

	if(MESSAGE_LEVEL > NONE){

		if(first_call){			
			fp = fopen("out/kd.txt", "w");
			first_call = 0;
		}
		else
			fp = fopen("out/kd.txt", "a");

		if(fp == NULL){
			printf("***Unable to open file\n");
			MESSAGE_LEVEL = NONE;
		}		
	}

	if(MESSAGE_LEVEL >= IMP)
		printTaskInfo(fp);


	for(task_no = 0; task_no < NUM_TASKS && sched; task_no++)
	{
		if(MESSAGE_LEVEL > NONE)
			fprintf(fp, "\tT%d\t\n", task_no);

		wcrt_PRE_MAX_KD(task_no, Response, fp);
		
		if(Response[task_no] > D[task_no])
			sched = false;
		
		if(MESSAGE_LEVEL >= IMP){
			fprintf(fp, "[T%d(C=%g,T=%ld,D=%ld) is %s]\n\n", task_no, C[task_no], T[task_no], D[task_no], sched ? "SCEDLABLE":"NOT SCEDLABLE");
		}

	}
	if(sched)
		Num_Executed_Tasks[PRE_MAX_KD]++;

	if(MESSAGE_LEVEL > NONE && fp != NULL)
		fclose(fp);
	
	return sched ? 1 : INFINITY;
}
