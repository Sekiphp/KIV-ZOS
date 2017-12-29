#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "loader.h"
#include "mft.h"

extern int pwd;
//extern MFT_LIST *mft_seznam[];

/* Ziska obsah danych clusteru, ktere nalezi stejnemu fragmentu - Jeden mfti muze mit vsak mnoho fragmentu */
char* get_cluster_content(int32_t fragment_start_addr, int32_t fragments_count){
    int sirka_bloku = CLUSTER_SIZE * fragments_count;
    char *ret;
    ret = (char*) malloc(sirka_bloku);
    FILE *fr;

    // todo: filename udelat globalni
    fr = fopen("ntfs.dat", "rb");
    if (fr != NULL) {
        fseek(fr, fragment_start_addr, SEEK_SET);
        fread(ret, sizeof(char), sirka_bloku, fr);

        fclose(fr);
    }

    return ret;
}

/* Ziska obsah vsech fragmentu pro soubor nebo slozku daneho UID */
char* get_mft_item_content(int32_t uid){
    int i, j, k;
    char *ret = malloc(CLUSTER_SIZE);
    MFT_LIST* mft_item_chceme;
    struct mft_fragment mftf;

    if (mft_seznam[uid] != NULL){
        mft_item_chceme = mft_seznam[uid];

        printf("je tu alespon jeden item co stoji za zminku %d\n", mft_item_chceme->ij);

        // projedeme vsechny itemy pro dane UID souboru
        // bylo by dobre si pak z tech itemu nejak sesortit fragmenty dle adres
        // zacneme iterovar pres ->dalsi
        i = 0;
        k = 0; // celkovy pocet zopracovanych fragmentu
        while (mft_item_chceme != NULL){
            i++;
            printf("pocet iteraci=%d\n", i);
            printf("Nacteny item s UID=%d ma nazev %s\n", mft_item_chceme->item.uid, mft_item_chceme->item.item_name);

            // precteme vsechny fragmenty z daneho mft itemu (je jich: MFT_FRAG_COUNT)
            for (j = 0; j < MFT_FRAG_COUNT; j++){
                k++;
                mftf = mft_item_chceme->item.fragments[j];

                if (mftf.fragment_start_address != 0) {
                    printf("Zpracovavam fragment %d ze souboru s UID %d, start=%d, count=%d\n", j, mft_item_chceme->item.uid, mftf.fragment_start_address, mftf.fragment_count);

                    // prubezne je potreba realokovat oblast tak, aby se mi podarailo nacist cely soubor
                    //if (j != 1){
                        int *tmp = realloc(ret, k * CLUSTER_SIZE);
                        if (tmp == NULL) return "ERROR";
                    //}

                    strcat(ret, get_cluster_content(mftf.fragment_start_address, mftf.fragment_count));
                }
            }

            // prehodim se na dalsi prvek
            mft_item_chceme = mft_item_chceme->dalsi;
        }
    }

    return ret;
}

int parsuj_pathu(char *path){
    char *p_c;
    int start_dir, uid;

    // zacinam v rootu ci nikoliv?
    if (strncmp(path, "/", 1) == 0){
        start_dir = 1;
    }
    else {
        start_dir = pwd;
    }
    printf("START DIR = %d\n", start_dir);

    p_c = strtok(path, " ");
    if (p_c != NULL){
        printf("Prvni: %s\n", p_c);
    }
    while((p_c = strtok(NULL, " ")) != NULL){
        printf("Ostatni: %s\n", p_c);
    }

    uid = start_dir;

    if (mft_seznam[uid]->item.isDirectory == 1) {
        printf("UID %d je adresarem\n", uid);

        return 0;
    }
    else {
        return -1;
    }
}


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
