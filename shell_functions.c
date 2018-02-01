#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <math.h>

#include "debugger.h"
#include "parametr.h"
#include "loader.h"
#include "mft.h"
#include "shell_functions.h"
#include "ntfs_helpers.h"
#include "boot_record.h"
#include "functions.h"

extern int pwd;
extern char output_file[100];

/*
    Zkopiruje s1 do s2
*/
void func_cp(char *cmd) {
    char *part1, *part2, *nazev_zdroj, *nazev_cil, *jen_cesta_zdroj, *jen_cesta_cil;
    int delka, ret_zdroj, ret_cil, zdroj_uid;

    // part 1
    cmd = strtok(NULL, " \n");
    part1 = (char *) malloc(sizeof(strlen(cmd)));
    strcpy(part1, cmd);
    part1[strlen(cmd)] = '\0';

    DEBUG_PRINT("PART 1: %s=%s\n", cmd, part1);

    // part 2
    cmd = strtok(NULL, " \n");
    part2 = (char *) malloc(strlen(cmd));
    strcpy(part2, cmd);
    part2[strlen(cmd)] = '\0';

    DEBUG_PRINT("PART 2: %s=%s\n", cmd, part2);

    // part 1
            nazev_zdroj = strrchr(part1, '/');
            if (nazev_zdroj != NULL) {
                nazev_zdroj++;

                delka = strlen(part1) - strlen(nazev_zdroj);
                jen_cesta_zdroj = (char *) malloc(delka * sizeof(char *));
                strncpy(jen_cesta_zdroj, part1, delka);
                jen_cesta_zdroj[delka] = '\0';

                ret_zdroj = parsuj_pathu(jen_cesta_zdroj, 1);
            }
            else {
                delka = strlen(part1);

                nazev_zdroj = part1;

                jen_cesta_zdroj = (char *) malloc(1);
                strncpy(jen_cesta_zdroj, "/", 1);
                jen_cesta_zdroj[1] = '\0';

                ret_zdroj = pwd;
            }

            zdroj_uid = get_uid_by_name(nazev_zdroj, ret_zdroj);

            DEBUG_PRINT("-- Full path: %s(%d)\n-- Filename: %s\n-- Path to dir: %s\n", part1, delka, nazev_zdroj, jen_cesta_zdroj);
            DEBUG_PRINT("-- RET ZDROJ: %d\n", ret_zdroj);
            DEBUG_PRINT("-- ZDROJ UID: %d\n\n", zdroj_uid);

    if (ret_zdroj == -1 || zdroj_uid == -1) {
        printf("FILE NOT FOUND\n");
        return;
    }

    // part 2
            nazev_cil = strrchr(part2, '/');
            if (nazev_cil != NULL) {
                nazev_cil++;

                delka = strlen(part2) - strlen(nazev_cil);
                jen_cesta_cil = (char *) malloc(delka);
                strncpy(jen_cesta_cil, part2, delka);
                jen_cesta_cil[delka] = '\0';

                ret_cil = parsuj_pathu(jen_cesta_cil, 1);
            }
            else {
                delka = strlen(part2);

                nazev_cil = part2;

                jen_cesta_cil = (char *) malloc(1);
                strncpy(jen_cesta_cil, "/", 1);
                jen_cesta_cil[1] = '\0';

                ret_cil = pwd;
            }
            DEBUG_PRINT("-- Full path: %s(%d)\n-- Filename: %s\n-- Path to dir: %s\n", part2, delka, nazev_cil, jen_cesta_cil);
            DEBUG_PRINT("-- RET CIL: %d\n", ret_cil);

    if (ret_cil == -1){
        printf("PATH NOT FOUND\n");
        return;
    }

    // nactu obsah kopirovaneho souboru
    char *obsah_kopirovaneho_souboru = get_file_content(zdroj_uid);
    DEBUG_PRINT("obsah_kopirovaneho_souboru=%s\n", obsah_kopirovaneho_souboru);

    // vytvorim si novy soubor
    vytvor_soubor(ret_cil, nazev_cil, obsah_kopirovaneho_souboru, -1, 0, 1);

    printf("OK\n");
}

/*
    Presune s1 do s2
*/
void func_mv(char *cmd){
    int i = 0;
    int ret_zdroj, ret_cil, delka, zdroj_uid;
    char *nazev_zdroj, *nazev_cil, *jen_cesta_zdroj, *jen_cesta_cil;
    char *part1, *part2;
    char buffer[CLUSTER_SIZE];
    char pom2[100];
    struct mft_item *mpom;
    int mft_size = sizeof(struct mft_item);
    FILE *fw;

    // part 1
    cmd = strtok(NULL, " \n");
    part1 = (char *) malloc(sizeof(strlen(cmd)));
    strcpy(part1, cmd);
    part1[strlen(cmd)] = '\0';
    //printf("PART 1: %s=%s\n", cmd, part1);

    // part 2
    cmd = strtok(NULL, " \n");
    part2 = (char *) malloc(strlen(cmd));
    strcpy(part2, cmd);
    part2[strlen(cmd)] = '\0';
    //printf("PART 2: %s=%s\n", cmd, part2);

    // part 1
            nazev_zdroj = strrchr(part1, '/');
            if (nazev_zdroj != NULL) {
                nazev_zdroj++;

                delka = strlen(part1) - strlen(nazev_zdroj);
                jen_cesta_zdroj = (char *) malloc(delka);
                strncpy(jen_cesta_zdroj, part1, delka);
                jen_cesta_zdroj[delka] = '\0';

                ret_zdroj = parsuj_pathu(jen_cesta_zdroj, 1);
            }
            else {
                delka = strlen(part1);

                nazev_zdroj = part1;

                jen_cesta_zdroj = (char *) malloc(1);
                strncpy(jen_cesta_zdroj, "/", 1);
                jen_cesta_zdroj[1] = '\0';

                ret_zdroj = pwd;
            }

            zdroj_uid = get_uid_by_name(nazev_zdroj, ret_zdroj);

            DEBUG_PRINT("-- Full path: %s(%d)\n-- Filename: %s\n-- Path to dir: %s\n", part1, delka, nazev_zdroj, jen_cesta_zdroj);
            DEBUG_PRINT("-- RET ZDROJ: %d\n", ret_zdroj);
            DEBUG_PRINT("-- ZDROJ UID: %d\n\n", zdroj_uid);


    // part 2
            nazev_cil = strrchr(part2, '/');
            if (nazev_cil != NULL) {
                nazev_cil++;

                delka = strlen(part2) - strlen(nazev_cil);
                jen_cesta_cil = (char *) malloc(delka);
                strncpy(jen_cesta_cil, part2, delka);
                jen_cesta_cil[delka] = '\0';

                ret_cil = parsuj_pathu(jen_cesta_cil, 1);
            }
            else {
                delka = strlen(part2);

                nazev_cil = part2;

                jen_cesta_cil = (char *) malloc(1);
                strncpy(jen_cesta_cil, "/", 1);
                jen_cesta_cil[1] = '\0';

                ret_cil = pwd;
            }
            DEBUG_PRINT("-- Full path: %s(%d)\n-- Filename: %s\n-- Path to dir: %s\n", part2, delka, nazev_cil, jen_cesta_cil);
            DEBUG_PRINT("-- RET CIL: %d\n", ret_cil);


    // odstranim odkaz z nadrazeneho adresare
    char *soucasny_obsah_zdroj = get_file_content(ret_zdroj);
    DEBUG_PRINT("soucasnost=%s\n", soucasny_obsah_zdroj);

    char *curLine = soucasny_obsah_zdroj;

    // obsah clusteru daneho adresare si ctu po radcich - co jeden radek to UID jednoho souboru nebo slozky
    i = 0;
    strcpy(buffer, "");
    while (curLine){
        char * nextLine = strchr(curLine, '\n');
        if (nextLine) *nextLine = '\0';  // temporarily terminate the current line

        if (atoi(curLine) != zdroj_uid){
            if (i != 0)
                strcat(buffer, "\n");

            strcat(buffer, curLine);
        }
        //printf("CURLINE = %s\n", curLine);

        if (nextLine) *nextLine = '\n';  // then restore newline-char, just to be tidy
        curLine = nextLine ? (nextLine + 1) : NULL;
        i++;
    }

    DEBUG_PRINT("BUFÍK=%s\n", buffer);
    // UID se musi zachovat kvuli linkum
    edit_file_content(ret_zdroj, buffer, mft_seznam[ret_zdroj]->item.item_name, ret_zdroj);


    // zapisu odkaz na soubor do noveho nadrazeneho adresare
    sprintf(pom2, "%d", zdroj_uid);
    append_file_content(ret_cil, pom2, 1);

    // prejmenovani
    if (strcmp(nazev_zdroj, nazev_cil) != 0) {
        DEBUG_PRINT("-- Soubor bude prejmenovan\n");

        strcpy(mft_seznam[zdroj_uid]->item.item_name, nazev_cil);

        mpom = malloc(mft_size);

        // zapisu mft
        mpom = &mft_seznam[zdroj_uid]->item;
        fw = fopen(output_file, "r+b");
        if (fw != NULL) {
            fseek(fw, bootr->mft_start_address + zdroj_uid * mft_size, SEEK_SET);
            fwrite(mpom, mft_size, 1, fw);
        }
        free((void *) mpom);
    }

    free((void *) part1);
    free((void *) part2);

    printf("OK\n");
}

/*
    Smaze soubor s1
*/
void func_rm(char *cmd){
    DEBUG_PRINT("RM\n");
    int ret, i, delka, kesmazani;
    char buffer[CLUSTER_SIZE];
    char *nazev;
    char *jen_cesta;

    cmd = strtok(NULL, " \n");

    // pripravim si cestu a nazev souboru pro vytvoreni
    nazev = strrchr(cmd, '/');
    if (nazev != NULL) {
        nazev++;

        delka = strlen(cmd) - strlen(nazev);
        jen_cesta = (char *) malloc(delka);
        strncpy(jen_cesta, cmd, delka);
        jen_cesta[delka] = '\0';

        ret = parsuj_pathu(jen_cesta, 1);
    }
    else {
        delka = strlen(cmd);
        nazev = (char *) malloc(delka);
        jen_cesta = (char *) malloc(delka);

        strncpy(nazev, cmd, delka);
        nazev[delka] = '\0';

        strncpy(jen_cesta, "/", 1);
        jen_cesta[1] = '\0';

        ret = pwd;
    }
    DEBUG_PRINT("-- Full path: %s(%d)\n-- Filename: %s\n-- Path to dir: %s\n", cmd, delka, nazev, jen_cesta);


    kesmazani = parsuj_pathu(cmd, 1);

    DEBUG_PRINT("RET %d, KESMAZANI %d\n", ret, kesmazani);

    if (ret == -1 || kesmazani == -1){
        printf("PATH NOT FOUND\n");
        return;
    }

    if (mft_seznam[kesmazani]->item.isDirectory == 1){
        printf("NOT A FILE\n");
        return;
    }

    // odstranim odkaz z nadrazeneho adresare
    char *soucasny_obsah = get_file_content(ret);
    DEBUG_PRINT("soucasnost=%s\n", soucasny_obsah);

    char *curLine = soucasny_obsah;

    // obsah clusteru daneho adresare si ctu po radcich - co jeden radek to UID jednoho souboru nebo slozky
    i = 0;
    strcpy(buffer, "");
    while (curLine){
        char * nextLine = strchr(curLine, '\n');
        if (nextLine) *nextLine = '\0';  // temporarily terminate the current line

        if (atoi(curLine) != kesmazani){
            if (i != 0)
                strcat(buffer, "\n");

            strcat(buffer, curLine);
        }
        //printf("CURLINE = %s\n", curLine);

        if (nextLine) *nextLine = '\n';  // then restore newline-char, just to be tidy
        curLine = nextLine ? (nextLine + 1) : NULL;
        i++;
    }

    DEBUG_PRINT("BUFÍK=%s\n", buffer);
    // UID se musi zachovat kvuli linkum
    edit_file_content(ret, buffer, mft_seznam[ret]->item.item_name, ret);

    delete_file(kesmazani);

    free((void *) jen_cesta);
    printf("OK\n");
}


/*
    Vytvori adresar a1
    validni: mkdir neco, mkdir /var/www/neco, ale /var/www uz musi existovat
*/
void func_mkdir(char *cmd){
    int ret, delka;
    char *nazev;
    char *jen_cesta;

    // zpracujeme si zadanou cestu
    cmd = strtok(NULL, " \n");
    if (cmd == NULL){
        printf("PATH NOT FOUND\n");
        return;
    }

    // pripravim si cestu a nazev souboru pro vytvoreni
    nazev = strrchr(cmd, '/');
    if (nazev != NULL) {
        nazev++;

        delka = strlen(cmd) - strlen(nazev);
        jen_cesta = (char *) malloc(delka);
        strncpy(jen_cesta, cmd, delka);
        jen_cesta[delka] = '\0';

        ret = parsuj_pathu(jen_cesta, 1);
    }
    else {
        delka = strlen(cmd);

        nazev = cmd;

        jen_cesta = (char *) malloc(1);
        strncpy(jen_cesta, "/", 1);
        jen_cesta[1] = '\0';

        ret = pwd;
    }

    DEBUG_PRINT("-- Full path: %s(%d)\n-- Filename: %s\n-- Path to dir: %s\n", cmd, delka, nazev, jen_cesta);


    // zkusim si tu cestu projit
    if (ret == -1){
        printf("PATH NOT FOUND\n");
        return;
    }
    else {
        // --- zde vytvorime slozku ---
        // dle bitmapy najdu prvni volny cluster a vypoctu si jeho adresu, fragment_count zvolim na 1
        // do prvniho fragmentu polozky mft_seznam[ret]->item zapisu nakonec UID noveho adresare
        zaloz_novou_slozku(ret, nazev);
    }


    free((void *) jen_cesta);

    DEBUG_PRINT("ls ret = %d\n", ret);

    printf("OK\n");
}


/*
    Smaze prazdny adresar
*/
void func_rmdir(char *cmd){
    DEBUG_PRINT("RMDIR\n");
    int ret, i;
    char buffer[CLUSTER_SIZE];

    cmd = strtok(NULL, " \n");
    ret = parsuj_pathu(cmd, 1);

    DEBUG_PRINT("RET ke smazani %d\n", ret);

    if (ret == -1){
        printf("PATH NOT FOUND\n");
        return;
    }

    if (mft_seznam[ret]->item.isDirectory == 0) {
        printf("NOT A DIRECTORY\n");
        return;
    }

    if (is_empty_dir(ret) > 1) {
        printf("NOT EMPTY\n");
        return;
    }

    // odstranim odkaz z nadrazeneho adresare
    char *soucasny_obsah = get_file_content(pwd);
    DEBUG_PRINT("soucasny obsah adresare=%s\n", soucasny_obsah);

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

    DEBUG_PRINT("Novy obsah adresare=%s\n", buffer);
    // UID se musi zachovat kvuli linkum
    edit_file_content(pwd, buffer, mft_seznam[pwd]->item.item_name, pwd);

    // smazu pozadovany soubor na disku
    delete_file(ret);

    printf("OK\n");
}


/*
    Vypise obsah adresare
    ls /var/www/neco je validni prikaz
*/
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

    DEBUG_PRINT("ls RET = %d\n", ret);

    // cesta neexistuje, nelze splnit pozadavek
    if (ret == -1){
        printf("PATH NOT FOUND\n");
        return;
    }

    ls_printer(ret);
}

/*
    Vypise obsah souboru
*/
void func_cat(char *cmd){
    int ret;

    cmd = strtok(NULL, " \n");

    ret = parsuj_pathu(cmd, 1);
    DEBUG_PRINT("vyparsovano: %d\n", ret);

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
    DEBUG_PRINT("_%s_%zd\n", cmd, strlen(cmd));

    kam = parsuj_pathu(cmd, 1);

    if (kam != -1){
        pwd = kam;
        printf("OK\n");
        DEBUG_PRINT("-- menim kurzor pwd: %d\n", kam);
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

    if (pwd == 0) {
        printf("/\n");
    }
    else if (pwd >= 0) {
        link_int = pwd;

        while (link_int > 0) {
            // /alservis, /www, /var
            strcpy(link, "/");
            strcat(link, mft_seznam[link_int]->item.item_name);

            // to co uz ve stringu je dam na konec
            strcpy(pom, full_link);
            strcpy(full_link, link);
            strcat(full_link, pom);

            // najdu si backlink dalsiho adresare v poradi
            link_int = get_backlink(link_int);
        }

        printf("%s\n", full_link);
    }
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

/*
    Nahraje soubor z pevneho disku do FS
*/
void func_incp(char *cmd){
    int i, ret, delka;
    FILE *f;
    char pc_file[100];
    char *nazev;
    char *jen_cesta;

    i = 0;

    // postupne cteni argumentu
    while((cmd = strtok(NULL, " \n")) != NULL) {
        if (i == 0) {
            // soubor k presunu z pocitace
            // overim jeho existenci
            strncpy(pc_file, cmd, strlen(cmd));
            pc_file[strlen(cmd)] = '\0';

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

                delka = strlen(cmd) - strlen(nazev) - 1;
                DEBUG_PRINT("strlen(cmd) - strlen(nazev) => strlen(%s) - strlen(%s)=%d\n", cmd, nazev, delka);

                jen_cesta = (char *) malloc(delka * sizeof(char *));
                strncpy(jen_cesta, cmd, delka);
                jen_cesta[delka] = '\0';

                ret = parsuj_pathu(jen_cesta, 1);
            }
            else {
                delka = strlen(cmd);
                nazev = cmd;

                jen_cesta = (char *) malloc(delka);
                strncpy(jen_cesta, "/", 1);
                jen_cesta[1] = '\0';

                ret = pwd;
            }

            DEBUG_PRINT("-- Full path: %s\n-- Filename: %s\n-- Path to dir: %s\n", cmd, nazev, jen_cesta);

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
    DEBUG_PRINT("-- Vyparsovana cesta: %d\n", ret);

    vytvor_soubor(ret, nazev, read_file_from_pc(pc_file), -1, 0, 1);

    free((void *) jen_cesta);
    printf("OK\n");
}

/*
    Nahraje soubor do PC
*/
void func_outcp(char *jen_cesta, char *externi_soubor){
    int ret;
    FILE *fw;
    char *obsah;

    DEBUG_PRINT("!!!%s!!!\n", jen_cesta);
    DEBUG_PRINT("!!!%s!!!\n", externi_soubor);


    // part 1, k presunu z FS
    DEBUG_PRINT("K presunu z FS\n");

    ret = parsuj_pathu(jen_cesta, 1);

    if (ret == -1){
        printf("PATH NOT FOUND\n");
        return;
    }

    obsah = get_file_content(ret);

    DEBUG_PRINT("OUT obsah: %s\n", obsah);
    DEBUG_PRINT("RET: %d\n", ret);


    // part 2, ulozim soubor do pc
    DEBUG_PRINT("Ulozim soubor do pc\n");

    fw = fopen(externi_soubor, "w");
    if (fw == NULL){
        printf("FILE %s NOT FOUND\n", externi_soubor);
        return;
    }

    fwrite(obsah, 1, strlen(obsah), fw);

    fclose(fw);

    printf("OK\n");
}

/*
    Provede defragmentaci
    Soubory se budou skladat pouze z jednoho fragmentu
*/
void func_defrag(){
    int i, j, k, clusteru, p_clusteru, zpracovany, adresa;
    int nova_bitmapa[CLUSTER_COUNT];
    FILE *fw;

    // prejmenuji puvodni soubor (backup)
    char *puvodni = (char *) malloc(100);
    char *new = (char *) malloc(100);
    strcpy(new, output_file);
    strcat(new, ".bak");
    strcat(new, "\0");
    strcpy(puvodni, output_file);
    strcat(puvodni, "\0");

    if (rename(output_file, new) == 0) {
        printf("VYTVARIM ZALOZNI SOUBOR .bak\n");
    }

    // provedu prislusne operace defragmentace
    fw = fopen(output_file, "wb");
    if (fw != NULL) {
        zpracovany = 0; // zpracovany cluster
        for (i = 0; i < CLUSTER_COUNT; i++) {
            if (mft_seznam[i] != NULL){
                // soubor stoji za zpracovani
                printf("Zpracovavam soubor %s\n", mft_seznam[i]->item.item_name);

                // nactu si obsah souboru
                strcpy(output_file, new);
                char *cely_soubor = get_file_content(i);
                strcpy(output_file, puvodni);

                //DEBUG_PRINT("/%s/\n", cely_soubor);

                clusteru = ceil((double) strlen(cely_soubor) / CLUSTER_SIZE);
                DEBUG_PRINT("+ Pro soubor %s (%d) potrebuji clusteru %d a ma delku %zu\n", mft_seznam[i]->item.item_name, mft_seznam[i]->item.uid, clusteru, strlen(cely_soubor));

                // zapisu si do bitmapy
                DEBUG_PRINT("zpracovany + clusteru = %d + %d = %d\n", zpracovany, clusteru, zpracovany+clusteru);
                for (j = zpracovany; j < zpracovany + clusteru; j++) {
                    nova_bitmapa[j] = 1;
                    DEBUG_PRINT("ALOKUJI PRVEK %d V BITMAPE\n", j);
                }

                // aktualizace mfti
                mft_seznam[i]->item.item_order = 1;
                mft_seznam[i]->item.item_order_total = 1;
                mft_seznam[i]->item.item_size = strlen(cely_soubor);

                // pripravim si fragmenty
                adresa = bootr->data_start_address + zpracovany * CLUSTER_SIZE;
                for (k = 0; k < MFT_FRAG_COUNT; k++) {
                    p_clusteru = clusteru;

                    // uz jsem zapsal prvni fragment
                    if (k == 1) {
                        adresa = -1;
                        p_clusteru = -1;
                    }
                    else {
                            fseek(fw, adresa, SEEK_SET);
                            fwrite(cely_soubor, strlen(cely_soubor), 1, fw);
                    }

                    mft_seznam[i]->item.fragments[k].fragment_start_address = adresa;
                    mft_seznam[i]->item.fragments[k].fragment_count = p_clusteru;
                    DEBUG_PRINT("pripravim si fragmenty: (%d, %d)\n", adresa, p_clusteru);
                }

                DEBUG_PRINT("----------------------------------------------------\n");

                // zrusim odkaz na dalsi prvek pameti
                mft_seznam[i]->dalsi = NULL;

                zpracovany += clusteru;
            }
        }
        fclose(fw);
    }

    // vytvorim novy soubor
    fw = fopen(output_file, "r+b");
    if (fw != NULL) {
        /* Zapiseme boot record */
        fseek(fw, 0, SEEK_SET);
        fwrite(bootr, sizeof(struct boot_record), 1, fw);

        /* Zapiseme startovaci bitmapu */
        fseek(fw, bootr->bitmap_start_address, SEEK_SET);
        fwrite(nova_bitmapa, 4, CLUSTER_COUNT, fw);

        /* Zapiseme MFT */
        adresa = bootr->mft_start_address;
        for (i = 0; i < CLUSTER_COUNT; i++) {
            if (mft_seznam[i] != NULL){
                DEBUG_PRINT("ZAPISUJI MFTI NA ADRESU %d\n", adresa);
                fseek(fw, adresa, SEEK_SET);
                fwrite(&mft_seznam[i]->item, sizeof(struct mft_item), 1, fw);

                adresa += sizeof(struct mft_item);
            }
        }

        fclose(fw);
    }

    free((void *) new);
    free((void *) puvodni);

    printf("OK\n");
}

/*
    Kontrola konzistence
    -> zkontroluje jeslti jsou soubory neposkozene (velikost souboru odpovida poctu alokovanych datovych bloku)
 */
void func_consist(){
    int i, rc;
    int pocet_vlaken = 2;

    pthread_t pt[pocet_vlaken];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    sdilenaPamet pamet;

    pamet.mutex = &mutex;
    pamet.zpracovany_cluster = -1;

    // start vlaken
    for (i = 0; i < pocet_vlaken; i++) {
        rc = pthread_create(&pt[i], NULL, kontrola_konzistence, (void *) &pamet);
        assert(0 == rc);
    }

    // Cekam na dokonceni vsech vlaken
    for (i = 0; i < pocet_vlaken; i++) {
        rc = pthread_join(pt[i], NULL);
        assert(0 == rc);
    }

    printf("OK\n");
}
