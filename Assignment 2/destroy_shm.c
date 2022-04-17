#include <sys/shm.h>
#include <stdio.h>
#include <semaphore.h>

typedef struct {
	sem_t mutex[2];
	int finished[2];
	
} proc_data;

typedef struct{
	double ta_time[2];
	double wt_time[2];
	double run_time[2];
} time_logging;


int main(void){

int shm_id_main = shmget(ftok("p1.c",51), sizeof(proc_data), 0666 | IPC_CREAT);
int shm_id_data = shmget(ftok("p2.c",51),(100+1)*sizeof(int),0666|IPC_CREAT);
int shmid_flags = shmget(ftok("p2.c", 52), (8)*sizeof(int), 0666|IPC_CREAT);
int shm_id_seek = shmget(ftok("p2.c",44),(100+1)*sizeof(int),0666|IPC_CREAT);
int shm_time_logging = shmget(ftok("main_sched.c", 51), sizeof(time_logging), 0666|IPC_CREAT);


if(shmctl(shm_id_main, IPC_RMID, NULL) == -1){
    exit(1);
}
if(shmctl(shm_id_data, IPC_RMID, NULL) == -1){
    exit(1);
}
if(shmctl(shm_id_seek, IPC_RMID, NULL) == -1){
    exit(1);
}
if(shmctl(shmid_flags, IPC_RMID, NULL) == -1){
    exit(1);
}
if(shmctl(shm_time_logging, IPC_RMID, NULL) == -1){
    exit(1);
}


return 0;
}