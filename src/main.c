#include "scheduler.h"
#include <string.h>

// Global tanimlar (Bellek ayirma)
Task* all_tasks[MAX_TASKS];
int total_task_count = 0;

void read_input_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Dosya acilamadi");
        exit(EXIT_FAILURE);
    }

    int arrival, priority, burst;
    int task_counter = 0;

    // Dosyayi satir satir oku (Virgullere dikkat)
    while (fscanf(file, "%d, %d, %d", &arrival, &priority, &burst) == 3) {
        
        Task* new_task = (Task*)malloc(sizeof(Task));
        new_task->id = task_counter;
        new_task->arrival_time = arrival;
        new_task->priority = priority;
        new_task->burst_time = burst;
        new_task->remaining_time = burst;
        new_task->state = TASK_READY;

        pthread_mutex_init(&new_task->mutex, NULL);
        pthread_cond_init(&new_task->cond, NULL);

        if (pthread_create(&new_task->thread, NULL, task_thread_function, (void*)new_task) != 0) {
            perror("Thread olusturulamadi");
            free(new_task);
            continue;
        }

        all_tasks[task_counter] = new_task;
        task_counter++;
    }
    
    total_task_count = task_counter;
    fclose(file);
    
    // Eger hic task okunmadiysa hata verelim
    if(total_task_count == 0) {
        printf("HATA: Dosyadan hicbir task okunamadi! Format '0, 1, 2' seklinde olmali.\n");
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Kullanim: ./bin/program giris.txt\n");
        return 1;
    }

    init_scheduler();
    read_input_file(argv[1]);
    schedule_simulation();

    return 0;
}