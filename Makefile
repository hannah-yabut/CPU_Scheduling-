#Name: Hannah Yabut & Ismael Robleh
#Student ID: 3131432 & 3149556
#May 10, 2026 
#Compile, run, and test userclean program for memory leaks or any potential errors

#Compiler 
CC := gcc-13

#Compiler flags
CFLAGS := -std=c11 -Wall -Wextra -pedantic -O2 -g

#Valgrind command
VALGRIND ?= valgrind

#Valgrind flags for detailed and thorough memory leak detection
VGFLAGS  ?= --leak-check=full --show-leak-kinds=all --track-origins=yes --errors-for-leak-kinds=all --error-exitcode=1

#Declare the following targets as makefile commands instead of file targets
.PHONY: all clean valgrind vg run_FCFS run_RR

#Build userclean executable
all: sched

#Link object file to into executable
sched: sched.o 
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

#Compile source file into object file
sched.o: sched.c sched.h queue.h
	$(CC) $(CFLAGS) -c $<

QUANTUM ?= 2
FILE	?= workload.txt

#Run executable automatically
run_FCFS: sched
	./sched --policy=FCFS --in=$(FILE)

run_RR: sched
	./sched --policy=RR --quantum=$(QUANTUM) --in=$(FILE)

POLICY ?= RR

#Run memory leak and error detection with Valgrind by using workload.txt file as input
valgrind: sched
	$(VALGRIND) $(VGFLAGS) ./sched --policy=$(POLICY) --quantum=$(QUANTUM) --in=$(FILE)

#Alias for Valgrind
vg: valgrind

#Remove all files from directory (specifically executable and object file)
clean:
	rm -f sched sched.o