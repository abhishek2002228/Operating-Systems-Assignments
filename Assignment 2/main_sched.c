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

#include "common_structs.h"

#define MAX_THREAD 16
#define THREAD_COUNT 8
#define TQ 1000 //1ms = 1000us



void generateSeekMap(char *fileName,int *mp){
	int i = 0;
	int t = 0;
	int temp;
	mp[i] = 0;
	FILE* file = fopen(fileName,"r");
	if(file == NULL){
		printf("Error opening file \n");
		exit(1);
	}
	while(!feof(file)){
		fscanf(file,"%d",&temp);
		t = ftell(file);
		mp[i+1] = t;
		i++;
	}
	fclose(file);
}

void printSeek(int *mp,int count){
	for(int i = 0; i <= count; i++){
		printf("%d => %d \n", i, mp[i]);
	}
}

int main(int argc, char *argv[])
{
	sem_t mutex[2];
	assert(argc == 3);
	proc_data *shared_memory,*sh;
	//sh = (proc_data *) malloc(sizeof(proc_data));
	char *fileName = argv[1];
	int count = atoi(argv[2]);
	int s1,s2; // status flags
	int *mp;
	key_t key_main, key_data;
	key_main = ftok("main_sched.c", 0x45);
	printf("%d \n",key_main);
	key_data = ftok("p1_sched.c",51);
	printf("%d \n",key_data);
	
	int shmid;
	int shm_id_main = shmget(ftok("p1.c",51), sizeof(proc_data), 0666 | IPC_CREAT);
	if(shm_id_main == -1){
		perror("Error in shmget 1");
		exit(1);
	}
	int shm_id_data = shmget(ftok("p2.c",51),(count+1)*sizeof(int),0666|IPC_CREAT);
	if(shm_id_data == -1){
		perror("Error in shmget 2");
		exit(1);
	}
	shared_memory = (proc_data *) shmat(shm_id_main, NULL, 0); // creating shared memory
	if(shared_memory == (proc_data *) -1){
		perror("Error in attaching main \n");
		exit(1);
	}
	int shm_id_seek = shmget(ftok("p2.c",44),(count+1)*sizeof(int),0666|IPC_CREAT);
	if(shm_id_seek == -1){
		perror("Error in shmget seek");
		exit(1);	
	}
	mp = (int *) shmat(shm_id_seek,NULL,0);
	//shared_memory->mp = (int * ) malloc(sizeof(int) * (count + 1));
	generateSeekMap(fileName,mp);
	printf("Done making seekmap \n");
   	shared_memory->finished[0] = 0;
	shared_memory->finished[1] = 0;
	sem_init(&shared_memory->mutex[0],0,0);
	sem_init(&shared_memory->mutex[1],0,0);


	pid_t p1_pid, p2_pid;
	pid_t pid_kill;
	printf("Done setting up shared memory \n");
	if(p1_pid = fork()){
		if(p2_pid = fork()){
			//main parent
			printf("In parent \n");
			pid_kill = p1_pid;
			kill(p1_pid,SIGSTOP);
			kill(p2_pid,SIGSTOP);
			printf("%d %d \n", p1_pid,p2_pid);
			printf("Going to kill %d \n",pid_kill);			
			while(1){
				//check if p1 and p2 finished
				if(shared_memory->finished[0] && shared_memory->finished[1])
					break;
				//RR
				for(int i=0; i<2; i++){
					if(!shared_memory->finished[i]){
						printf("Scheduling %d currently \n",pid_kill);
						kill(pid_kill,SIGCONT);
						// printf("Posting for %d \n",i);
						// for(int j = 0; j < THREAD_COUNT; j++){
						// 	sem_post(&shared_memory->mutex[i]);
						// }
						usleep(TQ);
						kill(pid_kill,SIGSTOP);
						if(pid_kill == p1_pid){
							pid_kill = p2_pid;
						}else{
							pid_kill = p1_pid;
						}
						// for(int j = 0; j < THREAD_COUNT; j++){
							// sem_wait(&shared_memory->mutex[i]);
						// }
						//printf("Sleeping \n");
					}
				}	
			}
			
			// wait(&s1);
			// wait(&s2);
			// printf("%d %d \n",s1,s2);
		}else{
			//this is P2

			if (  execlp("./p2_sched", "./p2_sched",argv[1], argv[2], NULL) == -1) {
				printf("Error in 2 \n");	
			}
		}
	}
	else {
		//this is P1
		if (  execlp("./p1_sched", "./p1_sched",argv[1], argv[2], NULL) == -1) {
			printf("Error in 2  \n");	
		}
	}
}
