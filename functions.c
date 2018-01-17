#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "loader.h"
#include "mft.h"
#include "shell_functions.h"
#include "boot_record.h"
#include "functions.h"

extern int pwd;

/* Ziska obsah vsech danych clusteru, ktere nalezi stejnemu fragmentu - Jeden mfti muze mit vsak mnoho fragmentu */
char* get_cluster_content(int32_t fragment_start_addr, int32_t fragments_count){
    int sirka_bloku = CLUSTER_SIZE * fragments_count;
    char *ret;
    ret = (char*) malloc(sirka_bloku);
    FILE *fr;

    // todo: filename udelat globalni
    fr = fopen("ntfs.dat", "rb");
    if (fr != NULL) {
        fseek(fr, fragment_start_addr, SEEK_SET);
        fread(ret, sizeof(char), sirka_bloku, fr);

        fclose(fr);
    }

    return ret;
}

int append_obsah_souboru(int uid, char *append){
    int i, j, adresa;
    char *ret;
    ret = (char *) malloc(CLUSTER_SIZE);
    MFT_LIST* mft_item_chceme;
    struct mft_fragment mftf;
    char *soucasny_obsah = get_mft_item_content(uid);
    FILE *fw;

    printf("Soucasny obsh souboru je: %s a ma delku %d --- \n", soucasny_obsah, strlen(soucasny_obsah));
    printf("Chci appendnout: %s\n", append);

    i = strlen(soucasny_obsah);
    fw = fopen("ntfs.dat", "r+b");
    if (fw != NULL) {
        // musim si vypocitat adresu, kam budu zapisovat
        adresa = 0;
        if (mft_seznam[uid] != NULL){
            mft_item_chceme = mft_seznam[uid];

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
            strcat(ret, get_cluster_content(adresa, 1));

            // pripojim k nemu co potrebuji a zapisu
            fseek(fw, adresa, SEEK_SET);
            strcat(ret, append);
            strcat(ret, "\n");
            fwrite(ret, 1, strlen(ret), fw);

            printf("Dokoncuji editaci clusteru/fragmentu; strlen=%d\n", strlen(ret));
        }
        else{
            return -1;
        }

        fclose(fw);
    }


    return -1;
}

/* Ziska obsah vsech fragmentu pro soubor nebo slozku daneho UID */
char* get_mft_item_content(int uid){
    int i, j, k;
    char *ret = malloc(CLUSTER_SIZE);
    MFT_LIST* mft_item_chceme;
    struct mft_fragment mftf;

    if (mft_seznam[uid] != NULL){
        mft_item_chceme = mft_seznam[uid];

        //printf("je tu alespon jeden item co stoji za zminku %d\n", mft_item_chceme->ij);

        // projedeme vsechny itemy pro dane UID souboru
        // bylo by dobre si pak z tech itemu nejak sesortit fragmenty dle adres
        // zacneme iterovar pres ->dalsi
        i = 0;
        k = 0; // celkovy pocet zopracovanych neprazdnych fragmentu
        while (mft_item_chceme != NULL){
            i++;
            printf("pocet iteraci=%d\n", i);
            printf("Nacteny item s UID=%d ma nazev %s\n", mft_item_chceme->item.uid, mft_item_chceme->item.item_name);

            // precteme vsechny fragmenty z daneho mft itemu (je jich: MFT_FRAG_COUNT)
            for (j = 0; j < MFT_FRAG_COUNT; j++){
                mftf = mft_item_chceme->item.fragments[j];

                if (mftf.fragment_start_address != 0) {
                    k++;
                    printf("Zpracovavam fragment %d ze souboru s UID %d, start=%d, count=%d\n", j, mft_item_chceme->item.uid, mftf.fragment_start_address, mftf.fragment_count);

                    // prubezne je potreba realokovat oblast tak, aby se mi podarilo nacist cely soubor
                    if (k != 1){
                        int *tmp = realloc(ret, k * CLUSTER_SIZE);
                        if (tmp == NULL) return "ERROR";
                    }

                    strcat(ret, get_cluster_content(mftf.fragment_start_address, mftf.fragment_count));
                }
            }

            // prehodim se na dalsi prvek
            mft_item_chceme = mft_item_chceme->dalsi;
        }
    }

    return ret;
}

/*
Pokusi se v obsahu daneho adresare najit jiny adresar za pomoci jeho jmena
@return UID hledaneho adresare nebo -1 pri chybe
*/
int get_uid_by_name(char *dir_name, int uid_pwd){
    struct mft_item mfti;
    int hledane;

    char *obsah = get_mft_item_content(uid_pwd);
    char *curLine = obsah;

    printf("Spoustim metodu *get_uid_by_name* s dir_name = %s a uid_pwd = %d\n\tTato polozka ma obsah clusteru: %s \n----------\n", dir_name, uid_pwd, obsah);

    // obsah clusteru daneho adresare si ctu po radcich - co jeden radek to UID jednoho souboru nebo slozky
    while (curLine){
        char * nextLine = strchr(curLine, '\n');
        if (nextLine) *nextLine = '\0';  // temporarily terminate the current line

        hledane = atoi(curLine);
        printf("*** nactene UID z clusteru %s (int=%d)\n", curLine, hledane);

        // tady si roparsuji MFT a zjistim jestli se shoduje nazev
        if (hledane < CLUSTER_COUNT && mft_seznam[hledane] != NULL){
            mfti = mft_seznam[hledane]->item;

            printf("\tHledane mfti s uid=%d (name=%s) NOT NULL\n", hledane, mfti.item_name);

            if (strcmp(mfti.item_name, dir_name) == 0 && mfti.isDirectory == 1) {
                printf("\tSHODA\n");
                return mfti.uid;
            }
        }

        if (nextLine) *nextLine = '\n';  // then restore newline-char, just to be tidy
        curLine = nextLine ? (nextLine + 1) : NULL;
    }

    return -1;
}

/* Prochazi danou cestu a vrati UID slozky, ktera je posledni nebo -1 pri chybe */
int parsuj_pathu(char *patha){
    char *p_c;
    int start_dir, uid_pom;
    char path[100];
    char buffer[1024];
    struct mft_item mfti;

    // Nelze pracovat primo s arg: https://stackoverflow.com/questions/8957829/strtok-segmentation-fault
    strncpy(path, patha, 100); // bez \0

    // zacinam v rootu ci nikoliv?
    if (strncmp(path, "/", 1) == 0){
        start_dir = 0;
    }
    else {
        start_dir = pwd;
    }
    //printf("START DIR = %d\n", start_dir);

    if (strchr(patha, '/') != NULL){
        // zde parsuji cestu zacinajici lomenem
        // parsuji jednotlive casti cesty a norim se hloubeji a hloubeji
        p_c = strtok(path, "/");
        if (p_c != NULL){
            uid_pom = get_uid_by_name(p_c, start_dir); // pokusim se prevest nazev na UID

            //printf("get_uid_by_name(%s, %d) = %d\n", p_c, start_dir, uid_pom);
            if (uid_pom == -1) return -1;
            start_dir = uid_pom; // jdu o slozku niz
        }
        while((p_c = strtok(NULL, "/")) != NULL){
            uid_pom = get_uid_by_name(p_c, start_dir); // pokusim se prevest nazev na UID

            //printf("get_uid_by_name(%s, %d) = %d\n", p_c, start_dir, uid_pom);
            if (uid_pom == -1) return -1;
            start_dir = uid_pom; // jdu o slozku niz
        }
    }
    else {
        // chci vypsat obsah aktualniho adresare
        strncpy(buffer, get_mft_item_content(pwd), 1024);
        // printf("obsah bufferu je: %s\n", buffer);

        printf("Napoveda: + slozka, - soubor\n");

        // iteruji pro kazdou polozku z adresare a hledam jeji nazev
        p_c = strtok(buffer, "\n");
        if (p_c != NULL){
            //printf("atoi(%s)=%d\n", p_c, atoi(p_c));
            mfti = mft_seznam[atoi(p_c)]->item;

            if (mfti.isDirectory == 1){
                printf("+ %s\n", mfti.item_name);
            }
            else{
                printf("- %s\n", mfti.item_name);
            }
        }
        while((p_c = strtok(NULL, "\n")) != NULL){
            //printf("atoi(%s)=%d\n", p_c, atoi(p_c));
            mfti = mft_seznam[atoi(p_c)]->item;

            if (mfti.isDirectory == 1){
                printf("+ %s\n", mfti.item_name);
            }
            else{
                printf("- %s\n", mfti.item_name);
            }
        }
    }

    return start_dir;
}

/* Vytvori novou slozku o zadanem nazvu */
int zaloz_novou_slozku(int pwd, char *name){
    int i, bitmap_free_index, new_cluster_start;
    FILE *fw;
    struct mft_item mfti;
    struct mft_fragment mftf;
    struct mft_item *mpom;
    char pomocnik[20], pomocnik2[5];

    strncpy(pomocnik, name, strlen(name)-1);
    printf("-- NAME OF NEW DIR=%s\n", pomocnik);

    // najdu volnou bitmapu
    bitmap_free_index = -1;
    for (i = 0; i < CLUSTER_COUNT; i++){
        if (mft_seznam[i] == NULL){
            printf("MFT index je %d\n", i);
            bitmap_free_index = i;
            break;
        }
    }

    if (bitmap_free_index != -1){
        new_cluster_start = bootr->data_start_address + bitmap_free_index * CLUSTER_SIZE;

        // ziskam prvni volny MFT LIST a zjistim volne UID, ktere nasledne pridelim nove slozce
        for (i = 0; i < CLUSTER_COUNT; i++){
            if (mft_seznam[i] == NULL){
                // tento prvek je volny, vyuziji jej tedy
                mpom = malloc(sizeof(struct mft_item));

                mftf.fragment_start_address = new_cluster_start; // start adresa ve VFS
                mftf.fragment_count = 1;

                mfti.uid = bitmap_free_index;
                mfti.isDirectory = 1;
                mfti.item_order = 1;
                mfti.item_order_total = 1;
                strcpy(mfti.item_name, pomocnik);
                mfti.item_size = 0; // zatim tam nic neni, takze nula
                mfti.fragments[0] = mftf;

                // prvek mam pripraveny
                // zaktualizuji si globalni pole a bitmapu
                ntfs_bitmap[bitmap_free_index] = 1;
                pridej_prvek(bitmap_free_index, mfti);

                // zapisu do souboru
                // todo: filename
                fw = fopen("ntfs.dat", "r+b");
                if(fw != NULL){
                    // mfti
                    mpom = &mfti;
                    printf("-- MFTI chci zapsat na adresu %lu\n", sizeof(struct boot_record) + bitmap_free_index * sizeof(struct mft_item));
                    fseek(fw, sizeof(struct boot_record) + (bitmap_free_index) * sizeof(struct mft_item), SEEK_SET);
                    fwrite(mpom, sizeof(struct mft_item), 1, fw);

                    // bitmap
                    printf("-- Bitmapu chci zapisovat na adresu %u\n", bootr->bitmap_start_address);
                    fseek(fw, bootr->bitmap_start_address, SEEK_SET);
                    fwrite(ntfs_bitmap, 4, CLUSTER_COUNT, fw);

                    // odkaz na slozku do nadrazeneho adresare
                    printf("-- Zapisuji odkaz na adresar %d do adresare %d\n", bitmap_free_index, pwd);
                    sprintf(pomocnik2, "%d", bitmap_free_index);
                    append_obsah_souboru(pwd, pomocnik2);

                    fclose(fw);
                }

                //free((void *) mfti);
                break;
            }
        }
    }

    return bitmap_free_index;
}
