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
    char pom[12];

    // tady mam pozadovanou celou cestu
    cmd = strtok(NULL, " ");
    if (cmd == NULL){
        printf("PATH NOT FOUND\n");
        return;
    }

    // zkusim si tu cestu projit
    ret = parsuj_pathu(cmd);
    if (ret == -1){
        printf("PATH NOT FOUND\n");
        return;
    }
    else {
        // --- zde vytvorime slozku ---
        // dle bitmapy najdu prvni volny cluster a vypoctu si jeho adresu, fragment_count zvolim na 1
        // do prvniho fragmentu polozky mft_seznam[ret]->item zapisu nakonec UID noveho adresare
        while((cmd = strtok(NULL, " ")) != NULL){
            printf("Ostatni: %s\n", cmd);
            strcpy(pom, cmd);
        }

        zaloz_novou_slozku(ret, pom);
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

    // tady mam pozadovanou celou cestu
    cmd = strtok(NULL, " ");
    if (cmd == NULL){
        printf("PATH NOT FOUND\n");
        return;
    }

    // zkusim si tu cestu projit
    ret = parsuj_pathu(cmd);
    if (ret == -1){
        printf("PATH NOT FOUND\n");
        return;
    }
    else {

    }

    printf("ls ret = %d\n", ret);

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
    printf("func incp");

    while((cmd = strtok(NULL, " ")) != NULL){
        printf("Ostatni: %s\n", cmd);
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
