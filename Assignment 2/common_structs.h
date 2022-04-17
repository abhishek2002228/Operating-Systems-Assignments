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
	sem_t mutex[2];
	int finished[2];
	
} proc_data;

typedef struct{
	double ta_time[2];
	double wt_time[2];
	double run_time[2];
} time_logging;

typedef struct thread_info {
	const char *fileName;
	int chunkSize;
	int skip_point;
	int thread_id;
	int ep;
	int sp;
	sem_t mutex;
	int thread_num;
} thread_info;\

typedef struct  {
	long mtype;
	int val;
} message_queue;

// queue data structure common to both scheduler
void enqueue(process p); // add to the back of the queue
process dequeue(); // add to the front of the queue
int isFull();
int isEmpty();
#endif
