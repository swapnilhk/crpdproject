#include<iostream>
#include<set>
#include<stdio.h>
#include<math.h>
#include"../lib/lp_solve_ux64/lp_lib.h"
#include"global.h"
#include"tda.h"
#include"nnp.h"

double solve_constraints_PRE_MAX_KD2(int this_task, double Response[], int nnp_max[NUM_TASKS][NUM_TASKS], int nnp_min[NUM_TASKS][NUM_TASKS], FILE *fp)
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
double PC_PRE_MAX_KD2(int this_task, double Response[], FILE *fp){
	int hp_task;
	static int nnpMax[NUM_TASKS][NUM_TASKS];
	static int nnpMin[NUM_TASKS][NUM_TASKS];	
	if (this_task >= 1){
		getNnp(this_task, Response, nnpMax, nnpMin);
		// Define constraints
		return solve_constraints_PRE_MAX_KD2(this_task, Response, nnpMax, nnpMin, fp);
	}
	else return 0;
}

int ResponseTimePreMaxKd2(){
	int task_no;
	bool sched = true;
	FILE *fp;
	double Response[NUM_TASKS];
	static int first_call = 1;

	if(MESSAGE_LEVEL > NONE){
		if(first_call){			
			fp = fopen("out/kd2.txt", "w");
			first_call = 0;
		}
		else fp = fopen("out/kd2.txt", "a");
		if(fp == NULL){
			printf("***Unable to open file\n");
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
	return sched ? 1 : 0;
}
