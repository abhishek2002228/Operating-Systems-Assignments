#ifndef COMMON_STRUCTS
#define COMMON_STRUCTS

typedef struct {
	int p_id;
	float run_time;
	float wait_time;
	float admit_time;
	char *run_command;
	int priority;
	int running;
	int finished;
} process;

#endif
