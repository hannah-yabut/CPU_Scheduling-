# CMPT 360, Assignment 2 - CPU Scheduling Simulator 
# Hannah Yabut and Ismael Robleh 
# Date: May 24, 2026 


# Academic Integrity 
" I certify that this submission represents entirely my own work."

# Overview 
This assignment simulates CPU scheduling using two scheduling ways, First Come First Serve (FCFS) and Round Robin (RR). The program reads a workload file with process ID's, arrival times, and outputs execution timeline, scheduling metrics and system statistics. 

# Build 
To compile the script use: "make all" 

# Run 
To run manual testing use: "make run" 
To check memory leaks use: "make valgrind" or "make vg" 
Example Runs: 
./sched --policy=FCFS --in=W1.txt
./sched --policy=RR --quantum=1 --in=W1.txt

# Solution Logic 
- Reads workload files line by line with getline() 
- Created queue.h, a file that contains a queue class used throughout the program 
- Dynamically resizes arrays using realloc() 
- Ignores comments and empty lines 
- Followed the skeleton outline of the file sched.c for the main functions used in the program 

# Status
The program works as intended throughout all the differnet test cases 

# Assumptions / Notes 
- Assumed that memory allocation of newly arriving jobs would be dynamid rather than static. This is reflected in the design choice using realloc()

