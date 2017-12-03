#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "loader.h"
#include "boot_record.h"
#include "mft.h"
#include "parametr.h"

/* Nacte NTFS ze souboru */
void *create_example(void *arg){
    printf("EXAMPLER starting...\n");

    int bitmapa[10], bitmapa2[10];
    int i, sizeof_mft_item;
    char str[5], pomocny[11], obsah_clusteru[1024];
    struct boot_record *br, *br2;
    struct mft_fragment mftf;
    struct mft_item *mfti;

    FILE *file, *file2;
    sdilenaPamet *param = (sdilenaPamet *) arg;

    printf("\tOteviram pro zapis soubor %s\n", param->soubor);
    file = fopen(param->soubor, "wb");
    if(file != NULL){
        // zapisu boot record
        br = malloc(sizeof(struct boot_record));
        strcpy(br->signature, "Hubacek");
        strcpy(br->volume_descriptor, "Puvodni vytvoreni 3.12.2017");
        br->disk_size = 1024 * 10;
        br->cluster_size = 1024;
        br->cluster_count = 10;
        br->mft_start_address = 288;
        br->bitmap_start_address = 2840 + 288;
        br->data_start_address = 2840 + 288 + 40;
        br->mft_max_fragment_count = 1;

        fwrite(br, sizeof(struct boot_record), 1, file);

        // zapisu mft
        for(i = 0; i < 10; i++){
           sprintf(str, "%d", i);

           mfti = malloc(sizeof(struct mft_item));

           mftf.fragment_start_address = 288 + 2840 + 40;
           mftf.fragment_count = 1;

           mfti->uid = i;
           mfti->isDirectory = 0;
           mfti->item_order = 1;
           mfti->item_order_total = 1;
           strcpy(pomocny, "");
           strcat(pomocny, "soubor");
           strcat(pomocny, str);
           strcat(pomocny, ".txt\0");
           strcpy(mfti->item_name, pomocny);
           mfti->item_size = 100;
           mfti->fragments[0] = mftf;

           fwrite(mfti, sizeof(struct mft_item), 1, file);
           free((void *) mfti);
        }

        // zapisu bitmapu - posunu se na zacatek oblasti pro bitmapu
        fseek(file, br->bitmap_start_address, SEEK_SET);
        for (i=0; i < br->cluster_count; i++){

            bitmapa[i] = 0;
            if (i % 2 == 0){
                bitmapa[i] = 1;
            }
        }
        fwrite(bitmapa, sizeof(bitmapa[i]), br->cluster_count, file);

        // zapisu init VFS
        for (i=0; i < br->cluster_count; ++i){
            fseek(file, br->data_start_address + i * br->cluster_size, SEEK_SET);

            sprintf(str, "%d", i);
            strcpy(pomocny, "");
            strcat(pomocny, "obsah souboru ");
            strcat(pomocny, str);
            fwrite(pomocny, sizeof(pomocny), br->cluster_count, file);
        }

        // uvolnim pamet
        free((void *) br);
        fclose(file);
    }
    printf("\t\t -> Zapsano %lu bajtu (boot record)\n", sizeof(struct boot_record));
    printf("\t\t -> Zaviram soubor %s\n", param->soubor);


    // ted to zkusime precist reverznim inzenyrstvim
    // prvni musim precit boot record protoze o zbytk nemam ani paru
    printf("\t! Pokusim se nacist NTFS soubor z disku !\n");
    printf("\tOteviram pro cteni soubor %s\n", param->soubor);

    br2 = malloc(sizeof(struct boot_record));
    file2 = fopen(param->soubor, "rb");
    if (file2 != NULL) {
        fread(br2, sizeof(struct boot_record), 1, file2);

        printf("\tBOOT RECORD:\n");
        printf("\t\tsignature: %s\n", br2->signature);
        printf("\t\tdesc: %s\n", br2->volume_descriptor);
        printf("\t\tCelkova velikost VFS: %d\n", br2->disk_size);
        printf("\t\tVelikost jednoho clusteru: %d\n", br2->cluster_size);
        printf("\t\tPocet clusteru: %d\n", br2->cluster_count);

        printf("\t\tAdresa pocatku mft: %d\n", br2->mft_start_address);
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

        printf("\t\tAdresa pocatku bitmapy: %d\n", br2->bitmap_start_address);
            fseek(file2, br2->bitmap_start_address, SEEK_SET);
            fread(bitmapa2, sizeof(bitmapa2[i]), br2->cluster_count, file2);
            for(i = 0; i < br->cluster_count; i++){
                printf("\t\t\t[%d]: %d\n", i, bitmapa2[i]);
            }

        printf("\t\tAdresa pocatku datovych bloku: %d\n", br2->data_start_address);
            for (i=0; i < br->cluster_count; ++i){
                fseek(file2, br2->data_start_address + i * br2->cluster_size, SEEK_SET);
                fread(obsah_clusteru, sizeof(obsah_clusteru), 1, file2);
                printf("\t\t\t[%d]: %s\n", i, obsah_clusteru);
            }

        printf("\t\tMAX frag count: %d\n", br2->mft_max_fragment_count);

        // uvolnim pamet
        free((void *) br2);
        fclose(file2);
    }
    printf("\t\t -> Zaviram soubor %s\n", param->soubor);


    /*
    printf("MFT item: %lu bajtu\n", sizeof(struct mft_item));
    printf("MFT fragment: %lu bajtu\n", sizeof(struct mft_fragment));
    printf(": 32t: %lu , 8t: %lu , int: %lu :\n", sizeof(int32_t), sizeof(int8_t), sizeof(int));
    printf("a %lu", sizeof(a));
    */


    printf("EXAMPLER ending\n");
    return NULL;
}
