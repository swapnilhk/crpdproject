#include <cstdlib>
#include <iostream>
#include<math.h>
#include<set>
#include"base_config.h"
#include"set_operations.cpp"
#include"common.h"

using namespace std;

// to scale Task Periods
float multFactor;

double Gamma_i_j_ECB_Only[NUM_TASKS][NUM_TASKS];
double Gamma_i_j_UCB_Only[NUM_TASKS][NUM_TASKS];
double Gamma_i_j_UCB_Union[NUM_TASKS][NUM_TASKS];
double Gamma_i_j_ECB_Union[NUM_TASKS][NUM_TASKS];

double Gamma_i_j_Staschulat[NUM_TASKS][NUM_TASKS];
double Gamma_i_j_ECB_Union_Multiset[NUM_TASKS][NUM_TASKS];
double Gamma_i_j_UCB_Union_Multiset[NUM_TASKS][NUM_TASKS];

double Gamma_i_j_Staschulat_PRE[NUM_TASKS][NUM_TASKS];
double Gamma_i_j_ECB_Union_Multiset_PRE[NUM_TASKS][NUM_TASKS];
double Gamma_i_j_UCB_Union_Multiset_PRE[NUM_TASKS][NUM_TASKS];

double Gamma_i_j_PRE_MAX[NUM_TASKS][NUM_TASKS];

long Num_Displaced_Blocks_ECB_Only[NUM_TASKS];
long Num_Displaced_Blocks_UCB_Only[NUM_TASKS];
long Num_Displaced_Blocks_UCB_Union[NUM_TASKS];
long Num_Displaced_Blocks_ECB_Union[NUM_TASKS];

long Num_Displaced_Blocks_Staschulat[NUM_TASKS];
long Num_Displaced_Blocks_ECB_Union_Multiset[NUM_TASKS];
long Num_Displaced_Blocks_UCB_Union_Multiset[NUM_TASKS];
long Num_Displaced_Blocks_ECB_UCB_Union_Multiset_Combined[NUM_TASKS];

long Num_Displaced_Blocks_Staschulat_PRE[NUM_TASKS];
long Num_Displaced_Blocks_ECB_Union_Multiset_PRE[NUM_TASKS];
long Num_Displaced_Blocks_UCB_Union_Multiset_PRE[NUM_TASKS];
long Num_Displaced_Blocks_ECB_UCB_Union_Multiset_Combined_PRE[NUM_TASKS];

long Num_Displaced_Blocks_PRE_MAX[NUM_TASKS];

/* Input */

// Exec times Malardalen
//double Cycles[NUM_TASKS] = {445, 504, 1252, 1351, 6573, 13449, 17088, 22146, 29160, 39962, 43319, 214076, 290782, 742585, 1567222};
double Cycles[15] = {445, 504, 1252, 1351, 6573, 13449, 17088, 22146, 29160, 39962, 43319, 214076, 290782, 742585, 1567222};

double Response[NUM_TASKS] = {0, 0, 0, 0};

double Response_NO_PREEMPT[NUM_TASKS];
double Response_ECB_Only[NUM_TASKS], Response_UCB_Only[NUM_TASKS], Response_UCB_Union[NUM_TASKS], Response_ECB_Union[NUM_TASKS];
double Response_Staschulat[NUM_TASKS];
double Response_ECB_Union_Multiset[NUM_TASKS], Response_UCB_Union_Multiset[NUM_TASKS], Response_ECB_UCB_Union_Multiset_Combined[NUM_TASKS]; 

double Response_Staschulat_PRE[NUM_TASKS];
double Response_ECB_Union_Multiset_PRE[NUM_TASKS], Response_UCB_Union_Multiset_PRE[NUM_TASKS], Response_ECB_UCB_Union_Multiset_Combined_PRE[NUM_TASKS]; 
double Response_PRE_MAX[NUM_TASKS];

int Cost[NUM_TASKS] = {2, 4, 6, 8};
int Useful_cost[NUM_TASKS] = {1, 2, 3, 4};

long PRE_ij_min[NUM_TASKS][NUM_TASKS];
long PRE_ij_max[NUM_TASKS][NUM_TASKS];
    
long PRE_min[NUM_TASKS];
long PRE_max[NUM_TASKS];

bool BDU_ONLY;
FILE* BDU_fp;

/* Auxilliary Functions */
/* Auxilliary Functions */
void clear_Response()
{
     for(int i = 0; i < NUM_TASKS; i++)
     {
             Response[i] = 0;
     }
}

long INTR_max(int i, int j)
{
    return (long) ceil( Response[j] / T[i]);
}

long INTR_min(int i, int j)
{
    return (long) floor( Response[j] / T[i]);
}

void Reset_PRE_min()
{
     int i, j;
     
     for(i=0; i < NUM_TASKS; i++)
     {
              for(j=0; j < NUM_TASKS; j++)
              {
                       PRE_ij_min[i][j] = 0;
              }
              PRE_min[i] = 0;
     }
        
}


void Reset_PRE_max()
{
     int i, j;
     
     for(i=0; i < NUM_TASKS; i++)
     {
              for(j=0; j < NUM_TASKS; j++)
              {
                       PRE_ij_max[i][j] = 0;
              }
              PRE_max[i] = 0;  
     }
      
}


void SetPRE_min(int i)
{
    int j; 
    
    
    long acc = 0;
    
    for(j=0; j < i; j++)
    {
            acc += PRE_ij_min[j][i];
    }

    PRE_min[i] = acc;    

}


void  SetPRE_max(int i)
{
    int j; 
    
    
    long acc = 0;
    
    for(j=0; j < i; j++)
    {
            acc += PRE_ij_max[j][i];
    }

    PRE_max[i] = acc;    

}


long calculate_pre_max_ij( int i, int j)
{
    int k;
    long acc, ret_val;
    
    acc = 0; 
    
    for ( k = i+1; k < j; k++)
    {
        acc += PRE_ij_min[i][k] * INTR_min( k, j);   
    }
    
    ret_val = INTR_max(i, j) - acc;
    
    if (ret_val < 0)
       ret_val = 0;
    
    return ret_val;
    
}


long calculate_pre_min_ij( int i, int j)
{
    int k;
    long acc, ret_val;
    
    acc = 0; 
    
    for ( k = i+1; k < j; k++)
    {
        acc += PRE_ij_max[i][k] * INTR_max( k, j);   
    }
    
    ret_val = INTR_min(i, j) - acc;
    
    if (ret_val < 0)
       ret_val = 0;
    
    return ret_val;
    
}


/* Cost[k] here means 
   - blocks of Task-k, 
      - that may be useful to other tasks, hence their use by task-k will replace blocks of other task in execution
      - OR that will be useful to tasks of priority lower than k (and higher than task-i under consideration)
*/
     
void ResponseMethod1()
{
     int i, j, k;
     float acc;
     FILE *fp;
     static int first_call = 1;

     if(MESSAGE_LEVEL > NONE){
	if(first_call){			
		fp = fopen("out/method1.txt", "w");
		first_call = 0;
	}
	else
		fp = fopen("out/method1.txt", "a");
	if(fp == NULL){
		printf("***Unable to open file\n");
		MESSAGE_LEVEL = NONE;
	}		
     }
     
     PRE_ij_min[0][0] = 0;
     PRE_ij_max[0][0] = 0;
     Response[0] = C[0];
     
     for (j = 1; j < NUM_TASKS; j++) 
     {
         int flag = 1; int num_iter = 0;
         
         Response[j] = C[j];
         
         do 
         {
            for (i = 0; i < j; i++)
            {
                 int pre_max_ij, pre_min_ij;
                 
                 float max_cost, min_cost;
             
                 pre_max_ij = calculate_pre_max_ij (i, j);
                 pre_min_ij = calculate_pre_min_ij (i, j); 
             
                 PRE_ij_max[i][j] = pre_max_ij;
                 PRE_ij_min[i][j] = pre_min_ij;
                 
                 //printf("The pre_max_ij for i=%d j=%d is %d \n\n", i, j, pre_max_ij);
             }
         
             SetPRE_max(j);
             SetPRE_min(j);
             
             /* Use different algos to calculate costs here */
             /*max_cost = PRE_max[j] * Cost[j];
             min_cost = PRE_min[j] * Cost[j];
             */
             
            acc = C[j];
             
             for ( k = 0; k < j; k++)
             {
                 acc += INTR_max(k, j) * (C[k] + Cost[k]);
             }
             
             if (acc <=  Response[j])
                flag = 0;
             else
             {
                 //printf("acc =%f, response = %f, going again \n\n", acc, Response[j]);
             }
             
             Response[j] = acc;
             
             num_iter++;
             
         } while (flag);
         
         //printf("For task %d, number of iterations are %d, acc=%f, response = %f \n\n", j, num_iter, acc, Response[j]);   
         
     } /* for-j */
     if(fp != NULL)
	fclose(fp);
}

/* By using PRE_j as the number of pre-emptions faced by task-j.
 * Here, cost[j] determines the cost of one pre-emption of task-j
 * that is, nothing but the number of maximum useful blocks at any execution point in task-j
 */
void ResponseMethod2()
{
     int i, j, k;
     float acc, pre_acc;
     FILE *fp;
     static int first_call = 1;

     if(MESSAGE_LEVEL > NONE){
	if(first_call){			
		fp = fopen("out/method2.txt", "w");
		first_call = 0;
	}
	else
		fp = fopen("out/method2.txt", "a");
	if(fp == NULL){
		printf("***Unable to open file\n");
		MESSAGE_LEVEL = NONE;
	}		
     }

     PRE_ij_min[0][0] = 0;
     PRE_ij_max[0][0] = 0;
     Response[0] = C[0];
     
     for (j = 1; j < NUM_TASKS; j++) 
     {
         int flag = 1; int num_iter = 0;
         
         Response[j] = C[j];
         
         do 
         {
            for (i = 0; i < j; i++)
            {
                 int pre_max_ij, pre_min_ij;
                 
                 float max_cost, min_cost;
             
                 pre_max_ij = calculate_pre_max_ij (i, j);
                 pre_min_ij = calculate_pre_min_ij (i, j); 
             
                 PRE_ij_max[i][j] = pre_max_ij;
                 PRE_ij_min[i][j] = pre_min_ij;
                 
                 //printf("The pre_max_ij for i=%d j=%d is %d \n\n", i, j, pre_max_ij);
             }
         
             SetPRE_max(j);
             SetPRE_min(j);
             
             /* Use different algos to calculate costs here */
             /*max_cost = PRE_max[j] * Cost[j];
             min_cost = PRE_min[j] * Cost[j];
             */
             
            acc = C[j];
             
             for ( k = 0; k < j; k++)
             {
                 acc += INTR_max(k, j) * C[k] ;
             }
             
             pre_acc = 0;
             
             pre_acc += PRE_max[j] * Useful_cost[j];
             
             
             acc = acc + pre_acc;
             
             if (acc <=  Response[j])
                flag = 0;
             else
             {
                 //printf("acc =%f, PRE_max[%d]= %d, pre_acc = %f, response = %f, going again \n\n", acc, j, PRE_max[j], pre_acc, Response[j]);
             }
             
             Response[j] = acc;
             
             num_iter++;
             
         } while (flag);
         
         //printf("For task %d, number of iterations are %d, acc=%f, response = %f, pre_max[%d]=%d \n\n", j, num_iter, acc, Response[j], j, PRE_max[j]);
         
     }
     if(fp != NULL)
           fclose(fp);
}



void quickCalculate()
{
     int i, j;

     for (j = 0; j < NUM_TASKS; j++)
     {
            for (i = 0; i < j; i++)
            {
                 int pre_max_ij, pre_min_ij;
                 
                 float max_cost, min_cost;
             
                 pre_max_ij = calculate_pre_max_ij (i, j);
                 pre_min_ij = calculate_pre_min_ij (i, j); 
             
                 PRE_ij_max[i][j] = pre_max_ij;
                 PRE_ij_min[i][j] = pre_min_ij;
                 
                 //printf("The pre_max_ij[%d][%d] =  %d \n\n", i, j, pre_max_ij);
                 //printf("The pre_min_ij[%d][%d] =  %d \n\n", i, j, pre_min_ij);
             }
         
             SetPRE_max(j);
             SetPRE_min(j);
             
             //printf("PRE_max_%d = %d \n\n", j, PRE_max[j]);
             //printf("PRE_min_%d = %d \n\n", j, PRE_min[j]);
         }
}




void Response_time_NO_PREEMPT()
{
     int i, j, k;
     float acc;
     int flag;
     int LAST_TASK = NUM_TASKS-1;
     
     static bool BDU_found = false;
     
     bool sched = true;
     FILE *fp;
     static int first_call = 1;

     if(MESSAGE_LEVEL > NONE){
	if(first_call){			
		fp = fopen("out/no_preempt.txt", "w");
		first_call = 0;
	}
	else
		fp = fopen("out/no_preempt.txt", "a");
	if(fp == NULL){
		printf("***Unable to open file\n");
		MESSAGE_LEVEL = NONE;
	}		
     }
     
     if (BDU_found == true)
     {
        fprintf(fp, "************** BreakDown Utilization already found for NO PREEMPT*********************\n \n");
        ////cout << "************** BreakDown Utilization already found for NO PREEMPT *********************" << endl << endl;
        
        if (BDU_ONLY)
           return;
     }
            
     clear_Response();
     
     fprintf(fp, "************** NO PREEMPT*********************\n \n");
     ////cout << "************** NO PREEMPT *********************" << endl << endl;
     
     Response[0] = C[0];
     Response_NO_PREEMPT[0] = Response[0];
     
     for (i = 1; (i < NUM_TASKS) && (sched==true); i++) 
     {
         flag = 1; int num_iter = 0;
         
         Response_NO_PREEMPT[i] = Response[i] = C[i];
         
         do 
         {
            
            acc = C[i];
            
             for ( j = 0; j < i; j++)
             {
                 acc += INTR_max(j, i) * C[j];               
             }
             
             
             if (acc <=  Response_NO_PREEMPT[i])
                flag = 0;
             else
             {
                 Response_NO_PREEMPT[i] = Response[i] = acc;
                 
                 if (Response[i] > D[i])
                 {
                     flag = 0;
                     sched = false;
                     LAST_TASK = i;
                 }         
             }
             
             num_iter++;
             
         } while (flag);
         
         //printf("For task %d, number of iterations are %d, acc=%f, response = %f \n\n", i, num_iter, acc, Response_NO_PREEMPT[i]);  
         fprintf(fp, "For task %d, number of iterations are %d, acc=%f, response = %f \n\n", i, num_iter, acc, Response_NO_PREEMPT[i]);    
         
     } /* for-i */
     
     flag = 1;
     for(i = 0; i <= LAST_TASK; i++)
     {
           if ( Response[i] > D[i])
           {
                flag = 0;
                sched = false;
                //printf("TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
                fprintf(fp, "TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
           }
           else
           {
               //printf("TASK %d  IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
               fprintf(fp, "TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
           }
     }
     if(sched ==false)
     {
             //printf("\nTASKSET is NOT schedulable under NO PREEMPT at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET is NOT schedulable under NO PREEMPT at TASKSET_UTIL = %f \n", taskSetUtil);       
     }
     else
     {
             //printf("\nTASKSET IS schedulable under NO PREEMPT at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET IS schedulable under NO PREEMPT at TASKSET_UTIL = %f \n", taskSetUtil);   
             
             Num_Executed_Tasks[NO_PREEMPT]++;
             /*if(BDU_found == false)
             {
                 BDU_found = true;
                 fprintf(BDU_fp, "\t\t NO PREEMPT \t\t\t\t %f \t\t %f \n\n", taskSetUtil, multFactor);
             }*/    
     }


     ////cout << "************** NO PREEMPT Ends *********************" << endl << endl;    
     fprintf(fp, "************* NO PREEMPT Ends ********************* \n\n");

     if(fp != NULL)
	fclose(fp);
     
}



void Response_time_ECB_Only()
{
     int i, j, k;
     double acc;
     int flag;
     int LAST_TASK = NUM_TASKS-1;
     
     static bool BDU_found = false;
     
     bool sched = true;
     FILE *fp;
     static int first_call = 1;
     extern std::set<int> TASK_ECB[NUM_TASKS], TASK_UCB[NUM_TASKS];

     if(MESSAGE_LEVEL > NONE){
	if(first_call){			
		fp = fopen("out/ecb_only.txt", "w");
		first_call = 0;
	}
	else
		fp = fopen("out/ecb_only.txt", "a");
	if(fp == NULL){
		printf("***Unable to open file\n");
		MESSAGE_LEVEL = NONE;
	}		
     }


     if (BDU_found == true)
     {
        fprintf(fp, "************** BreakDown Utilization already found for ECB Only*********************\n \n");
        //cout << "************** BreakDown Utilization already found for ECB Only *********************" << endl << endl;
        
        if (BDU_ONLY)
           return;
     }
     
     clear_Response();
     
     fprintf(fp, "************** ECB Only *********************\n \n");
     //cout << "************** ECB Only *********************" << endl << endl;
     
     Response_ECB_Only[0] = Response[0] = C[0];
     
     for (i = 1; (i < NUM_TASKS) && (sched == true); i++) 
     {
         flag = 1; int num_iter = 0;
         
         Response_ECB_Only[i] = Response[i] = C[i];
         
         do 
         {
            
            acc = C[i];
            
            Num_Displaced_Blocks_ECB_Only[i] = 0;
            
            for(int k = 0; k < NUM_TASKS; k++)
            {
                    Gamma_i_j_ECB_Only[k][i] = 0;
            }
             
             for ( j = 0; j < i; j++)
             {
                 acc += INTR_max(j, i) * (C[j] + (BRT * SET_MOD(TASK_ECB[j])) );
                 
                 Num_Displaced_Blocks_ECB_Only[i] +=  INTR_max(j, i) * SET_MOD(TASK_ECB[j]);
                 Gamma_i_j_ECB_Only[j][i] = INTR_max(j, i) * BRT * SET_MOD(TASK_ECB[j]); 
                 
                 /*//cout << "INTR_MAX is " << INTR_max(k, j) << endl;
                 
                 //cout << "C[k] is " << C[k] << endl; 
                 //cout << "BRT is " << BRT << endl;
                 //cout << "SET_MOD is " << SET_MOD(TASK_ECB[k]) << endl;
                 */
                 
                 // //cout << "Adding " << INTR_max(k, j) * (C[k] + (BRT * SET_MOD(TASK_ECB[k])) ) << endl;
                 
             }
             
             
             if (acc <=  Response[i])
                flag = 0;
             else
             {
                 Response_ECB_Only[i] = Response[i] = acc;
                
                if (Response[i] > D[i])
                {
                    flag = 0;
                    sched = false;
                    LAST_TASK = i;
                }
             }
             
             num_iter++;
             
         } while (flag);
         
         //printf("For task %d, number of iterations are %d, acc=%f, response = %f Num_Displaced_Blocks = %ld \n\n", i, num_iter, acc, Response_ECB_Only[i], Num_Displaced_Blocks_ECB_Only[i]);  
         fprintf(fp, "For task %d, number of iterations are %d, acc=%f, response = %f Num_Displaced_Blocks = %ld \n\n", i, num_iter, acc, Response_ECB_Only[i], Num_Displaced_Blocks_ECB_Only[i]);    
         
     } /* for-i */
     
     flag = 1;
     for(i = 0; i <= LAST_TASK; i++)
     {
           if ( Response[i] > D[i])
           {
                flag = 0;
                sched = false;
                //printf("TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
                fprintf(fp, "TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
           }
           else
           {
               //printf("TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
               fprintf(fp, "TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
           }
     }
     if(sched == false)
     {
             //printf("\nTASKSET NOT schedulable under ECB Only at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET is NOT schedulable under ECB Only at TASKSET_UTIL = %f \n", taskSetUtil);       
     }
     else
     {
             //printf("\nTASKSET IS schedulable under ECB Only at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET IS schedulable under ECB Only at TASKSET_UTIL = %f \n", taskSetUtil);
             
             Num_Executed_Tasks[ECB_ONLY]++;
             
             /*if(BDU_found == false)
             {
                 BDU_found = true;
                 fprintf(BDU_fp, "\t\t ECB Only \t\t\t\t %f \t\t %f \n\n", taskSetUtil, multFactor);
             }*/
     }
     

     //cout << "************** ECB Only Ends *********************" << endl << endl;    
     fprintf(fp, "************* ECB Only Ends ********************* \n\n");

     if(fp != NULL)
	fclose(fp);
     
}


void Response_time_UCB_Only()
{
     int i, j, k;
     double acc;
     int flag;
     int LAST_TASK = NUM_TASKS - 1;
     
     static bool BDU_found = false;
     
     bool sched = true;
     FILE *fp;
     static int first_call = 1;
     extern std::set<int> TASK_ECB[NUM_TASKS], TASK_UCB[NUM_TASKS];

     if(MESSAGE_LEVEL > NONE){
	if(first_call){			
		fp = fopen("out/ucb_only.txt", "w");
		first_call = 0;
	}
	else
		fp = fopen("out/ucb_only.txt", "a");
	if(fp == NULL){
		printf("***Unable to open file\n");
		MESSAGE_LEVEL = NONE;
	}		
     }


     
     if (BDU_found == true)
     {
        fprintf(fp, "************** BreakDown Utilization already found for UCB Only*********************\n \n");
        //cout << "************** BreakDown Utilization already found for UCB Only*********************" << endl << endl;
        
        if (BDU_ONLY)
           return;
     }
     
     clear_Response();
     
     fprintf(fp, "************** UCB Only *********************\n \n");
     //cout << "************** UCB Only *********************" << endl << endl;
     
     Response_UCB_Only[0] = Response[0] = C[0];
     
     for (i = 1; (i < NUM_TASKS) && (sched == true); i++) 
     {
         flag = 1; int num_iter = 0;
         
         Response_UCB_Only[i] = Response[i] = C[i];
         
         do 
         {
            
            acc = C[i];
            
            Num_Displaced_Blocks_UCB_Only[i] = 0;
            
            for(int k = 0; k < NUM_TASKS; k++)
            {
                    Gamma_i_j_UCB_Only[k][i] = 0;
            }
             
             for ( j = 0; j < i; j++)
             {
                 int Num_Blocks = 0;
                 
                 for (int k = j+1; k <=  i; k++)
                 {
                     if ( Num_Blocks < SET_MOD(TASK_UCB[k]) )
                        Num_Blocks = SET_MOD(TASK_UCB[k]) ;
                 } 
                 
                 acc += INTR_max(j, i) * (C[j] + (BRT * Num_Blocks) );
                 
                 Num_Displaced_Blocks_UCB_Only[i] +=  INTR_max(j, i) * Num_Blocks ;
                 Gamma_i_j_UCB_Only[j][i] = INTR_max(j, i) * BRT * Num_Blocks ; 
                 
             }
             
             
             if (acc <=  Response[i])
                flag = 0;
             else
             {
                 Response_UCB_Only[i] = Response[i] = acc;
                 if (Response [i] > D[i])
                 {
                     flag = 0;
                     sched = false;
                     LAST_TASK = i;
                 }
                 // printf("acc =%f, response = %f, going again \n\n", acc, Response[j]);
             }
             
             num_iter++;
             
         } while (flag);
         
         //printf("For task %d, number of iterations are %d, acc=%f, response = %f Num_Displaced_Blocks = %ld \n\n", i, num_iter, acc, Response_UCB_Only[i], Num_Displaced_Blocks_UCB_Only[i]); 
         fprintf(fp, "For task %d, number of iterations are %d, acc=%f, response = %f Num_Displaced_Blocks = %ld \n\n", i, num_iter, acc, Response_UCB_Only[i], Num_Displaced_Blocks_UCB_Only[i]);     
         
     } /* for-i */
     
     flag = 1;
     for(i = 0; i <= LAST_TASK; i++)
     {
           if ( Response[i] > D[i])
           {
                flag = 0;
                sched = false;
                //printf("TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
                fprintf(fp, "TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
           }
           else
           {
               //printf("TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
               fprintf(fp, "TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
           }
     }
     if(sched == false)
     {
             //printf("\nTASKSET is NOT schedulable under UCB Only at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET is NOT schedulable under UCB Only at TASKSET_UTIL = %f \n", taskSetUtil);       
     }
     else
     {
             //printf("\nTASKSET IS schedulable under UCB Only at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET IS schedulable under UCB Only at TASKSET_UTIL = %f \n", taskSetUtil);
             
             Num_Executed_Tasks[UCB_ONLY]++;
             
             /*if(BDU_found == false)
             {
                 BDU_found = true;
                 fprintf(BDU_fp, "\t\t UCB Only \t\t\t\t %f \t\t %f \n\n", taskSetUtil, multFactor);
             }*/
     }
     
     ////cout << "************** UCB Only Ends *********************" << endl << endl;
     fprintf(fp, "************* UCB Only Ends ********************* \n\n");
     
     if(fp != NULL)
	fclose(fp);
}



void Response_time_UCB_Union()
{
     int i, j, k;
     double acc;
     int flag;
     int LAST_TASK = NUM_TASKS - 1;
     
     std::set<int> workingSet1, workingSet2;
     
     static bool BDU_found = false;
     
     bool sched = true;
     FILE *fp;
     static int first_call = 1;
     extern std::set<int> TASK_ECB[NUM_TASKS], TASK_UCB[NUM_TASKS];

     if(MESSAGE_LEVEL > NONE){
	if(first_call){			
		fp = fopen("out/ucb_union.txt", "w");
		first_call = 0;
	}
	else
		fp = fopen("out/ucb_union.txt", "a");
	if(fp == NULL){
		printf("***Unable to open file\n");
		MESSAGE_LEVEL = NONE;
	}		
     }
     
     if (BDU_found == true)
     {
        fprintf(fp, "************** BreakDown Utilization already found for UCB Union*********************\n \n");
        //cout << "************** BreakDown Utilization already found for UCB Union *********************" << endl << endl;
        
        if (BDU_ONLY)
           return;
     }
     
     
     clear_Response();
     
     fprintf(fp, "************** UCB Union ********************* \n\n" );
     //cout << "************** UCB Union *********************" << endl << endl;
     
     Response_UCB_Union[0] = Response[0] = C[0];
     
     for (i = 1; (i < NUM_TASKS) && (sched==true); i++) 
     {
         flag = 1; int num_iter = 0;
         
         Response_UCB_Union[i] = Response[i] = C[i];
         
         do 
         {
            
            acc = C[i];
            
            Num_Displaced_Blocks_UCB_Union[i] = 0;
            
            for(int k = 0; k < NUM_TASKS; k++)
            {
                    Gamma_i_j_UCB_Union[k][i] = 0;
            }
             
             
             for ( j = 0; j < i; j++)
             {
                 int Num_Blocks = 0;
                 workingSet1.clear();
                 workingSet2.clear();
                 
                 // calculate Union UCB_aff_k_j
                 for (int k = j+1; k <=  i; k++)
                 {
                     std::set<int>::iterator it;
  
                     for(it=TASK_UCB[k].begin(); it!=TASK_UCB[k].end(); ++it)
                     {
                          workingSet1.insert(*it);
                     } 
                 }    
                 Set_Intersect(workingSet1, TASK_ECB[j], workingSet2);
                 Num_Blocks = SET_MOD(workingSet2);
                 
                 acc += INTR_max(j, i) * (C[j] + (BRT * Num_Blocks) );
                 
                 Num_Displaced_Blocks_UCB_Union[i] +=  INTR_max(j, i) * Num_Blocks ;
                 Gamma_i_j_UCB_Union[j][i] = INTR_max(j, i) * BRT * Num_Blocks ; 
                 
                 /*//cout << "INTR_MAX is " << INTR_max(k, j) << endl;
                 
                 //cout << "C[k] is " << C[k] << endl; 
                 //cout << "BRT is " << BRT << endl;
                 //cout << "SET_MOD is " << SET_MOD(TASK_ECB[k]) << endl;
                 */
                 
                 // //cout << "Adding " << INTR_max(k, j) * (C[k] + (BRT * SET_MOD(TASK_ECB[k])) ) << endl;
                 
             }
             
             
             if (acc <=  Response[i])
                flag = 0;
             else
             {
                 Response_UCB_Union[i] = Response[i] = acc;
                 
                 if (Response[i] > D[i])
                 {
                     flag = 0; 
                     sched = false;
                     LAST_TASK = i;
                 }
                 // printf("acc =%f, response = %f, going again \n\n", acc, Response[j]);
             }
             
             num_iter++;
             
         } while (flag);
         
         //printf("For task %d, number of iterations are %d, acc=%f, response = %f Num_Displaced_Blocks = %ld \n\n", i, num_iter, acc, Response_UCB_Union[i], Num_Displaced_Blocks_UCB_Union[i]); 
         fprintf(fp, "For task %d, number of iterations are %d, acc=%f, response = %f Num_Displaced_Blocks = %ld \n\n", i, num_iter, acc, Response_UCB_Union[i], Num_Displaced_Blocks_UCB_Union[i]);     
         
     } /* for-i */
     
     flag = 1;
     for(i = 0; i <= LAST_TASK; i++)
     {
           if ( Response[i] > D[i])
           {
                flag = 0;
                sched = false;
                //printf("TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
                fprintf(fp, "TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
           }
           else
           {
               //printf("TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
               fprintf(fp, "TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
           }
     }
     if(sched == false)
     {
             //printf("\nTASKSET is NOT schedulable under UCB Union at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET is NOT schedulable under UCB Union at TASKSET_UTIL = %f \n", taskSetUtil);       
     }
     else
     {
             //printf("\nTASKSET IS  schedulable under UCB Union at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET IS  schedulable under UCB Union at TASKSET_UTIL = %f \n", taskSetUtil); 
             
             Num_Executed_Tasks[UCB_UNION]++;
             
             /*if(BDU_found == false)
             {
                 BDU_found = true;
                 fprintf(BDU_fp, "\t\t UCB Union \t\t\t\t %f \t\t %f \n\n", taskSetUtil, multFactor);
             }*/
     }
     
     //cout << "************** UCB Union Ends *********************" << endl << endl;
     fprintf(fp, "************** UCB Union Ends ********************* \n\n");
     
     if(fp != NULL)
	fclose(fp);
}



void Response_time_ECB_Union()
{
     int i, j, k;
     double acc;
     int flag;
     int LAST_TASK = NUM_TASKS - 1;
     
     std::set<int> workingSet1, workingSet2;
     
     static bool BDU_found = false;
     
     bool sched = true;
     FILE *fp;
     static int first_call = 1;
     extern std::set<int> TASK_ECB[NUM_TASKS], TASK_UCB[NUM_TASKS];

     if(MESSAGE_LEVEL > NONE){
	if(first_call){			
		fp = fopen("out/ecb_union.txt", "w");
		first_call = 0;
	}
	else
		fp = fopen("out/ecb_union.txt", "a");
	if(fp == NULL){
		printf("***Unable to open file\n");
		MESSAGE_LEVEL = NONE;
	}		
     }
     
     if (BDU_found == true)
     {
        fprintf(fp, "************** BreakDown Utilization already found for ECB Union*********************\n \n");
        //cout << "************** BreakDown Utilization already found for ECB Union *********************" << endl << endl;
        
        if (BDU_ONLY)
           return;
     }
     
     clear_Response();
     
     fprintf(fp, "************** ECB Union ********************* \n\n"); 
     //cout << "************** ECB Union *********************" << endl << endl;
     
     Response_ECB_Union[0] = Response[0] = C[0];
     
     for (i = 1; (i < NUM_TASKS) && (sched == true); i++) 
     {
         flag = 1; int num_iter = 0;
         
         Response_ECB_Union[i] = Response[i] = C[i];
         
         do 
         {
            
            acc = C[i];
            
            Num_Displaced_Blocks_ECB_Union[i] = 0;
            
            for(int k = 0; k < NUM_TASKS; k++)
            {
                    Gamma_i_j_ECB_Union[k][i] = 0;
            }
             
             
             for ( j = 0; j < i; j++)
             {
                 int Num_Blocks = 0;
                 workingSet1.clear();
                 workingSet2.clear();
                 
                 // calculate Union_ECB
                 for (int h = 0; h <=  j; h++)
                 {
                     std::set<int>::iterator it;
  
                     for(it=TASK_ECB[h].begin(); it!=TASK_ECB[h].end(); ++it)
                     {
                          workingSet1.insert(*it);
                     } 
                 }    
                 
                 for (int k = j+1; k <= i; k++)
                 {
                     workingSet2.clear();
                     Set_Intersect(workingSet1, TASK_UCB[k], workingSet2);
                     if (Num_Blocks < SET_MOD(workingSet2))
                        Num_Blocks = SET_MOD(workingSet2);
                 }
                 
                 acc += INTR_max(j, i) * (C[j] + (BRT * Num_Blocks) );
                 
                 Num_Displaced_Blocks_ECB_Union[i] +=  INTR_max(j, i) * Num_Blocks ;
                 Gamma_i_j_ECB_Union[j][i] = INTR_max(j, i) * BRT * Num_Blocks ; 
             }
             
             if (acc <=  Response[i])
                flag = 0;
             else
             {
                 Response_ECB_Union[i] = Response[i] = acc;
                 
                 if (Response[i] > D[i])
                 {
                     flag = 0;
                     sched = false;
                     LAST_TASK = i;
                 }
                 // printf("acc =%f, response = %f, going again \n\n", acc, Response[j]);
             }
             
             num_iter++;
             
         } while (flag);
         
         //printf("For task %d, number of iterations are %d, acc=%f, response = %f Num_Displaced_Blocks = %ld \n\n", i, num_iter, acc, Response_ECB_Union[i], Num_Displaced_Blocks_ECB_Union[i]); 
         fprintf(fp, "For task %d, number of iterations are %d, acc=%f, response = %f Num_Displaced_Blocks = %ld \n\n", i, num_iter, acc, Response_ECB_Union[i], Num_Displaced_Blocks_ECB_Union[i]);     
         
     } /* for-i */
     
     flag = 1;
     for(i = 0; i <= LAST_TASK; i++)
     {
           if ( Response[i] > D[i])
           {
                flag = 0;
                sched = false;
                //printf("TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
                fprintf(fp, "TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
           }
           else
           {
               //printf("TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
               fprintf(fp, "TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
           }
     }
     if(sched == false)
     {
             //printf("\nTASKSET is NOT schedulable under ECB Union at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET is NOT schedulable under ECB Union at TASKSET_UTIL = %f \n", taskSetUtil);       
     }
     else
     {
             //printf("\nTASKSET IS  schedulable under ECB Union at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET IS  schedulable under ECB Union at TASKSET_UTIL = %f \n", taskSetUtil); 
             
             Num_Executed_Tasks[ECB_UNION]++;
             
             /*if(BDU_found == false)
             {
                 BDU_found = true;
                 fprintf(BDU_fp, "\t\t ECB Union \t\t\t\t %f \t\t %f \n\n", taskSetUtil, multFactor);
             }*/
     }
     
     
     ////cout << "************** ECB Union Ends *********************" << endl << endl;
     fprintf(fp, "************** ECB Union Ends ********************* \n\n");

     if(fp != NULL)
	fclose(fp);
     
}

void Response_time_Staschulat()
{
     int i, j, k;
     double acc;
     int flag;
     int LAST_TASK = NUM_TASKS-1;
     
     std::set<int> workingSet1, workingSet2;
     
     static bool BDU_found = false;
     
     bool sched = true;
     FILE *fp;
     static int first_call = 1;
     extern std::set<int> TASK_ECB[NUM_TASKS], TASK_UCB[NUM_TASKS];

     if(MESSAGE_LEVEL > NONE){
	if(first_call){			
		fp = fopen("out/staschulat.txt", "w");
		first_call = 0;
	}
	else
		fp = fopen("out/staschulat.txt", "a");
	if(fp == NULL){
		printf("***Unable to open file\n");
		MESSAGE_LEVEL = NONE;
	}		
     }
     
     if (BDU_found == true)
     {
        fprintf(fp, "************** BreakDown Utilization already found for Staschulat*********************\n \n");
        //cout << "************** BreakDown Utilization already found for Staschulat *********************" << endl << endl;
        
        if (BDU_ONLY)
           return;
     }
     
     clear_Response();
     
     fprintf(fp, "************** Staschulat ********************* \n\n");
     //cout << "************** Staschulat *********************" << endl << endl;
     
     Response_Staschulat[0] = Response[0] = C[0];
     
     for (i = 1; (i < NUM_TASKS) && (sched == true); i++) 
     {
         flag = 1; int num_iter = 0;
         
         Response_Staschulat[i] = Response[i] = C[i];
         
         do 
         { 
            int q; 
            std::multiset<int> M;
            std::multiset<int>::iterator it;
            
            acc = C[i];
            
            Num_Displaced_Blocks_Staschulat[i] = 0;
            
            for(k = 0; k < NUM_TASKS; k++)
            {
                    Gamma_i_j_Staschulat[k][i] = 0;
            }
             
             
             for ( j = 0; j < i; j++)
             {
                 
                 q=0;
                 for(k=j; k < i; k++)
                 {
                          q += INTR_max(k, i);
                 }
                 
                 M.clear();
                 for(k=j+1; k <= i; k++)
                 {
                     int EkRi = INTR_max(k, i);
                     workingSet1.clear();
                     it = M.begin();
                     for(int s=0; s< EkRi; s++)
                     {
                             int n = INTR_max(j, k);
                             Set_Intersect(TASK_UCB[k], TASK_ECB[j], workingSet1);
                             int val = SET_MOD(workingSet1);
                             for(int t = 0; t < n; t++)
                             {
                                     it = M.insert(val);
                             }
                     }
                 }
                 
                 int Num_Blocks = 0;
                 for(int l=0; l < q; l++)
                 {
                          int val = 0;
                          std::multiset<int>::iterator max_it = M.begin();
                           
                          for(it = M.begin(); it != M.end(); ++it)
                          {
                                 if (val < *it)
                                 {
                                    val = *it;
                                    max_it = it; 
                                 }                                         
                          }
                          M.erase(max_it);
                          
                          Num_Blocks += val;
                 }
                 
                 acc += (INTR_max(j, i) * C[j]) + (BRT * Num_Blocks);
                 
                 Num_Displaced_Blocks_Staschulat[i] +=  Num_Blocks ;
                 Gamma_i_j_Staschulat[j][i] = BRT * Num_Blocks ; 
                 
             }
             
             
             if (acc <=  Response[i])
                flag = 0;
             else
             {
                 Response_Staschulat[i] = Response[i] = acc;
                 
                 if (Response[i] > D[i]) 
                 {
                     flag = 0;
                     sched = false;
                     LAST_TASK = i;
                 // printf("acc =%f, response = %f, going again \n\n", acc, Response[j]);
                 }
             }
             
             num_iter++;
             
         } while (flag);
         
         //printf("For task %d, number of iterations are %d, acc=%f, response = %f Num_Displaced_Blocks = %ld \n\n", i, num_iter, acc, Response_Staschulat[i], Num_Displaced_Blocks_Staschulat[i]);
         fprintf(fp, "For task %d, number of iterations are %d, acc=%f, response = %f Num_Displaced_Blocks = %ld \n\n", i, num_iter, acc, Response_Staschulat[i], Num_Displaced_Blocks_Staschulat[i]);      
         
     } /* for-i */
     
     flag = 1;
     for(i = 0; i <= LAST_TASK; i++)
     {
           if ( Response[i] > D[i])
           {
                flag = 0;
                sched = false;
                //printf("TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
                fprintf(fp, "TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
           }
           else
           {
               //printf("TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
               fprintf(fp, "TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
           }
     }
     if(sched == false)
     {
             //printf("\nTASKSET NOT schedulable under STASCHULAT at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET is NOT schedulable under STASCHULAT at TASKSET_UTIL = %f \n", taskSetUtil);       
     }
     else
     {
             //printf("\nTASKSET IS  schedulable under STASCHULAT at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET IS schedulable under STASCHULAT at TASKSET_UTIL = %f \n", taskSetUtil); 
             
             Num_Executed_Tasks[STASCHULAT]++;
             
             /*if(BDU_found == false)
             {
                 BDU_found = true;
                 fprintf(BDU_fp, "\t\t Staschulat \t\t\t\t %f \t\t %f \n\n", taskSetUtil, multFactor);
             }*/
     }
     
     ////cout << "************** Staschulat Ends *********************" << endl << endl;
     fprintf(fp, "************** Staschulat Ends ********************* \n\n");
     
     if(fp != NULL)
	fclose(fp);
}

void Response_time_ECB_Union_Multiset(bool display=false)
{
     int i, j, k;
     double acc;
     int flag;
     int LAST_TASK = NUM_TASKS - 1;
     
     std::set<int> workingSet1, workingSet2;
     
     static bool BDU_found = false;
     
     bool sched = true;
     FILE *fp;
     static int first_call = 1;
     extern std::set<int> TASK_ECB[NUM_TASKS], TASK_UCB[NUM_TASKS];

     if(MESSAGE_LEVEL > NONE){
	if(first_call){			
		fp = fopen("out/ecb_union_multiset.txt", "w");
		first_call = 0;
	}
	else
		fp = fopen("out/ecb_union_multiset.txt", "a");
	if(fp == NULL){
		printf("***Unable to open file\n");
		MESSAGE_LEVEL = NONE;
	}		
     }

     if (BDU_found == true)
     {
        fprintf(fp, "************** BreakDown Utilization already found for ECB Union Multiset*********************\n \n");
        ////cout << "************** BreakDown Utilization already found for ECB Union Multiset *********************" << endl << endl;
        
        if (BDU_ONLY)
           return;
     }
     
     clear_Response();
     
     if (display)
     {
                 fprintf(fp, "************** ECB Union Multiset ********************* \n\n");
                 ////cout << "************** ECB Union Multiset *********************" << endl << endl;
     }
     
     Response_ECB_Union_Multiset[0] = Response[0] = C[0];
     
     for (i = 1; (i < NUM_TASKS) && (sched == true); i++) 
     {
         flag = 1; int num_iter = 0;
         
         Response_ECB_Union_Multiset[i] = Response[i] = C[i];
         
         do 
         { 
            int EjRi, EjRk, EkRi; 
            std::multiset<int> M;
            std::multiset<int>::iterator it;
            
            acc = C[i];
            
            Num_Displaced_Blocks_ECB_Union_Multiset[i] = 0;
            
            for(k = 0; k < NUM_TASKS; k++)
            {
                    Gamma_i_j_ECB_Union_Multiset[k][i] = 0;
            }
             
             
             for ( j = 0; j < i; j++)
             {
                 
                 EjRi = INTR_max(j, i);
                 
                 M.clear();
                 for(k=j+1; k <= i; k++)  // for-k-affinity
                 {
                     EjRk = INTR_max(j, k);
                     EkRi = INTR_max(k, i);
                     
                     workingSet1.clear();
                     it = M.begin();
                     
                     int numRepeat = EjRk * EkRi ;
                     for(int s=0; s < numRepeat; s++)
                     {
                             int val = 0;
                             workingSet1.clear();
                             workingSet2.clear();
                 
                             // calculate Union_ECB
                             for (int h = 0; h <=  j; h++)
                             {
                                 std::set<int>::iterator it;
  
                                 for(it=TASK_ECB[h].begin(); it!=TASK_ECB[h].end(); ++it)
                                 {
                                      workingSet1.insert(*it);
                                 } 
                             }    
                 
                             workingSet2.clear();
                             Set_Intersect(workingSet1, TASK_UCB[k], workingSet2);
                             val = SET_MOD(workingSet2);
                             M.insert(val);                       
                     } // for EjRk*EkRi
                 } // for-k-affinity
                 
                 int Num_Blocks = 0;
                 for(int l=0; l < EjRi; l++)
                 {
                          int val = 0;
                          std::multiset<int>::iterator max_it = M.begin();
                           
                          for(it = M.begin(); it != M.end(); ++it)
                          {
                                 if (val < *it)
                                 {
                                    val = *it;
                                    max_it = it; 
                                 }                                         
                          }
                          M.erase(max_it);
                          
                          Num_Blocks += val;
                 }
                 
                 acc += (INTR_max(j, i) * C[j]) + (BRT * Num_Blocks);
                 
                 Num_Displaced_Blocks_ECB_Union_Multiset[i] +=  Num_Blocks ;
                 Gamma_i_j_ECB_Union_Multiset[j][i] = BRT * Num_Blocks ; 
                                  
             }
             
             if (acc <=  Response[i])
                flag = 0;
             else
             {
                 Response_ECB_Union_Multiset[i] = Response[i] = acc;
                 
                 if (Response[i] > D[i])
                 {
                    flag = 0;
                    sched = false;
                    LAST_TASK = i;
                 }
                 // printf("acc =%f, response = %f, going again \n\n", acc, Response[j]);
             }
              
             num_iter++;
             
         } while (flag);
         
         if(display)
         {
                    printf("For task %d, number of iterations are %d, acc=%f, response = %f Num_Displaced_Blocks = %ld \n\n", i, num_iter, acc, Response_ECB_Union_Multiset[i], Num_Displaced_Blocks_ECB_Union_Multiset[i]); 
                    fprintf(fp, "For task %d, number of iterations are %d, acc=%f, response = %f Num_Displaced_Blocks = %ld \n\n", i, num_iter, acc, Response_ECB_Union_Multiset[i], Num_Displaced_Blocks_ECB_Union_Multiset[i]);     
         }
         
     } /* for-i */
     
     flag = 1;
     for(i = 0; i <= LAST_TASK; i++)
     {
           if ( Response[i] > D[i] || Response[i] == 0)
           {
                flag = 0;
                sched = false;
                
                if(display)
                {
                    printf("TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
                    fprintf(fp, "TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
                }
           }
           else
           {
               if(display)
               {
                    printf("TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
                    fprintf(fp, "TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
               }
           }
     }
     
     if (display)
     {
        if(sched == false)
        {
             printf("\nTASKSET is NOT schedulable under ECB-Union-MULTISET at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET is NOT schedulable under ECB-Union-MULTISET at TASKSET_UTIL = %f \n", taskSetUtil);       
        }
        else
        {
             printf("\nTASKSET IS schedulable under ECB-Union-MULTISET at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET IS  schedulable under-ECB-Union MULTISET at TASKSET_UTIL = %f \n", taskSetUtil); 
             
             
             
             /*if(BDU_found == false)
             {
                 BDU_found = true;
                 fprintf(BDU_fp, "\t\t ECB Union Multiset \t\t\t\t %f \t\t %f \n\n", taskSetUtil, multFactor);
             }*/
        }
        
        fprintf(fp, "************** ECB Union Multiset Ends ********************* \n\n" ); 
        //cout << "************** ECB Union Multiset Ends *********************" << endl << endl;
     }
     if(sched)
	Num_Executed_Tasks[ECB_UNION_MULTISET]++;
     if(fp != NULL)
	fclose(fp);
     
}

void Response_time_UCB_Union_Multiset(bool display=false)
{
     int i, j, k;
     double acc;
     int Num_Blocks;
     int flag;
     int LAST_TASK = NUM_TASKS - 1;
     
     std::set<int> workingSet1, workingSet2;
     std::set<int>::iterator it;
     
     static bool BDU_found = false;
     
     bool sched = true;
     FILE *fp;
     static int first_call = 1;
     extern std::set<int> TASK_ECB[NUM_TASKS], TASK_UCB[NUM_TASKS];

     if(MESSAGE_LEVEL > NONE){
	if(first_call){			
		fp = fopen("out/ucb_union_multiset.txt", "w");
		first_call = 0;
	}
	else
		fp = fopen("out/ucb_union_multiset.txt", "a");
	if(fp == NULL){
		printf("***Unable to open file\n");
		MESSAGE_LEVEL = NONE;
	}		
     }
     
     if (BDU_found == true)
     {
        fprintf(fp, "************** BreakDown Utilization already found for UCB Union Multiset*********************\n \n");
        ////cout << "************** BreakDown Utilization already found for UCB Union Multiset *********************" << endl << endl;
        
        if (BDU_ONLY)
           return;
     }
     
     clear_Response();
     
     if (display)
     {    
          fprintf(fp, "************** UCB Union Multiset ********************* \n\n");
          //cout << "************** UCB Union Multiset *********************" << endl << endl;
     }
     
     Response_UCB_Union_Multiset[0] = Response[0] = C[0];
     
     for (i = 1; (i < NUM_TASKS) && (sched == true); i++) 
     {
         flag = 1; int num_iter = 0;
         
         Response_UCB_Union_Multiset[i] = Response[i] = C[i];
         
         do 
         { 
            int EjRi, EjRk, EkRi; 
            std::multiset<int> M, M_ucb_ij, M_ecb_j;
            std::multiset<int>::iterator it1, it2;
            
            acc = C[i];
            
            Num_Displaced_Blocks_UCB_Union_Multiset[i] = 0;
            
            for(k = 0; k < NUM_TASKS; k++)
            {
                    Gamma_i_j_UCB_Union_Multiset[k][i] = 0;
            }
             
             
             for ( j = 0; j < i; j++)
             {
                 
                 EjRi = INTR_max(j, i);
                 
                 M_ucb_ij.clear();
                 for(k=j+1; k <= i; k++)  // for-k-affinity
                 {
                     EjRk = INTR_max(j, k);
                     EkRi = INTR_max(k, i);
                     
                     it1 = M_ucb_ij.begin();
                     
                     int numRepeat = EjRk * EkRi ;
                     for(int s=0; s < numRepeat; s++)
                     {
                 
                             for(it=TASK_UCB[k].begin(); it!=TASK_UCB[k].end(); ++it)
                             {
                                   M_ucb_ij.insert(*it);
                             }                        
                     } // for EjRk*EkRi
                 } // for-k-affinity
                 
                 M_ecb_j.clear();
                 for(int s=0; s < EjRi; s++)
                 {
                         for(it=TASK_ECB[j].begin(); it!=TASK_ECB[j].end(); ++it)
                         {
                                   M_ecb_j.insert(*it);
                         } 
                 }
                 
                 M.clear();
                 MultiSet_Intersect(M_ucb_ij, M_ecb_j, M);
                 
                 Num_Blocks = MultiSet_MOD(M);
                 
                 acc += (INTR_max(j, i) * C[j]) + (BRT * Num_Blocks);
                 
                 Num_Displaced_Blocks_UCB_Union_Multiset[i] +=  Num_Blocks ;
                 Gamma_i_j_UCB_Union_Multiset[j][i] = BRT * Num_Blocks ; 
                                
             }
             
             if (acc <=  Response[i])
                flag = 0;
             else
             {
                 Response_UCB_Union_Multiset[i] = Response[i] = acc;
                 
                 if(Response[i] > D[i])
                 {
                     flag = 0;
                     sched = false;
                     LAST_TASK = i;           
                 }
                 // printf("acc =%f, response = %f, going again \n\n", acc, Response[j]);
             }
              
             num_iter++;
             
         } while (flag);
             
         if (display)
         {
            printf("For task %d, number of iterations are %d, acc=%f, response = %f Num_Displaced_Blocks = %ld \n\n", i, num_iter, acc, Response_UCB_Union_Multiset[i], Num_Displaced_Blocks_UCB_Union_Multiset[i]); 
            fprintf(fp, "For task %d, number of iterations are %d, acc=%f, response = %f Num_Displaced_Blocks = %ld \n\n", i, num_iter, acc, Response_UCB_Union_Multiset[i], Num_Displaced_Blocks_UCB_Union_Multiset[i]);     
         }
         
     } /* for-i */
     
     flag = 1;
     for(i = 0; i <= LAST_TASK; i++)
     {
           if ( Response[i] > D[i] || Response[i] == 0)
           {
                flag = 0;
                sched = false;
                
                if(display)
                {
                   printf("TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
                   fprintf(fp, "TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
                }
           }
           else
           {
               if(display)
               {
                   printf("TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
                   fprintf(fp, "TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
               }
           }
     }
     
     if (display)
     {
        if(sched == false)
        {
             printf("\nTASKSET NOT schedulable under UCB-Union-MULTISET at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET NOT schedulable under UCB-Union-MULTISET at TASKSET_UTIL = %f \n", taskSetUtil);       
        }
        else
        {
             printf("\nTASKSET IS schedulable under UCB-Union-MULTISET at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET IS schedulable under-UCB-Union MULTISET at TASKSET_UTIL = %f \n", taskSetUtil); 
             
             
             
             /*if(BDU_found == false)
             {
                 BDU_found = true;
                 fprintf(BDU_fp, "\t\t UCB Union Multiset \t\t\t\t %f \t\t %f \n\n", taskSetUtil, multFactor);
             }*/
        }
        
        fprintf(fp, "************** UCB Union Multiset Ends ********************* \n\n" ); 
        //cout << "************** UCB Union Multiset Ends *********************" << endl << endl;
     }
     if(sched)
	Num_Executed_Tasks[UCB_UNION_MULTISET]++;
     if(fp != NULL)
	fclose(fp);
     
}

void Response_time_ECB_UCB_Union_Multiset_Combined()
{
     int flag;
     int LAST_TASK = NUM_TASKS - 1;
     
     static bool BDU_found = false;
     
     bool sched = true;
     FILE *fp;
     static int first_call = 1;
     extern std::set<int> TASK_ECB[NUM_TASKS], TASK_UCB[NUM_TASKS];

     if(MESSAGE_LEVEL > NONE){
	if(first_call){			
		fp = fopen("out/combined.txt", "w");
		first_call = 0;
	}
	else
		fp = fopen("out/combined.txt", "a");
	if(fp == NULL){
		printf("***Unable to open file\n");
		MESSAGE_LEVEL = NONE;
	}		
     }
     
     if (BDU_found == true)
     {
        fprintf(fp, "************** BreakDown Utilization already found for ECB-UCB-Union-Multiset-Combined*********************\n \n");
        ////cout << "************** BreakDown Utilization already found for ECB Union Multiset-Combined*********************" << endl << endl;
        
        if (BDU_ONLY)
           return;
     }
     
     // Response_time_ECB_Union_Multiset(false);
     // Response_time_UCB_Union_Multiset(false);
     
     //printf("************** ECB UCB Union Multiset Combined ********************* \n\n" );
     fprintf(fp, "************** ECB UCB Union Multiset Combined ********************* \n\n" );
     for (int i = 0; (i < NUM_TASKS) && (sched == true); i++)
     {
         if ( Response_ECB_Union_Multiset[i] < Response_UCB_Union_Multiset[i] && Response_ECB_Union_Multiset[i] != 0)
         {
             Response_ECB_UCB_Union_Multiset_Combined[i] = Response[i] = Response_ECB_Union_Multiset[i];
             Num_Displaced_Blocks_ECB_UCB_Union_Multiset_Combined[i] = Num_Displaced_Blocks_ECB_Union_Multiset[i];
         }
         else
         {
             Response_ECB_UCB_Union_Multiset_Combined[i] = Response[i] = Response_UCB_Union_Multiset[i];
             Num_Displaced_Blocks_ECB_UCB_Union_Multiset_Combined[i] = Num_Displaced_Blocks_UCB_Union_Multiset[i];
         }   
         
          if ( Response[i] > D[i] || Response[i] == 0)
           {
                sched = false;
                LAST_TASK = i;
           } 
         
         //printf("For task %d, response = %f Num_Displaced_Blocks = %d \n\n", i, Response_ECB_UCB_Union_Multiset_Combined[i], Num_Displaced_Blocks_ECB_UCB_Union_Multiset_Combined[i]);
         fprintf(fp, "For task %d, response = %f Num_Displaced_Blocks = %ld \n\n", i, Response_ECB_UCB_Union_Multiset_Combined[i], Num_Displaced_Blocks_ECB_UCB_Union_Multiset_Combined[i]);     
     }
     
     flag = 1;
     for(int i = 0; i <= LAST_TASK; i++)
     {
           if ( Response[i] > D[i] || Response[i] == 0)
           {
                flag = 0;
                sched = false;
                //printf("TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
                fprintf(fp, "TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
           }
           else
           {
               //printf("TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
               fprintf(fp, "TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
           }
     }
     if(sched==false)
     {
             //printf("\nTASKSET is NOT schedulable under ECB-UCB-Union-MULTISET at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET is NOT schedulable under ECB-UCB-Union-MULTISET at TASKSET_UTIL = %f \n", taskSetUtil);       
     }
     else
     {
             //printf("\nTASKSET IS  schedulable under ECB-UCB-Union-MULTISET at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET IS schedulable under-ECB-UCB-Union MULTISET at TASKSET_UTIL = %f \n", taskSetUtil);
             
             Num_Executed_Tasks[ECB_UCB_UNION_MULTISET_COMBINED]++; 
             
             /*if(BDU_found == false)
             {
                 BDU_found = true;
                 fprintf(BDU_fp, "\t\t ECB UCB Union Multiset Combined \t\t\t\t %f \t\t %f \n\n", taskSetUtil, multFactor);
             }*/
     }
     
     //printf("************** ECB UCB Union Multiset Combined Ends ********************* \n\n" );
     fprintf(fp, "************** ECB UCB Union Multiset Combined Ends ********************* \n\n" );

     if(fp != NULL)
	fclose(fp);
     
}


void Response_time_Staschulat_PRE()
{
     int i, j, k;
     double acc;
     int flag;
     int LAST_TASK = NUM_TASKS - 1;
     
     std::set<int> workingSet1, workingSet2;
     
     static bool BDU_found = false;

     bool sched = true;
     FILE *fp;
     static int first_call = 1;
     extern std::set<int> TASK_ECB[NUM_TASKS], TASK_UCB[NUM_TASKS];

     if(MESSAGE_LEVEL > NONE){
	if(first_call){			
		fp = fopen("out/staschulat_pre.txt", "w");
		first_call = 0;
	}
	else
		fp = fopen("out/staschulat_pre.txt", "a");
	if(fp == NULL){
		printf("***Unable to open file\n");
		MESSAGE_LEVEL = NONE;
	}		
     }
          
     if (BDU_found == true)
     {
        fprintf(fp, "************** BreakDown Utilization already found for Staschulat with PRE*********************\n \n");
        ////cout << "************** BreakDown Utilization already found for Staschulat with PRE *********************" << endl << endl;
        
        if (BDU_ONLY)
           return;
     }
     
     clear_Response();
     
     fprintf(fp, "************** Staschulat with PRE ********************* \n\n");
     //printf("************** Staschulat with PRE ********************* \n\n");
     
     
     Response_Staschulat_PRE[0] = Response[0] = C[0];
     
     Reset_PRE_min();
     Reset_PRE_max();
     
     for (i = 1; (i < NUM_TASKS) && (sched == true); i++) 
     {
         flag = 1; int num_iter = 0;
         
         Response_Staschulat_PRE[i] = Response[i] = C[i];
         
         do 
         { 
            int q; 
            std::multiset<int> M;
            std::multiset<int>::iterator it;
            
            acc = C[i];
            
            Num_Displaced_Blocks_Staschulat_PRE[i] = 0;
            
            for(k = 0; k < NUM_TASKS; k++)
            {
                    Gamma_i_j_Staschulat_PRE[k][i] = 0;
            }
            
            // calculate PRE values
            for (j = 0; j < i; j++)
            {
                 int pre_max_ij, pre_min_ij;
                 
                 float max_cost, min_cost;
             
                 pre_max_ij = calculate_pre_max_ij (j, i);
                 pre_min_ij = calculate_pre_min_ij (j, i); 
             
                 PRE_ij_max[j][i] = pre_max_ij;
                 PRE_ij_min[j][i] = pre_min_ij;
                 
             }
             SetPRE_max(i);
             SetPRE_min(i);
             
             // calculate for higher priority tasks
             for ( j = 0; j < i; j++)
             {
                 
                 q=0;
                 for(k=j; k < i; k++)
                 {
                          q += PRE_ij_max[k][i];
                 }
                 
                 M.clear();
                 for(k=j+1; k <= i; k++)
                 {
                     int EkRi = INTR_max(k, i);
                     workingSet1.clear();
                     it = M.begin();
                     for(int s=0; s< EkRi; s++)
                     {
                             int n = PRE_ij_max[j][k];
                             Set_Intersect(TASK_UCB[k], TASK_ECB[j], workingSet1);
                             int val = SET_MOD(workingSet1);
                             for(int t = 0; t < n; t++)
                             {
                                     it = M.insert(val);
                             }
                     }
                 }
                 
                 int Num_Blocks = 0;
                 for(int l=0; l < q; l++)
                 {
                          int val = 0;
                          std::multiset<int>::iterator max_it = M.begin();
                           
                          for(it = M.begin(); it != M.end(); ++it)
                          {
                                 if (val < *it)
                                 {
                                    val = *it;
                                    max_it = it; 
                                 }                                         
                          }
                          M.erase(max_it);
                          
                          Num_Blocks += val;
                 }
                 
                 acc += (INTR_max(j, i) * C[j]) + (BRT * Num_Blocks);
                 
                 Num_Displaced_Blocks_Staschulat_PRE[i] +=  Num_Blocks ;
                 Gamma_i_j_Staschulat_PRE[j][i] = BRT * Num_Blocks ; 
                 
             }
             
             
             if (acc <=  Response[i])
                flag = 0;
             else
             {
                 Response_Staschulat_PRE[i] = Response[i] = acc;
                 
                 if (Response[i] > D[i])
                 {
                    flag = 0;
                    sched = false;
                    LAST_TASK = i;
                 }
                 // printf("acc =%f, response = %f, going again \n\n", acc, Response[j]);
             }
             
             num_iter++;
             
         } while (flag);
         
         //printf("For task %d, number of iterations are %d, acc=%f, response = %f Num_Displaced_Blocks = %ld \n\n", i, num_iter, acc, Response_Staschulat_PRE[i], Num_Displaced_Blocks_Staschulat_PRE[i]);
         fprintf(fp, "For task %d, number of iterations are %d, acc=%f, response = %f Num_Displaced_Blocks = %ld \n\n", i, num_iter, acc, Response_Staschulat_PRE[i], Num_Displaced_Blocks_Staschulat_PRE[i]);      
         
     } // for-i 
     
     
     flag = 1;
     for(i = 0; i <= LAST_TASK; i++)
     {
           if ( Response[i] > D[i])
           {
                flag = 0;
                sched = false;
                //printf("TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
                fprintf(fp, "TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
           }
           else
           {
               //printf("TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
               fprintf(fp, "TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
           }
     }
     if(sched == false)
     {
             //printf("\nTASKSET  NOT schedulable under Staschulat-with-PRE at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET  NOT schedulable under Staschulat-with-PRE at TASKSET_UTIL = %f \n", taskSetUtil);       
     }
     else
     {
             //printf("\nTASKSET IS schedulable under Staschulat-with-PRE at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET IS schedulable under Staschulat-with-PRE at TASKSET_UTIL = %f \n", taskSetUtil); 
             
             Num_Executed_Tasks[STASCHULAT_PRE]++; 
             
             /*if(BDU_found == false)
             {
                 BDU_found = true;
                 fprintf(BDU_fp, "\t\t Staschulat with PRE \t\t\t\t %f \t\t %f \n\n", taskSetUtil, multFactor);
             }*/
     }
     
     
     ////cout << "************** Staschulat with PRE Ends *********************" << endl << endl;
     fprintf(fp,"************** Staschulat with PRE Ends ********************* \n\n");
 
     if(fp != NULL)
	fclose(fp);    
}

void Response_time_ECB_Union_Multiset_PRE(bool display=false)
{
     int i, j, k;
     double acc;
     int flag;
     int LAST_TASK = NUM_TASKS - 1;
     
     std::set<int> workingSet1, workingSet2;
     
     static bool BDU_found = false;
     
     bool sched = true;
     FILE *fp;
     static int first_call = 1;
     extern std::set<int> TASK_ECB[NUM_TASKS], TASK_UCB[NUM_TASKS];

     if(MESSAGE_LEVEL > NONE){
	if(first_call){			
		fp = fopen("out/ecb_union_multiset_pre.txt", "w");
		first_call = 0;
	}
	else
		fp = fopen("out/ecb_union_multiset_pre.txt", "a");
	if(fp == NULL){
		printf("***Unable to open file\n");
		MESSAGE_LEVEL = NONE;
	}		
     }
     
     if (BDU_found == true)
     {
        fprintf(fp, "************** BreakDown Utilization already found for ECB Union Multiset PRE*********************\n \n");
        ////cout << "************** BreakDown Utilization already found for ECB Union Multiset PRE*********************" << endl << endl;
        
        if (BDU_ONLY)
           return;
     }
     
     clear_Response();
     
     if (display)
     {
                 fprintf(fp, "************** ECB Union Multiset PRE********************* \n\n");
                 //cout << "************** ECB Union Multiset PRE*********************" << endl << endl;
     }
     
     Response_ECB_Union_Multiset_PRE[0] = Response[0] = C[0];
     
     Reset_PRE_min();
     Reset_PRE_max();
     
     for (i = 1; (i < NUM_TASKS) && (sched == true); i++) 
     {
         flag = 1; int num_iter = 0;
         
         Response_ECB_Union_Multiset_PRE[i] = Response[i] = C[i];
         
         do 
         { 
            int EjRi, EjRk, EkRi; 
            std::multiset<int> M;
            std::multiset<int>::iterator it;
            
            acc = C[i];
            
            Num_Displaced_Blocks_ECB_Union_Multiset_PRE[i] = 0;
            
            for(k = 0; k < NUM_TASKS; k++)
            {
                    Gamma_i_j_ECB_Union_Multiset_PRE[k][i] = 0;
            }
            
            // calculate PRE values
            for (j = 0; j < i; j++)
            {
                 int pre_max_ij, pre_min_ij;
                 
                 float max_cost, min_cost;
             
                 pre_max_ij = calculate_pre_max_ij (j, i);
                 pre_min_ij = calculate_pre_min_ij (j, i); 
             
                 PRE_ij_max[j][i] = pre_max_ij;
                 PRE_ij_min[j][i] = pre_min_ij;
                 
             }
             SetPRE_max(i);
             SetPRE_min(i);
             
             
             for ( j = 0; j < i; j++)
             {
                 
                 EjRi = INTR_max(j, i);
                 
                 M.clear();
                 for(k=j+1; k <= i; k++)  // for-k-affinity
                 {
                     EjRk = INTR_max(j, k);
                     EkRi = INTR_max(k, i);
                     
                     workingSet1.clear();
                     it = M.begin();
                     
                     int numRepeat = PRE_ij_max[j][k] * EkRi ;
                     for(int s=0; s < numRepeat; s++)
                     {
                             int val = 0;
                             workingSet1.clear();
                             workingSet2.clear();
                 
                             // calculate Union_ECB
                             for (int h = 0; h <=  j; h++)
                             {
                                 std::set<int>::iterator it;
  
                                 for(it=TASK_ECB[h].begin(); it!=TASK_ECB[h].end(); ++it)
                                 {
                                      workingSet1.insert(*it);
                                 } 
                             }    
                 
                             workingSet2.clear();
                             Set_Intersect(workingSet1, TASK_UCB[k], workingSet2);
                             val = SET_MOD(workingSet2);
                             M.insert(val);                       
                     } // for EjRk*EkRi
                 } // for-k-affinity
                 
                 int Num_Blocks = 0;
                 for(int l=0; l < PRE_ij_max[j][i]; l++)
                 {
                          int val = 0;
                          std::multiset<int>::iterator max_it = M.begin();
                           
                          for(it = M.begin(); it != M.end(); ++it)
                          {
                                 if (val < *it)
                                 {
                                    val = *it;
                                    max_it = it; 
                                 }                                         
                          }
                          M.erase(max_it);
                          
                          Num_Blocks += val;
                 }
                 
                 acc += (INTR_max(j, i) * C[j]) + (BRT * Num_Blocks);
                 
                 Num_Displaced_Blocks_ECB_Union_Multiset_PRE[i] +=  Num_Blocks ;
                 Gamma_i_j_ECB_Union_Multiset_PRE[j][i] = BRT * Num_Blocks ; 
                                  
             }
             
             if (acc <=  Response[i])
                flag = 0;
             else
             {
                 Response_ECB_Union_Multiset_PRE[i] = Response[i] = acc;
                 
                 if (Response[i] > D[i])
                 {
                    flag = 0;
                    sched = false;
                    LAST_TASK = i;
                 }
                 // printf("acc =%f, response = %f, going again \n\n", acc, Response[j]);
             }
              
             num_iter++;
             
         } while (flag);
         
         if(display)
         {
                    printf("For task %d, number of iterations are %d, acc=%f, response = %f Num_Displaced_Blocks = %ld \n\n", i, num_iter, acc, Response_ECB_Union_Multiset_PRE[i], Num_Displaced_Blocks_ECB_Union_Multiset_PRE[i]); 
                    fprintf(fp, "For task %d, number of iterations are %d, acc=%f, response = %f Num_Displaced_Blocks = %ld \n\n", i, num_iter, acc, Response_ECB_Union_Multiset_PRE[i], Num_Displaced_Blocks_ECB_Union_Multiset_PRE[i]);     
         }
         
     } /* for-i */
     
     flag = 1;
     for(i = 0; i <= LAST_TASK; i++)
     {
           if ( Response[i] > D[i] || Response[i] == 0)
           {
                flag = 0;
                sched = false;
                if(display)
                {
                     printf("TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
                     fprintf(fp, "TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
                }
           }
           else
           {
               if(display)
               {
                    printf("TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
                    fprintf(fp, "TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
               }
           }
     }
     
     if(display)
     {
        if(sched == false)
        {
             printf("\nTASKSET NOT schedulable under ECB-Union-Multiset-PRE at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET NOT schedulable under ECB-Union-Multiset-PRE at TASKSET_UTIL = %f \n", taskSetUtil);       
        }
        else
        {
             printf("\nTASKSET IS schedulable under ECB-Union-Multiset-PRE at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET IS schedulable under ECB-Union-Multiset-PRE at TASKSET_UTIL = %f \n", taskSetUtil);
             
             
             
             /*if(BDU_found == false)
             {
                 BDU_found = true;
                 fprintf(BDU_fp, "\t\t ECB Union Multiset PRE  \t\t\t\t %f \t\t %f \n\n", taskSetUtil, multFactor);
             }*/
        }
        
        fprintf(fp, "************** ECB Union Multiset PRE Ends ********************* \n\n" ); 
        ////cout << "************** ECB Union Multiset PRE Ends *********************" << endl << endl;
     }
     if(sched)
	Num_Executed_Tasks[ECB_UNION_MULTISET_PRE]++; 
     if(fp != NULL)
	fclose(fp);
}

void Response_time_UCB_Union_Multiset_PRE(bool display=false)
{
     int i, j, k;
     double acc;
     int Num_Blocks;
     int flag;
     int LAST_TASK = NUM_TASKS - 1;
     
     std::set<int> workingSet1, workingSet2;
     std::set<int>::iterator it;
     
     static bool BDU_found = false;
     
     bool sched = true;
     FILE *fp;
     static int first_call = 1;
     extern std::set<int> TASK_ECB[NUM_TASKS], TASK_UCB[NUM_TASKS];

     if(MESSAGE_LEVEL > NONE){
	if(first_call){			
		fp = fopen("out/ucb_union_multiset_pre.txt", "w");
		first_call = 0;
	}
	else
		fp = fopen("out/ucb_union_multiset_pre.txt", "a");
	if(fp == NULL){
		printf("***Unable to open file\n");
		MESSAGE_LEVEL = NONE;
	}		
     }
     
     if (BDU_found == true)
     {
        fprintf(fp, "************** BreakDown Utilization already found for UCB Union Multiset PRE*********************\n \n");
        ////cout << "************** BreakDown Utilization already found for UCB Union Multiset PRE*********************" << endl << endl;
        
        if (BDU_ONLY)
           return;
     }
     
     clear_Response();
     
     if (display)
     {    
          fprintf(fp, "************** UCB Union Multiset PRE ********************* \n\n");
          ////cout << "************** UCB Union Multiset PRE *********************" << endl << endl;
     }
     
     Response_UCB_Union_Multiset_PRE[0] = Response[0] = C[0];
     
     Reset_PRE_min();
     Reset_PRE_max();
     
     for (i = 1; (i < NUM_TASKS) && (sched == true); i++) 
     {
         flag = 1; int num_iter = 0;
         
         Response_UCB_Union_Multiset_PRE[i] = Response[i] = C[i];
         
         do 
         { 
            int EjRi, EjRk, EkRi; 
            std::multiset<int> M, M_ucb_ij, M_ecb_j;
            std::multiset<int>::iterator it1, it2;
            
            acc = C[i];
            
            Num_Displaced_Blocks_UCB_Union_Multiset_PRE[i] = 0;
            
            for(k = 0; k < NUM_TASKS; k++)
            {
                    Gamma_i_j_UCB_Union_Multiset_PRE[k][i] = 0;
            }
            
            // calculate PRE values
            for (j = 0; j < i; j++)
            {
                 int pre_max_ij, pre_min_ij;
                 
                 float max_cost, min_cost;
             
                 pre_max_ij = calculate_pre_max_ij (j, i);
                 pre_min_ij = calculate_pre_min_ij (j, i); 
             
                 PRE_ij_max[j][i] = pre_max_ij;
                 PRE_ij_min[j][i] = pre_min_ij;
                 
             }
             SetPRE_max(i);
             SetPRE_min(i);
             
             
             for ( j = 0; j < i; j++)
             {
                 
                 EjRi = INTR_max(j, i);
                 
                 M_ucb_ij.clear();
                 for(k=j+1; k <= i; k++)  // for-k-affinity
                 {
                     EjRk = INTR_max(j, k);
                     EkRi = INTR_max(k, i);
                     
                     it1 = M_ucb_ij.begin();
                     
                     int numRepeat = PRE_ij_max[j][k] * EkRi ;
                     for(int s=0; s < numRepeat; s++)
                     {
                 
                             for(it=TASK_UCB[k].begin(); it!=TASK_UCB[k].end(); ++it)
                             {
                                   M_ucb_ij.insert(*it);
                             }                        
                     } // for EjRk*EkRi
                 } // for-k-affinity
                 
                 M_ecb_j.clear();
                 for(int s=0; s < PRE_ij_max[j][i]; s++)
                 {
                         for(it=TASK_ECB[j].begin(); it!=TASK_ECB[j].end(); ++it)
                         {
                                   M_ecb_j.insert(*it);
                         } 
                 }
                 
                 M.clear();
                 MultiSet_Intersect(M_ucb_ij, M_ecb_j, M);
                 
                 Num_Blocks = MultiSet_MOD(M);
                 
                 acc += (INTR_max(j, i) * C[j]) + (BRT * Num_Blocks);
                 
                 Num_Displaced_Blocks_UCB_Union_Multiset_PRE[i] +=  Num_Blocks ;
                 Gamma_i_j_UCB_Union_Multiset_PRE[j][i] = BRT * Num_Blocks ; 
                                
             }
             
             if (acc <=  Response[i])
                flag = 0;
             else
             {
                 Response_UCB_Union_Multiset_PRE[i] = Response[i] = acc;
                 
                 if (Response[i] > D[i])
                 {
                    flag = 0;
                    sched = false;
                    LAST_TASK = i;
                 }
                 // printf("acc =%f, response = %f, going again \n\n", acc, Response[j]);
             }
              
             num_iter++;
             
         } while (flag);
         
         if (display)
         {
            printf("For task %d, number of iterations are %d, acc=%f, response = %f Num_Displaced_Blocks = %ld \n\n", i, num_iter, acc, Response_UCB_Union_Multiset_PRE[i], Num_Displaced_Blocks_UCB_Union_Multiset_PRE[i]); 
            fprintf(fp, "For task %d, number of iterations are %d, acc=%f, response = %f Num_Displaced_Blocks = %ld \n\n", i, num_iter, acc, Response_UCB_Union_Multiset_PRE[i], Num_Displaced_Blocks_UCB_Union_Multiset_PRE[i]);     
         }
         
     } /* for-i */
     
     flag = 1;
     for(i = 0; i <= LAST_TASK; i++)
     {
           if ( Response[i] > D[i] || Response[i] == 0)
           {
                flag = 0;
                sched = false;
                
                if(display)
                {
                     printf("TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
                     fprintf(fp, "TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
                }
           }
           else
           {
               if(display)
               {
                     printf("TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
                     fprintf(fp, "TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
               }
           }
     }
     
     if(display)
     {
         if(sched == false)
         {
             printf("\nTASKSET  NOT schedulable under UCB-Union-Multiset-PRE at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET  NOT schedulable under UCB-Union-Multiset-PRE at TASKSET_UTIL = %f \n", taskSetUtil);     
         }
         else
         {
             printf("\nTASKSET IS schedulable under UCB-Union-Multiset-PRE at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET IS schedulable under UCB-Union-Multiset-PRE at TASKSET_UTIL = %f \n", taskSetUtil); 
             
             
             
             /*if(BDU_found == false)
             {
                 BDU_found = true;
                 fprintf(BDU_fp, "\t\t UCB-Union-Multiset-PRE  \t\t\t\t %f \t\t %f \n\n", taskSetUtil, multFactor);
             }*/  
             
         }
         
         fprintf(fp, "************** UCB Union Multiset PRE Ends ********************* \n\n" ); 
         ////cout << "************** UCB Union Multiset PRE Ends *********************" << endl << endl;
     }
     if(sched)
	Num_Executed_Tasks[UCB_UNION_MULTISET_PRE]++;
     if(fp != NULL)
	fclose(fp);
}


void Response_time_ECB_UCB_Union_Multiset_Combined_PRE()
{
     int flag;
     int LAST_TASK = NUM_TASKS - 1;
     
     static bool BDU_found = false;
    
     bool sched = true;
     FILE *fp;
     static int first_call = 1;
     extern std::set<int> TASK_ECB[NUM_TASKS], TASK_UCB[NUM_TASKS];

     if(MESSAGE_LEVEL > NONE){
	if(first_call){			
		fp = fopen("out/combined_pre.txt", "w");
		first_call = 0;
	}
	else
		fp = fopen("out/combined_pre.txt", "a");
	if(fp == NULL){
		printf("***Unable to open file\n");
		MESSAGE_LEVEL = NONE;
	}		
     }
      
     if (BDU_found == true)
     {
        fprintf(fp, "************** BreakDown Utilization already found for ECB UCB Union Multiset Combined PRE*********************\n \n");
        //cout << "************** BreakDown Utilization already found for ECB UCB Union Multiset Combined PRE*********************" << endl << endl;
        
        if (BDU_ONLY)
           return;
     }
     
     // Response_time_ECB_Union_Multiset_PRE(false);
     // Response_time_UCB_Union_Multiset_PRE(false);
     
     //printf("************** ECB UCB Union Multiset Combined PRE ********************* \n\n" );
     fprintf(fp, "************** ECB UCB Union Multiset Combined PRE ********************* \n\n" );
     for (int i = 0; (i < NUM_TASKS) && (sched == true); i++)
     {
         if ( Response_ECB_Union_Multiset_PRE[i] < Response_UCB_Union_Multiset_PRE[i] && Response_ECB_Union_Multiset_PRE[i] != 0)
         {
             Response_ECB_UCB_Union_Multiset_Combined_PRE[i] = Response[i] = Response_ECB_Union_Multiset_PRE[i];
             Num_Displaced_Blocks_ECB_UCB_Union_Multiset_Combined_PRE[i] = Num_Displaced_Blocks_ECB_Union_Multiset_PRE[i];
         }
         else
         {
             Response_ECB_UCB_Union_Multiset_Combined_PRE[i] = Response[i] = Response_UCB_Union_Multiset_PRE[i];
             Num_Displaced_Blocks_ECB_UCB_Union_Multiset_Combined_PRE[i] = Num_Displaced_Blocks_UCB_Union_Multiset_PRE[i];
         } 
         
         if ( Response[i] > D[i] || Response[i] == 0)
         {
                sched = false; 
                LAST_TASK = i;
         }  
         
         //printf("For task %d, response = %f Num_Displaced_Blocks = %d \n\n", i, Response_ECB_UCB_Union_Multiset_Combined_PRE[i], Num_Displaced_Blocks_ECB_UCB_Union_Multiset_Combined_PRE[i]);
         fprintf(fp, "For task %d, response = %f Num_Displaced_Blocks = %ld \n\n", i, Response_ECB_UCB_Union_Multiset_Combined_PRE[i], Num_Displaced_Blocks_ECB_UCB_Union_Multiset_Combined_PRE[i]);     
     }
     
     flag = 1;
     for(int i = 0; i <= LAST_TASK; i++)
     {
           if ( Response[i] > D[i] || Response[i] == 0)
           {
                flag = 0;
                sched = false;
                //printf("TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
                fprintf(fp, "TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
           }
           else
           {
               //printf("TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
               fprintf(fp, "TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
           }
     }
     if(sched == false)
     {
             //printf("\nTASKSET NOT schedulable under ECB-UCB-Union-Multiset-PRE at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET  NOT schedulable under ECB-UCB-Union-Multiset-PRE at TASKSET_UTIL = %f \n", taskSetUtil);       
     }
     else
     {
             //printf("\nTASKSET IS schedulable under ECB-UCB-Union-Multiset-PRE at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET IS schedulable under ECB-UCB-Union-Multiset-PRE at TASKSET_UTIL = %f \n", taskSetUtil); 
             
             Num_Executed_Tasks[ECB_UCB_UNION_MULTISET_COMBINED_PRE]++;
             
             /*if(BDU_found == false)
             {
                 BDU_found = true;
                 fprintf(BDU_fp, "\t\t ECB-UCB-Union-Multiset-Combined PRE  \t\t\t\t %f \t\t %f \n\n", taskSetUtil, multFactor);
             }*/
     }
     
     //printf("************** ECB UCB Union Multiset Combined PRE Ends ********************* \n\n" );
     fprintf(fp, "************** ECB UCB Union Multiset Combined PRE Ends ********************* \n\n" );
     
     if(fp != NULL)
	fclose(fp);
}



void Response_time_PRE_MAX()
{
     int i, j, k;
     double acc;
     int flag;
     int LAST_TASK = NUM_TASKS - 1;
     
     std::set<int> workingSet1, workingSet2;
     
     static bool BDU_found = false;
     
     bool sched = true;
     FILE *fp;
     static int first_call = 1;
     extern std::set<int> TASK_ECB[NUM_TASKS], TASK_UCB[NUM_TASKS];

     if(MESSAGE_LEVEL > NONE){
	if(first_call){			
		fp = fopen("out/pre_max.txt", "w");
		first_call = 0;
	}
	else
		fp = fopen("out/pre_max.txt", "a");
	if(fp == NULL){
		printf("***Unable to open file\n");
		MESSAGE_LEVEL = NONE;
	}		
     }
     
     if (BDU_found == true)
     {
        fprintf(fp, "************** BreakDown Utilization already found for PRE MAX*********************\n \n");
        //cout << "************** BreakDown Utilization already found for PRE MAX*********************" << endl << endl;
        
        if (BDU_ONLY)
           return;
     }
     
     clear_Response();
     
     fprintf(fp, "************** PRE MAX ********************* \n\n");
     //printf("************** PRE MAx *********************\n\n"); 
     
     Response_PRE_MAX[0] = Response[0] = C[0];
     
     Reset_PRE_min();
     Reset_PRE_max();
     
     for (i = 1; (i < NUM_TASKS) && (sched == true); i++) 
     {
         flag = 1; int num_iter = 0;
         
         Response_PRE_MAX[i] = Response[i] = C[i];
         
         do 
         { 
            
            acc = C[i];
            
            Num_Displaced_Blocks_PRE_MAX[i] = 0;
            
            for(k = 0; k < NUM_TASKS; k++)
            {
                    Gamma_i_j_PRE_MAX[k][i] = 0;
            }
            
            // calculate PRE values
            for (j = 0; j < i; j++)
            {
                 int pre_max_ij, pre_min_ij;
                 
                 float max_cost, min_cost;
             
                 pre_max_ij = calculate_pre_max_ij (j, i);
                 pre_min_ij = calculate_pre_min_ij (j, i); 
             
                 PRE_ij_max[j][i] = pre_max_ij;
                 PRE_ij_min[j][i] = pre_min_ij;
                 
             }
             SetPRE_max(i);
             SetPRE_min(i);
             
             int Num_Blocks = 0;
             for ( j = 0; j <= i; j++)
             {
                 int blocks_j = 0; 
                 
                 for(k=0; k < j; k++)
                 {
                          workingSet1.clear();
                          workingSet2.clear();
                          Set_Union(TASK_ECB[k], TASK_ECB[j], workingSet1);
                          Set_Intersect(workingSet1, TASK_UCB[i], workingSet2);
                          blocks_j = PRE_ij_max[k][j] * INTR_max(j, i) * SET_MOD(workingSet2);
                          Num_Blocks += blocks_j;
                 }
                 if (j < i)
                 {
                       acc += (INTR_max(j, i) * C[j]);
                       Gamma_i_j_PRE_MAX[j][i] = BRT * blocks_j ;
                 }
             }   
                 
             acc += (BRT * Num_Blocks);
             Num_Displaced_Blocks_PRE_MAX[i] =  Num_Blocks ;
             
             if (acc <=  Response[i])
                flag = 0;
             else
             {
                 Response_PRE_MAX[i] = Response[i] = acc;
                 
                 if (Response[i] > D[i])
                 {
                     flag = 0;
                     sched = false;
                     LAST_TASK = i;
                 }
                 // printf("acc =%f, response = %f, going again \n\n", acc, Response[j]);
             }
              
             num_iter++;
             
         } while (flag);
         
         
         //printf("For task %d, number of iterations are %d, acc=%f, response = %f Num_Displaced_Blocks = %ld \n\n", i, num_iter, acc, Response_PRE_MAX[i], Num_Displaced_Blocks_PRE_MAX[i]); 
         fprintf(fp, "For task %d, number of iterations are %d, acc=%f, response = %f Num_Displaced_Blocks = %ld \n\n", i, num_iter, acc, Response_PRE_MAX[i], Num_Displaced_Blocks_PRE_MAX[i]);     
         
         
     } /* for-i */
     
     flag = 1;
     for(i = 0; i < NUM_TASKS; i++)
     {
           if ( Response[i] > D[i])
           {
                flag = 0;
                sched = false;
                //printf("TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
                fprintf(fp, "TASK %d NOT schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
           }
           else
           {
               //printf("TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
               fprintf(fp, "TASK %d IS schedulable at TASKSET_UTIL = %f Response = %f Deadline = %ld \n", i, taskSetUtil, Response[i], D[i]);
           }
     }
     if(sched == false)
     {
             //printf("\nTASKSET NOT schedulable under PRE-MAX at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET is NOT schedulable under PRE-MAX at TASKSET_UTIL = %f \n", taskSetUtil);       
     }
     else
     {
             //printf("\nTASKSET IS schedulable under PRE-MAX at TASKSET_UTIL = %f \n", taskSetUtil);
             fprintf(fp, "\nTASKSET IS schedulable under PRE-MAX at TASKSET_UTIL = %f \n", taskSetUtil);
             
             Num_Executed_Tasks[PRE_MAX]++; 
             
             /*if(BDU_found == false)
             {
                 BDU_found = true;
                 fprintf(BDU_fp, "\t\t PRE MAX  \t\t\t\t %f \t\t %f \n\n", taskSetUtil, multFactor);
             }*/
     }
     
     fprintf(fp, "************** PRE MAX Ends ********************* \n\n" ); 
     //cout << "************** PRE MAX Ends *********************" << endl << endl;

     if(fp != NULL)
	fclose(fp);
     
}


void Clear_ALL()
{

     for(int i=0; i < NUM_TASKS; i++)
     {
         
              Num_Displaced_Blocks_ECB_Only[i] = 0;
              Num_Displaced_Blocks_UCB_Only[i] = 0;
              Num_Displaced_Blocks_UCB_Union[i] = 0;
              Num_Displaced_Blocks_ECB_Union[i] = 0;

              Num_Displaced_Blocks_Staschulat[i] = 0;
              Num_Displaced_Blocks_ECB_Union_Multiset[i] = 0;
              Num_Displaced_Blocks_UCB_Union_Multiset[i] = 0;
              Num_Displaced_Blocks_ECB_UCB_Union_Multiset_Combined[i] = 0;

              Num_Displaced_Blocks_Staschulat_PRE[i] = 0;
              Num_Displaced_Blocks_ECB_Union_Multiset_PRE[i] = 0;
              Num_Displaced_Blocks_UCB_Union_Multiset_PRE[i] = 0;
              Num_Displaced_Blocks_ECB_UCB_Union_Multiset_Combined_PRE[i] = 0;

              Num_Displaced_Blocks_PRE_MAX[i] = 0;

              Response[i] = 0;

              Response_NO_PREEMPT[i] = 0;
              Response_ECB_Only[i] = 0; 
              Response_UCB_Only[i] = 0; 
              Response_UCB_Union[i] = 0; 
              Response_ECB_Union[i] = 0;
              Response_Staschulat[i] = 0;
              Response_ECB_Union_Multiset[i] = 0; 
              Response_UCB_Union_Multiset[i] = 0; 
              Response_ECB_UCB_Union_Multiset_Combined[i] = 0; 

              Response_Staschulat_PRE[i] = 0;
              Response_ECB_Union_Multiset_PRE[i] = 0; 
              Response_UCB_Union_Multiset_PRE[i] = 0; 
              Response_ECB_UCB_Union_Multiset_Combined_PRE[i] = 0; 
              Response_PRE_MAX[i] = 0;

              PRE_min[i] = 0;
              PRE_max[i] = 0;
    
             
             
             for(int j = 0; j < NUM_TASKS; j++)
             {
                     Gamma_i_j_ECB_Only[i][j] = 0;
                     Gamma_i_j_UCB_Only[i][j] = 0;
                     Gamma_i_j_UCB_Union[i][j] = 0;
                     Gamma_i_j_ECB_Union[i][j] = 0;

                     Gamma_i_j_Staschulat[i][j] = 0;
                     Gamma_i_j_ECB_Union_Multiset[i][j] = 0;
                     Gamma_i_j_UCB_Union_Multiset[i][j] = 0;

                     Gamma_i_j_Staschulat_PRE[i][j] = 0;
                     Gamma_i_j_ECB_Union_Multiset_PRE[i][j] = 0;
                     Gamma_i_j_UCB_Union_Multiset_PRE[i][j] = 0;

                     Gamma_i_j_PRE_MAX[i][j] = 0;
                     
                     PRE_ij_min[i][j] = 0; 
                     PRE_ij_max[i][j] = 0;
             }
      }
}

