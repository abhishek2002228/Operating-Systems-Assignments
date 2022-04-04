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

// queue data structure common to both scheduler
void enqueue(process p); // add to the back of the queue
process dequeue(); // add to the front of the queue
int isFull();
int isEmpty();
#endif
