#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>

#include "debugger.h"
#include "loader.h"
#include "boot_record.h"
#include "mft.h"

extern int pwd;

/* Nacte NTFS ze souboru */
void loader(char filename[]){
    FILE *fr;
    int i, sirka_mft, pocet_mft_bloku, sizeof_mft_item;
    struct mft_item mft_table, *mff;

    sizeof_mft_item = sizeof(struct mft_item);
    mff = malloc(sizeof_mft_item);

    printf("LOADER starting...\n");
    printf("\tZkousim otevrit soubor: %s\n", filename);

    fr = fopen(filename, "rb");
    if (fr == NULL) {
        // nepodarilo se otevrit soubory, tak jej zalozim

        printf("\tNepodarilo se otevrit soubor %s\n", filename);

        printf("\tZakladam soubor %s\n", filename);
        printf("\t\tPocet clusteru je %d\n",CLUSTER_COUNT);
        printf("\t\tVelikost clusteru je %d\n", CLUSTER_SIZE);

        zaloz_soubor(CLUSTER_SIZE, CLUSTER_COUNT, filename);
        fr = fopen(filename, "rb");
    }


    // nactu data ze souboru - ted uz mam jistotu, ze existuje
    if (fr != NULL) {
        // nactu data ze souboru
        bootr = malloc(sizeof(struct boot_record));

        printf("\tSoubor %s byl uspesne otevren\n", filename);

        // prvni musim precit boot record protoze o zbytku nemam ani paru
        fread(bootr, sizeof(struct boot_record), 1, fr);

        printf("\t\tBOOT RECORD:\n");
        printf("\t\t\tsignature: %s\n", bootr->signature);
        printf("\t\t\tdesc: %s\n", bootr->volume_descriptor);
        printf("\t\t\tCelkova velikost VFS: %d\n", bootr->disk_size);
        printf("\t\t\tVelikost jednoho clusteru: %d\n", bootr->cluster_size);
        printf("\t\t\tPocet clusteru: %d\n", bootr->cluster_count);
        printf("\t\t\tAdresa pocatku bitmapy: %d\n", bootr->bitmap_start_address);
            // nactu si bitmapu do globalni promenne
            fseek(fr, bootr->bitmap_start_address, SEEK_SET);
            fread(ntfs_bitmap, 4, bootr->cluster_count, fr);

        printf("\t\t\tAdresa pocatku mft: %d\n", bootr->mft_start_address);
            // zde si nactu mft_itemy do pole, ktere obsahuje struktury
            // nactitam jen takova mft, ktera maji uid != 0 (tj. neni pro ne v bitmape 0)

            sirka_mft = bootr->bitmap_start_address - bootr->mft_start_address;
            pocet_mft_bloku = sirka_mft / sizeof_mft_item;
            printf("\t\t\tpocet mft bloku je: %d\n", pocet_mft_bloku);

            for (i = 0; i < pocet_mft_bloku; i++) {
                fseek(fr, bootr->mft_start_address + i *sizeof_mft_item, SEEK_SET);
                fread(mff, sizeof_mft_item, 1, fr);
                mft_table = *mff;

                /*
                printf("\t\t\t--------------------------\n");
                printf("\t\t\tfread cte z pozice %d \n", (bootr->mft_start_address + i * sizeof_mft_item));
                */

                if (ntfs_bitmap[i] == 0 || mft_table.uid == UID_ITEM_FREE) {
                    /* printf("\t\t\tSkip MFT bloku s UID %d\n", mft_table.uid); */
                }
                else {
                    pridej_prvek_mft(mft_table.uid, mft_table);

                    /*
                    printf("\t\t\tUID: %d\n", mft_table.uid);
                    printf("\t\t\tIsDirectory: %d\n", mft_table.isDirectory);
                    printf("\t\t\tPoradi v MFT pri vice souborech: %d\n", mft_table.item_order);
                    printf("\t\t\tCelkovy pocet polozek v MFT: %d\n", mft_table.item_order_total);
                    printf("\t\t\tJmeno polozky: %s\n", mft_table.item_name);
                    printf("\t\t\tVelikost souboru v bytech: %d\n", mft_table.item_size);
                    printf("\t\t\tVelikost pole s itemy: %lu\n", sizeof(mft_table.fragments) / sizeof(struct mft_fragment));
                    */

                    if (i == 0) {
                        pwd = mft_table.uid;
                    }
                }
            }


        printf("\t\t\tAdresa pocatku datoveho bloku: %d\n", bootr->data_start_address);
            // tady data cist nebudu, v tomto souboru zjistuji jen strukturu

        free((void *) mff);
    	fclose(fr);
    }

    printf("LOADER ending\n");
}

/* Zalozi pseudoNFTS soubor obsahujici ROOT_DIR */
/* int cluster_size = Velikost clusteru (default = 1024) */
/* int clutser_count = Pocet clusteru (default = 10) */
void zaloz_soubor(int cluster_size, int cluster_count, char filename[]){
    FILE *fw;
    struct boot_record *bootr;
    struct mft_item *mfti;
    int i, bitmapa[cluster_count];

    /* Provedeme si nejake (pomocne) vypocty, vse s dostatecnou rezervou */
    int sirka_vsech_fragmentu = cluster_count * sizeof(struct mft_fragment); // pri spatnem scenari
    int sirka_vsech_clusteru = cluster_count * sizeof(struct mft_item); // pri spatnem scenari
    int sirka_bitmapy = 4 * cluster_count;

    int bitmap_start = sizeof(struct boot_record) + sirka_vsech_fragmentu + sirka_vsech_clusteru;
    int data_start = bitmap_start + sirka_bitmapy;


    /* Zacneme zapisovat do souboru */
    fw = fopen(filename, "wb");
    if (fw != NULL) {
        /* Zapiseme boot record */
        bootr = malloc(sizeof(struct boot_record));

        strcpy(bootr->signature, "Hubacek");
        strcpy(bootr->volume_descriptor, "pseudo NTFS 2017-2018");
        bootr->disk_size = cluster_size * cluster_count; // 10 * 1024
        bootr->cluster_size = cluster_size; // 1024
        bootr->cluster_count = cluster_count; // 10
        bootr->mft_start_address = sizeof(struct boot_record); // 288b
        bootr->bitmap_start_address = bitmap_start; // odrazim se od 288b
        bootr->data_start_address = data_start; // 4b ma jedna polozka, polozek je jako cluster_pocet
        bootr->mft_max_fragment_count = 1;

        fwrite(bootr, sizeof(struct boot_record), 1, fw);
        free((void *) bootr);

        /* Zapiseme startovaci bitmapu - vse volne krome 1 (ROOT_DIR) */
        /* Posunu se na zacatek oblasti pro bitmapu */
        fseek(fw, bitmap_start, SEEK_SET);
        bitmapa[0] = 1;
        for (i = 1; i < cluster_count; i++){
            bitmapa[i] = 0;
        }

        fwrite(bitmapa, 4, cluster_count, fw);

        /* Zapiseme ROOT_DIR do MFT tabulky, ROOT_DIR bude vzdy prvni v MFT tabulce*/
        /* ROOT_DIR se sklada z jdnoho itemu a jednoho fragmentu */
        /* Posunu se na zacatek oblasti MFT */
        fseek(fw, sizeof(struct boot_record), SEEK_SET);

        mfti = malloc(sizeof(struct mft_item));

        mfti->uid = 0;
        mfti->isDirectory = 1;
        mfti->item_order = 1;
        mfti->item_order_total = 1;
        strcpy(mfti->item_name, "ROOT_DIR");
        mfti->item_size = 1; // je tam backlink

        // zapisu prvni "plny" fragment (ROOT_DIR)
        mfti->fragments[0].fragment_start_address = data_start; // start adresa ve VFS
        mfti->fragments[0].fragment_count = 1; // pocet clusteru ve VFS od startovaci adresy

        // dalsi fragmenty z budou jen prazdne (pro poradek)
        // zacinam od jednicky
        for (i = 1; i < MFT_FRAG_COUNT; i++){
            mfti->fragments[i].fragment_start_address = -1;
            mfti->fragments[i].fragment_count = -1;
        }

        fwrite(mfti, sizeof(struct mft_item), 1, fw);
        free((void *) mfti);

        /* Tady bychom meli zapsat obsah ROOT_DIRU */
        /* Zapiseme odkaz na nadrazeny adresar */
        fseek(fw, data_start, SEEK_SET);
        char odkaz[2];
        odkaz[0] = '0';
        odkaz[1] = '\0';
        fwrite(odkaz, 1, 2, fw);

        fclose(fw);
    }
}
