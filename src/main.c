#include <stdio.h>
#include "scheduler.h"

int main(int argc, char* argv[]) {

    if (argc < 2) {
        printf("Kullanım: %s <giris_dosyasi>\n", argv[0]);
        return 1;
    }

    run_scheduler(argv[1]);   // DOSYA ADINI SCHEDULER’A GÖNDER
    return 0;
}
