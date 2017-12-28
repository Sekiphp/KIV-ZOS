#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "loader.h"
#include "mft.h"

extern int pwd;
extern MFT_LIST *mft_list[];

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



void func_mkdir(char *cmd){
    printf("func mkdir");

    while((cmd = strtok(NULL, " ")) != NULL){
        printf("Ostatni: %s\n", cmd);
    }
}



void func_rmdir(char *cmd){
    printf("func rmdir");

    while((cmd = strtok(NULL, " ")) != NULL){
        printf("Ostatni: %s\n", cmd);
    }
}



void func_ls(char *cmd){
    printf("func ls");

    while((cmd = strtok(NULL, " ")) != NULL){
        printf("Ostatni: %s\n", cmd);
    }
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

    get_mft_item_content(0);
    get_mft_item_content(1);
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


/* Ziska obsah danych clusteru, ktere nalezi stejnemu fragmentu - Jeden mfti muze mit vsak mnoho fragmentu */
char* get_fragment_content(int32_t fragment_start_addr, int32_t fragments_count){
    int sirka_bloku = CLUSTER_SIZE * fragments_count;
    char ret[sirka_bloku];
    FILE *fr;

    // todo: filename udelat globalni
    fr = fopen("ntfs.dat", "rb");
    if (fr != NULL) {
        fseek(fr, fragment_start_addr, SEEK_SET);
        fread(ret, 1, sirka_bloku, fr);

        fclose(fr);
    }

    return ret;
}

char* get_mft_item_content(int32_t uid){
    MFT_LIST *mft_item_chceme = mft_seznam[uid];
    struct mft_item mfti_pom;
    struct mft_fragment mftf_pom;
    int i, j;

    i = 0;
    if (mft_item_chceme != NULL){
        // projedeme vsechny itemy pro dane UID souboru
        // bylo by dobre si pak z tech itemu nejak sesortit fragmenty dle adres
        // zacneme iterovar pres ->dalsi
        while(1){
            i++;

            // precteme vsechny fragmenty (je jich: MFT_FRAG_COUNT)
            for(j = 0; j < MFT_FRAG_COUNT; j++){
                mfti_pom = mft_item_chceme->mft_item;
                printf("Nacteny item s UID=%d ma nazev %s\n", mfti_pom->uid, mfti->item_name);
            }

            // prehodim se na dalsi prvek
            mft_item_chceme = mft_item_chceme->dalsi;

            // ukonceni cyklu pokud uz dalsi polozka nenasleduje
            if(mft_item_chceme->dalsi == NULL){
                printf("Koncim pruchod cyklu po %d iteracich\n", i);
                break;
            }
        }

    }

    return "";
}


