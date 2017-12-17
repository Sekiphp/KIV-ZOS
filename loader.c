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
void loader(char filename[]){
    FILE *fr;
    struct boot_record *bootr;
    int cluster_size, cluster_count;

    printf("LOADER starting...\n");
    printf("\tZkousim otevrit soubor: %s\n", filename);

    bootr = malloc(sizeof(struct boot_record));
    fr = fopen(filename, "rb");
    if (fr == NULL) {
        printf("\tNepodarilo se otevrit soubor %s\n", filename);

        cluster_size = 1024;
        cluster_count = 10;

        printf("\tZakladam soubor %s\n", filename);
        printf("\t\tPocet clusteru je %d\n",cluster_count);
        printf("\t\tVelikost clusteru je %d\n", cluster_size);

        zaloz_soubor(cluster_size, cluster_count, filename);
    }


    free((void *) bootr);
    printf("LOADER ending\n");
}

/* Zalozi pseudoNFTS soubor obsahujici ROOT_DIR */
/* int cluster_size = Velikost clusteru (default = 1024) */
/* int clutser_count = Pocet clusteru (default = 10) */
int zaloz_soubor(int cluster_size, int cluster_count, char filename[]){
    FILE *file;
    struct boot_record *bootr;
    struct mft_item *mfti;
    struct mft_fragment mftf;
    int i, bitmapa[cluster_count];

    /* Provedeme si nejake (pomocne) vypocty, vse s dostatecnou rezervou */
    int sirka_vsech_fragmentu = cluster_count * sizeof(struct mft_fragment); // pri spatnem scenari
    int sirka_vsech_clusteru = cluster_count * sizeof(struct mft_item); // pri spatnem scenari
    int sirka_bitmapy = 4 * cluster_count;

    int bitmap_start = sizeof(struct boot_record) + sirka_vsech_fragmentu + sirka_vsech_clusteru;
    int data_start = bitmap_start + sirka_bitmapy;


    /* Zacneme zapisovat do souboru */
    file = fopen(filename, "wb");
    if(file != NULL){
        /* Zapiseme boot record */
        bootr = malloc(sizeof(struct boot_record));

        strcpy(bootr->signature, "Hubacek");
        strcpy(bootr->volume_descriptor, "pseudo NTFS 2017");
        bootr->disk_size = cluster_size * cluster_count; // 10 * 1024
        bootr->cluster_size = cluster_size; // 1024
        bootr->cluster_count = cluster_count; // 10
        bootr->mft_start_address = sizeof(struct boot_record); // 288b
        bootr->bitmap_start_address = bitmap_start; // odrazim se od 288b
        bootr->data_start_address = data_start; // 4b ma jedna polozka, polozek je jako cluster_pocet
        bootr->mft_max_fragment_count = 1;

        fwrite(bootr, sizeof(struct boot_record), 1, file);
        free((void *) bootr);

        /* Zapiseme startovaci bitmapu - vse volne krome 1 (ROOT_DIR) */
        /* Posunu se na zacatek oblasti pro bitmapu */
        fseek(file, bitmap_start, SEEK_SET);
        bitmapa[0] = 1;
        for (i = 1; i < cluster_count; i++){
            bitmapa[i] = 0;
        }
        fwrite(bitmapa, sizeof(bitmapa[i]), cluster_count, file);

        /* Zapiseme ROOT_DIR do MFT tabulky, ROOT_DIR bude vzdy prvni v MFT tabulce*/
        /* ROOT_DIR se sklada z jdnoho itemu a jednoho fragmentu */
        /* Posunu se na zacatek oblasti MFT */
        fseek(file, sizeof(struct boot_record), SEEK_SET);

        mfti = malloc(sizeof(struct mft_item));

        mftf.fragment_start_address = data_start; // start adresa ve VFS
        mftf.fragment_count = 1; // pocet clusteru ve VFS od startovaci adresy

        mfti->uid = 1;
        mfti->isDirectory = 1;
        mfti->item_order = 1;
        mfti->item_order_total = 1;
        strcpy(mfti->item_name, "ROOT_DIR");
        mfti->item_size = 0; // zatim tam nic neni, takze nula
        mfti->fragments[0] = mftf;

        fwrite(mfti, sizeof(struct mft_item), 1, file);
        free((void *) mfti);

        /* Tady bychom meli zapsat obceh ROOT_DIRU */
        /* Jelikoz v nem ale pri prvnim spusteni nic neni, tak nic nezapisujeme :) */

        fclose(file);
    }

    return 1;
}
