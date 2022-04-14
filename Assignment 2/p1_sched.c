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
#include <errno.h>
#include <semaphore.h>

#define MAX_THREAD 16
#define THREAD_COUNT 8
int count = 0;
int read = 0;
unsigned long long sum =0;
int def_print = 0;
int shmid;

//gcc -pthread p1.c -o p1.out -lm

typedef struct {
	sem_t mutex[2];
	int finished[2];
} proc_data;

typedef struct thread_info {
	char *fileName;
	int chunkSize;
	int skip_point;
	int thread_id;
	int ep;
	int sp;
	sem_t mutex;
} thread_info;

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
		if(def_print){
			printf("Starting point at i %d and sp is %ld\n",i+1,t);
		}
		mp[i+1] = t;
		i++;
	}
	fclose(file);
}

void *thread_worker(void * args ){
	thread_info *t_if =  (thread_info *)args;
	FILE* file = fopen(t_if->fileName,"r");
	int *p;
	if(def_print){
		printf("Reading on %d \n",t_if->thread_id);
	}
	if(file == NULL){
		printf("Error opening file \n");
		exit(1);
	}

	fseek (file ,t_if->skip_point, SEEK_SET);
	int i;
	int temp;
	p = shmat(shmid,0,0);
	
	if(p == (int*)-1){
      perror("Error attaching to shared memory\n");
      exit(1);
    }
    if(t_if->sp > count) {
    	return;
    } 
    //printf("start point %d and end point is %d \n",t_if->sp,t_if->ep);
	for(i = t_if->sp; i < t_if->ep  && i < count;i++){	
        sem_wait(&t_if->mutex);
		fscanf(file,"%d",&temp);
		if(def_print){
			printf("%d ",temp);
		}
		p[i] = temp;
		//sum += temp;
		if(temp < 0){
			printf("Error occ\n");
		}

		read++;
	}
	//printf("end point %d \n",i);
	fclose(file);
	if(def_print){
		printf("\n");
	}
}

int main(int argc, char const *argv[]){
	printf("Starting P1\n");
	// assert(argc == 3);
	for(int i  =  0 ; i < argc; i++){
		printf("%s \n",argv[i]);
	}
	// assert(argc == 3);
	char *fileName = argv[1];
	proc_data *shared_memory;
	shared_memory = shmat(shmget(ftok("main_sched.c", 0x45), sizeof(proc_data), 0666 | IPC_CREAT), 0, 0); // creating shared memory
	count = atoi(argv[2]);
	FILE *data_file = fopen("data_p1.txt","wc");
	clock_t start, end;
	if( (shmid = shmget(ftok("p1.c",51),count*sizeof(int),0666 | IPC_CREAT))== -1){
      perror("Error in shmget\n");
      exit(1);
    }

    int *mp = (int * ) malloc(sizeof(int) * (count + 1));
    generateSeekMap(fileName,mp);

	printf("File name is %s and no of ints are %d \n",fileName,count);
    int tc = THREAD_COUNT;

    //int chunkSize = count/tc;
    int chunkSize = (count%tc == 0) ? count/tc : count/tc +1;
    //printf("No of threads is %d  and chunk size is %d \n",tc,chunkSize);

    pthread_t thread[tc];
    read = 0;
    start = clock();
    int sp = 0;
    for(int j = 0; j < tc; j++){
        thread_info *ti = malloc(sizeof(thread_info));
        ti->chunkSize = chunkSize*(j+1);
        ti->thread_id = j;
        ti->fileName = fileName;
        ti->sp = sp;
        ti->skip_point = mp[sp];
        ti->ep = sp + chunkSize;
        sp += chunkSize;
		ti->mutex = shared_memory->mutex[0];
        printf("start point is %d and ends at %d \n",ti->sp,ti->ep);
        pthread_create(&thread[j],NULL,thread_worker,ti);
    }

    for(int j = 0; j < tc; j++){
        pthread_join(thread[j],NULL);
    }
    end = clock();
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    // fprintf(data_file, "%d Threads take %f s\n",tc,cpu_time_used);
    fprintf(data_file, "%f\n",cpu_time_used);
    //sleep(5);
	fclose(data_file);
	shared_memory->finished[0] = 1;
	return 0;
}