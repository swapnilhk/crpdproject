#include"kd.h"
#include<stdio.h>
#include<math.h>
#include<set>
#include"lp_lib.h"
#include"global.h"
#include"set_operations.h"

static int **nnpMax;
static int **nnpMin;

#define min3(a,b,c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))

static double inv_max(int hp_task, int lp_task, double Response[]){
	return ceil(Response[lp_task] / T[hp_task]);
}

static double inv_min(int hp_task, int lp_task, double Response[]){
	return floor(Response[lp_task] / T[hp_task]);
}

static double calc_nnpMax(int hp_task, int lp_task, double Response[]){
	int i;
	double ret_val = inv_max(hp_task, lp_task, Response);
	for(i = hp_task + 1;i < lp_task; i++)
		ret_val -= nnpMin[hp_task][i] * inv_min(i, lp_task, Response);
	return ret_val > 0 ? ret_val : 0;
}

static double calc_nnpMin(int hp_task, int lp_task, double Response[]){
	int i;
	double ret_val = inv_min(hp_task, lp_task, Response);
	for(i = hp_task + 1;i < lp_task; i++)
		ret_val -= nnpMax[hp_task][i] * inv_max(i, lp_task, Response);
	return ret_val > 0 ? ret_val : 0;
}

static void getNnp(int this_task, double Response[]){
	int hp_task;
	if (this_task >= 1)	
		for(hp_task = this_task-1; hp_task >= 0; hp_task--){
			nnpMax[hp_task][this_task] = calc_nnpMax(hp_task, this_task, Response);
			nnpMin[hp_task][this_task] = calc_nnpMin(hp_task, this_task, Response);
		}
}

static double costEcbUnion(int hpTask, int lpTask, double Response[]){
	std::set<int> workingSet1, workingSet2, workingSet3;
	int i;	
	workingSet1.clear();
	workingSet2.clear();
	for(i = 0; i <= hpTask; i++)
		Set_Union(workingSet1, TASK_ECB[i], workingSet1);
	Set_Intersect(workingSet1, TASK_UCB[lpTask], workingSet2);
	return BRT * SET_MOD(workingSet2);
}

static double solve_constraints_PRE_MAX_KD(int this_task, double Response[], FILE *fp)
{
	lprec *lp;
	int numVar = 0, *var = NULL, ret = 0, i, j, k, var_count;
	double *coeff = NULL, lhs,rhs, obj;
	char col_name[10];
	
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
				sprintf(col_name, "NNP%d_%d", i, j);
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
				sprintf(col_name,"NNP%d_%d", i, j);
				var[var_count] = get_nameindex(lp, col_name, FALSE);
				coeff[var_count] = ceil(Response[this_task] / T[j]);
				var_count++;
			}
			lhs= 0;
			for(i = 0; i < j; i++)
				lhs+= nnpMin[i][j];
			lhs*= floor(Response[this_task]/T[j]);

			rhs = 0;
			for(i = 0; i < j; i++)
				rhs += nnpMax[i][j];
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
					sprintf(col_name,"NNP%d_%d", i, j);
					var[var_count] = get_nameindex(lp, col_name, FALSE);
					coeff[var_count] = ceil(Response[this_task] / T[j]);
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
				lhs= floor(Response[this_task]/T[j]) * nnpMin[i][j];
				sprintf(col_name,"NNP%d_%d", i, j);
				var[0] = get_nameindex(lp, col_name, FALSE);
				coeff[0] = ceil(Response[this_task] / T[j]);
				if(!add_constraintex(lp, 1, coeff, var, GE, lhs))
					ret = 3;

				rhs = min3(
						ceil(Response[this_task]/T[i]),
						ceil(Response[this_task]/T[j]) * ceil(Response[j]/T[i]),
						ceil(Response[this_task]/T[j]) * nnpMax[i][j]
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
				sprintf(col_name,"NNP%d_%d", i, j);
				var[var_count] = get_nameindex(lp, col_name, FALSE);
				coeff[var_count] = ceil(Response[this_task] / T[j]);
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
				sprintf(col_name,"NNP%d_%d", i, j);
				var[var_count] = get_nameindex(lp, col_name, FALSE);
				coeff[var_count] = costEcbUnion(i, j, Response) * ceil(Response[this_task] / T[j]);
				var_count++;
			}
		}
		if(!set_obj_fnex(lp, var_count, coeff, var))
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
		fprintf(stderr, "\nLP ERROR = %d\n", ret);
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
static double PC_PRE_MAX_KD(int this_task, double Response[], FILE *fp){
	int hp_task;
	if (this_task >= 1){
		getNnp(this_task, Response);
		return solve_constraints_PRE_MAX_KD(this_task, Response, fp);
	}
	else return 0;
}

int ResponseTimePreMaxKd(){
	int task_no;
	bool sched = true;
	FILE *fp;
	double *Response = 	(double*)malloc(sizeof(*Response) * NUM_TASKS);
	nnpMin = Make2DintArrayInt(NUM_TASKS, NUM_TASKS);
	nnpMax = Make2DintArrayInt(NUM_TASKS, NUM_TASKS);
	static int first_call = 1;
	char * filename = "out/kd.txt";

	if(MESSAGE_LEVEL > NONE){
		if(first_call){			
			fp = fopen(filename, "w");
			first_call = 0;
		}
		else fp = fopen(filename, "a");
		if(fp == NULL){
			fprintf(stderr, "***Unable to open file %s\n", filename);
			MESSAGE_LEVEL = NONE;
		}		
	}
	if(MESSAGE_LEVEL >= IMP)
		printTaskInfo(fp);
	for(task_no = 0; task_no < NUM_TASKS && sched; task_no++){
		if(MESSAGE_LEVEL > NONE)
			fprintf(fp, "\tT%d\t\n", task_no);
		wcrt(task_no, Response, fp, PC_PRE_MAX_KD);
		if(Response[task_no] > D[task_no])
			sched = false;
		if(MESSAGE_LEVEL >= IMP)
			fprintf(fp, "[T%d(C=%g,T=%ld,D=%ld) is %s]\n\n", task_no, C[task_no], T[task_no], D[task_no], sched ? "SCEDLABLE":"NOT SCEDLABLE");
	}
	if(sched)
		Num_Executed_Tasks[PRE_MAX_KD]++;
	if(MESSAGE_LEVEL > NONE && fp != NULL)
		fclose(fp);	
	free(Response);	
	free2DintArrayInt(nnpMin, NUM_TASKS);
	free2DintArrayInt(nnpMax, NUM_TASKS);
	return sched ? 1 : 0;
}

static double solve_constraints_PRE_MAX_KD2(int this_task, double Response[], FILE *fp)
{
	lprec *lp;
	int numVar = 0, *var = NULL, ret = 0, i, j, k, var_count;
	double *coeff = NULL, lhs,rhs, obj;
	char col_name[10];
	
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
				sprintf(col_name,"%dNNP%d_%d",this_task, i, j);
				var[var_count] = get_nameindex(lp, col_name, FALSE);
				coeff[var_count] = 1;
				var_count++;
			}

			lhs= 0;
			for(i = 0; i < j; i++)
				lhs+= nnpMin[i][j];
			lhs*= floor(Response[this_task]/T[j]);

			rhs = 0;
			for(i = 0; i < j; i++)
				rhs += nnpMax[i][j];
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
				lhs= floor(Response[this_task]/T[j]) * nnpMin[i][j];
				sprintf(col_name,"%dNNP%d_%d", this_task, i, j);
				var[0] = get_nameindex(lp, col_name, FALSE);
				coeff[0] = 1;
				if(!add_constraintex(lp, 1, coeff, var, GE, lhs))
					ret = 3;

				rhs = min3(
						ceil(Response[this_task]/T[i]),
						ceil(Response[this_task]/T[j]) * ceil(Response[j]/T[i]),
						ceil(Response[this_task]/T[j]) * nnpMax[i][j]
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
		var_count = 0;
		for(i = 0 ; i < this_task; i++){
			for(j = i+1 ; j<= this_task; j++){
				sprintf(col_name,"%dNNP%d_%d",this_task, i, j);
				var[var_count] = get_nameindex(lp, col_name, FALSE);
				coeff[var_count] = costEcbUnion(i, j, Response);
				var_count++;
			}
		}
		if(!set_obj_fnex(lp, var_count, coeff, var))
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
		fprintf(stderr, "\nLP ERROR = %d\n", ret);
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
static double PC_PRE_MAX_KD2(int this_task, double Response[], FILE *fp){
	int hp_task;
	if (this_task >= 1){
		getNnp(this_task, Response);
		// Define constraints
		return solve_constraints_PRE_MAX_KD2(this_task, Response, fp);
	}
	else return 0;
}

int ResponseTimePreMaxKd2(){
	int task_no;
	bool sched = true;
	FILE *fp;
	double *Response = (double*)malloc(sizeof(*Response) * NUM_TASKS);
	nnpMin = Make2DintArrayInt(NUM_TASKS, NUM_TASKS);
	nnpMax = Make2DintArrayInt(NUM_TASKS, NUM_TASKS);
	static int first_call = 1;
	char * filename = "out/kd2.txt";

	if(MESSAGE_LEVEL > NONE){
		if(first_call){			
			fp = fopen(filename, "w");
			first_call = 0;
		}
		else fp = fopen(filename, "a");
		if(fp == NULL){
			fprintf(stderr, "***Unable to open file %s\n", filename);
			MESSAGE_LEVEL = NONE;
		}		
	}
	if(MESSAGE_LEVEL >= IMP)
		printTaskInfo(fp);
	for(task_no = 0; task_no < NUM_TASKS && sched; task_no++){
		if(MESSAGE_LEVEL > NONE)
			fprintf(fp, "\tT%d\t\n", task_no);
		wcrt(task_no, Response, fp, PC_PRE_MAX_KD2);		
		if(Response[task_no] > D[task_no])
			sched = false;		
		if(MESSAGE_LEVEL >= IMP)
			fprintf(fp, "[T%d(C=%g,T=%ld,D=%ld) is %s]\n\n", task_no, C[task_no], T[task_no], D[task_no], sched ? "SCEDLABLE":"NOT SCEDLABLE");
	}
	if(sched)
		Num_Executed_Tasks[PRE_MAX_KD2]++;
	if(MESSAGE_LEVEL > NONE && fp != NULL)
		fclose(fp);
	free(Response);
	free2DintArrayInt(nnpMin, NUM_TASKS);
	free2DintArrayInt(nnpMax, NUM_TASKS);
	return sched ? 1 : 0;
}
