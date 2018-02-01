#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "debugger.h"
#include "shell.h"
#include "shell_functions.h"
#include "mft.h"
#include "parametr.h"
#include "boot_record.h"

#define MAX 200

/* Posloucha prikazy pro ovladani virtualniho operacniho systemu */
void shell() {
    FILE *fr;
    char command[MAX];
    char *p_c, *p_c2;
    int use_file = 0;

    printf("SHELL booting...\n");

    /* infinite loop - cekam na prikazy */
    while(1){
        printf("$ Zadejte prikaz: ");

        if (use_file == 0) {
            fgets(command, MAX, stdin);
        }
        else {
            fgets(command, MAX, fr);

            if (feof(fr) == 1) {
                fclose(fr);
                use_file = 0;
            }

            if (strlen(command) <= 2) {
                continue;
            }

            printf("%s", command);
        }

        p_c = strtok(command, " ");

        if(strncmp(p_c, "cp", 2) == 0){
            func_cp(p_c);
        }
        else if(strncmp(p_c, "mv", 2) == 0){
            func_mv(p_c);
        }
        else if(strncmp(p_c, "mkdir", 5) == 0){
            func_mkdir(p_c);
        }
        else if(strncmp(p_c, "rmdir", 5) == 0){
            func_rmdir(p_c);
            continue;
        }
        else if(strncmp(p_c, "rm", 2) == 0){
            func_rm(p_c);
        }
        else if(strncmp(p_c, "ls", 2) == 0){
            func_ls(p_c);
        }
        else if(strncmp(p_c, "cat", 3) == 0){
            func_cat(p_c);
        }
        else if(strncmp(p_c, "cd", 2) == 0){
            func_cd(p_c);
        }
        else if(strncmp(p_c, "pwd", 3) == 0){
            func_pwd();
        }
        else if(strncmp(p_c, "info", 4) == 0){
            func_info(p_c);
        }
        else if(strncmp(p_c, "incp", 4) == 0){
            func_incp(p_c);
        }
        else if(strncmp(p_c, "outcp", 5) == 0){
            p_c = strtok(NULL, " ");
            p_c2 = strtok(NULL, " \n");
            func_outcp(p_c, p_c2);
        }
        else if(strncmp(p_c, "load", 4) == 0){
            p_c = strtok(NULL, " \n");
            fr = fopen(p_c, "r");
            if (fr != NULL)
                use_file = 1;
            else
                printf("FILE NOT FOUND\n");
        }
        else if(strncmp(p_c, "defrag", 6) == 0){
            func_defrag();
        }
        else if(strncmp(p_c, "consist", 7) == 0){
            func_consist();
        }
        else if(strncmp(p_c, "exit", 4) == 0){
            break;
        }
    }

    printf("SHELL ending\n");
}
