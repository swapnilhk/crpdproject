#ifndef UNIFORM_DISTRIBUTION_H
#define UNIFORM_DISTRIBUTION_H
#include<stdio.h>
int initBenchmark(int numTasks, FILE *fp);
void createTaskSetUniformDistribution(float totalUtil, int minPeriod, int maxPeriod, FILE *fp);
int createTaskSetConstantValues(int,double);
void freeBenchmark(void);
#endif
