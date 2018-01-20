#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "loader.h"
#include "mft.h"
#include "shell_functions.h"
#include "ntfs_helpers.h"
#include "boot_record.h"
#include "functions.h"

extern int pwd;
extern char output_file[100];

void func_cp(char *cmd){
    while((cmd = strtok(NULL, " ")) != NULL){
        printf("Ostatni: %s\n", cmd);
    }
}


void func_mv(char *cmd){
    while((cmd = strtok(NULL, " ")) != NULL){
        printf("Ostatni: %s\n", cmd);
    }
}



void func_rm(char *cmd){
    int ret;

    cmd = strtok(NULL, " \n");

    ret = parsuj_pathu(cmd, 1);

    printf("RET %d", ret);

    if (ret == -1){
        printf("PATH NOT FOUND\n");
        return;
    }

    if (mft_seznam[ret]->item.isDirectory == 1){
        printf("NOT A FILE\n");
        return;
    }

    delete_file(ret);
    printf("OK\n");
}


/* validni: mkdir neco, mkdir /var/www/neco, ale /var/www uz musi existovat */
void func_mkdir(char *cmd){
    int ret;

    // zpracujeme si zadanou cestu
    cmd = strtok(NULL, " \n");
    if (cmd == NULL){
        printf("PATH NOT FOUND\n");
        return;
    }
    else {
        ret = parsuj_pathu(cmd, 0);
    }

    // zkusim si tu cestu projit
    if (ret == -1){
        printf("PATH NOT FOUND\n");
        return;
    }
    else {
        // --- zde vytvorime slozku ---
        // dle bitmapy najdu prvni volny cluster a vypoctu si jeho adresu, fragment_count zvolim na 1
        // do prvniho fragmentu polozky mft_seznam[ret]->item zapisu nakonec UID noveho adresare
        zaloz_novou_slozku(ret, cmd);
    }

    printf("ls ret = %d\n", ret);
}


/*
    Smaze prazdny adresar
*/
void func_rmdir(char *cmd){
    int ret, i;
    char buffer[CLUSTER_SIZE];

    cmd = strtok(NULL, " \n");

    ret = parsuj_pathu(cmd, 1);

    printf("RET %d", ret);

    if (ret == -1){
        printf("PATH NOT FOUND\n");
        return;
    }

    if (mft_seznam[ret]->item.isDirectory == 0){
        printf("NOT A DIRECTORY\n");
        return;
    }

    if (is_empty_dir(ret) == 0){
        printf("NOT EMPTY\n");
        return;
    }

    delete_file(ret);

    // odstranim odkaz z nadrazeneho adresare
    char *soucasny_obsah = get_file_content(pwd);
    printf("soucasnost=%s\n", soucasny_obsah);

    char *curLine = soucasny_obsah;

    // obsah clusteru daneho adresare si ctu po radcich - co jeden radek to UID jednoho souboru nebo slozky
    i = 0;
    strcpy(buffer, "");
    while (curLine){
        char * nextLine = strchr(curLine, '\n');
        if (nextLine) *nextLine = '\0';  // temporarily terminate the current line

        if (atoi(curLine) != ret){
            if (i != 0)
                strcat(buffer, "\n");

            strcat(buffer, curLine);
        }
        //printf("CURLINE = %s\n", curLine);

        if (nextLine) *nextLine = '\n';  // then restore newline-char, just to be tidy
        curLine = nextLine ? (nextLine + 1) : NULL;
        i++;
    }

    printf("BUFÍK=%s\n", buffer);
    // UID se musi zachovat kvuli linkum
    edit_file_content(pwd, buffer, mft_seznam[ret]->item.item_name, mft_seznam[ret]->item.uid);

    printf("OK\n");
}


/* ls /var/www/neco je validni prikaz */
void func_ls(char *cmd){
    int ret;

    // zkusim si tu cestu projit
    cmd = strtok(NULL, " \n");
    if (cmd == NULL){
        ret = parsuj_pathu("", 1);
    }
    else {
        ret = parsuj_pathu(cmd, 1);
    }

    // cesta neexistuje, nelze splnit pozadavek
    if (ret == -1){
        printf("PATH NOT FOUND\n");
        return;
    }

    ls(ret);

    //printf("ls ret = %d\n", ret);
}

/*
    Vypise obsah souboru
*/
void func_cat(char *cmd){
    int ret;

    cmd = strtok(NULL, " \n");

    ret = parsuj_pathu(cmd, 1);
    printf("vyparsovano: %d\n", ret);

    // cesta neexistuje, nelze splnit pozadavek
    if (ret == -1){
        printf("FILE NOT FOUND\n");
        return;
    }

    // je posledni uid soubor a ne slozka
    if (mft_seznam[ret]->item.isDirectory == 1){
        printf("FILE NOT FOUND\n");
        return;
    }

    printf("%s\n", get_file_content(ret));
}


/*
    Posun v adresarich
*/
void func_cd(char *cmd){
    int kam;

    cmd = strtok(NULL, " \n");
    printf("_%s_%zd\n", cmd, strlen(cmd));

    kam = parsuj_pathu(cmd, 1);

    if (kam != -1){
        pwd = kam;
        printf("OK\n");
        printf("-- menim kurzor pwd: %d\n", kam);
    }
    else {
        printf("PATH NOT FOUND\n");
        return;
    }
}


/*
    Print working directory
*/
void func_pwd(){
    char link[20], full_link[200], pom[200];
    int link_int;

    if (pwd >= 0) {
        if (pwd == 0) {
            printf("/\n");
        }
        else {
            link_int = pwd;

            while(link_int > 0){
                // /alservis, /www, /var
                strcpy(link, "/");
                strcat(link, mft_seznam[link_int]->item.item_name);

                // to co uz ve stringu je dam na konec
                strcpy(pom, full_link);
                strcpy(full_link, link);
                strcat(full_link, pom);

                link_int = get_backlink(link_int);
            }

            printf("%s\n", full_link);
        }
    }
    //printf("PWD = %d\n", pwd);
}


/*
    Vypise informace z FS o danem souboru
*/
void func_info(char *cmd){
    int ret, j, k, adr;
    struct mft_item mfti;
    MFT_LIST* mft_itemy;
    struct mft_fragment mftf;

    cmd = strtok(NULL, " \n");
    ret = parsuj_pathu(cmd, 1);

    if (ret == -1) {
        printf("FILE NOT FOUND\n");
        return;
    }

    mfti = mft_seznam[ret]->item;

    printf("NAME - UID - SIZE\n");
    printf("%s - %d - %d\n", mfti.item_name, mfti.uid, mfti.item_size);

    printf("FRAGMENTY & CLUSTERY:\n");

    if (mft_seznam[ret] != NULL){
        mft_itemy = mft_seznam[ret];

        // projedeme vsechny itemy pro dane UID souboru
        k = 0; // celkovy pocet zopracovanych neprazdnych fragmentu
        while (mft_itemy != NULL){
            mfti = mft_itemy->item;

            // precteme vsechny fragmenty z daneho mft itemu (je jich: MFT_FRAG_COUNT)
            for (j = 0; j < MFT_FRAG_COUNT; j++){
                mftf = mfti.fragments[j];

                if (mftf.fragment_start_address != 0 && mftf.fragment_count > 0) {
                    k++;
                    adr = (mftf.fragment_start_address - bootr->data_start_address) / bootr->cluster_size;
                    printf("-- Fragment start=%d, count=%d, clusterID=%d\n", mftf.fragment_start_address, mftf.fragment_count, adr);
                }
            }

            // prehodim se na dalsi prvek
            mft_itemy = mft_itemy->dalsi;
        }
    }

    printf("Pocet fragmentu: %d\n", k);
}



void func_incp(char *cmd){
    int i, ret, delka;
    FILE *f;
    char pc_file[100];
    char *nazev;
    char *jen_cesta;

    i = 0;

    // postupne cteni argumentu
    while((cmd = strtok(NULL, " \n")) != NULL){
        if (i == 0){
            // soubor k presunu z pocitace
            // overim jeho existenci
            strncpy(pc_file, cmd, strlen(cmd));
            f = fopen(pc_file, "r");
            if (f == NULL){
                printf("FILE %s NOT FOUND\n", pc_file);
                return;
            }
        }
        else {
            // najdu cilove misto pro ulozeni

            // pripravim si cestu a nazev souboru pro vytvoreni
            nazev = strrchr(cmd, '/');
            if (nazev != NULL) {
                nazev++;

                delka = strlen(cmd) - strlen(nazev);
                jen_cesta = (char *) malloc(delka);
                strncpy(jen_cesta, cmd, delka - 1);
            }
            else {
                delka = strlen(cmd);
                nazev = (char *) malloc(delka);
                jen_cesta = (char *) malloc(delka);
                strncpy(nazev, cmd, delka);
                strncpy(jen_cesta, "/", 1);
            }

            printf("-- Full path: %s\n-- Filename: %s\n-- Path to dir: %s\n", cmd, nazev, jen_cesta);

            ret = parsuj_pathu(jen_cesta, 1);
            if (ret == -1){
                printf("PATH %s NOT FOUND\n", jen_cesta);
                return;
            }
        }

        i++;
    }

    if (i != 2) {
        printf("TOO FEW ARGS\n");
        return;
    }

    // tady uz mohu bezpecne zpracovavat
    printf("-- Vyparsovana cesta: %d\n", ret);

    vytvor_soubor(ret, nazev, read_file_from_pc(pc_file), -1);
}



void func_outcp(char *cmd){
    while((cmd = strtok(NULL, " \n")) != NULL){
        printf("Ostatni: %s\n", cmd);
    }
}


void func_defrag(){

}

void func_consist(){

}
