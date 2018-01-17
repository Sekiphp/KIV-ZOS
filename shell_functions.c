#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "loader.h"
#include "mft.h"
#include "shell_functions.h"
#include "boot_record.h"
#include "functions.h"

extern int pwd;

void func_cp(char *cmd){
    printf("func cp");

    while((cmd = strtok(NULL, " ")) != NULL){
        printf("Ostatni: %s\n", cmd);
    }
}


void func_mv(char *cmd){
    printf("func mv");

    while((cmd = strtok(NULL, " ")) != NULL){
        printf("Ostatni: %s\n", cmd);
    }
}



void func_rm(char *cmd){
    printf("func rm");

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
    printf("func rmdir");

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

    //printf("ls ret = %d\n", ret);
}

void func_cat(char *cmd){
    printf("func cat");

    while((cmd = strtok(NULL, " ")) != NULL){
        printf("Ostatni: %s\n", cmd);
    }
}



void func_cd(char *cmd){
    printf("func cd");

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
    printf("func info");

    while((cmd = strtok(NULL, " ")) != NULL){
        printf("Ostatni: %s\n", cmd);
    }

    printf("NAME - UID - SIZE - FRAGMENTY - CLUSTERY\n");
//    printf("NAME %s", );

    printf("Data z clusteru s UID=1: %s\n", get_mft_item_content(1));
    printf("parsuj pathu = %d\n", parsuj_pathu("/var/www/diginex.cz"));
}



void func_incp(char *cmd){
    int i = 0;
    //printf("func incp");

    while((cmd = strtok(NULL, " ")) != NULL){
        if (i == 0){
            // zpracovavam prvni argument - najdu v PC
            int size = 0;
            FILE *f = fopen(cmd, "rb");
            if (f == NULL)
            {
                *result = NULL;
                return -1; // -1 means file opening fail
            }
            fseek(f, 0, SEEK_END);
            size = ftell(f);
            printf("size=%d\n", size);
            fseek(f, 0, SEEK_SET);
            *result = (char *)malloc(size+1);
            if (size != fread(*result, sizeof(char), size, f))
            {
                free(*result);
                return -2; // -2 means file reading fail
            }
            fclose(f);
            (*result)[size] = 0;
            //return size;
        }
        else {
            // najdu cilove misto pro ulozeni

        }

        i++;
    }
}



void func_outcp(char *cmd){
    printf("func outcp");

    while((cmd = strtok(NULL, " ")) != NULL){
        printf("Ostatni: %s\n", cmd);
    }
}



void func_load(char *cmd){
    printf("func load");

    while((cmd = strtok(NULL, " ")) != NULL){
        printf("Ostatni: %s\n", cmd);
    }
}
