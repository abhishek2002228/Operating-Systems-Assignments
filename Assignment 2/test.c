#include<stdio.h>
#include<assert.h>
#include<pthread.h>
#include<math.h>
#include<string.h>
#include<stdlib.h>


int main(int argc, char const *argv[]){
	assert(argc == 3);
	char *fileName = argv[1];
	int count = atoi(argv[2]);
	FILE* file = fopen(fileName,"r");
	if(file == NULL){
		printf("Error opening file \n");
		exit(1);
	}
	printf("Success \n");
	int temp;
	int i = 0;
	int t = 0;
	//fseek(file,3*sizeof(int),SEEK_SET);
	int *mp = (int *) malloc(sizeof(int) * count);
	mp[i] = 0;
	while(!feof(file)){
		fscanf(file,"%d",&temp);
		t = ftell(file);
		printf("Starting point at i %d and sp is %ld\n",i+1,t);
		mp[i+1] = t;
		i++;
	//printf("%d \n",temp);
	
	}
	printf("value of i is %d \n",i);
	
	// for(int j = 0; j < 5; j++){
	// int tc = pow(2,j);
	// int chunk_size = (count%tc == 0) ? count/tc : count/tc ;
	// printf("chunk size is %d \n",chunk_size);
	// int sp = 0;
	// for(int i = 0; i < tc; i++){
	// 	printf("start point is %d and end point is %d\n", sp, sp + chunk_size);
	// 	sp += chunk_size + 1;
	// }
	// printf("-------------------\n");
	// }
	return 0;
}
