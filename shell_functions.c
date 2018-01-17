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

    ls(ret);

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
    int i, j, k, size, ret, potreba_clusteru;
    char * result;
    FILE *f;
    char pom[100];

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
            potreba_clusteru = (int) (size / CLUSTER_SIZE);
            int volne_clustery[potreba_clusteru];

            k = 0;
            for (j = 0; j < CLUSTER_COUNT; j++) {
                if (ntfs_bitmap[j] == 0) {
                    // volna
                    volne_clustery[k] = j;
                    k++;
                }
            }

            if (k != potreba_clusteru){
                printf("ERROR - NOT ENOUGH CLUSTERS\n");
                return;
            }

            FILE *fw;
            fw = fopen("ntfs.dat", "r+b");
            if(fw != NULL){
                // aktualizuji bitmapu v souboru
                // + zapnim virtualni clustery (nactene ze souboru)
                for (j = 0; j < k; j++){
                    ntfs_bitmap[volne_clustery[j]] = 1;
                }
                fseek(fw, bootr->bitmap_start_address, SEEK_SET);
                fwrite(ntfs_bitmap, 4, CLUSTER_COUNT, fw);

                // aktualizuji virtualni mft

                // aktualizuji mft v souboru

                // zapisu obsah clusteru do souboru

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

    if (i != 2){
        printf("TOO FEW ARGS\n");
        return;
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
