# include <stdio.h>
# include <stdlib.h>
 
// Header files for threads
#include <pthread.h>
 
 
// Math
#include <math.h>
 
// Time to solve
#include <time.h>
 
// Fork/ Proc related headers
#include<unistd.h>
#include<sys/wait.h>
 
// Error handling
#include<errno.h>
#include <assert.h>
#include <string.h>
 
// global variable to store minisquare size;
int ms_size;
 
// compile command
// gcc -pthread sudoku.c -o sudoku.out -lm
 
#define NUM_THREADS 6
 
typedef struct {
	long int begin;
	long int end;
	int row;
	int column;
	int s;
	int t_id;
	int lr_fit;
	int board[36][36];
} lookup_range;
 
lookup_range lr_arr[NUM_THREADS];
pthread_t threads[NUM_THREADS];
 
 
void read_grid_from_file(int size, char *ip_file, int grid[36][36]) {
	FILE *fp;
	int i, j;
	fp = fopen(ip_file, "r");
	for (i=0; i<size; i++) 
		for (j=0; j<size; j++) {
			fscanf(fp, "%d", &grid[i][j]);
	}
} 
 
void print_grid(int size, int grid[36][36]) {
	int i, j;
	/* The segment below prints the grid in a standard format. Do not change */
	for (i=0; i<size; i++) {
		for (j=0; j<size; j++)
			printf("%d\t", grid[i][j]);
		printf("\n");
	}
}
 
// checks if configuration is valid
// needs to be modified for multi threading.
int check(int board[36][36], int row, int col, int c, int s){
 
    for(int i = 0; i < s; i++) {
        if(board[i][col] != 0 && board[i][col] == c) return 0; //check row
        if(board[row][i] != 0 && board[row][i] == c) return 0; //check column
        if(board[ms_size * (row / ms_size) + i / ms_size][ ms_size * (col / ms_size) + i % ms_size] != 0 && 
board[ms_size * (row / ms_size) + i / ms_size][ms_size * (col / ms_size) + i % ms_size] == c) return 0; //check sqrt(s)*sqrt(s) block
    }
    return 1;
}
 
void *func(void *val){
	lookup_range *l = val;
	printf("Working on row %d column %d thread %d \n",l->row,l->column,l->t_id);
	for(int i = l->begin; i <= l->end; i++){
		if(check(l->board, l->row,l->column, i,l->s) == 1){
			l->board[l->row][l->column] = i;
			printf(" settled on %d on thread %d \n",i, l->t_id);
			l->lr_fit = 1;
			return NULL;
			// if(solve(l->board,l->s,threads)){
			// 	return NULL;
			// }else{
			// 	l->board[l->row][l->column] = 0;
			// }
		}
	}
}
 
int solve(int board[36][36], int s, pthread_t t[NUM_THREADS]) {
    for(int i = 0; i < s; i++){
        for(int j = 0; j < s; j++){
            if(board[i][j] == 0){
            	printf(" Found empty at row %d and column %d \n",i,j);
            	// spawn ms_size threads;
            	for(int k = 0; k < ms_size; k++){
            		 memcpy(lr_arr[k].board, board, sizeof (int) * s* s);
 
            		lr_arr[k].begin = (s/ms_size)*k +1;
            		lr_arr[k].end = ((s/ms_size)*(k+1));
            		lr_arr[k].row = i;
            		lr_arr[k].column = j;
            		lr_arr[k].t_id = k;
            		lr_arr[k].s = s;
            		lr_arr[k].lr_fit = 0;
            		if(pthread_create(&t[k],NULL,func, (void *)&lr_arr[k])){
			// Error happened.
						printf("Error making thread %d \n", k);
						exit(1);
					}
            	}
 
            	for(int k = 0; k < ms_size; k++){
                    pthread_join(t[k],NULL);

					if(lr_arr[k].lr_fit == 1){
						memcpy(board,lr_arr[k].board,sizeof(board));
						printf("kuch hoa %d \n",k);
						solve(board,s,threads);
					}
 
					
				}
 
                // for(int c = 1; c <= s; c++){
                //     if(check(board, i, j, c,s) == 1){
                //         board[i][j] = c; 
                //         if(solve(board,s,t))
                //             return 1; 
                //         else
                //             board[i][j] = 0; 
                //     }
                // }
                return 0;
            }
        }
    }
    return 1;
}
 
 
int main(int argc, char *argv[]) {
	int grid[36][36], size, i, j;
	
	if (argc != 3) {
		printf("Usage: ./sudoku.out grid_size inputfile");
		exit(-1);
	}
	
	size = atoi(argv[1]);
	ms_size = (int) sqrt(size);
	printf(" mini sqaure size is %d \n",ms_size);
	assert(size == 1 || size == 4 || size == 9 || size == 16 || size == 25 || size == 36);
	read_grid_from_file(size, argv[2], grid);
	
	/* Do your thing here */
	print_grid(size,grid);
	printf("Now solving \n");
	clock_t t;
	t = clock();
	solve(grid,size,threads);
	t = clock()-t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC;
	print_grid(size, grid);
	printf("took %f seconds to solve \n", time_taken);
	return 0;
}