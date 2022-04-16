all: main_sched.c 
	gcc -g -Wall -pthread -o main_sched.out main_sched.c -lm
	gcc -g -Wall -pthread -o p1_sched.out p1_sched.c -lm
	gcc -g -Wall -pthread -o p2_sched.out p2_sched.c -lm

clean: 
	rm -rf *.o
	rm -rf *.out