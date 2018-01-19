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
    while((cmd = strtok(NULL, " ")) != NULL){
        printf("Ostatni: %s\n", cmd);
    }
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



void func_rmdir(char *cmd){
    while((cmd = strtok(NULL, " ")) != NULL){
        printf("Ostatni: %s\n", cmd);
    }
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



void func_info(char *cmd){
    int ret;
    struct mft_item mfti;

    cmd = strtok(NULL, " \n");
    ret = parsuj_pathu(cmd, 1);

    if (ret == -1) {
        printf("FILE NOT FOUND\n");
        return;
    }

    mfti = mft_seznam[ret]->item;

    printf("NAME - UID - SIZE\n");
    printf("%s - %d - %d\n", mfti.item_name, mfti.uid, mfti.item_size);

    printf("-- FRAGMENTY:\n");
    printf("-- CLUSTERY:\n");
}



void func_incp(char *cmd){
    int i, ret;
    FILE *f;
    char pc_file[100];

    i = 0;

    // postupne cteni argumentu
    while((cmd = strtok(NULL, " ")) != NULL){
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
            printf("Cesta k parsovani je: --%s--\n", cmd);

            ret = parsuj_pathu(cmd, 1);
            if (ret == -1){
                printf("PATH %s NOT FOUND\n", cmd);
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

    vytvor_soubor_z_pc(ret, pc_file);
}



void func_outcp(char *cmd){
    while((cmd = strtok(NULL, " ")) != NULL){
        printf("Ostatni: %s\n", cmd);
    }
}



void func_load(char *cmd){
    while((cmd = strtok(NULL, " ")) != NULL){
        printf("Ostatni: %s\n", cmd);
    }
}

void func_defrag(){

}

void func_consist(){

}
