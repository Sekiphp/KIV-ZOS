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
        // nepodarilo se otevrit soubory, tak jej zalozim

        printf("\tNepodarilo se otevrit soubor %s\n", filename);

        cluster_size = 1024;
        cluster_count = 10;

        printf("\tZakladam soubor %s\n", filename);
        printf("\t\tPocet clusteru je %d\n",cluster_count);
        printf("\t\tVelikost clusteru je %d\n", cluster_size);

        if (zaloz_soubor(cluster_size, cluster_count, filename) != 1){
            printf("\t\tChyba zalozeni souboru");
        }
    }
    fclose(fr);

    // nactu data ze souboru - ted uz mam jistotu, ze existuje
    fr = fopen(filename, "rb");
    if (fr != NULL) {
        // nactu data ze souboru

        printf("\tSoubor %s byl uspesne otevren\n", filename);

        // prvni musim precit boot record protoze o zbytku nemam ani paru
        fread(bootr, sizeof(struct boot_record), 1, fr);

        printf("\t\tBOOT RECORD:\n");
        printf("\t\t\tsignature: %s\n", bootr->signature);
        printf("\t\t\tdesc: %s\n", bootr->volume_descriptor);
        printf("\t\t\tCelkova velikost VFS: %d\n", bootr->disk_size);
        printf("\t\t\tVelikost jednoho clusteru: %d\n", bootr->cluster_size);
        printf("\t\t\tPocet clusteru: %d\n", bootr->cluster_count);
        printf("\t\t\tAdresa pocatku mft: %d\n", bootr->mft_start_address);
        printf("\t\t\tAdresa pocatku bitmapy: %d\n", bootr->bitmap_start_address);
        printf("\t\t\tAdresa pocatku datoveho bloku: %d\n", bootr->data_start_address);

            int sizeof_mft_item = sizeof(struct mft_item);
            int sirka_mft = bootr->bitmap_start_address - bootr->mft_start_address;
            int pocet_mft_bloku = sirka_mft / sizeof_mft_item;
            printf("\t\t\tpocet mft bloku je: %d", pocet_mft_bloku);

            struct mft_item *mft_table = malloc(sizeof_mft_item);
            for(i = 0; i < pocet_mft_bloku; i++){
                fseek(fr, bootr->mft_start_address + i *sizeof_mft_item, SEEK_SET);
                fread(mft_table, sizeof_mft_item, 1, fr);

                printf("\t\t\t--------------------------\n");
                printf("\t\t\tfread cte z pozice %d \n", (bootr->mft_start_address + i * sizeof_mft_item));
                printf("\t\t\tUID: %d\n", mft_table->uid);
                printf("\t\t\tIsDirectory: %d\n", mft_table->isDirectory);
                printf("\t\t\tPoradi v MFT pri vice souborech: %d\n", mft_table->item_order);
                printf("\t\t\tCelkovy pocet polozek v MFT: %d\n", mft_table->item_order_total);
                printf("\t\t\tJmeno polozky: %s\n", mft_table->item_name);
                printf("\t\t\tVelikost souboru v bytech: %d\n", mft_table->item_size);
                printf("\t\t\tVelikost pole s itemy: %lu\n", sizeof(mft_table->fragments));
            }
            free((void *) mft_table);

    }
    fclose(fr);

/*
            sizeof_mft_item = sizeof(struct mft_item);
            int sirka_mft = br2->bitmap_start_address - br2->mft_start_address;
            int pocet_mft_bloku = sirka_mft / sizeof_mft_item;
            printf("\t\t\tpocet mft bloku je: %d", pocet_mft_bloku);

            struct mft_item *mft_table = malloc(sizeof_mft_item);
            for(i = 0; i < pocet_mft_bloku; i++){
                fseek(file2, br2->mft_start_address + i *sizeof_mft_item, SEEK_SET);
                fread(mft_table, sizeof_mft_item, 1, file2);

                printf("\t\t\t--------------------------\n");
                printf("\t\t\tfread cte z pozice %d \n", (br2->mft_start_address + i * sizeof_mft_item));
                printf("\t\t\tUID: %d\n", mft_table->uid);
                printf("\t\t\tIsDirectory: %d\n", mft_table->isDirectory);
                printf("\t\t\tPoradi v MFT pri vice souborech: %d\n", mft_table->item_order);
                printf("\t\t\tCelkovy pocet polozek v MFT: %d\n", mft_table->item_order_total);
                printf("\t\t\tJmeno polozky: %s\n", mft_table->item_name);
                printf("\t\t\tVelikost souboru v bytech: %d\n", mft_table->item_size);
                printf("\t\t\tVelikost pole s itemy: %lu\n", sizeof(mft_table->fragments));
            }
            free((void *) mft_table);
*/



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
    else {
        return 0;
    }

    return 1;
}
