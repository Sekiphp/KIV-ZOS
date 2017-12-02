#include <stdio.h>
#include <stdlib.h>

#include "loader.h"
#include "boot_record.h"
#include "mft_item.h"
#include "mft_fragment.h"

/* Nacte NTFS ze souboru */
void *loader(void *arg){
    printf("LOADER starting...\n");

    printf("LOADER ending\n");
    return NULL;
}
