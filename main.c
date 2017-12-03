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

int main(int argc, char *argv[]){
    pthread_t pt[2];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    int rc, i;
	i = 55;
char pom[50];
	strcpy(pom, argv[1]);
//sdilenaPamet param = { &mutex, i, pom};
sdilenaPamet pamet;
pamet.mutex = &mutex;
pamet.pokus = i;
strcpy(pamet.soubor, pom);
	if (argc != 2){
        printf("ERROR: Program je spusten bez parametru!\n");
        return -1;
    }

    //param->mutex = &mutex;
//	param->pokus = 159;
//	strcpy(param->soubor, argv[1]);
printf("main %s\n", pamet.soubor);
    // NTFS loader from file
    rc = pthread_create(&pt[0], NULL, loader, (void *) &pamet);
    assert(0 == rc);

    // prikazovy interpret
    rc = pthread_create(&pt[1], NULL, shell, (void *) &pamet);
    assert(0 == rc);

    // Cekam na dokonceni vsech vlaken
    for(i = 0; i < 2; i++){
        rc = pthread_join(pt[i], NULL);
    }

//	free((void *) param);
    printf("NTFS end\n");
    return 0;
}
