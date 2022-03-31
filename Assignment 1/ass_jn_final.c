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
int total_threads = 0;
int ec = 0;
int found = 0;
 
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

typedef struct {
	int board[36][36];
	int row;
	int col;
	int c;
	int s;
	int fit;
} checkrc;

typedef struct {
	int board[36][36];
	int row;
	int col;
	int c;
	int s;
	int fit;
} checkms;

void * checkRowCol(void *params){
	checkrc* rc = (checkrc*)params;
	for(int i=0; i<rc->s; i++){
		if(rc->board[i][rc->col] == rc->c || rc->board[rc->row][i] == rc->c) rc->fit = 0;
	}
}

void * checkMiniSq(void *params){
	checkms* ms = (checkms*)params;
	ms->row = ms->row - ms->row%ms_size; // ms start point row 
    	ms->col = ms->col - ms->col%ms_size; // ms start point col
    
	for(int i=0;i<ms_size;i++){
		for(int j=0;j<ms_size;j++)
		    if(ms->board[ms->row+i][ms->col+j] == ms->c){
		        ms->fit = 0;
		    }
	    }
}
 

int check(int board[36][36], int row, int col, int c, int s){
    /*for(int i=0;i<s;i++){
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
    */
    checkrc c1; checkms c2;
    memcpy(c1.board, board, sizeof (int) *36* 36);//why copying memory here
    memcpy(c2.board, board, sizeof (int) *36* 36);//we are not changing anything right?
    c1.row = row; c2.row = row; c1.col = col; c2.col = col; c1.c = c; c2.c = c; c1.s = s; c2.s = s;
    c1.fit = 1; c2.fit = 1;
    tc_glob = tc_glob+2;
    pthread_t t[2];
    pthread_create(&t[0],NULL,checkRowCol, (void *)&c1);
    pthread_create(&t[1],NULL,checkMiniSq, (void *)&c2);
    pthread_join(t[0],NULL); pthread_join(t[1],NULL);
    if(c1.fit && c2.fit){return 1;}
    return 0;
    //return 1;
}
 
typedef struct {
	int avl_options[36];
	int n;
} avl_opt;

int board_glob[36][36];
int board_solve_id = -1;
 
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
 
int solve(int board[36][36], int s,FILE *fptr) {
	int r = -1, c = -1;
	int k = findEmpty(board,s,&r,&c);
	//printf("Solve function: running for row: %d column: %d\n",r,c);
	
	
    if(k != 0 && r != -1 && c != -1){
    	avl_opt reduceSpace = reduceSolSpace(board,s,r,c);
    	if(reduceSpace.n == 0){
    		return 0;
    	}
    	for(int i = 0; i < reduceSpace.n; i++){
    		int v = reduceSpace.avl_options[i];
        	/*if(log__){
            	fprintf(fptr,"now trying %d \n",c);
        	}*/
    		if(check(board, r, c, v,s) == 1){
                board[r][c] = v; 
                if(solve(board,s,fptr))
                    return 1; 
                else
                    board[r][c] = 0; 
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
	int empty;
} game;
 

void *depth9Solver(void *val){
    game *l = val;
    if(check(l->board,l->row,l->col,l->val_attempt,l->size) == 1){
        l->board[l->row][l->col] = l->val_attempt;
        //printf("In depth3 solver, now going into solve function\n");
        if(solve(l->board,l->size,NULL) == 0){
            l->solved = 0; 
            pthread_exit(NULL);
            return NULL;
        }else {
            l-> solved = 1;
            /*printf("In depth 3 solver \n");
            //printf("No of empty %d \n",emptyCount(l->board,l->size));
            print_grid(l->size,l->board);*/
            found = 1;
            memcpy(board_glob, l->board, sizeof (int) *36* 36);
            board_solve_id = l->t_id;
            pthread_exit(NULL);
            return NULL;
        }
    }else{
        l->solved = 0;
        pthread_exit(NULL);
        return NULL;
    }
}




void *depth8Solver(void *val){
	game *l = val;
	if(check(l->board,l->row,l->col,l->val_attempt,l->size) == 1){
		l->board[l->row][l->col] = l->val_attempt;
        int r = -1,c = -1;
        int k = findEmpty(l->board,l->size,&r,&c);
        avl_opt opts;
        if(k != 0 && r != -1 && c != -1){ opts = reduceSolSpace(l->board,l->size,r,c);}
        while(k!=0 && r!=-1 && c!=-1 && opts.n==1){
        	l->board[r][c] = opts.avl_options[0];
		//printf("Fill %d on row: %d, col: %d\n",opts.avl_options[0],r,c);
		k = findEmpty(l->board,l->size,&r,&c);
		if(k==0 || r==-1 || c==-1){break;}
		opts = reduceSolSpace(l->board,l->size,r,c);
        }
        
        
        
        if(k != 0 && r != -1 && c != -1){
            opts = reduceSolSpace(l->board,l->size,r,c);
            pthread_t thirdLayer[36];
            game *gb3 = (game*) malloc(opts.n * sizeof(game)); 
            for(int i = 0; i < opts.n; i++){
                memcpy(gb3[i].board, l->board, sizeof (int) *36* 36);
                gb3[i].size = l->size;
                gb3[i].t_id = tc_glob;
                gb3[i].val_attempt = opts.avl_options[i];
                gb3[i].row = r;
                gb3[i].col = c;
                gb3[i].empty = l->empty-1;
                tc_glob++;
                //printf("depth3solver: Making thread for empty space row: %d col: %d\n",r,c);
                //printf("Total threads: %d\n",tc_glob);
                //printf("Empty spaces left %d\n", gb3[i].empty);
                if(pthread_create(&thirdLayer[i],NULL,depth9Solver, (void *)&gb3[i])){
                    printf("Error making thread %d at tc val %d \n", i,tc_glob);
                    exit(1);
                }
            }
 
            for(int i = 0; i < opts.n; i++){
                pthread_join(thirdLayer[i],NULL);
                if(gb3[i].solved){
                    //printf("Found winner at thread %d \n",i);
                    l->solved = 1;
                    pthread_exit(NULL);
                    //print_grid(game_board[k].size,game_board[k].board);
                }
            }
        }else {
            // board is filled completely
            l-> solved = 1;
            //print_grid(l->size,l->board);
            found = 1;
            memcpy(board_glob, l->board, sizeof (int) *36* 36);
            board_solve_id = l->t_id;
            pthread_exit(NULL);
            return NULL;
        }
	}else {
		l->solved = 0;
		pthread_exit(NULL);
		return NULL;
	}
}

void *depth7Solver(void *val){
	game *l = val;
	if(check(l->board,l->row,l->col,l->val_attempt,l->size) == 1){
		l->board[l->row][l->col] = l->val_attempt;
        int r = -1,c = -1;
        int k = findEmpty(l->board,l->size,&r,&c);
        avl_opt opts;
        if(k != 0 && r != -1 && c != -1){ opts = reduceSolSpace(l->board,l->size,r,c);}
        while(k!=0 && r!=-1 && c!=-1 && opts.n==1){
        	l->board[r][c] = opts.avl_options[0];
		//printf("Fill %d on row: %d, col: %d\n",opts.avl_options[0],r,c);
		k = findEmpty(l->board,l->size,&r,&c);
		if(k==0 || r==-1 || c==-1){break;}
		opts = reduceSolSpace(l->board,l->size,r,c);
        }
        
        
        
        if(k != 0 && r != -1 && c != -1){
            opts = reduceSolSpace(l->board,l->size,r,c);
            pthread_t thirdLayer[36];
            game *gb3 = (game*) malloc(opts.n * sizeof(game)); 
            for(int i = 0; i < opts.n; i++){
                memcpy(gb3[i].board, l->board, sizeof (int) *36* 36);
                gb3[i].size = l->size;
                gb3[i].t_id = tc_glob;
                gb3[i].val_attempt = opts.avl_options[i];
                gb3[i].row = r;
                gb3[i].col = c;
                gb3[i].empty = l->empty-1;
                tc_glob++;
                //printf("depth3solver: Making thread for empty space row: %d col: %d\n",r,c);
                //printf("Total threads: %d\n",tc_glob);
                //printf("Empty spaces left %d\n", gb3[i].empty);
                if(pthread_create(&thirdLayer[i],NULL,depth8Solver, (void *)&gb3[i])){
                    printf("Error making thread %d at tc val %d \n", i,tc_glob);
                    exit(1);
                }
            }
 
            for(int i = 0; i < opts.n; i++){
                pthread_join(thirdLayer[i],NULL);
                if(gb3[i].solved){
                    //printf("Found winner at thread %d \n",i);
                    l->solved = 1;
                    pthread_exit(NULL);
                    //print_grid(game_board[k].size,game_board[k].board);
                }
            }
        }else {
            // board is filled completely
            l-> solved = 1;
            //print_grid(l->size,l->board);
            found = 1;
            memcpy(board_glob, l->board, sizeof (int) *36* 36);
            board_solve_id = l->t_id;
            pthread_exit(NULL);
            return NULL;
        }
	}else {
		l->solved = 0;
		pthread_exit(NULL);
		return NULL;
	}
}

void *depth6Solver(void *val){
	game *l = val;
	if(check(l->board,l->row,l->col,l->val_attempt,l->size) == 1){
		l->board[l->row][l->col] = l->val_attempt;
        int r = -1,c = -1;
        int k = findEmpty(l->board,l->size,&r,&c);
        avl_opt opts;
        if(k != 0 && r != -1 && c != -1){ opts = reduceSolSpace(l->board,l->size,r,c);}
        while(k!=0 && r!=-1 && c!=-1 && opts.n==1){
        	l->board[r][c] = opts.avl_options[0];
		//printf("Fill %d on row: %d, col: %d\n",opts.avl_options[0],r,c);
		k = findEmpty(l->board,l->size,&r,&c);
		if(k==0 || r==-1 || c==-1){break;}
		opts = reduceSolSpace(l->board,l->size,r,c);
        }
        
        
        
        if(k != 0 && r != -1 && c != -1){
            opts = reduceSolSpace(l->board,l->size,r,c);
            pthread_t thirdLayer[36];
            game *gb3 = (game*) malloc(opts.n * sizeof(game)); 
            for(int i = 0; i < opts.n; i++){
                memcpy(gb3[i].board, l->board, sizeof (int) *36* 36);
                gb3[i].size = l->size;
                gb3[i].t_id = tc_glob;
                gb3[i].val_attempt = opts.avl_options[i];
                gb3[i].row = r;
                gb3[i].col = c;
                gb3[i].empty = l->empty-1;
                tc_glob++;
                //printf("depth3solver: Making thread for empty space row: %d col: %d\n",r,c);
                //printf("Total threads: %d\n",tc_glob);
                //printf("Empty spaces left %d\n", gb3[i].empty);
                if(pthread_create(&thirdLayer[i],NULL,depth7Solver, (void *)&gb3[i])){
                    printf("Error making thread %d at tc val %d \n", i,tc_glob);
                    exit(1);
                }
            }
 
            for(int i = 0; i < opts.n; i++){
                pthread_join(thirdLayer[i],NULL);
                if(gb3[i].solved){
                    //printf("Found winner at thread %d \n",i);
                    l->solved = 1;
                    pthread_exit(NULL);
                    //print_grid(game_board[k].size,game_board[k].board);
                }
            }
        }else {
            // board is filled completely
            l-> solved = 1;
            //print_grid(l->size,l->board);
            found = 1;
            memcpy(board_glob, l->board, sizeof (int) *36* 36);
            board_solve_id = l->t_id;
            pthread_exit(NULL);
            return NULL;
        }
	}else {
		l->solved = 0;
		pthread_exit(NULL);
		return NULL;
	}
}

void *depth5Solver(void *val){
	game *l = val;
	if(check(l->board,l->row,l->col,l->val_attempt,l->size) == 1){
		l->board[l->row][l->col] = l->val_attempt;
        int r = -1,c = -1;
        int k = findEmpty(l->board,l->size,&r,&c);
        avl_opt opts;
        if(k != 0 && r != -1 && c != -1){ opts = reduceSolSpace(l->board,l->size,r,c);}
        while(k!=0 && r!=-1 && c!=-1 && opts.n==1){
        	l->board[r][c] = opts.avl_options[0];
		//printf("Fill %d on row: %d, col: %d\n",opts.avl_options[0],r,c);
		k = findEmpty(l->board,l->size,&r,&c);
		if(k==0 || r==-1 || c==-1){break;}
		opts = reduceSolSpace(l->board,l->size,r,c);
        }
        
        
        
        if(k != 0 && r != -1 && c != -1){
            opts = reduceSolSpace(l->board,l->size,r,c);
            pthread_t thirdLayer[36];
            game *gb3 = (game*) malloc(opts.n * sizeof(game)); 
            for(int i = 0; i < opts.n; i++){
                memcpy(gb3[i].board, l->board, sizeof (int) *36* 36);
                gb3[i].size = l->size;
                gb3[i].t_id = tc_glob;
                gb3[i].val_attempt = opts.avl_options[i];
                gb3[i].row = r;
                gb3[i].col = c;
                gb3[i].empty = l->empty-1;
                tc_glob++;
                //printf("depth3solver: Making thread for empty space row: %d col: %d\n",r,c);
                //printf("Total threads: %d\n",tc_glob);
                //printf("Empty spaces left %d\n", gb3[i].empty);
                if(pthread_create(&thirdLayer[i],NULL,depth6Solver, (void *)&gb3[i])){
                    printf("Error making thread %d at tc val %d \n", i,tc_glob);
                    exit(1);
                }
            }
 
            for(int i = 0; i < opts.n; i++){
                pthread_join(thirdLayer[i],NULL);
                if(gb3[i].solved){
                    //printf("Found winner at thread %d \n",i);
                    l->solved = 1;
                    pthread_exit(NULL);
                    //print_grid(game_board[k].size,game_board[k].board);
                }
            }
        }else {
            // board is filled completely
            l-> solved = 1;
            //print_grid(l->size,l->board);
            found = 1;
            memcpy(board_glob, l->board, sizeof (int) *36* 36);
            board_solve_id = l->t_id;
            pthread_exit(NULL);
            return NULL;
        }
	}else {
		l->solved = 0;
		pthread_exit(NULL);
		return NULL;
	}
}

void *depth4Solver(void *val){
	game *l = val;
	if(check(l->board,l->row,l->col,l->val_attempt,l->size) == 1){
		l->board[l->row][l->col] = l->val_attempt;
        int r = -1,c = -1;
        int k = findEmpty(l->board,l->size,&r,&c);
        avl_opt opts;
        if(k != 0 && r != -1 && c != -1){ opts = reduceSolSpace(l->board,l->size,r,c);}
        while(k!=0 && r!=-1 && c!=-1 && opts.n==1){
        	l->board[r][c] = opts.avl_options[0];
		//printf("Fill %d on row: %d, col: %d\n",opts.avl_options[0],r,c);
		k = findEmpty(l->board,l->size,&r,&c);
		if(k==0 || r==-1 || c==-1){break;}
		opts = reduceSolSpace(l->board,l->size,r,c);
        }
        
        
        
        if(k != 0 && r != -1 && c != -1){
            opts = reduceSolSpace(l->board,l->size,r,c);
            pthread_t thirdLayer[36];
            game *gb3 = (game*) malloc(opts.n * sizeof(game)); 
            for(int i = 0; i < opts.n; i++){
                memcpy(gb3[i].board, l->board, sizeof (int) *36* 36);
                gb3[i].size = l->size;
                gb3[i].t_id = tc_glob;
                gb3[i].val_attempt = opts.avl_options[i];
                gb3[i].row = r;
                gb3[i].col = c;
                gb3[i].empty = l->empty-1;
                tc_glob++;
                //printf("depth3solver: Making thread for empty space row: %d col: %d\n",r,c);
                //printf("Total threads: %d\n",tc_glob);
                //printf("Empty spaces left %d\n", gb3[i].empty);
                if(pthread_create(&thirdLayer[i],NULL,depth5Solver, (void *)&gb3[i])){
                    printf("Error making thread %d at tc val %d \n", i,tc_glob);
                    exit(1);
                }
            }
 
            for(int i = 0; i < opts.n; i++){
                pthread_join(thirdLayer[i],NULL);
                if(gb3[i].solved){
                    //printf("Found winner at thread %d \n",i);
                    l->solved = 1;
                    pthread_exit(NULL);
                    //print_grid(game_board[k].size,game_board[k].board);
                }
            }
        }else {
            // board is filled completely
            l-> solved = 1;
           // print_grid(l->size,l->board);
            found = 1;
            memcpy(board_glob, l->board, sizeof (int) *36* 36);
            board_solve_id = l->t_id;	
            pthread_exit(NULL);
            return NULL;
        }
	}else {
		l->solved = 0;
		pthread_exit(NULL);
		return NULL;
	}
}

void *depth3Solver(void *val){
	game *l = val;
	if(check(l->board,l->row,l->col,l->val_attempt,l->size) == 1){
		l->board[l->row][l->col] = l->val_attempt;
        int r = -1,c = -1;
        int k = findEmpty(l->board,l->size,&r,&c);
        avl_opt opts;
        if(k != 0 && r != -1 && c != -1){ opts = reduceSolSpace(l->board,l->size,r,c);}
        while(k!=0 && r!=-1 && c!=-1 && opts.n==1){
        	l->board[r][c] = opts.avl_options[0];
		//printf("Fill %d on row: %d, col: %d\n",opts.avl_options[0],r,c);
		k = findEmpty(l->board,l->size,&r,&c);
		if(k==0 || r==-1 || c==-1){break;}
		opts = reduceSolSpace(l->board,l->size,r,c);
        }
        
        
        
        if(k != 0 && r != -1 && c != -1){
            opts = reduceSolSpace(l->board,l->size,r,c);
            pthread_t thirdLayer[36];
            game *gb3 = (game*) malloc(opts.n * sizeof(game)); 
            for(int i = 0; i < opts.n; i++){
                memcpy(gb3[i].board, l->board, sizeof (int) *36* 36);
                gb3[i].size = l->size;
                gb3[i].t_id = tc_glob;
                gb3[i].val_attempt = opts.avl_options[i];
                gb3[i].row = r;
                gb3[i].col = c;
                gb3[i].empty = l->empty-1;
                tc_glob++;
                //printf("depth3solver: Making thread for empty space row: %d col: %d\n",r,c);
                //printf("Total threads: %d\n",tc_glob);
                //printf("Empty spaces left %d\n", gb3[i].empty);
                if(pthread_create(&thirdLayer[i],NULL,depth4Solver, (void *)&gb3[i])){
                    printf("Error making thread %d at tc val %d \n", i,tc_glob);
                    exit(1);
                }
            }
 
            for(int i = 0; i < opts.n; i++){
                pthread_join(thirdLayer[i],NULL);
                if(gb3[i].solved){
                    //printf("Found winner at thread %d \n",i);
                    l->solved = 1;
                    pthread_exit(NULL);
                    //print_grid(game_board[k].size,game_board[k].board);
                }
            }
        }else {
            // board is filled completely
            l-> solved = 1;
            //print_grid(l->size,l->board);
            found = 1;
            memcpy(board_glob, l->board, sizeof (int) *36* 36);
            board_solve_id = l->t_id;
            pthread_exit(NULL);
            return NULL;
        }
	}else {
		l->solved = 0;
		pthread_exit(NULL);
		return NULL;
	}
}
 
void *depth2Solver(void *val){
	game *l = val;
	if(check(l->board,l->row,l->col,l->val_attempt,l->size) == 1){
		l->board[l->row][l->col] = l->val_attempt;
		l->empty--;
        int r = -1,c = -1;
        int k = findEmpty(l->board,l->size,&r,&c);
        avl_opt opts;
        if(k != 0 && r != -1 && c != -1){ opts = reduceSolSpace(l->board,l->size,r,c);}
        while(k!=0 && r!=-1 && c!=-1 && opts.n==1){
        	l->board[r][c] = opts.avl_options[0];
		//printf("Fill %d on row: %d, col: %d\n",opts.avl_options[0],r,c);
		k = findEmpty(l->board,l->size,&r,&c);
		if(k==0 || r==-1 || c==-1){break;}
		opts = reduceSolSpace(l->board,l->size,r,c);
        }
        
        
        
        if(k != 0 && r != -1 && c != -1){
            opts = reduceSolSpace(l->board,l->size,r,c);
            pthread_t thirdLayer[36];
            game *gb3 = (game*) malloc(opts.n * sizeof(game)); 
            for(int i = 0; i < opts.n; i++){
                memcpy(gb3[i].board, l->board, sizeof (int) *36* 36);
                gb3[i].size = l->size;
                gb3[i].t_id = tc_glob;
                gb3[i].val_attempt = opts.avl_options[i];
                gb3[i].row = r;
                gb3[i].col = c;
                gb3[i].empty = l->empty;
                tc_glob++;
                //printf("depth2solver: Making thread for empty space row: %d col: %d\n",r,c);
                //printf("Total threads: %d\n",tc_glob);
                //printf("Empty spaces left %d\n", l->empty);
                if(pthread_create(&thirdLayer[i],NULL,depth3Solver, (void *)&gb3[i])){
                    printf("Error making thread %d at tc val %d \n", i,tc_glob);
                    exit(1);
                }
            }
 
            for(int i = 0; i < opts.n; i++){
                pthread_join(thirdLayer[i],NULL);
                if(gb3[i].solved){
                    //printf("Found winner at thread %d \n",i);
                    l->solved = 1;
                    pthread_exit(NULL);
                    //print_grid(game_board[k].size,game_board[k].board);
                }
            }
        }else {
            // board is filled completely
            l-> solved = 1;
            //print_grid(l->size,l->board);
            found = 1;
            memcpy(board_glob, l->board, sizeof (int) *36* 36);
            board_solve_id = l->t_id;
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
 
		l->board[l->row][l->col] = l->val_attempt;
		l->empty--;
		int r = -1,c = -1; avl_opt opts;
		int k = findEmpty(l->board,l->size,&r,&c);
		if(k != 0 && r != -1 && c != -1){opts = reduceSolSpace(l->board,l->size,r,c);}
		while(opts.n == 1 && k!=0 && r!=-1 && c!=-1){
			l->board[r][c] = opts.avl_options[0];
			//printf("Fill %d on row: %d, col: %d\n",opts.avl_options[0],r,c);
			k = findEmpty(l->board,l->size,&r,&c);
			if(k==0 || r==-1 || c==-1){break;}
			opts = reduceSolSpace(l->board,l->size,r,c);
		}
		if(k != 0 && r != -1 && c != -1){
			opts = reduceSolSpace(l->board,l->size,r,c);
			pthread_t secondLayer[36];
			game *gb2 = (game*) malloc(opts.n * sizeof(game)); 
			for(int i = 0; i < opts.n; i++){
				memcpy(gb2[i].board, l->board, sizeof (int) *36* 36);
				gb2[i].size = l->size;
				gb2[i].t_id = tc_glob;
				gb2[i].val_attempt = opts.avl_options[i];
				gb2[i].row = r;
				gb2[i].col = c;
				gb2[i].empty = l->empty;
				tc_glob++;
				//printf("Multithread solver: Making thread for empty space row: %d col: %d\n",r,c);
				//printf("Total threads: %d\n",tc_glob);
				//printf("Empty spaces left %d\n", l->empty);
				if(pthread_create(&secondLayer[i],NULL,depth2Solver, (void *)&gb2[i])){
					printf("Error making thread %d at tc val %d \n", k,tc_glob);
					exit(1);
				}
			}
 
			for(int i = 0; i < opts.n; i++){
				pthread_join(secondLayer[i],NULL);
				if(gb2[i].solved){
			    		//printf("Found winner at thread %d \n",i);
			    		l->solved = 1;
		      				pthread_exit(NULL);
			    		//print_grid(game_board[k].size,game_board[k].board);
				    }
			}
		}else {
			// board is filled completely
			l-> solved = 1;
			printf("Printing in multiThreadSolver \n");
			found = 1;
            		memcpy(board_glob, l->board, sizeof (int) *36* 36);
            		board_solve_id = l->t_id;
			//print_grid(l->size,l->board);
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
	while(reduceSpace.n == 1 && k!=0 && r!=-1 && c!=-1){
		board[r][c] = reduceSpace.avl_options[0];
		//printf("Fill %d on row: %d, col: %d\n",reduceSpace.avl_options[0],r,c);
		k = findEmpty(board,s,&r,&c);
		if(k==0 || r==-1 || c==-1){
			//printf("No spawn needed \n");
			//printf("Empty on board %d \n",emptyCount(board,s));
			//print_grid(s,board);
			reduceSpace.n = 0;
			break;
		}
		reduceSpace = reduceSolSpace(board,s,r,c);
	}
	if(k != 0 && r != -1 && c != -1){
	for(int i = 0; i < reduceSpace.n; i++){
		memcpy(game_board[i].board, board, sizeof (int) *36* 36);
		game_board[i].size = s;
		game_board[i].t_id = i;
		game_board[i].val_attempt = reduceSpace.avl_options[i];
	}

	//printf(" In the first step going to make %d threads \n ", reduceSpace.n);
	//printf("Possible values for this position are \n");
	/*for(int i = 0; i < reduceSpace.n; i++){
		printf("%d ",reduceSpace.avl_options[i]);
	}
	printf("\n");*/
	for(int i = 0; i < reduceSpace.n; i++){
		game_board[i].row = r;
		game_board[i].col = c;
		game_board[i].empty = ec;
		tc_glob++;
		//printf("Thread Spawner: Making thread for empty space row: %d col: %d\n",r,c);
		//printf("Total threads: %d\n",tc_glob);
		//printf("Empty spaces left %d\n", ec);
		if(pthread_create(&threads[i],NULL,multiThreadSolver, (void *)&game_board[i])){
		    printf("Error making thread %d at tc val %d \n", i,tc_glob);
		    exit(1);
		}

	}

	//printf("Value of tc_glob after spawning s threads %d \n", tc_glob);
	for(int i = 0; i < reduceSpace.n; i++){
		pthread_join(threads[i],NULL);
		if(game_board[i].solved){
		    //printf("Found winner at thread %d \n",i);
		    return 0;
		    //print_grid(game_board[k].size,game_board[k].board);
		}
	}
	}
	else {
		// board is filled completely
		printf("No spawn needed \n");
		print_grid(s,board);
		return 0;
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
	printf("No of empty blocks in the grid is %d \n", ec);
 
	if(thread_spawner(grid,size) == -1){
        printf("Error no empty spaces in the grid \n");
        print_grid(size,grid);
    }
    	if(found == 1 && board_solve_id != -1){
        printf("Empty on board %d \n",emptyCount(board_glob,size));
        printf("Board solve id is %d \n",board_solve_id);
        print_grid(size,board_glob);
	}else {
		printf("Some error occured, given puzzle cannot be solved \n");
	}
	printf(" %d threads made \n",tc_glob);
	return 0;
}
