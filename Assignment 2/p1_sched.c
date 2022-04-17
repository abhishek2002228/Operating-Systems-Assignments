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

#include "common_structs.h"

#define MAX_THREAD 16
#define THREAD_COUNT 8
int count = 0;
int read = 0;
unsigned long long sum =0;
int def_print = 0;
int shmid;
int shmid_flags;
//gcc -pthread p1.c -o p1.out -lm

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
			printf("Starting point at i %d and sp is %d\n",i+1,t);
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
		printf("No read so far %d \n",read);
	}
	if(file == NULL){
		printf("Error opening file \n");
		exit(1);
	}

	fseek (file ,t_if->skip_point, SEEK_SET);
	int i;
	int temp;
	p = shmat(shmid,NULL,0);
	
	if(p == (int*)-1){
      perror("Error attaching to shared memory\n");
      exit(1);
    }
	int *q;
	q = shmat(shmid_flags, NULL, 0);
	if(q == (int *)-1){
		perror("Error in attaching shm flags\n");
		exit(1);
	}
    if(t_if->sp > count) {
    	return NULL;
    } 
    //printf("start point %d and end point is %d \n",t_if->sp,t_if->ep);
	for(i = t_if->sp; i < t_if->ep  && i < count;i++){	
        // sem_wait(&t_if->mutex);
		fscanf(file,"%d",&temp);
		if(def_print){
			printf("%d \n",temp);
		}
		p[i] = temp;
		// printf("P1 working \n");
		//sum += temp;
		if(temp < 0){
			printf("Error occ\n");
		}

		read++;
	}
	// printf("end point %d \n",i);
	fclose(file);
	q[t_if->thread_num] = 1;
	if(def_print){
		printf("\n");
	}
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
    	printf("Trig sub P1\n");
        td->tv_nsec += NS_PER_SECOND;
        td->tv_sec--;
    }
    else if (td->tv_sec < 0 && td->tv_nsec > 0)
    {
    	printf("Trig sub P1_2\n");
        td->tv_nsec -= NS_PER_SECOND;
        td->tv_sec++;
    }
}

int main(int argc, char const *argv[]){
	int shm_time_logging = shmget(ftok("main_sched.c", 51), sizeof(time_logging), 0666);
	if(shm_time_logging == -1){
		perror("Error in shm time logging P1\n");
		exit(1);
	}
	time_logging *time_log = shmat(shm_time_logging, NULL, 0);
	
	printf("Starting p1\n");
	assert(argc == 3);
	//for(int i = 0; i < argc;i++){
	//	printf("%s \n", argv[i]);
	//}
	const char *fileName = argv[1];
	proc_data *shared_memory,*sh;
	int *mp;
	//int shm_id_2 = shmget(ftok("main_sched.c", 0x45), sizeof(proc_data), 0666);
	//sh = shmat(shm_id_2, 0, 0); // creating shared memory
	
	key_t key_main;
	key_main = ftok("p1_sched.c",51);
	int shm_id_main = shmget(key_main, sizeof(proc_data), 0666);
	if(shm_id_main == -1){
		perror("Error in attach p1 \n");
		exit(1);
	}
	shared_memory = (proc_data *) shmat(shm_id_main, NULL, 0);
	printf("Main shared memory attached \n");
	//shared_memory = (proc_data *) malloc(sizeof(proc_data));
	//shared_memory = (proc_data *) shmat(shmget(ftok("main_sched.c", 0x45), sizeof(proc_data), 0666), NULL, 0); // creating shared memory

	count = atoi(argv[2]);
	
	if(shared_memory == (proc_data*)-1){
      perror("Error attaching P1 \n");
      exit(errno);
    }
	FILE *data_file = fopen("data_p1.txt","wc");
	if(data_file == NULL){
		printf("Error opening file \n");
	}
	if( (shmid = shmget(ftok("p2_sched.c",51),(count+1)*sizeof(int),0666))== -1){
      perror("Error in shmget P1 \n");
      exit(1);
    }
	printf("P1 attached transfer data memory \n");

	if((shmid_flags = shmget(ftok("p2_sched.c", 52), (THREAD_COUNT)*sizeof(int), 0666)) == -1){
		perror("Error in shmget P1_flags \n");
		exit(1);
	}
	printf("P1 attached flags memory \n");
	int shm_id_seek = shmget(ftok("p2_sched.c",44),(count+1)*sizeof(int),0666|IPC_CREAT);
	if(shm_id_seek == -1){
		perror("Error in shmget seek");
		exit(1);	
	}
	mp = (int *) shmat(shm_id_seek,NULL,0);
	//printf("Ready to make map \n");
    //int *mp = (int * ) malloc(sizeof(int) * (count + 1));
    //generateSeekMap(fileName,mp);
	printf("P1 BR5 \n");
	printf("P1 File name is %s and no of ints are %d \n",fileName,count);
    int tc = THREAD_COUNT;

    //int chunkSize = count/tc;
    int chunkSize = (count%tc == 0) ? count/tc : count/tc +1;
    //printf("No of threads is %d  and chunk size is %d \n",tc,chunkSize);

    pthread_t thread[tc];
    printf("P1 No of threads %d \n",tc);
    read = 0;
    int sp = 0;
    printf("P1 Ready to make threads \n");
	// clock_t st_time, en_time;
	struct timespec st_time,en_time,delta;
	// st_time = clock();
	clock_gettime(CLOCK_REALTIME,&st_time);
    for(int j = 0; j < tc; j++){
        thread_info *ti = malloc(sizeof(thread_info));
        // printf("P1 okay so far \n");
        ti->chunkSize = chunkSize*(j+1);
        // printf("P1 okay so far 170\n");
        ti->thread_id = j;
        // printf("P1 okay so far 172\n");
        ti->fileName = fileName;
        // printf("P1 okay so far 174\n");
        ti->sp = sp;
        // printf("P1 okay so far 176\n");
        ti->skip_point = mp[sp];
        // printf("P1 okay so far 178\n");
        ti->ep = sp + chunkSize;
        // printf("P1 okay so far 180\n");
        sp += chunkSize;
        // printf("P1 okay just far \n");
		ti->mutex = shared_memory->mutex[0];
		ti->thread_num = j;
		// printf("Make one \n");
        // printf("start point is %d and ends at %d \n",ti->sp,ti->ep);
        pthread_create(&thread[j],NULL,thread_worker,ti);
    }

    for(int j = 0; j < tc; j++){
        pthread_join(thread[j],NULL);
    }
	// en_time = clock();
	clock_gettime(CLOCK_REALTIME,&en_time);
	// double ta_time = ((double)(en_time-st_time))/CLOCKS_PER_SEC;
	sub_timespec(st_time,en_time,&delta);
	time_log->ta_time[0].tv_sec += (int)delta.tv_sec;
	time_log->ta_time[0].tv_nsec += delta.tv_nsec;
	// time_log->ta_time[0] = ta_time;
    fprintf(data_file, "%d.%.9lds\n",(int)time_log->wt_time[0].tv_sec,time_log->wt_time[0].tv_nsec);
    //(5);
	fclose(data_file);
	shared_memory->finished[0] = 1;
	return 0;
}
