#include<stdio.h>
#include<math.h>
#include<string.h>
#include<stdlib.h>
#include <time.h>

unsigned long long sum = 0;

int main(int argc, char const *argv[]){
	char *fileName = argv[1];
	int count = atoi(argv[2]);
	FILE *data_file = fopen(fileName,"r");
	clock_t start, end;
	int temp;
	start =clock();
	for(int i = 0; i < count; i++){
		fscanf(data_file,"%d",&temp);
		sum += temp;
	}
	fclose(data_file);
	end = clock();
	printf("Sum is %lld \n",sum);
	double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("Time taken on a single thread is %f \n",cpu_time_used);
	return 0;
}
