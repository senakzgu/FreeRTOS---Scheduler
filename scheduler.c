#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "scheduler.h"
TaskQueue queues[NUM_QUEUES];


int current_time = 0;   // HATA OLMAMASI İÇİN BURADA TANIMLADIK


void run_scheduler() {
    printf("Scheduler basladi.\n");

    Task* rt = create_task(1, 0, 4);   // RT görev: zaman=1, öncelik=0, süre=4 saniye
    Task* t1 = create_task(0, 1, 5);   // Normal görev
    Task* t2 = create_task(2, 2, 3);   // Normal görev


    // Tüm kuyrukları başlat
    for (int i = 0; i < NUM_QUEUES; i++)
        init_queue(&queues[i]);


    for (int time = 0; time < 5; time++) {
        printf("Zaman = %d\n", time);

        if (t1->arrival_time == time) {
            enqueue(&queues[t1->priority], t1);
            printf("  Görev %d kuyruğa eklendi (öncelik=%d)\n", t1->id, t1->priority);
        }

        if (t2->arrival_time == time) {
            enqueue(&queues[t2->priority], t2);
            printf("  Görev %d kuyruğa eklendi (öncelik=%d)\n", t2->id, t2->priority);
        }
        if (rt->arrival_time == time) {
            enqueue(&queues[rt->priority], rt);
            printf("  RT Görev %d kuyruğa eklendi (öncelik=%d)\n", rt->id, rt->priority);
        }


        Task* running = NULL;

        // Kuyruk öncelik sırası: 0 → 1 → 2 → 3
        for (int level = 0; level < NUM_QUEUES; level++) {
            // 1) Eğer RT kuyruğunda görev varsa → sadece oradan çek
            if (queues[0].count > 0) {
                running = dequeue(&queues[0]);
            }
            else {
                // 2) RT yoksa MLFQ sırası: 1 → 2 → 3
                for (int level = 1; level < NUM_QUEUES; level++) {
                    if (queues[level].count > 0) {
                        running = dequeue(&queues[level]);
                        break;
                    }
                }
            }
        }

        if (running == NULL) {
            printf("  CPU boş (kuyruk boş)\n");
        } else {
            printf("  CPU: Görev %d çalışıyor (öncelik=%d, kalan=%d)\n",
                running->id, running->priority, running->remaining_time);

            running->remaining_time--;

        if (running->remaining_time <= 0) {
            running->state = TASK_TERMINATED;
            printf("  Görev %d tamamlandı.\n", running->id);
        } else {
            // Görevin önceliğini bir alt seviyeye düşür
            if (running->priority < 3)
                running->priority++;

            // Görev bitmediyse doğru kuyruğa geri koy
            enqueue(&queues[running->priority], running);

            printf("  Görev %d alt seviyeye indi (yeni öncelik=%d, kalan=%d)\n",
                running->id, running->priority, running->remaining_time);
        }

        }

    }

    printf("Scheduler bitti.\n");
}



// ----------------------
// Kuyruk Başlatma
// ----------------------
void init_queue(TaskQueue* q) {
    q->front = 0;
    q->rear = -1;
    q->count = 0;
}

// ----------------------
// Kuyruğa ekle
// ----------------------
void enqueue(TaskQueue* q, Task* t) {
    if (q->count == MAX_TASKS) {
        printf("Kuyruk dolu! Görev eklenemiyor.\n");
        return;
    }
    q->rear = (q->rear + 1) % MAX_TASKS;
    q->items[q->rear] = t;
    q->count++;
}

// ----------------------
// Kuyruktan çıkar
// ----------------------
Task* dequeue(TaskQueue* q) {
    if (q->count == 0) {
        printf("Kuyruk bos!\n");
        return NULL;
    }
    Task* t = q->items[q->front];
    q->front = (q->front + 1) % MAX_TASKS;
    q->count--;
    return t;
}


Task* create_task(int arrival, int priority, int burst) {
    Task* t = (Task*)malloc(sizeof(Task));
    t->id = rand() % 1000; // şimdilik rasgele id
    t->arrival_time = arrival;
    t->priority = priority;
    t->burst_time = burst;
    t->remaining_time = burst;
    t->state = TASK_READY;
    return t;
}


