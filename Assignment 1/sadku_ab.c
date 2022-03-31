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
#define SIZE_GRID 36*36
#define NUM_THREADS 36
pthread_t threads[NUM_THREADS];
int log__ = 0; // flag to log values tried. please don't change to 1 unless u want hdd fried


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

typedef struct {
	int avl_options[36];
	int n;
} avl_opt;

avl_opt reduceSolSpace(int board[36][36],int s, int row, int col){
	int taken[37];
	memset(taken,0,sizeof(int)*(s+1));
	for(int i = 0; i < s; i++) {
        if(board[i][col] != 0) taken[board[i][col]] = 1; //check row
        if(board[row][i] != 0) taken[board[row][i]] = 1; //check column
        if(board[ms_size * (row / ms_size) + i / ms_size][ ms_size * (col / ms_size) + i % ms_size] != 0) {
        	taken[board[ms_size * (row / ms_size) + i / ms_size][ ms_size * (col / ms_size) + i % ms_size]] = 1;
    	}
    }

    avl_opt ret;
    memset(ret.avl_options,0,sizeof(ret.avl_options));
    ret.n = 0;
    for(int i = 1; i <= s;i++){
    	if(!taken[i]){
    		ret.avl_options[ret.n++] = i;
    	}
    }
    return ret;
}

int solve(int board[36][36], int s,FILE *fptr) {
    for(int i = 0; i < s; i++){
        for(int j = 0; j < s; j++){
            if(board[i][j] == 0){

            	// use this for later stage 2 for optimization
            	avl_opt reduceSpace = reduceSolSpace(board,s,i,j);
            	if(log__){
	            	fprintf(fptr,"currently working on row %d col %d \n",i,j);
            	}
				for(int k = 0; k < reduceSpace.n; k++){
            		int c = reduceSpace.avl_options[k];
	            	if(log__){
		            	fprintf(fptr,"now trying %d \n",c);
	            	}
            		if(check(board, i, j, c,s) == 1){
                        board[i][j] = c; 
                        if(solve(board,s,fptr))
                            return 1; 
                        else
                            board[i][j] = 0; 
                    }
            	}
            	// should use this bottom non optimized part
                // for(int c = 1; c <= s; c++){
                //     if(check(board, i, j, c,s) == 1){
                //         board[i][j] = c; 
                //         if(solve(board,s))
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

typedef struct {
	int t_id;
	int row,col;
	int val_attempt;
	int size;
	int solved;
	int board[36][36];
} game;

void *multiThreadSolver(void *val){
	game *l = val;	
	if(check(l->board,l->row,l->col,l->val_attempt,l->size) == 1){
		printf("Will continue with thread no %d and val filled %d \n", l->t_id,l->val_attempt);
		
		// log file setup
		FILE *fptr;
		char buffer[32]; // The filename buffer.
   
    	snprintf(buffer, sizeof(char) * 32, "./logs/run%i_file%i.txt",l->size,l->t_id);
    	fptr = fopen(buffer, "wb");
		// log file setup end

		l->board[l->row][l->col] = l->val_attempt;
		if(solve(l->board,l->size,fptr) == 0){
			// couldn't solve
			l->solved = 0; 
			printf("Solved attempt failed for thread %d & val %d \n",l->t_id,l->val_attempt);
			fprintf(fptr,"Failed to solve \n");
			fclose(fptr);
			return NULL;
		}else {
			l-> solved = 1;
			fprintf(fptr,"Solve successful \n");
			fclose(fptr);
			return NULL;
		}
	}else{
		l->solved = 0;
		return NULL;	
	}
}

int thread_spawner(int board[36][36], int s){
	game game_board[36];
	for(int i = 0; i < s; i++){
		memcpy(game_board[i].board, board, sizeof (int) *36* 36);
		game_board[i].size = s;
		game_board[i].t_id = i;
		game_board[i].val_attempt = i+1;
	}
	int first_found = 0;
	for(int i = 0; i < s; i++){
        for(int j = 0; j < s; j++){
            if(board[i][j] == 0){
            	first_found = 1;
            	avl_opt reduceSpace = reduceSolSpace(board,s,i,j);
            	printf("Possible values for this position are \n");
            	for(int k = 0; k < reduceSpace.n; k++){
            		printf("%d ",reduceSpace.avl_options[k]);
            	}
            	printf("\n");
            	for(int k = 0; k < s; k++){
            		game_board[k].row = i;
            		game_board[k].col = j;
	            	if(pthread_create(&threads[k],NULL,multiThreadSolver, (void *)&game_board[k])){
						printf("Error making thread %d \n", k);
						exit(1);
					}

					// pthread_join(threads[k],NULL);
					
				}

				for(int k = 0; k < s; k++){
	            	pthread_join(threads[k],NULL);
	            	if(game_board[k].solved){
	            		printf("Found winner at thread %d \n",k);
	            		print_grid(game_board[k].size,game_board[k].board);
	            	}
				}

            	break;
            }
        }
        if(first_found == 1){
        	break;
        }
    }
}

int find_empty(int board[36][36], int size, int *row, int *column){
	for(int i = 0; i < size; i++){
		for(int j = 0; j < size; j++){
			if(board[i][j] == 0){
				*row = i;
				*column = j;
				return 1;
			}
		}
	}
	return 0;
}

void reduce_space(int board[36][36], int size, int row, int column, int *reduced_array){
	int a, b, c;
	for(int i = 0; i < size; i++){
		if((a = board[row][i]) != 0) //checking columns
			reduced_array[a-1] = 1;
		if((b = board[i][column]) != 0) //checking rows
			reduced_array[b-1] = 1;
		if((c =board[ms_size*(row/ms_size)+i/ms_size][ms_size*(column/ms_size)+i%ms_size]) != 0)
			reduced_array[c-1] = 1;//checking mini-square
	}
	return;
}

void debug_check(){
	for(int i = 10000; i>= 0; i--){
		;
	}
	return;
}

int sudoku_solver(int board[36][36], int size){
	int row, column;
	if(!find_empty(board, size, &row, &column)){
		debug_check();
		return 1;
	}
	//int reduced_array[36] = {0};
	//reduce_space(board, size, row, column, reduced_array);

	for(int c = 1; c <= size; c++){
		if(check(board, row, column, c, size)){
			board[row][column] = c;
			if(sudoku_solver(board, size))
				return 1;
			else
				board[row][column] = 0;
		}
	}
	return 0;	
}
/*
struct board{
	int grid[36][36];
	int solved;
	int size;
	int thread_num;
	int row;
	int column;
};

int mt_sudoku(int grid[36][36], int size){
	int row, column;
	pthread_t threads2[36][36];

	if(!find_empty(grid, size, &row, &column)){
		debug_check();
		return 1;
	}
	//int reduced_array[36] = {0};
	//reduce_space(board, size, row, column, reduced_array);
	
	struct board *boards[36][36];

	for(int i = 0; i < size; i++){
		for(int j = 0; j < size; j++){
			if(grid[i][j] == 0){
				memcpy(boards[i][j]->grid, grid, sizeof(grid));
				boards[i][j]->solved = 0;
				boards[i][j]->size = size;
				boards[i][j]->thread_num = size*i + j;
				boards[i][j]->row = i;
				boards[i][j]->column = j;
				pthread_create(&threads2[i][j],NULL, mt_sudoku_solver, boards[i][j]);

				}			
			}
	}
	
	int solved_one = 0;
}
*/
typedef struct thread_data {
	int grid[36][36];
	int size;
	int c;
	int solved;
} thread_data;

int find_empty2(thread_data *tdata, int *row, int *column){
	int size = tdata->size;
	for(int i = 0; i < size; i++){
		for(int j = 0; j < size; j++){
			if(tdata->grid[i][j] == 0){
				*row = i;
				*column = j;
				return 1;
			}
		}
	}
	return 0;
}


void *mt_sudo(void *arg){
	thread_data *tdata = (thread_data *)arg;
	int row, column;
	if(!find_empty2(tdata, &row, &column)){
		tdata->solved = 1;
		pthread_exit(NULL);
	}

	thread_data tdatas[36];
	pthread_t threads[36] = {0};

	for(int c = 1; c <= tdata->size; c++){
		if(check(tdata->grid, row, column, c, tdata->size)){
			memcpy(tdatas[c-1].grid, tdata->grid, SIZE_GRID*sizeof(tdata->grid[0][0]));
			tdatas[c-1].grid[row][column] = c; 
			tdatas[c-1].size = tdata->size;
			tdatas[c-1].c = c;
			tdatas[c-1].solved = 0;
			pthread_create(&threads[c-1], NULL, mt_sudo, (void *)&tdatas[c-1]);	
		}
	}

	for(int c = 1; c<= tdata->size; c++){
		if(threads[c-1] != 0){
			pthread_join(threads[c-1], NULL);
			if(tdatas[c-1].solved == 1){
				tdata->solved = 1;
				memcpy(tdata->grid, tdatas[c-1].grid, SIZE_GRID*sizeof(tdatas[c-1].grid[0][0]));
				pthread_exit(NULL);
			}
		}

	}
	tdata->solved = 0;
	pthread_exit(NULL);

}

int mt_sudo_main(int grid[36][36], int size){
	thread_data tdata;
	memcpy(tdata.grid, grid, SIZE_GRID*sizeof(grid[0][0]));
	tdata.size = size;
	tdata.solved = 0;

	pthread_t thread;
	pthread_create(&thread, NULL, mt_sudo, (void *)&tdata);
	pthread_join(thread, NULL);

	memcpy(grid, tdata.grid, SIZE_GRID*sizeof(tdata.grid[0][0]));
	return tdata.solved;
}


int main(int argc, char *argv[]) {
	int grid[36][36], size, i, j;
	
	if (argc != 3) {
		printf("Usage: ./sudoku.out grid_size inputfile");
		exit(-1);
	}
	
	size = atoi(argv[1]);
	ms_size = (int) sqrt(size); //minisquare size
	printf(" mini sqaure size is %d \n",ms_size);
	assert(size == 1 || size == 4 || size == 9 || size == 16 || size == 25 || size == 36);
	read_grid_from_file(size, argv[2], grid);
	
	/* Do your thing here */
	print_grid(size,grid);

	printf("\n");

	printf("Now solving \n");
//	clock_t t;
//	t = clock();
//	solve(grid,size);
	int ret = mt_sudo_main(grid, size);

	printf("return value: %d\n", ret);
//	thread_spawner(grid,size);
//	t = clock()-t;
//    double time_taken = ((double)t)/CLOCKS_PER_SEC;
	print_grid(size, grid);
//	printf("took %f seconds to solve \n", time_taken);
	return 0;
}

