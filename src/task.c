#include "scheduler.h"
#include <unistd.h> 

// Oncelige gore renk secimi
const char* get_color(int priority) {
    switch(priority) {
        case 0: return RED;
        case 1: return YELLOW;
        case 2: return BLUE;
        case 3: return MAGENTA;
        default: return RESET;
    }
}

void* task_thread_function(void* arg) {
    Task* task = (Task*)arg;

    while (task->remaining_time > 0) {
        
        // 1. Scheduler'dan sinyal bekle
        pthread_mutex_lock(&task->mutex);
        while (task->state != TASK_RUNNING) {
            pthread_cond_wait(&task->cond, &task->mutex);
        }
        pthread_mutex_unlock(&task->mutex);

        // 2. Ekrana Yazdir (SS'teki format)
        // Format: 57.0000 sn task12 yürütülüyor (id:0012 öncelik:3 kalan süre:1 sn)
        printf("%s%d.0000 sn task%d yürütülüyor \t(id:%04d \töncelik:%d \tkalan süre:%d sn)%s\n", 
               get_color(task->priority),
               current_time, 
               task->id, 
               task->id, 
               task->priority, 
               task->remaining_time, 
               RESET);

        // 1 saniye eksilt
        task->remaining_time--;

        // 3. Scheduler'a kontrolu birak (Durumu guncelleme scheduler.c'de yapilacak)
        // Burada sadece thread senkronizasyonu sagliyoruz.
        pthread_mutex_lock(&task->mutex);
        if (task->remaining_time == 0) {
            task->state = TASK_TERMINATED;
        } else {
            task->state = TASK_READY; 
        }
        pthread_mutex_unlock(&task->mutex);
        
        // Islem bitince thread tekrar wait durumuna gececek
    }
    return NULL;
}