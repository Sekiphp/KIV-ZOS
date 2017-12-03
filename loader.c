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
    struct boot_record *br2;
    FILE *fr;

    printf("LOADER starting...\n");

    printf("louda %s\n", param->soubor);



    br2 = malloc(sizeof(struct boot_record));
    fr = fopen(param->soubor, "rb");
    if (fr != NULL) {
        fread(br2, sizeof(struct boot_record), 1, fr);
        fclose(fr);
    }
    printf("%s\n",br2->signature);





    printf("LOADER ending\n");
    return NULL;
}
