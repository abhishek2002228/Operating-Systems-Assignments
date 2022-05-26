# Operating Systems (CSF372) Assignments

## Group Members:

<ul>
<li>Sukrit Kumar (2019AAPS0231H)</li>
<li>Abhishek Revinipati (2019A3PS0415H)</li>
<li>Achyut Bajpai (2019A8PS0499H)</li>
<li>Parth Chauhan (2019A3PS0414H)</li>
<li>Shubham Priyank (2019AAPS0467H)</li>
<li>Shubham Singla (2019A3PS0392H)</li>
<li>Jahnavi Gurdasani (2019AAPS0556H)</li>
<li>Dhruv Makwana (2019A3PS0381H)</li>
</ul>

# Assignment 1 Multithreaded sudoku solver 
Doesn't work for 36x36 grid sizes

Compile `gcc -pthread sudoku.c -o sudoku.out -lm`

Usage: `./sudoku.out grid_size grid_name`

# Assignment 2 Approximate scheduler RR and Preemptive Priority 
Very approximate scheduler. Priority is inverse of runtime, only runs on 2 processes, but can be extended to more by using a queue or tree structure.

Compile all : `make all`

Usage : `main_sched.out or main_sched_priority.out filename no of ints to sum`

Eg : `./main_sched.out ./TestFiles/hundred.txt 100`
