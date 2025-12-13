#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"

TaskQueue queues[NUM_QUEUES];
Task* all_tasks[MAX_TASKS];
int task_count = 0;
int global_task_name = 1;



// ===================================================
// giris.txt'i okur
// ===================================================
void load_tasks_from_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("giris.txt acilamadi!\n");
        return;
    }

    int arrival, priority, burst;

    while (fscanf(file, "%d, %d, %d", &arrival, &priority, &burst) == 3) {
        Task* t = create_task(arrival, priority, burst);
        all_tasks[task_count++] = t;
    }

    fclose(file);
}



// ===================================================
// Görevi tüm kuyruklardan temizler
// ===================================================
void remove_from_all_queues(Task* t) {

    for (int q = 0; q < NUM_QUEUES; q++) {

        TaskQueue* Q = &queues[q];
        if (Q->count == 0) continue;

        Task* new_items[MAX_TASKS];
        int new_count = 0;

        for (int i = 0; i < Q->count; i++) {
            Task* item = Q->items[(Q->front + i) % MAX_TASKS];
            if (item != t)
                new_items[new_count++] = item;
        }

        for (int i = 0; i < new_count; i++)
            Q->items[i] = new_items[i];

        Q->front = 0;
        Q->rear  = new_count - 1;
        Q->count = new_count;
    }
}



// ===================================================
// Tum gorevler bitti mi?
// ===================================================
int all_tasks_finished() {
    for (int i = 0; i < task_count; i++)
        if (all_tasks[i]->state != TASK_TERMINATED)
            return 0;

    return 1;
}



// ===================================================
// ANA SCHEDULER DÖNGÜSÜ
// ===================================================
void run_scheduler(const char* filename) {

    printf("Scheduler basladi.\n");

    load_tasks_from_file(filename);

    int max_arrival = 0;
    for (int i = 0; i < task_count; i++)
        if (all_tasks[i]->arrival_time > max_arrival)
            max_arrival = all_tasks[i]->arrival_time;

    for (int i = 0; i < NUM_QUEUES; i++)
        init_queue(&queues[i]);

    int time = 0;

    while (1) {

        // ---------------------------------------------------
        // 1) Yeni gelen görevleri ilgili kuyruga ekle
        // ---------------------------------------------------
        for (int i = 0; i < task_count; i++) {
            Task* t = all_tasks[i];

            if (t->arrival_time == time)
                enqueue(&queues[t->priority], t);
        }

        // ---------------------------------------------------
        // 2) Timeout kontrolü — 20 saniye çalışmamış görev
        // ---------------------------------------------------
        for (int i = 0; i < task_count; i++) {

            Task* t = all_tasks[i];

            if (t->state != TASK_TERMINATED && t->last_run_time >= 0) {

                if (time - t->last_run_time >= 20) {

                    printf("%.4f sn   proses zaman asimi (id:%04d kalan:%d sn)\n",
                           (float)time, t->id, t->remaining_time);

                    t->state = TASK_TERMINATED;
                    t->timed_out = 1;

                    remove_from_all_queues(t);
                }
            }
        }

        // ---------------------------------------------------
        // 3) CPU SECIMI — RT > MLFQ
        // ---------------------------------------------------
        Task* running = NULL;

        if (queues[0].count > 0) {
            running = dequeue(&queues[0]);
        }
        else {
            for (int q = 1; q < NUM_QUEUES; q++) {
                if (queues[q].count > 0) {
                    running = dequeue(&queues[q]);
                    break;
                }
            }
        }

        // ---------------------------------------------------
        // 4) CPU bossa
        // ---------------------------------------------------
        if (running == NULL) {

            if (all_tasks_finished()) {
                printf("%.4f sn   CPU bos\n", (float)time);
                break;
            }

            printf("%.4f sn   CPU bos\n", (float)time);
            time++;
            continue;
        }


        // ---------------------------------------------------
        // 5) Görev ilk kez mi çalışıyor?
        // ---------------------------------------------------
        running->last_run_time = time;

        if (!running->has_started) {

            running->has_started = 1;

            printf("%s%.4f sn   %s basladi (id:%04d oncelik:%d kalan:%d)%s\n",
                   running->color, (float)time, running->name, running->id,
                   running->priority, running->remaining_time, "\033[0m");
        }
        else {
            printf("%s%.4f sn   %s devam ediyor (id:%04d kalan:%d)%s\n",
                   running->color, (float)time, running->name, running->id,
                   running->remaining_time, "\033[0m");
        }

        running->state = TASK_RUNNING;


        // ---------------------------------------------------
        // 6) Yürütme mesajı
        // ---------------------------------------------------
        printf("%s%.4f sn   %s yurutuluyor (kalan:%d)%s\n",
               running->color, (float)time, running->name,
               running->remaining_time, "\033[0m");

        running->remaining_time--;
        running->run_time++;


        // ---------------------------------------------------
        // 7) Görev tamamlandı mı?
        // ---------------------------------------------------
        if (running->remaining_time <= 0) {

            running->state = TASK_TERMINATED;

            printf("%s%.4f sn   %s sonlandi (id:%04d)%s\n",
                   running->color, (float)time, running->name,
                   running->id, "\033[0m");

            time++;
            continue;
        }


        // ---------------------------------------------------
        // 8) Real-time (öncelik 0) → kesilmez, tekrar aynı kuyruk
        // ---------------------------------------------------
        if (running->priority == 0) {
            running->state = TASK_WAITING;
            enqueue(&queues[0], running);
            time++;
            continue;
        }


        // ---------------------------------------------------
        // 9) MLFQ normal görev → askıya al, önceliği düşür
        // ---------------------------------------------------
        printf("%s%.4f sn   %s askida (id:%04d)%s\n",
               running->color, (float)time, running->name,
               running->id, "\033[0m");

        if (running->priority < 3)
            running->priority++;

        running->state = TASK_WAITING;
        enqueue(&queues[running->priority], running);

        time++;
    }

    print_simulation_summary(time);
    printf("Scheduler bitti.\n");
}



// ===================================================
// Kuyruk işlemleri
// ===================================================
void init_queue(TaskQueue* q) {
    q->front = 0;
    q->rear  = -1;
    q->count = 0;
}

void enqueue(TaskQueue* q, Task* t) {
    if (q->count == MAX_TASKS) return;
    q->rear = (q->rear + 1) % MAX_TASKS;
    q->items[q->rear] = t;
    q->count++;
}

Task* dequeue(TaskQueue* q) {
    if (q->count == 0) return NULL;

    Task* t = q->items[q->front];
    q->front = (q->front + 1) % MAX_TASKS;
    q->count--;

    return t;
}



// ===================================================
// Görev oluşturma
// ===================================================
Task* create_task(int arrival, int priority, int burst) {

    Task* t = malloc(sizeof(Task));

    t->id = rand() % 1000;
    sprintf(t->name, "task%d", global_task_name++);

    t->arrival_time   = arrival;
    t->priority       = priority;
    t->burst_time     = burst;
    t->remaining_time = burst;

    t->state = TASK_READY;

    t->run_time      = 0;
    t->timed_out     = 0;
    t->has_started   = 0;
    t->last_run_time = arrival;

    char* colors[] = {
        "\033[31m","\033[32m","\033[33m",
        "\033[34m","\033[35m","\033[36m","\033[37m"
    };

    t->color = colors[rand() % 7];

    return t;
}



// ===================================================
// Özet yazdırma
// ===================================================
void print_simulation_summary(int total_time) {

    int finished = 0;
    int timeout  = 0;

    int min_time = 9999;
    int max_time = -1;

    Task* shortest = NULL;
    Task* longest  = NULL;

    for (int i = 0; i < task_count; i++) {

        Task* t = all_tasks[i];

        if (t->timed_out)
            timeout++;
        else if (t->remaining_time == 0)
            finished++;

        if (t->run_time < min_time) {
            min_time = t->run_time;
            shortest = t;
        }
        if (t->run_time > max_time) {
            max_time = t->run_time;
            longest  = t;
        }
    }

    printf("\n===================================\n");
    printf("         SIMULASYON OZETI\n");
    printf("===================================\n");

    printf("Toplam gorev sayisi     : %d\n", task_count);
    printf("Tamamlanan gorev sayisi : %d\n", finished);
    printf("Zaman asimi gorev sayisi: %d\n", timeout);
    printf("Toplam calisma suresi   : %d sn\n", total_time);

    if (shortest)
        printf("En kisa calisan : %s (id:%04d) -> %d sn\n",
               shortest->name, shortest->id, shortest->run_time);

    if (longest)
        printf("En uzun calisan : %s (id:%04d) -> %d sn\n",
               longest->name, longest->id, longest->run_time);

    printf("===================================\n\n");
}
