#ifndef HEADER_NNP
#define HEADER_NNP

#include"baseConfig.h"

double inv_max(int hp_task, int lp_task, double Response[]){
	return ceil(Response[lp_task] / T[hp_task]);
}

double inv_min(int hp_task, int lp_task, double Response[]){
	return floor(Response[lp_task] / T[hp_task]);
}

double calc_nnp_max(int hp_task, int lp_task, double Response[], int nnp_max[NUM_TASKS][NUM_TASKS], int nnp_min[NUM_TASKS][NUM_TASKS]){
	int i;
	double ret_val = inv_max(hp_task, lp_task, Response);
	for(i = hp_task + 1;i < lp_task; i++)
		ret_val -= nnp_min[hp_task][i] * inv_min(i, lp_task, Response);
	return ret_val > 0 ? ret_val : 0;
}

double calc_nnp_min(int hp_task, int lp_task, double Response[], int nnp_max[NUM_TASKS][NUM_TASKS], int nnp_min[NUM_TASKS][NUM_TASKS]){
	int i;
	double ret_val = inv_min(hp_task, lp_task, Response);
	for(i = hp_task + 1;i < lp_task; i++)
		ret_val -= nnp_max[hp_task][i] * inv_max(i, lp_task, Response);
	return ret_val > 0 ? ret_val : 0;
}

void getNnp(int this_task, double Response[], int nnp_max[NUM_TASKS][NUM_TASKS], int nnp_min[NUM_TASKS][NUM_TASKS]){
	int hp_task;
	if (this_task >= 1){		
		for(hp_task = this_task-1; hp_task >= 0; hp_task--){
			nnp_max[hp_task][this_task] = calc_nnp_max(hp_task, this_task, Response, nnp_max, nnp_min);
			nnp_min[hp_task][this_task] = calc_nnp_min(hp_task, this_task, Response, nnp_max, nnp_min);
		}
	}
}
#endif
