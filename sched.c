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
#include "queue.h"
#include "stdbool.h"

void insertion_sort_pid(job_t* array, int n) {
    for (int i = 1; i < n; ++i)
    {
        int key = array[i].pid;
        int temp_arrival = array[i].arrival;
        int temp_cpu_time = array[i].cpu_time;
        int j = i - 1;
       
        while (j >= 0 && array[j].pid > key)
        {
            array[j + 1].pid = array[j].pid;
            array[j + 1].arrival = array[j].arrival;
            array[j + 1].cpu_time = array[j].cpu_time;
            j--;
        }
        array[j + 1].pid = key;
        array[j + 1].arrival = temp_arrival;
        array[j + 1].cpu_time = temp_cpu_time;
    }
}

void insertion_sort_arrival(job_t* array, int n) {
    for (int i = 1; i < n; ++i)
    {
        int key = array[i].arrival;
        int temp_pid = array[i].pid;
        int temp_cpu_time = array[i].cpu_time;
        int j = i - 1;

        while (j >= 0 && array[j].arrival > key)
        {
            array[j + 1].pid = array[j].pid;
            array[j + 1].arrival = array[j].arrival;
            array[j + 1].cpu_time = array[j].cpu_time;
            j--;
        }
        array[j + 1].arrival = key;
        array[j + 1].pid = temp_pid;
        array[j + 1].cpu_time = temp_cpu_time;
    }
}

void print_time_array(int n)
{
    printf("%-*s", 6, "time:");
    for (int i = 0; i < n; i++)
    {
        printf(" %6d", i);
    }
    printf("\n");
}

void print_run_array(int* array, int n)
{
    printf("%-*s", 6, "run:");
    for (int i = 0; i < n; i++)
    {
        if (array[i] != -1)
        {
            printf(" %6d", array[i]);
        }
        else
        {
            printf(" %6c", '-');
        }
    }
    printf("\n");
}

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

void print_job_systems(sim_metrics_t out)
{
    printf("System: ctx_switches=%d, avgTAT=%.3f, avgRESP=%.3f \n", out.context_switches, out.avg_tat, out.avg_resp);
}

int sum_tat(job_details_t* job_details, int n)
{
    int total = 0;
    for (int i = 0; i < n; i++)
    {
        total += job_details[i].tat;
    }
    return total;
}

int sum_resp(job_details_t* job_details, int n)
{
    int total = 0;
    for (int i = 0; i < n; i++)
    {
        total += job_details[i].resp;
    }
    return total;
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
                    // used sice of pointer instead of struct 
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
                (*jobs)[line_num].pid = pid;
                (*jobs)[line_num].arrival = arrival;
                (*jobs)[line_num].cpu_time = cpu_time;
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

    if (line_num == 0)
    {
        fprintf(stderr, "No jobs present in file!");

        fclose(file);
        file = NULL;
        return 1;
    }

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

    insertion_sort_pid((*jobs), line_num);

    insertion_sort_arrival((*jobs), line_num);

    for (int i = 0; i < line_num; i++)
    {
        printf("Sorted by PID --> PID: %d\t Arrival time: %d\t CPU time: %d\n", (*jobs)[i].pid, (*jobs)[i].arrival, (*jobs)[i].cpu_time);
    }

    *n = line_num;

    free(line); // frees memory 
    line = NULL; 

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

        while (num_jobs_finished < n)
        {
            if (cpu_tick >= capacity)
            {
                capacity *= 2;
                int* new_cpu_runtime_array = realloc(cpu_runtime_array, capacity * sizeof(int));

                if (new_cpu_runtime_array == NULL)
                {
                    fprintf(stderr, "Memory reallocation failed in simulate!");
                    free(cpu_runtime_array);
                    cpu_runtime_array = NULL;
                    return 1;
                }
                cpu_runtime_array = new_cpu_runtime_array;
            }

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

            for (int i = 0; i < n; i++)
            {
                if (!is_in_queue(&queue, jobs[i].pid) && jobs[i].arrival == cpu_tick)
                {
                    enqueue(&queue, jobs[i].pid);
                }
            }

            if (!is_empty(&queue))
            {
                // checks if prpcess has never been scheduled and reports 1st cpu time 
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
                cpu_runtime_array[cpu_tick] = -1;
            }
            cpu_tick++;
        }

        (*out).avg_tat = (double)sum_tat(job_details, n) / n;
        (*out).avg_resp = (double)sum_resp(job_details, n) / n;
        (*out).context_switches--;

        print_time_array(cpu_tick - 1);
        print_run_array(cpu_runtime_array, cpu_tick - 1);
        print_job_details(job_details, n);

        free(cpu_runtime_array);
        cpu_runtime_array = NULL;
    }
    else
    {
        //RR Implementation
    }

    free(job_details);
    job_details = NULL;

    return 0;
}

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

