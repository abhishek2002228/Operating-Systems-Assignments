# include <stdio.h>
# include <stdlib.h>

// Header files for threads
#include <pthread.h>


// Math
#include <math.h>

// Fork/ Proc related headers
#include<unistd.h>
#include<sys/wait.h>

// Error handling
#include<errno.h>
#include <assert.h>

// global variable to store minisquare size;
int ms_size;

// compile command
// gcc -pthread sudoku.c -o sudoku.out -lm


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
board[ms_size * (row / ms_size) + i / ms_size][ms_size * (col / ms_size) + i % ms_size] == c) return 0; //check 3*3 block
    }
    return 1;
}

int solve(int board[36][36], int s) {
    for(int i = 0; i < s; i++){
        for(int j = 0; j < s; j++){
            if(board[i][j] == 0){
                for(int c = 1; c <= s; c++){
                    if(check(board, i, j, c,s) == 1){
                        board[i][j] = c; 
                        if(solve(board,s))
                            return 1; 
                        else
                            board[i][j] = 0; 
                    }
                }
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
	solve(grid,size);
	print_grid(size, grid);
	return 0;
}
