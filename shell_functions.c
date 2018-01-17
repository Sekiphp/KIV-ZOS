#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "loader.h"
#include "mft.h"
#include "shell_functions.h"
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


/* validni: mkdir neco, mkdir /var/www/neco */
void func_mkdir(char *cmd){
    int ret;

    // zpracujeme si zadanou cestu
    cmd = strtok(NULL, " ");
    if (cmd == NULL){
        printf("PATH NOT FOUND\n");
        return;
    }
    else {
        printf("Cesta k parsovani je: --%s--\n", cmd);

        ret = parsuj_pathu(cmd);
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
    cmd = strtok(NULL, " ");
    if (cmd == NULL){
        ret = parsuj_pathu("");
    }
    else {
        printf("Cesta k parsovani je: --%s--\n", cmd);

        ret = parsuj_pathu(cmd);
    }

    // cesta neexistuje, nelze splnit pozadavek
    if (ret == -1){
        printf("PATH NOT FOUND\n");
        return;
    }

    ls(ret);

    //printf("ls ret = %d\n", ret);
}

void func_cat(char *cmd){
    while((cmd = strtok(NULL, " ")) != NULL){
        printf("Ostatni: %s\n", cmd);
    }
}



void func_cd(char *cmd){
    while((cmd = strtok(NULL, " ")) != NULL){
        printf("Ostatni: %s\n", cmd);
    }
}


/*
print working directory
return: PATH
*/
void func_pwd(char *cmd){
    if (pwd > 0) {
        if (pwd == 1) {
            printf("/\n");
        }
        else {
            printf("Jsi ve slozce %d\n", pwd);
        }
    }
    printf("PWD = %d\n", pwd);
}



void func_info(char *cmd){
    while((cmd = strtok(NULL, " ")) != NULL){
        printf("Ostatni: %s\n", cmd);
    }

    printf("NAME - UID - SIZE - FRAGMENTY - CLUSTERY\n");
//    printf("NAME %s", );

    printf("Data z clusteru s UID=1: %s\n", get_mft_item_content(1));
    printf("parsuj pathu = %d\n", parsuj_pathu("/var/www/diginex.cz"));
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

            ret = parsuj_pathu(cmd);
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