/*
 * Student Name: Hannah Yabut and Ismael Robleh 
 * Student ID:  3131432 & 3149556
 * Date:         ____________________________________________
 * File:         ____________________________________________
 * Description:  ____________________________________________
 */


#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "sched.h"

void insertion_sort_pid(job_t* array, int n) {
    for (int i = 1; i < n; i++)
    {
        int key = array[i].pid;
        int j = i - 1;
       
        while (j >= 0 && array[j].pid > key)
        {
            array[j + 1].pid = array[j].pid;
            j--;
        }
        array[j + 1].pid = key;
    }
}

void insertion_sort_arrival(job_t* array, int n) {
    for (int i = 1; i < n; i++)
    {
        int key = array[i].arrival;
        int j = i - 1;

        while (j >= 0 && array[j].arrival > key)
        {
            array[j + 1].arrival = array[j].arrival;
            j--;
        }
        array[j + 1].arrival = key;
    }
}

static void usage(const char* prog){
    /* one-line usage only (per spec) */
    fprintf(stderr, "Usage: %s --policy=FCFS|RR [--quantum=N] --in=FILE\n", prog);
}

/* TODO: strict CLI
   Required: --policy=FCFS|RR, --in=FILE
   Optional: --quantum=N (required iff RR; N>0)
   On error: print usage() and return non-zero.
*/
int parse_args(int argc, char** argv, sim_cfg_t* cfg, const char** in_path)
{
    (void)argc; (void)argv; (void)cfg; (void)in_path;
    
    int seen_policy = 0; 
    int seen_in = 0; 
    int seen_quantum = 0; 

    cfg->quantum = 0; 
    cfg->policy = 0; 
    *in_path = NULL; 

    for (int i = 1; i < argc; i++)
    {
        if (strncmp(argv[i], "--policy=", 9) == 0)
        {
            const char* val = argv[i] + 9; 
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
            char* end = NULL;
            long q = strtol(argv[i] + 10, &end, 10);
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
            usage(argv[0]);
            return 1; 
        }
    }
    if (!seen_policy || !seen_in)
    {
         usage(argv[0]);
            return 1; 
    }
    if (cfg->policy == POL_RR && !seen_quantum) 
    {
        usage(argv[0]);
        return 1;
    }
    if (cfg->policy == POL_FCFS && seen_quantum) 
    {
        usage(argv[0]);
        return 1;
    }

    return 0;
}

/* TODO: read "PID ARRIVAL CPU_TIME", ignore lines starting with '#', validate:
   pid>=0, arrival>=0, cpu>0. Sort by (arrival, pid). Return 0 on success.
*/
int load_workload(const char* path, job_t** jobs, int* n){
    (void)path; (void)jobs; (void)n;

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

    int workload_status; //0 = success, 1 = unsuccesful

    int capacity = 10;
    job_t* jobs_array = malloc(sizeof(job_t) * capacity);
    if (jobs_array == NULL)
    {
        fprintf(stderr, "Memory allocation failed in workload!");
        fclose(file);
        return 1;
    }

    while ((num_read = getline(&line, &length, file)) != EOF) {
        if (line[0] == '#' || line[0] == '\n') continue;

        if (line[num_read - 1] == '\n')
        {
            line[num_read - 1] = '\0';
        }

        char junk;
        int matched = sscanf(line, "%d %d %d %s", &pid, &arrival, &cpu_time, &junk);
        if (matched == 3)
        {
            if (pid >= 0 && arrival >= 0 && cpu_time > 0)
            {
                if (line_num >= capacity) {
                    capacity *= 2;
                    job_t* new_jobs_array = realloc(jobs_array, sizeof(job_t) * capacity);

                    if (new_jobs_array == NULL)
                    {
                        fprintf(stderr, "Memory reallocation failed in workload!");
                        free(jobs_array);
                        free(line);
                        workload_status = 1;
                        break;
                    }
                    jobs_array = new_jobs_array;
                }
                jobs_array[line_num].pid = pid;
                jobs_array[line_num].arrival = arrival;
                jobs_array[line_num].cpu_time = cpu_time;
                line_num++;
            }
            else
            {
                fprintf(stderr, "Incorrect values for workloads!\n");
                workload_status = 1;
                break;
            }
        }
        else
        {
            fprintf(stderr, "Incorrect formatting of the table!\n");
            workload_status = 1;
            break;
        }
    }

    //TODO: Make sure multiple processes do not have same PID
    for (int i = 0; i < line_num; i++)
    {
        for (int j = 1; j < line_num; j++)
        {
            if (jobs_array[i].pid == jobs_array[j].pid)
            {
                workload_status = 1;
                break;
            }
        }
    }

    insertion_sort_pid(jobs_array, line_num);

    *n = line_num;

    workload_status = 0;

    free(jobs_array);
    jobs_array = NULL;

    fclose(file);
    file = NULL;

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
*/
int simulate(const job_t* jobs, int n, const sim_cfg_t* cfg, sim_metrics_t* out){
    (void)jobs; (void)n; (void)cfg; (void)out;
    fprintf(stderr, "TODO: simulate() not implemented\n");
    return 1;
}

int main(int argc, char** argv){
    sim_cfg_t cfg; const char* in_path=NULL;
    int pr = parse_args(argc, argv, &cfg, &in_path);
    if (pr != 0) return 1;

    job_t* jobs=NULL; int n=0;
    if (load_workload(in_path, &jobs, &n) != 0) return 2;

    printf("Number of processes: %d, File path: %s\n", n, in_path);

    /*
    sim_metrics_t m;
    if (simulate(jobs, n, &cfg, &m) != 0) { free(jobs); return 3; }
    */

    free(jobs);
    jobs = NULL;

    return 0;
}

