#include<stdio.h>
#include<assert.h>
#include<pthread.h>
#include<math.h>
#include<string.h>
#include<stdlib.h>
#include <time.h>
#include <assert.h>

#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include<errno.h>
#include <wait.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_THREAD 16
#define THREAD_COUNT 8
#define TQ 1000 //1ms = 1000us

typedef struct {
	sem_t mutex[2];
	int finished[2];
} proc_data;

int main(int argc, char *argv[])
{
	sem_t mutex[2];
	assert(argc == 3);
	proc_data *shared_memory;
	pthread_t P1, P2;
	shared_memory = shmat(shmget(ftok("main_sched.c", 0x45), sizeof(proc_data), 0666 | IPC_CREAT), 0, 0); // creating shared memory

	//initializing the shared memory
	shared_memory->finished[0] = 0;
	shared_memory->finished[1] = 0;
	sem_init(&mutex[0], 0, 0);
	sem_init(&mutex[1], 0, 0);
	shared_memory->mutex[0] = mutex[0];
	shared_memory->mutex[1] = mutex[1];

	pid_t p1_pid, p2_pid;

	if(p1_pid = fork())
	{
		if(p2_pid = fork())
		{
			//main parent
			while(1)
			{
				//check if p1 and p2 finished
				if(shared_memory->finished[0] && shared_memory->finished[1])
					break;
				//RR
				for(int i=0; i<2; i++)
				{
					if(!shared_memory->finished[i])
					{
						for(int j = 0; j < THREAD_COUNT; j++)
						{
							sem_post(&mutex[i]);
						}
						usleep(TQ);
					}
				}	
			}
		}
		else
		{
			//this is P2
			printf("wwedddP2\n");
			if ( execlp("./p2_sched.out", argv[1], argv[2], NULL) == -1) {
				printf("Error in 2 \n");	
			}
		}
	}
	else
	{
		//this is P1
		printf("wwedddP1\n");
		// execlp("p1_sched.out", argv[1], argv[2], NULL);
		if ( execlp("./p1_sched.out", argv[1], argv[2], NULL) == -1) {
			printf("Error in 2  \n");	
		}

	}
}
