#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>

#include "loader.h"
#include "shell.h"

typedef struct param{
    pthread_mutex_t *mutex;
} PARAM;

int main(int argc, char *argv[]){
    pthread_t pt[2];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    int rc, i;
    PARAM *param = (PARAM *) malloc(sizeof(PARAM));

	if (argc != 2){
        printf("ERROR: Program je spusten bez parametru!\n");
        return -1;
    }

    param->mutex = &mutex;

    // NTFS loader from file
    rc = pthread_create(&pt[0], NULL, loader, (void *) param);
    assert(0 == rc);

    // prikazovy interpret
    rc = pthread_create(&pt[0], NULL, shell, (void *) param);
    assert(0 == rc);

    // Cekam na dokonceni vsech vlaken
    for(i = 0; i < 2; i++){
        rc = pthread_join(pt[i], NULL);
    }

    printf("NTFS end\n");
    return 0;
}
