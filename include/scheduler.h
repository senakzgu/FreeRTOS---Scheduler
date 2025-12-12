#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdlib.h>

#define MAX_TASKS 100
#define NUM_QUEUES 4

// Görev durumları
typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_WAITING,
    TASK_TERMINATED
} TaskState;

// Görev yapısı
typedef struct {
    int id;
    char name[16];

    int arrival_time;
    int priority;
    int burst_time;
    int remaining_time;

    int run_time;            // CPU’da çalıştığı toplam süre
    int start_time;          // Görev ilk kez CPU’ya alındığı an (TIMEOUT için)
    int timed_out;           // Timeout ile mi bitti? 1 = evet
    int total_life_time;     // (İstersen kullanırsın ama final kodda zorunlu değil)
    int has_started;


    char* color;
    TaskState state;
} Task;

// Kuyruk yapısı
typedef struct {
    Task* items[MAX_TASKS];
    int front;
    int rear;
    int count;
} TaskQueue;

extern TaskQueue queues[NUM_QUEUES];
extern Task* all_tasks[MAX_TASKS];
extern int task_count;

// Fonksiyonlar
void run_scheduler(const char* filename);

void init_queue(TaskQueue* q);
void enqueue(TaskQueue* q, Task* t);
Task* dequeue(TaskQueue* q);

Task* create_task(int arrival, int priority, int burst);
void print_simulation_summary(int total_time);

#endif
