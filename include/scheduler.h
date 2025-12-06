#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_TASKS 100
#define NUM_QUEUES 4     
#define QUANTUM_TIME 1   

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_WAITING,
    TASK_TERMINATED
} TaskState;

typedef struct {
    int id;                 
    int arrival_time;       
    int priority;           
    int burst_time;         
    int remaining_time;     
    TaskState state;        
    
    pthread_t thread;       
    pthread_mutex_t mutex;  
    pthread_cond_t cond;    
} Task;

typedef struct {
    Task* tasks[MAX_TASKS]; 
    int count;              
    int front;              
    int rear;               
} TaskQueue;

// Global Degiskenler
extern TaskQueue queues[NUM_QUEUES]; 
extern Task* all_tasks[MAX_TASKS];
extern int total_task_count;
extern int current_time;

// Renk Kodlari (Terminal icin)
#define RESET   "\033[0m"
#define RED     "\033[31m"      // P0 (Real Time)
#define YELLOW  "\033[33m"      // P1
#define BLUE    "\033[34m"      // P2
#define MAGENTA "\033[35m"      // P3
#define GREEN   "\033[32m"      // Baslangic/Bitis

void init_scheduler();
void schedule_simulation(); 
void* task_thread_function(void* arg);

#endif