#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "loader.h"
#include "mft.h"
#include "shell_functions.h"
#include "boot_record.h"

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

/*
Pokusi se v obsahu daneho adresare najit jiny adresar za pomoci jeho jmena
@return UID hledaneho adresare nebo -1 pri chybe
*/
int get_uid(char *dir_name, int uid_pwd){
    char *obsah = get_mft_item_content(uid_pwd);
    char * curLine = obsah;
    struct mft_item mfti;
    int hledane;

    printf("Spoustim metodu *get_uid* s dir_name = %s a uid_pwd = %d\n\tTato polozka ma obsah clusteru: %s \n----------\n", dir_name, uid_pwd, obsah);

    while (curLine){
        char * nextLine = strchr(curLine, '\n');
        if (nextLine) *nextLine = '\0';  // temporarily terminate the current line
        hledane = atoi(curLine);
        printf("*** nactene UID z clusteru %s (int=%d)\n", curLine, hledane);

        // tady si roparsuji MFT a zjistim jestli se shoduje nazev
        if (hledane < CLUSTER_COUNT && mft_seznam[hledane] != NULL){
            mfti = mft_seznam[hledane]->item;

            printf("\tHledane mfti s uid=%d (name=%s) NOT NULL\n", hledane, mfti.item_name);

            if (strcmp(mfti.item_name, dir_name) == 0 && mfti.isDirectory == 1) {
                printf("\tSHODA\n");
                return mfti.uid;
            }
        }

        if (nextLine) *nextLine = '\n';  // then restore newline-char, just to be tidy
        curLine = nextLine ? (nextLine + 1) : NULL;
    }

    return -1;
}

int parsuj_pathu(char *patha){
    char *p_c;
    int start_dir, uid, uid_pom;
    char path[100];

    // Nelze pracovat primo s arg: https://stackoverflow.com/questions/8957829/strtok-segmentation-fault
    strcpy(path, patha);

    // zacinam v rootu ci nikoliv?
    if (strncmp(path, "/", 1) == 0){
        start_dir = 1;
    }
    else {
        start_dir = pwd;
    }
    printf("START DIR = %d\n", start_dir);

    if (strchr(patha, '/') != NULL){
        // parsuji jednotlive casti cesty a norim se hloubeji a hloubeji
        p_c = strtok(path, "/");
        if (p_c != NULL){
            uid_pom = get_uid(p_c, start_dir);

            printf("get_uid(%s, %d) = %d\n", p_c, start_dir, uid_pom);
            if (uid_pom == -1) return -1;
            start_dir = uid_pom;
        }
        while((p_c = strtok(NULL, "/")) != NULL){
            uid_pom = get_uid(p_c, start_dir);

            printf("get_uid(%s, %d) = %d\n", p_c, start_dir, uid_pom);
            if (uid_pom == -1) return -1;
            start_dir = uid_pom;
        }
    }

    return start_dir;
}

int zaloz_novou_slozku(int32_t pwd, char *name){
    int i, bitmap_free_index, new_cluster_start;
    FILE *fw;
    MFT_ITEM *mfti;
    MFT_FRAGMENT *mftf;

    // najdu volnou bitmapu
    bitmap_free_index = -1;
    for (i = 0; i < CLUSTER_COUNT; i++){
        if (ntfs_bitmap[i] == 0){
            bitmap_free_index = i;
            break;
        }
    }

    if (bitmap_free_index != -1){
        new_cluster_start = bootr->data_start_address + bitmap_free_index * CLUSTER_SIZE;

        // ziskam prvni volny MFT LIST a dne nej zjistim volne UID, ktere pridelim nove slozce
        for (i = 0; i < CLUSTER_COUNT; i++){
            if (mft_seznam[i] == NULL){
                // tento prvek je volny, vyuziji jej tedy
                mfti = malloc(sizeof(struct mft_item));
                mftf = malloc(sizeof(struct mft_fragment));

                mftf->fragment_start_address = new_cluster_start; // start adresa ve VFS
                mftf->fragment_count = 1;

                mfti->uid = bitmap_free_index;
                mfti->isDirectory = 1;
                mfti->item_order = 1;
                mfti->item_order_total = 1;
                strcpy(mfti->item_name, name);
                mfti->item_size = 0; // zatim tam nic neni, takze nula
                mfti->fragments[0] = mftf;

                // prvek mam pripraveny
                // zaktualizuji si globalni pole a bitmapu
                ntfs_bitmap[bitmap_free_index] = 1;
                pridej_prvek(bitmap_free_index, mfti);

                // zapisu do souboru
                // todo: filename
                fw = fopen("ntfs.dat", "wb");
                if(fw != NULL){
                    // mfti
                    fseek(fw, sizeof(struct boot_record) + bitmap_free_index * sizeof(struct mft_item), SEEK_SET);
                    fwrite(mfti, sizeof(struct mft_item), 1, fw);

                    // bitmap
                    fseek(fw, bootr->bitmap_start_address, SEEK_SET);
                    fwrite(ntfs_bitmap, 4, CLUSTER_COUNT, fw);

                    // odkaz na slozku do nadrazeneho adresare

                    fclose(fw);
                }

                free((void *) mfti);
                free((void *) mftf);
                break;
            }
        }
    }

    return bitmap_free_index;
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

        zaloz_novou_slozku(ret, pom)
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
