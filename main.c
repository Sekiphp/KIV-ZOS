#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "loader.h"
#include "shell.h"
#include "parametr.h"

const int32_t UID_ITEM_FREE = 0;

// hlavni vstupni trida aplikace
int main(int argc, char *argv[]){
    pthread_t pt[2];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    sdilenaPamet pamet;
    int rc;

    // kontrola poctu parametru
    if (argc != 2){
        printf("ERROR: Program je spusten bez parametru!\n");
        return -1;
    }

    // priprava sdilenych atributu pro vsechny vlakna
    pamet.mutex = &mutex;
    strcpy(pamet.soubor, argv[1]);

    // NTFS loader from file
    rc = pthread_create(&pt[0], NULL, loader, (void *) &pamet);
    assert(0 == rc);

    // prikazovy interpret
    rc = pthread_create(&pt[1], NULL, shell, (void *) &pamet);
    assert(0 == rc);

    // Cekam na dokonceni vsech vlaken
    for(i = 0; i < 2; i++){
        rc = pthread_join(pt[i], NULL);
        assert(0 == rc);
    }

    printf("NTFS end\n");
    return 0;
}
