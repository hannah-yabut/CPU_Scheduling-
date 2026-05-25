/*
 * Student Name: Hannah Yabut and Ismael Robleh 
 * Student ID:  3131432 & 3149556
 * Date:         May 24, 2026 
 * File:         sched.c 
 * Description:  Main file where the functions to simulate a CPU are. This is done through two differeny ways, FCFS and RR. 
 */


#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "sched.h"
#include "queue.h"
#include "stdbool.h"

/*
Description: sorts array of jobs in ascending order based on PID using insertion sort 
Parameteres: array, n 
Return: None 
*/
void insertion_sort_pid(job_t* array, int n) {
    for (int i = 1; i < n; ++i)
    {
        int key = array[i].pid;
        int temp_arrival = array[i].arrival;
        int temp_cpu_time = array[i].cpu_time;
        int j = i - 1;
       
        //Keep replacing the left most elements of the key until j is out of bounds or the elements are already smaller or equal to the key
        while (j >= 0 && array[j].pid > key)
        {
            //Replace values
            array[j + 1].pid = array[j].pid;
            array[j + 1].arrival = array[j].arrival;
            array[j + 1].cpu_time = array[j].cpu_time;
            j--;
        }
        //Set the temporary values back to the replaced values to avoid corruption
        array[j + 1].pid = key;
        array[j + 1].arrival = temp_arrival;
        array[j + 1].cpu_time = temp_cpu_time;
    }
}

/*
Description: sorts array of jobs in ascending order based on arrival time 
Parameteres: array, n
Return: None 
*/
void insertion_sort_arrival(job_t* array, int n) {
    for (int i = 1; i < n; ++i)
    {
        int key = array[i].arrival;
        int temp_pid = array[i].pid;
        int temp_cpu_time = array[i].cpu_time;
        int j = i - 1;

        //Keep replacing the left most elements of the key until j is out of bounds or the elements are already smaller or equal to the key
        while (j >= 0 && array[j].arrival > key)
        {
            //Replace values
            array[j + 1].pid = array[j].pid;
            array[j + 1].arrival = array[j].arrival;
            array[j + 1].cpu_time = array[j].cpu_time;
            j--;
        }
        //Set the temporary values back to the replaced values to avoid corruption
        array[j + 1].arrival = key;
        array[j + 1].pid = temp_pid;
        array[j + 1].cpu_time = temp_cpu_time;
    }
}

/*
Description: Prints the CPU timeline 
Parameteres: n 
Return: None 
*/
void print_time_array(int n)
{   
    //Reserve 4 spaces with a left-alignment
    printf("%-*s", 4, "time:");
    for (int i = 0; i < n; i++)
    {
        printf(" %2d", i);
    }
    printf("\n");
}

/*
Description: Prints the run cpu ticks 
Parameteres: array, n 
Return: None 
*/
void print_run_array(int* array, int n)
{   
    //Reserve 4 spaces with a left-alignment
    printf("%-*s", 4, "run :");
    for (int i = 0; i < n; i++)
    {
        //Non idling cpu ticks
        if (array[i] != -1)
        {
            printf(" %2d", array[i]);
        }
        else
        {
            //Idle cpu ticks
            printf(" %3c", '-');
        }
    }
    printf("\n");
}

/*
Description: Prints the detailed job details as per the specifications in the output 
Parameteres:  job_details, n 
Return: None 
*/
void print_job_details(job_details_t* job_details, int n)
{
    for (int i = 0; i < n; i++)
    {  
        printf("P%d: first run=%d completion=%d TAT=%d RESP=%d \n", job_details[i].pid, 
            job_details[i].first_run, 
            job_details[i].completion, 
            job_details[i].tat, 
            job_details[i].resp);
    }
}

/*
Description: Prints the CPU system metrics 
Parameteres: out 
Return: None 
*/
void print_job_systems(sim_metrics_t out)
{   
    printf("System: ctx_switches=%d, avgTAT=%.3f, avgRESP=%.3f \n", out.context_switches, out.avg_tat, out.avg_resp);
}

/*
Description: Calculates total turnaround time of all jobs 
Parameteres: job_details, n 
Return: total 
*/
int sum_tat(job_details_t* job_details, int n)
{
    int total = 0;
    for (int i = 0; i < n; i++)
    {
        total += job_details[i].tat;
    }
    return total;
}
/*
Description: calculates total response time of all jobs 
Parameteres: job_details, n 
Return: total 
*/
int sum_resp(job_details_t* job_details, int n)
{
    int total = 0;
    for (int i = 0; i < n; i++)
    {
        total += job_details[i].resp;
    }
    return total;
}

/*
Description: Prints the usage message if the user input is incorrect 
Parameteres: prog 
Return: None 
*/
static void usage(const char* prog){
    /* one-line usage only (per spec) */
    fprintf(stderr, "Usage: %s --policy=FCFS|RR [--quantum=N] --in=FILE\n", prog);
}

/* TODO: strict CLI
   Required: --policy=FCFS|RR, --in=FILE
   Optional: --quantum=N (required iff RR; N>0)
   On error: print usage() and return non-zero.
   Description: parses and validates command line args to ensure requireed arguments are present and validates policy and quantums 
   Parameteres: argc, argv, cfg, in_path 
   Return: 0 or 1 
*/


int parse_args(int argc, char** argv, sim_cfg_t* cfg, const char** in_path)
{
    (void)argc; (void)argv; (void)cfg; (void)in_path;
    
    int seen_policy = 0; 
    int seen_in = 0; 
    int seen_quantum = 0; 
    // setting default vals before arguments 
    cfg->quantum = 0; 
    cfg->policy = 0; 
    *in_path = NULL; 
    // loop through command line args while skipping program name 
    for (int i = 1; i < argc; i++)
    {   
        if (strncmp(argv[i], "--policy=", 9) == 0)
        {
            const char* val = argv[i] + 9; // getting value after "--policy="
            if (strcmp(val, "FCFS") == 0)
            {
                cfg->policy = POL_FCFS; 
            }
            else if (strcmp(val, "RR") == 0)
            {
                cfg->policy = POL_RR;  
            }
            else 
            {
                usage(argv[0]); // not one of the strict CLI return usage message 
                return 1; 
            }
            seen_policy = 1; // change flag to seen 
        }
        else if (strncmp(argv[i], "--quantum=", 10) == 0)
        {
            // convert quantum value from str to a value 
            char* end = NULL;
            long q = strtol(argv[i] + 10, &end, 10);
            // reject empty, invalid, or negative quantum values 
            if (*end != '\0' || q <= 0) 
            {
                usage(argv[0]); 
                return 1; 
            }
            cfg->quantum = (int)q; 
            seen_quantum = 1; 
        }
        else if (strncmp(argv[i], "--in=", 5) == 0)
        {
            // store input file 
            *in_path = argv[i] + 5; 
            if (**in_path == '\0') // if the file input line is empty
            {
                usage(argv[0]); // return usage message 
                return 1; 
            }
            seen_in = 1; // change in flag 
        }
        else
        {
            usage(argv[0]); // if unknown args, return usage message 
            return 1; 
        }
    }
    if (!seen_policy || !seen_in) // policy and input file needed 
    {
         usage(argv[0]);
            return 1; 
    }
    if (cfg->policy == POL_RR && !seen_quantum) // RR needs quantum value 
    {
        usage(argv[0]);
        return 1;
    }
    if (cfg->policy == POL_FCFS && seen_quantum) // FCFS should not have quantum value 
    {
        usage(argv[0]);
        return 1;
    }

    return 0;
}

/* TODO: read "PID ARRIVAL CPU_TIME", ignore lines starting with '#', validate:
   pid>=0, arrival>=0, cpu>0. Sort by (arrival, pid). Return 0 on success.
   Description: reads from input file while ignoring comments and empty lines. Validates process values, 
   dynamically allocates memory for jobs, and sorts jobs by arrival time and PID 
   Parameteres: path, jobs, n 
   Return: 0 or 1 
*/
int load_workload(const char* path, job_t** jobs, int* n){
    (void)path; (void)jobs; (void)n;

    //Open file
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    char* line = NULL;
    size_t length = 0;
    ssize_t num_read;

    int pid, arrival, cpu_time;
    int line_num = 0;

    int workload_status = 0; //0 = success, 1 = unsuccesful

    int capacity = 10;
    *jobs = (job_t*)malloc(capacity * sizeof(job_t));
    if (*jobs == NULL)
    {
        fprintf(stderr, "Memory allocation failed in workload!");
        fclose(file);
        file = NULL;
        return 1;
    }

    //Read through the file line by line 
    while ((num_read = getline(&line, &length, file)) != EOF) {

        //Skip '#' and newline characters if they are present
        if (line[0] == '#' || line[0] == '\n') continue;

        //Replace newlines characters with the null operator 
        if (line[num_read - 1] == '\n')
        {
            line[num_read - 1] = '\0';
        }

        char junk;
        //Scan each line for exactly 3 integers and return the number of matched characters
        int matched = sscanf(line, "%d %d %d %s", &pid, &arrival, &cpu_time, &junk);

        //If the format is correct (exactly 3 integers are present on each line)
        if (matched == 3)
        {
            //Validate the variables
            if (pid >= 0 && arrival >= 0 && cpu_time > 0)
            {
                //Check if there are more entries than the currently allocated capacity (If true allocate more space)
                if (line_num >= capacity) {
                    capacity *= 2;
                    // used size of pointer instead of struct 
                    job_t* new_jobs_array = realloc(*jobs, capacity * sizeof(job_t)); 

                    if (new_jobs_array == NULL)
                    {
                        fprintf(stderr, "Memory reallocation failed in workload!");
                        free(*jobs);
                        free(line);
                        workload_status = 1;
                        break;
                    }
                    *jobs = new_jobs_array;
                }
                //Populate the jobs struct with the information on each line
                (*jobs)[line_num].pid = pid;
                (*jobs)[line_num].arrival = arrival;
                (*jobs)[line_num].cpu_time = cpu_time;
                line_num++;
            }
            else
            {
                //If simulation variables are incorrect (pid, arrival, or cpu time)
                fprintf(stderr, "Incorrect values for workloads!\n");
                workload_status = 1;
                break;
            }
        }
        else
        {
            //If the entries of the table is incorrect (not exactly 3 integers are present)
            fprintf(stderr, "Incorrect formatting of the table!\n");
            workload_status = 1;
            break;
        }
    }

    //If there are no entries in the table
    if (line_num == 0)
    {
        fprintf(stderr, "No jobs present in file!");

        fclose(file);
        file = NULL;
        return 1;
    }

    //Iterate through each populated jobs and make sure that they have all unique PID's
    int j = 1;
    for (int i = 0; i < line_num; i++)
    {
        for (; j < line_num; j++)
        {
            if ((*jobs)[i].pid == (*jobs)[j].pid)
            {
                printf("Processes in workload must have unique PID's!");
                workload_status = 1;
                break;
            }
        }
        j++;
    }

    //Sort by PID
    insertion_sort_pid((*jobs), line_num);

    //Sort by arrival time
    insertion_sort_arrival((*jobs), line_num);

    //Set the number of entries to the number of lines
    *n = line_num;

    free(line); // frees memory 
    line = NULL; 

    fclose(file);
    file = NULL;

    //Return the status of the workload function
    return workload_status;
}

/* TODO: discrete-time CPU simulation with FIFO ready queue.
   FCFS: never preempt. RR: quantum countdown; when it hits 0 and job not finished, enqueue at tail.
   Track: first run (first time scheduled) and completion (t+1 on finish).
   Build timeline run[t]=pid or -1 (idle). After finishing:
     - ctx = count PID->PID changes (ignore idle)
     - TAT = completion-arrival; RESP = first run-arrival
   Print exactly:
     time: 0 1 2 ... T-1
     run : <pid or -> per tick
     Pk: first run=... completion=... TAT=... RESP=...
     System: ctx_switches=..., avgTAT=..., avgRESP=...
     Description: simulates CPU scheduling using FCFS or RR. 
     Parameteres: jobs, n, cfg, out 
     Return: 0 or 1 
*/
int simulate(const job_t* jobs, int n, const sim_cfg_t* cfg, sim_metrics_t* out){
    (void)jobs; (void)n; (void)cfg; (void)out;

    int capacity = 10;
    int* cpu_runtime_array = (int*)malloc(capacity * sizeof(int));
    if (cpu_runtime_array == NULL)
    {
        fprintf(stderr, "Memory allocation failed in simulate!");
        return 1;
    }

    job_details_t* job_details = (job_details_t*)malloc(n * sizeof(job_details_t));

    if (job_details == NULL)
    {
        fprintf(stderr, "Memory allocation failed in simulate!");
        return 1;
    }
    
    //Initialize job details parameters
    for (int i = 0; i < n; i++) {
        job_details[i].pid = jobs[i].pid;
        job_details[i].first_run = -1;
        job_details[i].completion = 0;
        job_details[i].tat = 0;
        job_details[i].resp = 0;
        job_details[i].set_first_run = false;
    }
    //FCFS Implementation
    if (cfg->policy == POL_FCFS)
    {
        Queue queue;

        initialize_queue(&queue);
        int index = 0;
        int curr_process_runtime = 0;
        int cpu_tick = 0;
        int num_jobs_finished = 0;
        (*out).context_switches = 0;

        //Loop until all of the jobs have been completed
        while (num_jobs_finished < n)
        {
            //Allocate more space when capacity is full
            if (cpu_tick >= capacity)
            {
                capacity *= 2;
                int* new_cpu_runtime_array = realloc(cpu_runtime_array, capacity * sizeof(int));

                if (new_cpu_runtime_array == NULL)
                {
                    fprintf(stderr, "Memory reallocation failed in FCFS!");
                    free(cpu_runtime_array);
                    cpu_runtime_array = NULL;
                    return 1;
                }
                cpu_runtime_array = new_cpu_runtime_array;
            }

            //If the current process has finished
            if (curr_process_runtime == jobs[index].cpu_time)
            {
                job_details[index].completion = cpu_tick;
                job_details[index].tat = job_details[index].completion - jobs[index].arrival;
                dequeue(&queue);
                curr_process_runtime = 0;
                index++;
                num_jobs_finished++;
                (*out).context_switches++;
            }

            //Iterate through all the jobs and check if they aren't already in the queue and enqueue them if they have arrived
            for (int i = 0; i < n; i++)
            {
                if (!is_in_queue(&queue, jobs[i].pid) && jobs[i].arrival == cpu_tick)
                {
                    enqueue(&queue, jobs[i].pid);
                }
            }

            //Check if there is at least a process running
            if (!is_empty(&queue))
            {
                // checks if process has never been scheduled and reports 1st cpu time 
                if (job_details[index].first_run == -1)
                {
                    job_details[index].pid = jobs[index].pid;
                    job_details[index].first_run = cpu_tick;
                    job_details[index].set_first_run = true;

                    job_details[index].resp = job_details[index].first_run - jobs[index].arrival;
                }
                curr_process_runtime += 1;
                cpu_runtime_array[cpu_tick] = jobs[index].pid;
            }
            else
            {
                //There are no process running currently (CPU is idling)
                cpu_runtime_array[cpu_tick] = -1;
            }
            cpu_tick++;
        }

        //Calculate avg metrics
        (*out).avg_tat = (double)sum_tat(job_details, n) / n;
        (*out).avg_resp = (double)sum_resp(job_details, n) / n;
        (*out).context_switches--;

        print_time_array(cpu_tick - 1);
        print_run_array(cpu_runtime_array, cpu_tick - 1);
        print_job_details(job_details, n);

        free(cpu_runtime_array);
        cpu_runtime_array = NULL;

        free(job_details);
        job_details = NULL; 
        return 0; 
    }
    else
    {
        //RR Implementation
        Queue queue;
        initialize_queue(&queue);

        //Allocate space 
        int *jobs_left = malloc(n * sizeof(int));
        int *added_jobs = calloc(n, sizeof(int));

        if (jobs_left == NULL || added_jobs == NULL) 
        {
            fprintf(stderr, "Memory allocation failed in RR.\n");
            free(jobs_left);
            free(added_jobs);
            free(cpu_runtime_array); 
            free(job_details);
            return 1; // safely exit 
        }

        //Iterate through all the remaining jobs and set their cpu time
        for (int i = 0; i < n; i++) 
        {
            jobs_left[i] = jobs[i].cpu_time; 
        }

        int cpu_tick = 0; 
        int num_jobs_finished = 0; 
        int current_index = -1; 
        int quantum_used = 0; 

        //Loop until all of the jobs have been completed
        while (num_jobs_finished < n)
        {
            //Allocate more space when capacity is full
            if (cpu_tick >= capacity)
            {
                capacity *= 2; // double capacity 
                int* new_cpu_runtime_array = realloc(cpu_runtime_array, capacity * sizeof(int));

                if (new_cpu_runtime_array == NULL)
                {
                    fprintf(stderr, "Memory reallocation failed in RR!\n");
                    free(jobs_left);
                    free(added_jobs);
                    free(cpu_runtime_array);
                    return 1;
                }
                cpu_runtime_array = new_cpu_runtime_array; 
            }
            // Add jobs that just arrived to ready queue 
            for (int i = 0; i < n; i++)
            {
                if (!added_jobs[i] && jobs[i].arrival == cpu_tick)
                {
                    enqueue(&queue, jobs[i].pid);
                    added_jobs[i] = 1; 
                }
            }
        

            // if no process running, choose next ready process
            if (current_index == -1 && !is_empty(&queue))
            {
                int next_pid = dequeue(&queue);
                for (int i = 0; i < n; i++)
                {
                    if (jobs[i].pid == next_pid)
                    {
                        current_index = i; 
                        break; 
                    }
                }
                quantum_used = 0; 
                if (job_details[current_index].first_run == -1)
                {
                    job_details[current_index].first_run = cpu_tick;
                    job_details[current_index].resp = job_details[current_index].first_run - jobs[current_index].arrival;
                }
            }

            //If there is a process currently running
            if (current_index != -1)
            {
                cpu_runtime_array[cpu_tick] = jobs[current_index].pid;

                jobs_left[current_index]--;
                quantum_used++;

                //Process done at the end of this tick
                if (jobs_left[current_index] == 0)
                {
                    job_details[current_index].completion = cpu_tick + 1;
                    job_details[current_index].tat = job_details[current_index].completion - jobs[current_index].arrival;

                    current_index = -1;
                    quantum_used = 0;
                    num_jobs_finished++;
                }
                //Quantum expired, put the process at back of queue
                else if (quantum_used == cfg->quantum)
                {
                    enqueue(&queue, jobs[current_index].pid);
                    current_index = -1;
                    quantum_used = 0;
                }
            }
            else
            {
                //No process running at this tick (CPU idle)
                cpu_runtime_array[cpu_tick] = -1;
            }

            cpu_tick++;
        }

        out->context_switches = 0;
        int prev = -1;

        //Iterate through the cpu ticks and check if the current PID is different from the previous PID (context switch)
        for (int i = 0; i < cpu_tick; i++)
        {
            int cur = cpu_runtime_array[i];

            if (cur >= 0 && prev >= 0 && cur != prev)
            {
                out->context_switches++;
            }

            if (cur >= 0)
            {
                prev = cur;
            }
        }
        //Metric averages
        out->avg_tat = (double)sum_tat(job_details, n) / n;
        out->avg_resp = (double)sum_resp(job_details, n) / n;
        
        // call printing functions 
        print_time_array(cpu_tick);
        print_run_array(cpu_runtime_array, cpu_tick);
        print_job_details(job_details, n);

        // free memory 
        free(jobs_left);
        jobs_left = NULL;

        free(added_jobs);
        added_jobs = NULL;

        free(cpu_runtime_array);
        cpu_runtime_array = NULL;

        free(job_details);
        job_details = NULL;

        return 0;
    }
}

/*
Description: Calls helper functions to simulate CPU scheduling 
Parameteres: argc, argv 
Return: 0 
*/
int main(int argc, char** argv){
    sim_cfg_t cfg; const char* in_path=NULL;
    int pr = parse_args(argc, argv, &cfg, &in_path);
    if (pr != 0) return 1;

    job_t* jobs=NULL; int n=0;
    if (load_workload(in_path, &jobs, &n) != 0) return 2;

    sim_metrics_t m;
    if (simulate(jobs, n, &cfg, &m) != 0) { free(jobs); return 3; }

    print_job_systems(m);

    free(jobs);
    jobs = NULL;

    return 0;
}

