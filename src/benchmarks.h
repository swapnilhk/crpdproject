#ifndef UNIFORM_DISTRIBUTION_H
#define UNIFORM_DISTRIBUTION_H
#include<stdio.h>
void initUniformDistributionBenchmark(FILE* fp);
void initConstantValuesBenchmark(FILE* fp);
void createTaskSetUniformDistribution(float totalUtil, int minPeriod, int maxPeriod);
void createTaskSetConstantValues(void);
#endif
