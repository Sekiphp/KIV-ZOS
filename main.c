#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debugger.h"
#include "loader.h"
#include "shell.h"
#include "parametr.h"
#include "boot_record.h"
#include "mft.h"

extern int ntfs_bitmap[]; // v loader.c
char output_file[100];

// hlavni vstupni trida aplikace
int main(int argc, char *argv[]){
    pthread_t pt[2];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    sdilenaPamet pamet;
    int i, rc;
    boot = malloc(sizeof(struct boot_record));

    // kontrola poctu parametru
    if (argc != 2){
        printf("ERROR: Program je spusten bez parametru!\n");
        return -1;
    }

    // priprava sdilenych atributu pro vsechny vlakna
    pamet.mutex = &mutex;
    strcpy(pamet.soubor, argv[1]);
    strcpy(output_file, argv[1]);

    // mft seznam na null
    for(i = 0; i < CLUSTER_COUNT; i++){
       mft_seznam[i] = NULL;
    }
    printf("mft seznam NULL\n");

    // checker - pokud soubor neexistuje, tak ho vytvorim
    loader(argv[1]);

    // kontrola nacteni
    for(i = 0; i < CLUSTER_COUNT; i++){
        printf("ntfs_bitmap[%d]=%d\n", i, ntfs_bitmap[i]);
    }

    // prikazovy interpret
    rc = pthread_create(&pt[1], NULL, shell, (void *) &pamet);
    assert(0 == rc);

    // Cekam na dokonceni vsech vlaken
    for(i = 1; i < 2; i++){
        rc = pthread_join(pt[i], NULL);
        assert(0 == rc);
    }

    // uvolneni pameti
    free((void *) bootr);

    printf("NTFS end\n");
    return 0;
}
