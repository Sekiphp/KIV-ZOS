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
    int i, j, k, size, ret, potreba_clusteru, adresa;
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

                // aktualizuji virtualni mft

                // aktualizuji mft v souboru

                // zapisu obsah clusteru do souboru
                // v result muzu mit třeba 5000 znaku, tj rozdelovat po CLUSTER_SIZE
                for (j = 0; j < potreba_clusteru; j++){
                    adresa = bootr->data_start_address + volne_clustery[j] * CLUSTER_SIZE;
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

    if (i != 2){
        printf("TOO FEW ARGS\n");
        return;
    }
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