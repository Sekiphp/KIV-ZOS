#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "loader.h"
#include "boot_record.h"

/* Nacte NTFS ze souboru */
void *loader(void *arg){
    printf("LOADER starting...\n");

	struct boot_record *br = malloc(sizeof(struct boot_record));
	strcpy(br->signature, "Hubacek");
	strcpy(br->volume_descriptor, "Puvodni vytvoreni 3.12.2017");
	br->disk_size = 1024 * 100;
	br->cluster_size = 1024;
	br->cluster_count = 50;
	br->mft_start_address = 1024;
	br->bitmap_start_address = 2048;
	br->data_start_address = 3 * 1024;
	br->mft_max_fragment_count = 32;

	FILE * file= fopen("output", "wb");
if (file != NULL) {
    fwrite(br, sizeof(struct boot_record), 1, file);
    fclose(file);
}
printf("zapsano\n");



struct boot_record *br2=malloc(sizeof(struct boot_record));
    FILE * file2= fopen("output", "rb");
    if (file2 != NULL) {
        fread(br2, sizeof(struct boot_record), 1, file2);
        fclose(file2);
    }
    printf("%s\n",br2->signature);



    printf("LOADER ending\n");
    return NULL;
}
