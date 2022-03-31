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
 
#define NUM_THREADS 36
pthread_t threads[NUM_THREADS];
int log__ = 0; // flag to log values tried. please don't change to 1 unless u want hdd fried
int tc_glob = 0; // global thread count;
 
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
/*int check(int board[36][36], int row, int col, int c, int s){
 
    for(int i = 0; i < s; i++) {
        if(board[i][col] != 0 && board[i][col] == c) return 0; //check row
        if(board[row][i] != 0 && board[row][i] == c) return 0; //check column
        if(board[ms_size * (row / ms_size) + i / ms_size][ ms_size * (col / ms_size) + i % ms_size] != 0 && 
board[ms_size * (row / ms_size) + i / ms_size][ms_size * (col / ms_size) + i % ms_size] == c) return 0; //check sqrt(s)*sqrt(s) block
    }
    return 1;
}*/
int check(int board[36][36], int row, int col, int c, int s){
    for(int i=0;i<s;i++){
        if(board[i][col] == c || board[row][i] == c) return 0;
    }

    row = row - row%ms_size; // ms start point row 
    col = col - col%ms_size; // ms start point col

    for(int i=0;i<ms_size;i++){
        for(int j=0;j<ms_size;j++)
            if(board[row+i][col+j] == c){
                return 0;
            }
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
            	if(reduceSpace.n == 0){
            		return 0;
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
 
int findEmpty(int board[36][36], int s, int * r,int  * c){
	for(int i = 0; i <s; i++){
		for(int j = 0; j < s; j++){
			if(board[i][j] == 0){
				*r = i;
				*c = j;
				return 1;
			}
		}
	}
	return 0;
}

void *depth3Solver(void *val){
    game *l = val;
    if(check(l->board,l->row,l->col,l->val_attempt,l->size) == 1){
        l->board[l->row][l->col] = l->val_attempt;
        if(solve(l->board,l->size,NULL) == 0){
            // couldn't solve
            l->solved = 0; 
            //printf("Solved attempt failed for thread %d & val %d \n",l->t_id,l->val_attempt);
            //fprintf(fptr,"Failed to solve \n");
            //fclose(fptr);
            pthread_exit(NULL);
            return NULL;
        }else {
            l-> solved = 1;
            print_grid(l->size,l->board);
            //fclose(fptr);
            pthread_exit(NULL);
            return NULL;
        }
    }else{
        l->solved = 0;
        pthread_exit(NULL);
        return NULL;
    }
}
 
void *depth2Solver(void *val){
	game *l = val;
	if(check(l->board,l->row,l->col,l->val_attempt,l->size) == 1){
		l->board[l->row][l->col] = l->val_attempt;
        int r = -1,c = -1;
        int k = findEmpty(l->board,l->size,&r,&c);
        if(k != 0 && r != -1 && c != -1){
            avl_opt opts = reduceSolSpace(l->board,l->size,r,c);
            pthread_t thirdLayer[36];
            game *gb3 = (game*) malloc(opts.n * sizeof(game)); 
            for(int i = 0; i < opts.n; i++){
                memcpy(gb3[i].board, l->board, sizeof (int) *36* 36);
                gb3[i].size = l->size;
                gb3[i].t_id = i;
                gb3[i].val_attempt = opts.avl_options[i];
                gb3[i].row = r;
                gb3[i].col = c;
                tc_glob++;
                if(pthread_create(&thirdLayer[i],NULL,depth3Solver, (void *)&gb3[i])){
                    printf("Error making thread %d at tc val %d \n", i,tc_glob);
                    exit(1);
                }
            }
 
            for(int i = 0; i < opts.n; i++){
                pthread_join(thirdLayer[i],NULL);
                if(gb3[i].solved){
                    printf("Found winner at thread %d \n",i);
                    l->solved = 1;
                    pthread_exit(NULL);
                    //print_grid(game_board[k].size,game_board[k].board);
                }
            }
        }else {
            // board is filled completely
            l-> solved = 1;
            print_grid(l->size,l->board);
            pthread_exit(NULL);
            return NULL;
        }
	}else {
		l->solved = 0;
		pthread_exit(NULL);
		return NULL;
	}
}	
 
void *multiThreadSolver(void *val){
	game *l = val;	
	if(check(l->board,l->row,l->col,l->val_attempt,l->size) == 1){
		//printf("Will continue with thread no %d and val filled %d \n", l->t_id,l->val_attempt);
		
		// log file setup
		//FILE *fptr;
		//char buffer[32]; // The filename buffer.
   
    	//snprintf(buffer, sizeof(char) * 32, "./logs/run%i_file%i.txt",l->size,l->t_id);
    	//fptr = fopen(buffer, "wb");
		// log file setup end
 
		l->board[l->row][l->col] = l->val_attempt;
		int r = -1,c = -1;
		int k = findEmpty(l->board,l->size,&r,&c);
		if(k != 0 && r != -1 && c != -1){
			avl_opt opts = reduceSolSpace(l->board,l->size,r,c);
			pthread_t secondLayer[36];
			game *gb2 = (game*) malloc(opts.n * sizeof(game)); 
			for(int i = 0; i < opts.n; i++){
				memcpy(gb2[i].board, l->board, sizeof (int) *36* 36);
				gb2[i].size = l->size;
				gb2[i].t_id = i;
				gb2[i].val_attempt = opts.avl_options[i];
				gb2[i].row = r;
				gb2[i].col = c;
				tc_glob++;
				if(pthread_create(&secondLayer[i],NULL,depth2Solver, (void *)&gb2[i])){
					printf("Error making thread %d at tc val %d \n", k,tc_glob);
					exit(1);
				}
			}
 
			for(int i = 0; i < opts.n; i++){
				pthread_join(secondLayer[i],NULL);
				if(gb2[i].solved){
            		printf("Found winner at thread %d \n",i);
            		l->solved = 1;
      				pthread_exit(NULL);
            		//print_grid(game_board[k].size,game_board[k].board);
	            }
			}
		}else {
			// board is filled completely
			l-> solved = 1;
			print_grid(l->size,l->board);
			pthread_exit(NULL);
			return NULL;
		}
	}else{
		l->solved = 0;
		pthread_exit(NULL);
		return NULL;	
	}
}
 
int thread_spawner(int board[36][36], int s){
	game game_board[36];
    int r = -1,c = -1;
    int k = findEmpty(board,s,&r,&c);
    if(k == 0 || r == -1 || c == -1){
        return -1;
    }
    avl_opt reduceSpace = reduceSolSpace(board,s,r,c);

	for(int i = 0; i < reduceSpace.n; i++){
		memcpy(game_board[i].board, board, sizeof (int) *36* 36);
		game_board[i].size = s;
		game_board[i].t_id = i;
		game_board[i].val_attempt = reduceSpace.avl_options[i];
	}

    printf(" In the first step going to make %d threads \n ", reduceSpace.n);
    printf("Possible values for this position are \n");
    for(int i = 0; i < reduceSpace.n; i++){
        printf("%d ",reduceSpace.avl_options[i]);
    }
    printf("\n");
    for(int i = 0; i < reduceSpace.n; i++){
        game_board[i].row = r;
        game_board[i].col = c;
        tc_glob++;
        if(pthread_create(&threads[i],NULL,multiThreadSolver, (void *)&game_board[i])){
            printf("Error making thread %d at tc val %d \n", i,tc_glob);
            exit(1);
        }

        // pthread_join(threads[k],NULL);
        
    }

    printf("Value of tc_glob after spawning s threads %d \n", tc_glob);
    for(int i = 0; i < reduceSpace.n; i++){
        pthread_join(threads[i],NULL);
        if(game_board[i].solved){
            printf("Found winner at thread %d \n",i);
            return 0;
            //print_grid(game_board[k].size,game_board[k].board);
        }
    }

	return 0;
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

 
	if(thread_spawner(grid,size) == -1){
        printf("Error no empty spaces in the grid \n");
        print_grid(size,grid);
    }
	printf(" %d threads made \n",tc_glob);
	return 0;
}
