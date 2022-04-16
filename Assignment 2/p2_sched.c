#include<stdio.h>
#include<assert.h>
#include<pthread.h>
#include<math.h>
#include<string.h>
#include<stdlib.h>

#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <time.h>
#include<errno.h>
#include <semaphore.h>

#include "common_structs.h"

#define MAX_THREAD 16
#define THREAD_COUNT 8
int count = 0;
unsigned long long sum = 0;
int read = 0;
int shmid, shmid_flags;
pthread_mutex_t mutex;

//gcc -pthread p1.c -o p1.out -lm

void *thread_worker(void * args ){
	thread_info *t_if =  (thread_info *)args;
	int temp;
	int *p = shmat(shmid,NULL,0);
	if(p == (int*)-1){
      perror("Error attaching to shared memory\n");
      exit(1);
    }
    long long thread_sum = 0;
	int *q = shmat(shmid_flags, NULL, 0);
	if(q == (int *)-1){
		perror("Error in attaching shm flags\n");
		exit(1);
	}
	while(q[t_if->thread_num] != 1){
		;
	}
	for(int i = t_if->sp; i < t_if->ep && i < count;i++){
		// sem_wait(&t_if->mutex);
		//printf("%d ",p[i]);
		//printf("P2 working \n");
		thread_sum += p[i];


		read++;
	}
	//sem_wait(&t_if->mutex);
	pthread_mutex_lock(&mutex);
	sum += thread_sum;
	printf("P2 sum is %lld \n",sum);
	pthread_mutex_unlock(&mutex);

	//printf("\n");
}

int main(int argc, char const *argv[]){
	int shm_time_logging = shmget(ftok("main_sched.c", 51), sizeof(time_logging), 0666);
	if(shm_time_logging == -1){
		perror("Error in shm time logging P2\n");
		exit(1);
	}
	time_logging *time_log = shmat(shm_time_logging, NULL, 0);
	printf("Starting P2\n");
	//assert(argc == 3);
	assert(argc == 3);
	const char *fileName = argv[1];

	proc_data *shared_memory;
	shared_memory = shmat(shmget(ftok("p1.c",51), sizeof(proc_data), 0666), NULL, 0); // creating shared memory
	count = atoi(argv[2]);
	FILE *data_file = fopen("data_p2.txt","wc");
	if( (shmid = shmget(ftok("p2.c",51),count*sizeof(int),0666))== -1){
  		perror("Error in shmget P2\n");
  		exit(1);
	}
	if((shmid_flags = shmget(ftok("p2.c", 52), (THREAD_COUNT)*sizeof(int), 0666)) == -1){
		perror("Error in shmget P2_flags \n");
		exit(1);
	}
	printf("P2 attached flags memory \n");
	printf("P2 File name is %s and no of ints are %d \n",fileName,count);
	int tc = THREAD_COUNT;
	printf("P2 No of threads is %d \n",tc);
	int chunkSize = (count%tc == 0) ?count/tc : count/tc + 1;
	pthread_t thread[tc];
	sum = 0;
	int sp = 0;
	printf("P2 BR4 \n");
	clock_t st_time, en_time;
	st_time = clock();
	for(int j = 0; j < tc; j++){
		thread_info *ti = malloc(sizeof(thread_info));
		ti->chunkSize = chunkSize;
		ti->thread_id = j;
		ti->fileName = fileName;
		ti->sp = sp;
		ti->ep = sp + chunkSize;
		sp += chunkSize;
		ti->mutex = shared_memory->mutex[1];
		ti->thread_num = j;
		// printf("P2 Making threads\n");
		pthread_create(&thread[j],NULL,thread_worker,ti);
	}

	for(int j = 0; j < tc; j++){
		pthread_join(thread[j],NULL);
	}
	printf("P2 Done \n");
	printf("Sum  %lld \n",sum);
	en_time = clock();
	double ta_time = ((double)(en_time-st_time))/CLOCKS_PER_SEC;
	time_log->ta_time[1] = ta_time;
	//fprintf(data_file, "%d Threads take %f s\n",tc,cpu_time_used);
	//sleep(5);
	fclose(data_file);
	// clear out all the shared memory
	if(shmctl(shmid,IPC_RMID,NULL) == -1){
      exit(1);
    }
	shared_memory->finished[1] = 1;
	return 0;
}
