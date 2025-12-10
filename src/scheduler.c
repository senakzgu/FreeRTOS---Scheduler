#include "scheduler.h"
#include <unistd.h> 

TaskQueue queues[NUM_QUEUES];
int current_time = 0;
int completed_task_count = 0;

void init_scheduler() {
    for (int i = 0; i < NUM_QUEUES; i++) {
        queues[i].count = 0;
        queues[i].front = 0;
        queues[i].rear = 0;
    }
}

void push_to_queue(int priority, Task* task) {
    if (priority >= NUM_QUEUES) priority = NUM_QUEUES - 1;
    int r = queues[priority].rear;
    queues[priority].tasks[r] = task;
    queues[priority].rear = (r + 1) % MAX_TASKS;
    queues[priority].count++;
}

Task* pop_queue(int priority) {
    if (queues[priority].count == 0) return NULL;
    Task* t = queues[priority].tasks[queues[priority].front];
    queues[priority].front = (queues[priority].front + 1) % MAX_TASKS;
    queues[priority].count--;
    return t;
}

Task* peek_queue(int priority) {
    if (queues[priority].count == 0) return NULL;
    return queues[priority].tasks[queues[priority].front];
}

void schedule_simulation() {
    printf("\n"); // Baslangic boslugu

    // Sonsuz dongu degil, isler bitene kadar
    while (completed_task_count < total_task_count) {
        
        // 1. Yeni gelenleri kontrol et ve kuyruga ekle
        for (int i = 0; i < total_task_count; i++) {
            if (all_tasks[i]->arrival_time == current_time) {
                push_to_queue(all_tasks[i]->priority, all_tasks[i]);
                
                // Format: 56.0000 sn task11 başladı (id:0011 öncelik:3 kalan süre:2 sn)
                printf("%s%d.0000 sn task%d başladı \t\t(id:%04d \töncelik:%d \tkalan süre:%d sn)%s\n", 
                       GREEN, current_time, all_tasks[i]->id, all_tasks[i]->id, 
                       all_tasks[i]->priority, all_tasks[i]->burst_time, RESET);
            }
        }

        // 2. En yuksek oncelikli taski bul (0 -> 1 -> 2 -> 3)
        Task* current_task = NULL;
        int current_q = -1;

        for (int i = 0; i < NUM_QUEUES; i++) {
            if (queues[i].count > 0) {
                current_task = peek_queue(i); // Sadece bak, cikarma
                current_q = i;
                break;
            }
        }

        if (current_task != NULL) {
            // Eger task daha once baslamis ve simdi sira tekrar geldiyse "devam" yaz
            if (current_task->burst_time != current_task->remaining_time) {
                 printf("%s%d.0000 sn task%d devam \t\t(id:%04d \töncelik:%d \tkalan süre:%d sn)%s\n", 
                       BLUE, current_time, current_task->id, current_task->id, 
                       current_task->priority, current_task->remaining_time, RESET);
            }

            // Task'i calistir
            pthread_mutex_lock(&current_task->mutex);
            current_task->state = TASK_RUNNING;
            pthread_cond_signal(&current_task->cond);
            pthread_mutex_unlock(&current_task->mutex);

            usleep(10000); // Thread'in ekrana yazmasi icin minik bekleme
        }

        // Zamani ilerlet
        usleep(50000); // Similasyon hizi
        current_time++;

        // Task durumunu kontrol et
        if (current_task != NULL) {
            // Task bitti mi?
            if (current_task->state == TASK_TERMINATED || current_task->remaining_time == 0) {
                pop_queue(current_q); // Kuyruktan at
                completed_task_count++;
                
                printf("%s%d.0000 sn task%d sonlandı \t\t(id:%04d \töncelik:%d \tkalan süre:0 sn)%s\n", 
                       GREEN, current_time, current_task->id, current_task->id, 
                       current_task->priority, RESET);
            } 
            else {
                // Task bitmedi, Priority 0 degilse oncelik duser (Feedback Queue)
                pop_queue(current_q); // Mevcut kuyruktan cikar
                
                int new_priority = current_task->priority;
                if (current_task->priority < 3) { // 3 degilse onceligi dusur (sayi artar)
                    new_priority++;
                    current_task->priority = new_priority;
                    printf("%s%d.0000 sn task%d askıya al -> Q%d \t(id:%04d \töncelik:%d \tkalan süre:%d sn)%s\n", 
                           RED, current_time, current_task->id, new_priority, 
                           current_task->id, current_task->priority, current_task->remaining_time, RESET);
                } else {
                    // Zaten en dusukte (Q3), round robin devam (Sona ekle)
                    // Ekran goruntusunde Q3 icin ozel bir "askiya al" yazmiyor ama mantik ayni
                }
                
                push_to_queue(new_priority, current_task); // Yeni (veya ayni) kuyrugun sonuna ekle
            }
        }
    }

    // Ozet Raporu
    printf("\n===== Simülasyon Özeti =====\n");
    printf("Toplam görev: %d\n", total_task_count);
    printf("Toplam simülasyon süresi: %d saniye\n", current_time);
}