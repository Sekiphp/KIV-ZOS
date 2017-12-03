#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shell.h"
#include "shell_functions.h"
#include "mft.h"
#include "parametr.h"

#define MAX 200

/* Posloucha prikazy pro ovladani virtualniho operacniho systemu */
void *shell(void *arg){
    sdilenaPamet *param = (sdilenaPamet *) arg;
    FILE *fr;
    char command[MAX];
    char *p_c;

    printf("SHELL booting...\n");

    pthread_mutex_lock(param->mutex);
        fr = fopen(param->soubor, "rb");
        if (fr != NULL) {
            fread(boot, sizeof(struct boot_record), 1, fr);
            fclose(fr);
        }
    	printf("boot shell: %s\n", boot->signature);
    pthread_mutex_unlock(param->mutex);

    /* infinite loop - cekam na prikazy */
    while(1){
        printf("Zadejte prikaz: ");
        fgets(command, MAX, stdin);

        p_c = strtok(command, " ");
        if (p_c != NULL){
            printf("Prvni: %s\n", p_c);
        }

        if(strcmp(p_c, "cp") == 0){
            func_cp(p_c);
        }
        if(strcmp(p_c, "mv") == 0){
            func_mv(p_c);
        }
        if(strcmp(p_c, "rm") == 0){
            func_rm(p_c);
        }
        if(strcmp(p_c, "mkdir") == 0){
            func_mkdir(p_c);
        }
        if(strcmp(p_c, "rmdir") == 0){
            func_rmdir(p_c);
        }
        if(strcmp(p_c, "ls") == 0){
            func_ls(p_c);
        }
        if(strcmp(p_c, "cat") == 0){
            func_cat(p_c);
        }
        if(strcmp(p_c, "cd") == 0){
            func_cd(p_c);
        }
        if(strcmp(p_c, "pwd") == 0){
            func_pwd(p_c);
        }
        if(strcmp(p_c, "info") == 0){
            func_info(p_c);
        }
        if(strcmp(p_c, "incp") == 0){
            func_incp(p_c);
        }
        if(strcmp(p_c, "outcp") == 0){
            func_outcp(p_c);
        }
        if(strcmp(p_c, "load") == 0){
            func_load(p_c);
        }
    }

    printf("SHELL ending\n");
    return NULL;
}
