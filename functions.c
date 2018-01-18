#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "loader.h"
#include "mft.h"
#include "shell_functions.h"
#include "boot_record.h"
#include "functions.h"
#include "parametr.h"
#include "ntfs_helpers.h"

extern int pwd;
extern char output_file[100];

/*
    Prelozi jmeno adresare na UID
     -> nacte si obsah clusteru soucasneho adresare a prochazi vsechny tyto adresare a hleda shodnost jmena
    @param dir_name Jmeno adresare pro preklad
    @param uid_pwd Soucasna slozka, kde se nachazim (ze ktere vychazim)
*/
int get_uid_by_name(char *dir_name, int uid_pwd){
    struct mft_item mfti;
    int hledane, i;

    char *obsah = get_file_content(uid_pwd);
    char *curLine = obsah;

    printf("get_uid_by_name(dir_name = %s, uid_pwd = %d)\n\tObsah clusteru: %s \n----------\n", dir_name, uid_pwd, obsah);

    // obsah clusteru daneho adresare si ctu po radcich - co jeden radek to UID jednoho souboru nebo slozky
    i = 0;
    while (curLine){
        char * nextLine = strchr(curLine, '\n');
        if (nextLine) *nextLine = '\0';  // temporarily terminate the current line

        if (i != 0){
            // skip prvni radky v clusteru - je tam backlink
            hledane = atoi(curLine);
            printf("\tnactene UID z clusteru %s (int=%d)\n", curLine, hledane);

            // tady si roparsuji MFT a zjistim jestli se shoduje nazev
            if (hledane < CLUSTER_COUNT && mft_seznam[hledane] != NULL){
                mfti = mft_seznam[hledane]->item;

                printf("\t\tHledane mfti s uid=%d (name=%s) %s NOT NULL\n", hledane, mfti.item_name, dir_name);

                if (strcmp(mfti.item_name, dir_name) == 0 && mfti.isDirectory == 1) {
                    printf("\t\tSHODA\n");
                    return mfti.uid;
                }
            }
        }
        else {
            printf("\tBacklink teto slozky je %s\n", curLine);
        }

        if (nextLine) *nextLine = '\n';  // then restore newline-char, just to be tidy
        curLine = nextLine ? (nextLine + 1) : NULL;

        i++;
    }

    return -1;
}

int is_name_unique(char *newname, int uid_pwd){
    if (get_uid_by_name(newname, uid_pwd) == -1) {
        return 1;
    }

    return 0;
}

/* Prochazi danou cestu a vrati UID slozky, ktera je posledni nebo -1 pri chybe */
int parsuj_pathu(char *patha, int cd){
    char *p_c;
    int start_dir, uid_pom;
    char path[100];

    // Nelze pracovat primo s arg: https://stackoverflow.com/questions/8957829/strtok-segmentation-fault
    strncpy(path, patha, 100); // bez \0

    // zacinam v rootu ci nikoliv?
    if (strncmp(path, "/", 1) == 0){
        start_dir = 0;
    }
    else {
        start_dir = pwd;
    }
    printf("START DIR = %d\n", start_dir);

if(strcmp(patha, "") != 0) {
    if (strchr(patha, '/') != NULL){
        // zde parsuji cestu zacinajici lomenem
        // parsuji jednotlive casti cesty a norim se hloubeji a hloubeji
        p_c = strtok(path, "/");
        if (p_c != NULL){
            uid_pom = get_uid_by_name(p_c, start_dir); // pokusim se prevest nazev na UID

            printf("get_uid_by_name(%s, %d) = %d\n", p_c, start_dir, uid_pom);
            if (uid_pom == -1) return -1;
            start_dir = uid_pom; // jdu o slozku niz
        }
        while((p_c = strtok(NULL, "/")) != NULL){
            uid_pom = get_uid_by_name(p_c, start_dir); // pokusim se prevest nazev na UID

            printf("get_uid_by_name(%s, %d) = %d\n", p_c, start_dir, uid_pom);
            if (uid_pom == -1) return -1;
            start_dir = uid_pom; // jdu o slozku niz
        }
    }
    else {
	if (cd == 1) {
            // pouziva ce pro prikaz cd
            printf("V ceste neni /\n");
            uid_pom = get_uid_by_name(patha, start_dir); // pokusim se prevest nazev na UID

            if (uid_pom == -1) return -1;
            start_dir = uid_pom;
        }
    }
}
    return start_dir;
}


/* Vytvori novou slozku o zadanem nazvu */
int zaloz_novou_slozku(int pwd, char *name){
    int i, j, bitmap_free_index, new_cluster_start;
    FILE *fw;
    struct mft_item mfti;
    struct mft_fragment mftf;
    struct mft_item *mpom;
    char pomocnik[20], pomocnik2[5], pom[5];

    memset(pomocnik, ' ', 20);
    strncpy(pomocnik, name, strlen(name)-1);

    if (is_name_unique(pomocnik, pwd) != 1){
        printf("EXIST\n");
        return -1;
    }

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


                // dalsi fragmenty z budou jen prazdne (pro poradek)
                mftf.fragment_start_address = 0;
                mftf.fragment_count = 0;

                // zacinam od jednicky
                for (j = 1; j < MFT_FRAG_COUNT; j++){
                    mfti.fragments[j] = mftf;
                }

                // prvek mam pripraveny
                // zaktualizuji si globalni pole a bitmapu
                ntfs_bitmap[bitmap_free_index] = 1;
                pridej_prvek(bitmap_free_index, mfti);

                // zapisu do souboru
                fw = fopen(output_file, "r+b");
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
                    append_file_content(pwd, pomocnik2);

                    // odkaz na nadrazenou slozku do teto slozky - backlink
                    // budou to prvni zapsana data v teto slozce
                    printf("-- Zapisuji backlink na adresar %d do adresare %d, adresa je %d\n", pwd, bitmap_free_index, bootr->data_start_address + bitmap_free_index * CLUSTER_SIZE);
                    sprintf(pom, "%d", pwd);
                    fseek(fw, bootr->data_start_address + bitmap_free_index * CLUSTER_SIZE, SEEK_SET);
                    fwrite(pom, 1, strlen(pom), fw);

                    fclose(fw);
                }

                //free((void *) mfti);
                break;
            }
        }
    }

    return bitmap_free_index;
}

/* Ziskani informaci o souborech ve slozce */
void ls(int uid) {
    char buffer[1024];
    char *p_c;
    int i = 0;

    // chci vypsat obsah aktualniho adresare
    strncpy(buffer, get_file_content(uid), 1024);
    // printf("obsah bufferu je: %s\n", buffer);

    printf("-- Napoveda: + slozka, - soubor --\n");
    printf("--- NAZEV ----- VELIKOST - UID ---\n");

    // iteruji pro kazdou polozku z adresare a hledam jeji nazev
    p_c = strtok(buffer, "\n");
    printf("ID nadrazene slozky je %s\n", p_c);
    /* prvni odkaz je odkaz na nadrazenou slozku
    if (p_c != NULL){
        ls_printer(p_c);
        i++;
    }*/
    while((p_c = strtok(NULL, "\n")) != NULL){
        ls_printer(p_c);
        i++;
    }

    printf("-- Celkem souboru: %d --\n", i);
}

/* Pro konkretni UID vypise info o souboru (prikaz ls) */
void ls_printer(char *p_c) {
    struct mft_item mfti;

    mfti = mft_seznam[atoi(p_c)]->item;

    printf(" ");
    if (mfti.isDirectory == 1){
        printf("+");
    }
    else{
        printf("-");
    }
    printf(" %-15s %-7d %d\n", mfti.item_name, mfti.item_size, mfti.uid);
}



void vytvor_soubor_z_pc(int cilova_slozka, char *pc_soubor){
 /*   int i, j, k, size, ret, potreba_clusteru, adresa;
    char * result;
    FILE *f;
    char pom[100], buffer[CLUSTER_SIZE];

    i = 0;
    size = 0;

    while((cmd = strtok(NULL, " ")) != NULL){
        if (i == 0){
            // zpracovavam prvni argument - najdu v PC

            strncpy(pom, cmd, strlen(cmd));
            f = fopen(pom, "r");
            if (f == NULL){
                printf("FILE %s NOT FOUND\n", pom);
                return; // -1 means file opening fail
            }

            fseek(f, 0, SEEK_END);
            size = ftell(f);
            printf("size=%d\n", size);

            fseek(f, 0, SEEK_SET);
            result = (char *)malloc(size+1);
            if (size != fread(result, sizeof(char), size, f))
            {
                printf("OPENING FILE ERROR\n");
                free((void *) result);
                return; // -2 means file reading fail
            }
            fclose(f);

            // hledam volne clustery v bitmape
            potreba_clusteru = size / CLUSTER_SIZE + 1;
            int volne_clustery[potreba_clusteru];

            printf("-- Je potreba %d volnych clusteru\n", potreba_clusteru);

            k = 0;
            for (j = 0; j < CLUSTER_COUNT; j++) {
                if (ntfs_bitmap[j] == 0) {
                    // volna
                    volne_clustery[k] = j;
                    k++;
                }

                if (k == potreba_clusteru) {
                    break;
                }
            }

            if (k != potreba_clusteru){
                printf("ERROR - NOT ENOUGH CLUSTERS (%d)\n", k);
                return;
            }

printf("OK\n");

            FILE *fw;
            fw = fopen(output_file, "r+b");
            if(fw != NULL){
                // aktualizuji bitmapu v souboru
                // + zapnim virtualni clustery (nactene ze souboru)
                for (j = 0; j < k; j++){
                    ntfs_bitmap[volne_clustery[j]] = 1;
                }
                fseek(fw, bootr->bitmap_start_address, SEEK_SET);
                fwrite(ntfs_bitmap, 4, CLUSTER_COUNT, fw);

                // reseni spojitosti a nespojitosti bitmapy
                int spoj_len = 1;
                int starter = 0;

                for(j = 0; j < potreba_clusteru; j++){
                    printf("%d: spojity: %d ?= %d\n", i, volne_clustery[j+1], volne_clustery[j]+1);
                    if(volne_clustery[j+1] == volne_clustery[j]+1){
                        spoj_len = spoj_len + 1;

                        if (spoj_len == 2){
                            starter = volne_clustery[j];
                            printf("\t starter = %d\n", starter);
                        }
                    }
                    else{
                        if(spoj_len != 1){
                            printf("Muzu zpracovat spojity blok, ktery zacina na %d a je dlouhy %d\n", starter, spoj_len);
                        }

                        spoj_len = 1;
                        starter = 0;
                    }
                }

                if(spoj_len != 1){
                    printf("Muzu zpracovat spojity blok, ktery zacina na %d a je dlouhy %d\n", starter, spoj_len);
                }

                // prace s mft
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
                fw = fopen(output_file, "r+b");
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

                // aktualizuji virtualni mft

                // aktualizuji mft v souboru

                // zapisu obsah clusteru do souboru
                // v result muzu mit třeba 5000 znaku, tj rozdelovat po CLUSTER_SIZE
                for (j = 0; j < potreba_clusteru; j++){
                    adresa = bootr->data_start_address + volne_clustery[j] * CLUSTER_SIZE;
                    printf("-- Zapisuji na adresu %d\n", adresa);

                    strncpy(buffer, result + (j * CLUSTER_SIZE), CLUSTER_SIZE);
                    buffer[CLUSTER_SIZE] = '\0';

                    set_cluster_content(adresa, buffer);
                }

                fclose(fw);
            }

            printf("%s\n", result);
        }
        else {
            // najdu cilove misto pro ulozeni
            printf("Cesta k parsovani je: --%s--\n", cmd);

            ret = parsuj_pathu(cmd);
        }

        i++;
    }
*/
}
