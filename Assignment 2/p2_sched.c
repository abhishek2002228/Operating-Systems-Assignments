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
#define MAX_THREAD 16
#define THREAD_COUNT 8
int count = 0;
unsigned long long sum = 0;
int read = 0;
int shmid;
pthread_mutex_t mutex;

//gcc -pthread p1.c -o p1.out -lm

typedef struct {
	sem_t mutex[2];
	int sched[2];
	int finished[2];
} proc_data;

typedef struct thread_info {
	char *fileName;
	int chunkSize;
	int thread_id;
	int ep;
	int sp;
	sem_t mutex;
} thread_info;



void *thread_worker(void * args ){
	thread_info *t_if =  (thread_info *)args;
	int temp;
	int *p = shmat(shmid,0,0);
	if(p == (int*)-1){
      perror("Error attaching to shared memory\n");
      exit(1);
    }
    long long thread_sum = 0;
	for(int i = t_if->sp; i < t_if->ep && i < count;i++){
		sem_wait(&t_if->mutex);
		//printf("%d ",p[i]);
		thread_sum += p[i];

		read++;
	}
	//sem_wait(&t_if->mutex);
	pthread_mutex_lock(&mutex);
	sum += thread_sum;
	pthread_mutex_unlock(&mutex);

	//printf("\n");
}

int main(int argc, char const *argv[]){
	printf("Starting P2\n");
	//assert(argc == 3);
	for(int i  =  0 ; i < argc; i++){
		printf("%s \n",argv[i]);
	}
	// assert(argc == 3);
	char *fileName = argv[1];

	proc_data *shared_memory;
	shared_memory = shmat(shmget(ftok("main_sched.c", 0x45), sizeof(proc_data), 0666 | IPC_CREAT), 0, 0); // creating shared memory

	count = atoi(argv[2]);
	FILE *data_file = fopen("data_p2.txt","wc");
	clock_t start, end;
	if( (shmid = shmget(ftok("p1.c",51),count*sizeof(int),0666))== -1){
      perror("Error in shmget\n");
      exit(1);
    }
    //sleep(3);
	printf("File name is %s and no of ints are %d \n",fileName,count);
	int tc = THREAD_COUNT;
	printf("No of threads is %d \n",tc);
	int chunkSize = (count%tc == 0) ?count/tc : count/tc + 1;
	pthread_t thread[tc];
	sum = 0;
	start = clock();
	int sp = 0;
	for(int j = 0; j < tc; j++){
		thread_info *ti = malloc(sizeof(thread_info));
		ti->chunkSize = chunkSize;
		ti->thread_id = j;
		ti->fileName = fileName;
		ti->sp = sp;
		ti->ep = sp + chunkSize;
		sp += chunkSize;
		ti->mutex = shared_memory->mutex[1];
		pthread_create(&thread[j],NULL,thread_worker,ti);
	}

	for(int j = 0; j < tc; j++){
		pthread_join(thread[j],NULL);
	}
	printf("Sum  %lld \n",sum);
	end = clock();
	double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	//fprintf(data_file, "%d Threads take %f s\n",tc,cpu_time_used);
	fprintf(data_file, "%f\n",cpu_time_used);
	//sleep(5);
	fclose(data_file);
	// clear out all the shared memory
	if(shmctl(shmid,IPC_RMID,NULL) == -1){
      exit(1);
    }
	shared_memory->finished[1] = 1;
	return 0;
}