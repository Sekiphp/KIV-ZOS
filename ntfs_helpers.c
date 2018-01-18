#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mft.h"
#include "ntfs_helpers.h"
#include "loader.h"
#include "shell_functions.h"
#include "boot_record.h"
#include "functions.h"
#include "parametr.h"

extern int pwd;
extern char output_file[100];


/*
    Ziska data z jednoho clusteru
    @param adresa Adresa pocatku datoveho bloku
    @return Obsah bloku
*/
char* get_cluster_content(int32_t adresa) {
    FILE *fr;
    char *ret;

    ret = (char*) malloc(CLUSTER_SIZE);

    fr = fopen(output_file, "rb");
    if (fr != NULL) {
        fseek(fr, adresa, SEEK_SET);
        fread(ret, sizeof(char), CLUSTER_SIZE, fr);

        fclose(fr);
    }

    return ret;
}

/*
    Prepise obsah clusteru
    @param adresa Adresa pocatku datoveho bloku
    @param obsah Novy obsah bloku
    @return Zaporne hodnoty jsou chybne
*/
int set_cluster_content(int32_t adresa, char *obsah) {
    FILE *f;

    f = fopen(output_file, "r+b");
    if (f != NULL) {
        fseek(f, adresa, SEEK_SET);
        fwrite(obsah, 1, CLUSTER_SIZE, f);

        fclose(f);
        return 1;
    }

    return -1;
}

/*
    Ziska obsah vsech fragmentu patricich do clusteru
    @param fragment Struktura fragmentu, kterou chceme cist
    @return Obsah celeho fragmentu
*/
char* get_fragment_content(struct mft_fragment fragment) {
    int adresa, bloku, i;
    char *ret;

    adresa = fragment.fragment_start_address;
    bloku = fragment.fragment_count;
    ret = (char*) malloc(bloku * CLUSTER_SIZE);

    if (adresa != 0) {
        for (i = 0; i < bloku; i++) {
            strcat(ret, get_cluster_content(adresa));

            adresa = adresa + CLUSTER_SIZE;
        }
    }

    return ret;
}

/*
    Ziska obsah celeho souboru - precte si vsechny udaje z MFTLISTU (MFTI, MFTF)
    @param file_uid UID souboru, ktery chceme cist
    @return Obsah celeho souboru
*/
char* get_file_content(int file_uid) {
    int i, j, k;
    char *ret;
    MFT_LIST* mft_itemy;
    struct mft_item mfti;
    struct mft_fragment mftf;

    // alokujeme si zakladni velikost pro jeden cluster
    ret = (char*) malloc(CLUSTER_SIZE);


    if (mft_seznam[file_uid] != NULL){
        mft_itemy = mft_seznam[file_uid];

        printf("Je tu alespon jeden item co stoji za zminku %d\n", mft_itemy->ij);

        // projedeme vsechny itemy pro dane UID souboru
        // bylo by dobre si pak z tech itemu nejak sesortit fragmenty dle adres
        // zacneme iterovar pres ->dalsi
        i = 0;
        k = 0; // celkovy pocet zopracovanych neprazdnych fragmentu
        while (mft_itemy != NULL){
            mfti = mft_itemy->item;
            i++;

            printf("[%d] Nacteny item s UID=%d ma nazev %s\n", i, mfti.uid, mfti.item_name);

            // precteme vsechny fragmenty z daneho mft itemu (je jich: MFT_FRAG_COUNT)
            for (j = 0; j < MFT_FRAG_COUNT; j++){
                mftf = mfti.fragments[j];

                if (mftf.fragment_start_address != 0) {
                    k++;
                    printf("-- Fragment %d ze souboru s UID %d, start=%d, count=%d\n", j, mfti.uid, mftf.fragment_start_address, mftf.fragment_count);

                    // prubezne je potreba realokovat oblast tak, aby se mi podarilo nacist cely soubor
                    if (k != 1){
                        int *tmp = realloc(ret, k * CLUSTER_SIZE);
                        if (tmp == NULL) return "ERROR";
                    }

                    strcat(ret, get_fragment_content(mftf));
                }
            }

            // prehodim se na dalsi prvek
            mft_itemy = mft_itemy->dalsi;
        }
    }

    return ret;
}

/*
 !!! REFAKTOTING NUTNY !!!
 */
int append_file_content(int file_uid, char *append){
    int i, j, adresa;
    char *ret;
    ret = (char *) malloc(CLUSTER_SIZE);
    MFT_LIST* mft_item_chceme;
    struct mft_fragment mftf;
    char *soucasny_obsah = get_file_content(file_uid);
    FILE *fw;

    printf("Soucasny obsh souboru je: %s a ma delku %zd --- \n", soucasny_obsah, strlen(soucasny_obsah));
    printf("Chci appendnout: %s\n", append);

    i = strlen(soucasny_obsah);
    fw = fopen(output_file, "r+b");
    if (fw != NULL) {
        // musim si vypocitat adresu, kam budu zapisovat
        adresa = 0;
        if (mft_seznam[file_uid] != NULL){
            mft_item_chceme = mft_seznam[file_uid];

            // projedeme vsechny itemy pro dane UID souboru
            // bylo by dobre si pak z tech itemu nejak sesortit fragmenty dle adres
            // zacneme iterovar pres ->dalsi
            i = 0;
            while (mft_item_chceme != NULL){
                i++;
                printf("[%d] Nacteny item s UID=%d ma nazev %s\n", i, mft_item_chceme->item.uid, mft_item_chceme->item.item_name);

                // precteme vsechny fragmenty z daneho mft itemu (maximalne je jich: MFT_FRAG_COUNT)
                for (j = 0; j < MFT_FRAG_COUNT; j++){
                    mftf = mft_item_chceme->item.fragments[j];

                    // najdu si posledni fragment s adresou
                    if (mftf.fragment_start_address != 0) {
                        adresa = mftf.fragment_start_address;
                    }
                }

                // prehodim se na dalsi prvek
                mft_item_chceme = mft_item_chceme->dalsi;
            }
        }

        if (adresa != 0){
            // nactu obsah daneho clusteru
            fseek(fw, adresa, SEEK_SET);
            strcat(ret, get_cluster_content(adresa));

            // pripojim k nemu co potrebuji a zapisu
            fseek(fw, adresa, SEEK_SET);
            strcat(ret, append);
            strcat(ret, "\n");
            fwrite(ret, 1, strlen(ret), fw);

            printf("Dokoncuji editaci clusteru/fragmentu; strlen=%zd\n", strlen(ret));
        }
        else{
            return -1;
        }

        fclose(fw);
    }


    return -1;
}