#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>

#include "loader.h"
#include "boot_record.h"
#include "mft.h"
#include "parametr.h"

/* Nacte NTFS ze souboru */
void * loader(void * arg){
    sdilenaPamet *param = (sdilenaPamet *) arg;

    printf("LOADER starting...\n");

    printf("louda %s\n", param->soubor);



    struct boot_record *br2 = malloc(sizeof(struct boot_record));
    FILE * file2= fopen("output", "rb");
    if (file2 != NULL) {
        fread(br2, sizeof(struct boot_record), 1, file2);
        fclose(file2);
    }
    printf("%s\n",br2->signature);





    printf("LOADER ending\n");
    return NULL;
}
