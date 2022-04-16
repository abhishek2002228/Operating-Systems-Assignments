all: main_sched.c 
	gcc -g -Wall -pthread -o main_sched main_sched.c -lm
	gcc -g -Wall -pthread -o p1_sched p1_sched.c -lm
	gcc -g -Wall -pthread -o p2_sched p2_sched.c -lm

clean: 
	$(RM) myprog
