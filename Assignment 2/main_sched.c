#include<stdio.h>
#include<assert.h>
#include<pthread.h>
#include<math.h>
#include<string.h>
#include<stdlib.h>
#include <time.h>

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

enum { NS_PER_SECOND = 1000000000 };

void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td){
    td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
    td->tv_sec  = t2.tv_sec - t1.tv_sec;
    if (td->tv_sec > 0 && td->tv_nsec < 0)
    {
    	printf("Trig sub 1\n");
        td->tv_nsec += NS_PER_SECOND;
        td->tv_sec--;
    }
    else if (td->tv_sec < 0 && td->tv_nsec > 0)
    {
    	printf("Trig sub 2\n");
        td->tv_nsec -= NS_PER_SECOND;
        td->tv_sec++;
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
	
	//int shmid;
	int shm_id_main = shmget(ftok("p1_sched.c",51), sizeof(proc_data), 0666 | IPC_CREAT);
	if(shm_id_main == -1){
		perror("Error in shmget 1");
		exit(1);
	}
	int shm_id_data = shmget(ftok("p2_sched.c",51),(count+1)*sizeof(int),0666|IPC_CREAT);
	if(shm_id_data == -1){
		perror("Error in shmget 2");
		exit(1);
	}
	shared_memory = (proc_data *) shmat(shm_id_main, NULL, 0); // creating shared memory
	if(shared_memory == (proc_data *) -1){
		perror("Error in attaching main \n");
		exit(1);
	}
	int shmid_flags;
	if((shmid_flags = shmget(ftok("p2_sched.c", 52), (THREAD_COUNT)*sizeof(int), 0666|IPC_CREAT)) == -1){
		perror("Error in shmget main_flags \n");
		exit(1);
	}
	int *shm_flags = (int *)shmat(shmid_flags, NULL, 0);
	for(int k = 0; k < THREAD_COUNT; k++){
		shm_flags[k] = 0; //initialize flags to 0
	}
	int shm_id_seek = shmget(ftok("p2_sched.c",44),(count+1)*sizeof(int),0666|IPC_CREAT);
	if(shm_id_seek == -1){
		perror("Error in shmget seek\n");
		exit(1);	
	}

	int shm_time_logging = shmget(ftok("main_sched.c", 51), sizeof(time_logging), 0666|IPC_CREAT);
	if(shm_time_logging == -1){
		perror("Error in shm time logging\n");
		exit(1);
	}

	time_logging *time_log = shmat(shm_time_logging, NULL, 0);
	mp = (int *) shmat(shm_id_seek,NULL,0);
	//shared_memory->mp = (int * ) malloc(sizeof(int) * (count + 1));
	generateSeekMap(fileName,mp);
	printf("Done making seekmap \n");
   	shared_memory->finished[0] = 0;
	shared_memory->finished[1] = 0;
	sem_init(&shared_memory->mutex[0],0,0);
	sem_init(&shared_memory->mutex[1],0,0);

	// clock_t wt_st[2];
	// clock_t wt_en[2];
	struct timespec wt_st[2],wt_en[2],delta_wait;
	struct timespec switch_st[2],switch_en[2],delta;


	pid_t p1_pid, p2_pid;
	pid_t pid_kill;
	printf("Done setting up shared memory \n");
	if(p1_pid = fork()){
		if(p2_pid = fork()){
			//main parent
			printf("In parent \n");
			pid_kill = p1_pid;
			kill(p1_pid,SIGSTOP);
			// wt_st[0] = clock();
			clock_gettime(CLOCK_REALTIME,&wt_st[0]);
			kill(p2_pid,SIGSTOP);
			// wt_st[1] = clock();
			clock_gettime(CLOCK_REALTIME,&wt_st[1]);

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
						clock_gettime(CLOCK_REALTIME,&switch_st[i]);
						kill(pid_kill,SIGCONT);
						clock_gettime(CLOCK_REALTIME,&switch_en[i]);
						sub_timespec(switch_st[i],switch_en[i],&delta);
						time_log->switch_time[i].tv_sec += (int)delta.tv_sec;
						time_log->switch_time[i].tv_nsec += delta.tv_nsec; 
				
						// wt_en[i] = clock();
						clock_gettime(CLOCK_REALTIME,&wt_en[i]);
						sub_timespec(wt_st[i],wt_en[i],&delta_wait);
						time_log->wt_time[i].tv_sec += (int)delta_wait.tv_sec;
						time_log->wt_time[i].tv_nsec += delta_wait.tv_nsec; 
					
						// time_log->wt_time[i] += ((double)(wt_en[i]-wt_st[i]))/CLOCKS_PER_SEC;
						usleep(TQ);

						clock_gettime(CLOCK_REALTIME,&switch_st[i]);
						kill(pid_kill,SIGSTOP);
						clock_gettime(CLOCK_REALTIME,&switch_en[i]);
						sub_timespec(switch_st[i],switch_en[i],&delta);
						time_log->switch_time[i].tv_sec += (int)delta.tv_sec;
						time_log->switch_time[i].tv_nsec += delta.tv_nsec; 

						if(pid_kill == p1_pid){
							// wt_st[0] = clock();
							clock_gettime(CLOCK_REALTIME,&wt_st[0]);

							pid_kill = p2_pid;
						}else{
							// wt_st[1] = clock();
							clock_gettime(CLOCK_REALTIME,&wt_st[1]);

							pid_kill = p1_pid;
						}
					}
				}	
			}
			printf("P1 and P2 done\n");
			if(shmctl(shm_id_main, IPC_RMID, NULL) == -1){
				exit(1);
			}
			if(shmctl(shm_id_seek, IPC_RMID, NULL) == -1){
				exit(1);
			}
			if(shmctl(shmid_flags, IPC_RMID, NULL) == -1){
				exit(1);
			}

			printf("Turnaround time for P1: %d.%.9lds\n", (int)time_log->ta_time[0].tv_sec,time_log->ta_time[0].tv_nsec);
			printf("Turnaround time for P2: %d.%.9lds\n", (int)time_log->ta_time[1].tv_sec,time_log->ta_time[1].tv_nsec);
			printf("Waiting time for P1: %d.%.9lds\n", (int)time_log->wt_time[0].tv_sec,time_log->wt_time[0].tv_nsec);
			printf("Waiting time for P2: %d.%.9lds\n", (int)time_log->wt_time[1].tv_sec,time_log->wt_time[1].tv_nsec);
			printf("Average Turnaround Time: %d.%.9lds\n",(int)(time_log->ta_time[0].tv_sec+time_log->ta_time[1].tv_sec)/2,(time_log->ta_time[0].tv_nsec + time_log->ta_time[1].tv_nsec)/2 );
			printf("Average Waiting Time: %d.%.9lds\n",(int)(time_log->wt_time[0].tv_sec+time_log->wt_time[1].tv_sec)/2,(time_log->wt_time[0].tv_nsec + time_log->wt_time[1].tv_nsec)/2 );
			printf("Switching overhead for P1: %d.%.9lds\n",(int)time_log->switch_time[0].tv_sec,time_log->switch_time[0].tv_nsec);
			printf("Switching overhead for P2: %d.%.9lds\n",(int)time_log->switch_time[0].tv_sec,time_log->switch_time[1].tv_nsec);
			
			if(shmctl(shm_time_logging, IPC_RMID, NULL) == -1){
				exit(1);
			}

			// wait(&s1);
			// wait(&s2);
			// printf("%d %d \n",s1,s2);
		}else{
			//this is P2

			if (  execlp("./p2_sched.out", "./p2_sched.out",argv[1], argv[2], NULL) == -1) {
				printf("Error in 2 \n");	
			}
		}
	}
	else {
		//this is P1
		if (  execlp("./p1_sched.out", "./p1_sched.out",argv[1], argv[2], NULL) == -1) {
			printf("Error in 2  \n");	
		}
	}
}
