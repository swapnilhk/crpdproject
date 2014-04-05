#ifndef HEADER_TDA
#define HEADER_TDA

double sigmaTda(int thisTask, double Response[]){
	double RNew = 0;
	int hpTask = thisTask - 1;
	while(hpTask >= 0){
		RNew += ceil(Response[thisTask]/T[hpTask]) * C[hpTask];
		hpTask = hpTask - 1;
	}
	return RNew;
}

double wcrt(int thisTask, double Response[], FILE *fp, double (*PC)(int, double[], FILE*)){
	double RNew;
	RNew = C[thisTask];
	Response[thisTask] = 0;
	while(RNew != Response[thisTask] && (Response[thisTask] = RNew) <= D[thisTask]){
		RNew = C[thisTask] 
			+ sigmaTda(thisTask, Response) 
			+ PC(thisTask, Response, fp);// Time demand equation
		if(MESSAGE_LEVEL >= IMP)
			fprintf(fp, "T%d(D=%ld) Response time: Old = %g, New = %g\n\n", thisTask, D[thisTask], Response[thisTask], RNew);
	}
	return RNew;
}

#endif
