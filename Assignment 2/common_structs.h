#ifndef COMMON_STRUCTS
#define COMMON_STRUCTS

typedef struct {
	int p_id;
	double run_time;
	double wait_time;
	double admit_time;
	char *run_command;
	int priority;
	int running;
	int finished;
} process;

typedef struct {
	int finished[2];
	
} proc_data;

typedef struct{
	double ta_time[2];
	double wt_time[2];
	double run_time[2];
	struct timespec switch_time[2];
} time_logging;

typedef struct thread_info {
	const char *fileName;
	int chunkSize;
	int skip_point;
	int thread_id;
	int ep;
	int sp;
	int thread_num;
} thread_info;

#endif
