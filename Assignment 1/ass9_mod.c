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
 
// global variable to store minisquare size; sqrt(size)
int ms_size;
 
// compile command
// gcc -pthread sudoku.c -o sudoku.out -lm
 
#define NUM_THREADS 36
int log__ = 0; // flag to log values for debugging 
int tc_glob = 0; // global thread count;
int ec = 0;
int pc = 0;
int found = 0;
//static pthread_mutex_t printf_mutex;

int thread_spawner(int board[36][36], int s,int useThreads,int call_id);
 
void read_grid_from_file(int size, char *ip_file, int grid[36][36]) {
	FILE *fp;
	int i, j;
	fp = fopen(ip_file, "r");
	for (i=0; i<size; i++) {
		for (j=0; j<size; j++) {
			fscanf(fp, "%d", &grid[i][j]);
		}
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

// function to save the output of the grid, used on a server if ssh connection is broken.
void write_grid(int size,int grid[36][36]){
	int i,j;
	FILE *fp;
	fp = fopen("output.txt","w");
	for(int i = 0; i < size; i++){
		for(int j = 0; j < size; j++){
			fprintf(fp,"%d\t",grid[i][j]);
		}
		fprintf(fp,"\n");
	}
	fclose(fp);
	
}
 
// function to check if value c can be added to the board at row,col position. 
int check(int board[36][36], int row, int col, int c, int s){
    for(int i=0;i<s;i++){
        if(board[i][col] == c || board[row][i] == c) return 0;
    }

    row = row - row%ms_size; // mini-square start point row 
    col = col - col%ms_size; // mini-square start point col

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

// function to find the possible values an empty cell can take up  
avl_opt reduceSolSpace(int board[36][36],int s, int row, int col){
    int taken[37];
    memset(taken,0,sizeof(int)*(s+1));

    for(int i=0;i<s;i++){
        if(board[i][col] != 0){
        	taken[board[i][col]] = 1;
        }
        if(board[row][i] != 0){
        	taken[board[row][i]] = 1; 
       } 
    }

    row = row - row%ms_size; // ms start point row 
    col = col - col%ms_size; // ms start point col

    for(int i=0;i<ms_size;i++){
        for(int j=0;j<ms_size;j++){
            if(board[row+i][col+j] != 0){
                taken[board[row+i][col+j]] =1;
            }
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

// function to find the first empty cell with fewest possible options
int findEmpty(int board[36][36], int s, int * r,int  * c){
	int min_r = -1, min_c = -1;
	int min_len = 40;
	for(int i = 0; i <s; i++){
		for(int j = 0; j < s; j++){
			if(board[i][j] == 0){
				avl_opt opts = reduceSolSpace(board,s,i,j);
				if(opts.n < min_len){
					min_len = opts.n;
					min_r = i;
					min_c = j;
				}
			}
		}
	}
	*r = min_r;
	*c = min_c;
	if(*r == -1 || *c == -1 ) return 0;
	return 1;
}

int emptyCount(int board[36][36], int s){
	int c = 0;
	for(int i = 0; i < s;i ++){
		for(int j = 0; j < s; j++){
			if(board[i][j] == 0) 
				c++;
		}
	}
	return c;
}
 
int solve(int board[36][36], int s) {
	int r = -1, c = -1;
	int k = findEmpty(board,s,&r,&c);
	
	
	
    if(k != 0 && r != -1 && c != -1){
    	avl_opt reduceSpace = reduceSolSpace(board,s,r,c);
    	if(reduceSpace.n == 0){
    		return 0;
    	}
    	for(int i = 0; i < reduceSpace.n; i++){
    		int v = reduceSpace.avl_options[i];
    		if(check(board, r, c, v,s) == 1){
                board[r][c] = v; 
                if(solve(board,s))
                    return 1; 
                else{
                    board[r][c] = 0; 
                }
            }
    	}
        return 0;
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

int board_glob[128][36][36];
int board_solve_id = -1;

void *solve_thread(void *arg){
    game *g = (game *)arg;
    int r = g->row, c = g->col;
    int v = g->val_attempt;
    int s = g->size;
    int board[36][36];
    for(int i = 0; i < s; i++){
        for(int j = 0; j < s; j++){
            board[i][j] = g->board[i][j];
        }
    }
    if(check(board,r,c,v,s) == 1){
        board[r][c] = v;
		int ut = tc_glob <= 8 ? 1 : 0;
        if(thread_spawner(board,s,ut,g->t_id) == 1 && emptyCount(board,s) == 0 && !found){	
            g->solved = 1;
            found = 1;
            memcpy(board_glob[g->t_id], board, sizeof(board));
            board_solve_id = g->t_id;
            pthread_exit(NULL);
        }
        else{
            board[r][c] = 0;
        }
    }
    g->solved = 0;
    pthread_exit(NULL);
}

void kill_threads(pthread_t t_p[], int n,int exp){
	for (int i = 0; i < n; i++){
		if(i != exp){
			pthread_cancel(t_p[i]);
		}
	}
}
  
int thread_spawner(int board[36][36], int s,int useThreads,int call_id){
	game game_board[36];
	pthread_t threads[36];
    int r = -1,c = -1;
    int k = findEmpty(board,s,&r,&c);
    if(k == 0 || r == -1 || c == -1){
        return -1;
    }
    avl_opt reduceSpace = reduceSolSpace(board,s,r,c);
    while(reduceSpace.n == 1 && k!=0 && r!=-1 && c!=-1){
		board[r][c] = reduceSpace.avl_options[0];
		if(found == 1){
			break;
		}
		k = findEmpty(board,s,&r,&c);
		if(k==0 || r==-1 || c==-1 && emptyCount(board,s) == 0 && !found){
			found = 1;
			int t_id = call_id == -1 ? 0 : call_id; 
			memcpy(board_glob[t_id], board, 36*36*sizeof(int));
            		board_solve_id = t_id;
			reduceSpace.n = 0;
			return 1;
		}
		reduceSpace = reduceSolSpace(board,s,r,c);
	}


	if(found){
		return 1; 
	}
	// board still unsolved 
	if(reduceSpace.n != 0 && r != -1 && c != -1 && useThreads == 1){
		
		for(int i = 0; i < reduceSpace.n; i++){
			memcpy(game_board[i].board, board, sizeof (int) *36* 36);
			game_board[i].size = s;
			game_board[i].t_id = tc_glob;
			game_board[i].val_attempt = reduceSpace.avl_options[i];
			game_board[i].row = r;
			game_board[i].col = c;
			tc_glob++;
			if(pthread_create(&threads[i],NULL,solve_thread, (void *)&game_board[i])){
				printf("Error making thread %d at tc val %d \n", i,tc_glob);
				exit(1);
			}
		}
		for(int i = 0; i < reduceSpace.n; i++){
			pthread_join(threads[i],NULL);
			if(game_board[i].solved){
				return 1;
			}
		}
	} else {

		for(int i = 0; i < reduceSpace.n; i++){
			board[r][c] = reduceSpace.avl_options[i];
			if(solve(board,s)){
				return 1;
			}else{
				board[r][c] = 0;
			}
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
	ec = emptyCount(grid,size);
	printf("No of empty blocks in the grid initially is %d \n", ec);
 
	if(thread_spawner(grid,size,1,-1) == -1){
        	printf("Error no empty spaces in the grid \n");
        	print_grid(size,grid);
        	return 1;
    	}
        if(found == 1 && board_solve_id != -1){
        	printf("Empty on board %d \n",emptyCount(board_glob[board_solve_id],size));
    		printf("Board solve id is %d \n",board_solve_id);
    		print_grid(size,board_glob[board_solve_id]);
	}else {
		printf("Some error occured \n");
	}
	printf(" %d threads made \n",tc_glob+1	);
	return 0;
}
