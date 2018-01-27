#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debugger.h"
#include "loader.h"
#include "mft.h"
#include "shell_functions.h"
#include "boot_record.h"
#include "ntfs_helpers.h"
#include "functions.h"
#include "parametr.h"

extern int pwd;
extern char output_file[100];

/*
    Vrati volne UID, ktere je volne pro pojmenovani souboru
*/
int get_volne_uid() {
    int i;

    // pochazim potencionalni list a hledam prvni volny index
    for (i = 0; i < CLUSTER_COUNT; i++) {
        if (mft_seznam[i] == NULL){
            DEBUG_PRINT("-- Volne UID je: %d\n", i);
            return i;
        }
    }

    return -1;
}

/*
    Prelozi jmeno adresare na UID
     -> nacte si obsah clusteru soucasneho adresare a prochazi vsechny tyto adresare a hleda shodnost jmena
     -> musi to byt reseno timto zpusobem kvuli backlinku
    @param dir_name Jmeno adresare pro preklad
    @param uid_pwd Soucasna slozka, kde se nachazim (ze ktere vychazim)
*/
int get_uid_by_name(char *dir_name, int uid_pwd){
    struct mft_item mfti;
    int hledane, i, dir_len, cmp_len;

    char *curLine = get_file_content(uid_pwd);

    dir_len = strlen(dir_name);

    char *dirname;
    dirname = (char *) malloc(dir_len);

    //memset(pomocnik, '', 20);
    strncpy(dirname, dir_name, dir_len);

    //printf("EXISTN _%s_\n", dirname);

    // DEBUG_PRINT("get_uid_by_name(dirname = %s, uid_pwd = %d)\n", dirname, uid_pwd);
    DEBUG_PRINT("\tObsah clusteru: %s \n----------\n", curLine);

    // obsah clusteru daneho adresare si ctu po radcich - co jeden radek to UID jednoho souboru nebo slozky
    i = 0;
    while (curLine){
        char * nextLine = strchr(curLine, '\n');
        if (nextLine) *nextLine = '\0';  // temporarily terminate the current line

        if (i != 0){
            // skip prvni radky v clusteru - je tam backlink
            hledane = atoi(curLine);
            //DEBUG_PRINT("\tnactene UID z clusteru %s (int=%d)\n", curLine, hledane);

            // tady si roparsuji MFT a zjistim jestli se shoduje nazev
            if (hledane < CLUSTER_COUNT && mft_seznam[hledane] != NULL){
                mfti = mft_seznam[hledane]->item;
                cmp_len = strlen(mfti.item_name);

                DEBUG_PRINT("\t\tHledane mfti s uid=%d (name=%s) %s cmp_len=%dNOT NULL\n", hledane, mfti.item_name, dirname, cmp_len);

                // todo - isDirectory ... nelze overit unikatnost jmena
                // if (strncmp(mfti.item_name, dirname, cmp_len) == 0 && mfti.isDirectory == 1) {
                if (strncmp(mfti.item_name, dirname, cmp_len) == 0) {
                    DEBUG_PRINT("\t\tSHODA\n");
                    return mfti.uid;
                }
            }
        }
        else {
            // ../../ relativni cesty
            DEBUG_PRINT("\tBacklink teto slozky je %s\n", curLine);

            if (strncmp(dirname, "..", 2) == 0){
                DEBUG_PRINT("Vracim se zpatky\n");
                return atoi(curLine);
            }
        }

        if (nextLine) *nextLine = '\n';  // then restore newline-char, just to be tidy
        curLine = nextLine ? (nextLine + 1) : NULL;

        i++;
    }

    free((void *) dirname);

    return -1;
}

/*
    Zkontroluje jestli je jmeno souboru v danem umisteni unikatni
    @param newname Nazev souboru
    @param uid_pwd Pracovni adresar, ve kterem hledame
*/
int is_name_unique(char *newname, int uid_pwd){
    if (get_uid_by_name(newname, uid_pwd) == -1) {
        return 1;
    }

    return 0;
}

/*
    Vrati odkaz na rodicovksou slozku bliz ke korenu FS
    @param uid_pwd Soucasna slozka
*/
int get_backlink(int uid_pwd) {
    char *curLine = get_file_content(uid_pwd);

    // obsah clusteru daneho adresare si ctu po radcich - co jeden radek to UID jednoho souboru nebo slozky
    while (curLine){
        char * nextLine = strchr(curLine, '\n');
        if (nextLine) *nextLine = '\0';  // temporarily terminate the current line

        DEBUG_PRINT("\tBacklink slozky (UID=%d) je %s = %d\n", uid_pwd, curLine, atoi(curLine));

        return atoi(curLine);

        // tady by bylo prehozeni na dalsi radek, ale to uz neni potreba :)
    }

    return -1;
}

/*
    Prochazi danou cestu a vrati UID slozky, ktera je posledni nebo -1 pri chybe
    @param patha Cela cesta v FS (muze byt i relativni)
    @param cd Pouziva se jen pro funkci cd...
*/
int parsuj_pathu(char *patha, int cd){
    char *p_c;
    int start_dir;
    char path[100];

    // Nelze pracovat primo s arg: https://stackoverflow.com/questions/8957829/strtok-segmentation-fault
    strncpy(path, patha, 100); // bez \0

    // zacinam v rootu ci nikoliv?
    if (strncmp(path, "/", 1) == 0){
        start_dir = 0;
    }
    else {
        start_dir = pwd;
    }
    DEBUG_PRINT("START DIR = %d\n", start_dir);

    if (strcmp(patha, "") != 0) {
        if (strchr(patha, '/') != NULL){
            // zde parsuji cestu zacinajici lomenem
            // parsuji jednotlive casti cesty a norim se hloubeji a hloubeji
            p_c = strtok(path, "/");
            while( p_c != NULL ) {
                start_dir = get_uid_by_name(p_c, start_dir); // pokusim se prevest nazev na UID

                if (start_dir == -1) return -1;
                // jdu o slozku niz

                p_c = strtok(NULL, "/");
            }
        }
        else {
    	   if (cd == 1) {
                // pouziva se pro prikaz cd
                start_dir = get_uid_by_name(patha, start_dir); // pokusim se prevest nazev na UID

                if (start_dir == -1) return -1;
            }
        }
    }

    return start_dir;
}


/*
    Vytvori novou slozku o zadanem nazvu
    @param pwd UID slozky kde mam slozku vytvorit
    @param name Jmeno slozky pro vytvoreni
*/
int zaloz_novou_slozku(int pwd, char *name){
    int i, j, bitmap_free_index, new_cluster_start, volne_uid;
    FILE *fw;
    struct mft_item mfti;
    struct mft_fragment mftf;
    struct mft_item *mpom;
    char pomocnik2[5], pom[5];

    if (is_name_unique(name, pwd) != 1){
        //printf("EXIST _%s_\n", name);
        return -1;
    }

    sprintf(pom, "%d", pwd);

    DEBUG_PRINT("-- NAME OF NEW DIR=%s\n", name);

    // najdu volne UID
    volne_uid = get_volne_uid();

    // najdu volnou bitmapu
    bitmap_free_index = -1;
    for (i = 0; i < CLUSTER_COUNT; i++){
        if (ntfs_bitmap[i] == 0){
            DEBUG_PRINT("index volne bitmapy je %d\n", i);
            bitmap_free_index = i;
            break;
        }
    }

    if (bitmap_free_index != -1){
        new_cluster_start = bootr->data_start_address + bitmap_free_index * CLUSTER_SIZE;

        // ziskam prvni volny MFT LIST a zjistim volne UID, ktere nasledne pridelim nove slozce
        for (i = 0; i < CLUSTER_COUNT; i++){
            if (mft_seznam[i] == NULL){
                // tento prvek je volny, vyuziji jej tedy
                mpom = malloc(sizeof(struct mft_item));

                mftf.fragment_start_address = new_cluster_start; // start adresa ve VFS
                mftf.fragment_count = 1;

                mfti.uid = volne_uid;
                mfti.isDirectory = 1;
                mfti.item_order = 1;
                mfti.item_order_total = 1;
                strcpy(mfti.item_name, name);
                mfti.item_size = strlen(pom); // zatim tam nic neni, takze nula
                mfti.fragments[0] = mftf;

                // dalsi fragmenty z budou jen prazdne (pro poradek)
                mftf.fragment_start_address = 0;
                mftf.fragment_count = 0;

                // zacinam od jednicky
                for (j = 1; j < MFT_FRAG_COUNT; j++){
                    mfti.fragments[j] = mftf;
                }

                // prvek mam pripraveny
                // zaktualizuji si globalni pole a bitmapu
                ntfs_bitmap[bitmap_free_index] = 1;
                pridej_prvek_mft(volne_uid, mfti);

                // zapisu do souboru
                fw = fopen(output_file, "r+b");
                if(fw != NULL){
                    // mfti
                    mpom = &mfti;
                    DEBUG_PRINT("-- MFTI chci zapsat na adresu %lu\n", sizeof(struct boot_record) + volne_uid * sizeof(struct mft_item));
                    fseek(fw, sizeof(struct boot_record) + volne_uid * sizeof(struct mft_item), SEEK_SET);
                    fwrite(mpom, sizeof(struct mft_item), 1, fw);

                    // bitmap
                    DEBUG_PRINT("-- Bitmapu chci zapisovat na adresu %u\n", bootr->bitmap_start_address);
                    fseek(fw, bootr->bitmap_start_address, SEEK_SET);
                    fwrite(ntfs_bitmap, 4, CLUSTER_COUNT, fw);

                    // odkaz na slozku do nadrazeneho adresare (zapis do clusteru)
                    DEBUG_PRINT("-- Zapisuji odkaz na adresar %d do adresare %d\n", volne_uid, pwd);
                    sprintf(pomocnik2, "%d", volne_uid);
                    append_file_content(pwd, pomocnik2, 1);

                    // odkaz na nadrazenou slozku do teto slozky - backlink
                    // budou to prvni zapsana data v teto slozce
                    DEBUG_PRINT("-- Zapisuji backlink na adresar %d do adresare %d, adresa je %d\n", pwd, volne_uid, bootr->data_start_address + bitmap_free_index * CLUSTER_SIZE);
                    fseek(fw, bootr->data_start_address + bitmap_free_index * CLUSTER_SIZE, SEEK_SET);
                    fwrite(pom, 1, strlen(pom), fw);

                    fclose(fw);
                }

                break;
            }
        }
    }

    return volne_uid;
}

/* Ziskani informaci o souborech ve slozce */
void ls_printer(int uid) {
    char *p_c;
    int i = 0;
    struct mft_item mfti;

    // chci vypsat obsah aktualniho adresare
    char *buffer = get_file_content(uid);
    DEBUG_PRINT("obsah bufferu je: %s\n", buffer);

    printf("--- NAZEV ----- VELIKOST - UID ---\n");

    // iteruji pro kazdou polozku z adresare a hledam jeji nazev
    p_c = strtok(buffer, "\n");
    DEBUG_PRINT("ID nadrazene slozky je %s\n", p_c);

    // prvni odkaz je odkaz na nadrazenou slozku

    while((p_c = strtok(NULL, "\n")) != NULL){
        if (mft_seznam[atoi(p_c)] != NULL){
            mfti = mft_seznam[atoi(p_c)]->item;

            printf(" ");
            if (mfti.isDirectory == 1){
                printf("+");
            }
            else{
                printf("-");
            }
            printf(" %-15s %-7d %d\n", mfti.item_name, mfti.item_size, mfti.uid);
        }

        i++;
    }

    printf("-- Celkem souboru: %d --\n", i);
}

int is_empty_dir(int file_uid) {
    char *curLine = get_file_content(file_uid);
    int i = 0;

    // obsah clusteru daneho adresare si ctu po radcich - co jeden radek to UID jednoho souboru nebo slozky
    while (curLine){
        char * nextLine = strchr(curLine, '\n');
        if (nextLine) *nextLine = '\0';  // temporarily terminate the current line

        i++;

        if (nextLine) *nextLine = '\n';  // then restore newline-char, just to be tidy
        curLine = nextLine ? (nextLine + 1) : NULL;
    }

    return i;
}

char* read_file_from_pc(char *pc_soubor){
    int size;
    FILE *fr;
    char *ret;

    fr = fopen(pc_soubor, "r");
    if (fr != NULL){
        // zjistim si delku souboru
        fseek(fr, 0, SEEK_END);
        size = ftell(fr);

        ret = (char *) malloc(size);
        DEBUG_PRINT("-- size souboru %s je=%d\n", pc_soubor, size);

        // prectu soubor do promenne
        fseek(fr, 0, SEEK_SET);
        fread(ret, 1, size, fr);

        DEBUG_PRINT("-- nacteno z pocitace: %s\n", ret);

        fclose(fr);
    }

    return ret;
}


/*
    Vytvori soubor z pocitace (incp)
*/
void vytvor_soubor(int cilova_slozka, char *filename, char *text, int puvodni_uid, int is_dir, int odkaz){
    int i, j, l, size, potreba_clusteru, volne_uid, spoj_len, starter;
    FILE *fw;
    char pom[20];
    struct mft_fragment mftf;
    struct mft_item mfti;
    struct mft_item *mpom;

    // delka textu
    size = strlen(text);

    // volne UID pro soubor
    if (puvodni_uid == -1){
        volne_uid = get_volne_uid();
    }
    else {
        volne_uid = puvodni_uid;
    }

    // kolik budu potrebovat najit clusteru
    potreba_clusteru = size / CLUSTER_SIZE + 1;
    int volne_clustery[potreba_clusteru];

    DEBUG_PRINT("-- Je potreba %d volnych clusteru\n", potreba_clusteru);

    j = 0;
    for (i = 0; i < CLUSTER_COUNT; i++) {
        if (ntfs_bitmap[i] == 0) {
            // volna
            volne_clustery[j] = i;
            DEBUG_PRINT("-- Volny cluster: %d\n", i);
            j++;
        }

        if (j == potreba_clusteru) {
            break;
        }
    }

    if (j != potreba_clusteru){
        printf("ERROR - NOT ENOUGH CLUSTERS (%d)\n", j);
        return;
    }

    // otevru si spojeni s nasim fs
    fw = fopen(output_file, "r+b");
    if (fw != NULL) {

        // pripravim si mft item
        mfti.uid = volne_uid;
        mfti.isDirectory = is_dir;
        mfti.item_order = 1;
        mfti.item_order_total = 1;
        strcpy(mfti.item_name, filename);
        mfti.item_size = strlen(text); // zatim tam nic neni, takze nula

        // reseni spojitosti a nespojitosti bitmapy
        spoj_len = 1;
        starter = 0;
        l = 0;
        for (j = 0; j < potreba_clusteru; j++) {
            //printf("%d: spojity: %d ?= %d\n", i, volne_clustery[j+1], volne_clustery[j]+1);
            if (volne_clustery[j+1] == volne_clustery[j]+1) {
                spoj_len = spoj_len + 1;

                if (spoj_len == 2){
                    starter = volne_clustery[j];
                    //printf("\t starter = %d\n", starter);
                }
            }
            else {
                if (spoj_len != 1) {
                    //printf("Muzu zpracovat spojity blok, ktery zacina na %d a je dlouhy %d\n", starter, spoj_len);
                    DEBUG_PRINT("1) f(%d, %d)\n", starter, spoj_len);

                    mftf.fragment_start_address = bootr->data_start_address + starter * CLUSTER_SIZE; // adresa do VFS do clusteru
                    DEBUG_PRINT("1) mftf.fragment_start_address = %d\n", mftf.fragment_start_address);
                }
                else {
                    DEBUG_PRINT("2) f(%d, %d)\n", volne_clustery[j], spoj_len);

                    mftf.fragment_start_address = bootr->data_start_address + volne_clustery[j] * CLUSTER_SIZE; // adresa do VFS do clusteru
                }

                mftf.fragment_count = spoj_len;
                mfti.fragments[l] = mftf;

                DEBUG_PRINT("X) mftf.fragment_start_address = %d\n", mftf.fragment_start_address);

                // prubezne (po castech) vkladam obsah noveho souboru
                text = set_fragment_content(mftf, text);

                l++;

                spoj_len = 1;
                starter = 0;
            }
        }

        if (spoj_len != 1) {
            //printf("Muzu zpracovat spojity blok, ktery zacina na %d a je dlouhy %d\n", starter, spoj_len);
            DEBUG_PRINT("3) f(%d, %d)\n", starter, spoj_len);

            mftf.fragment_start_address = bootr->data_start_address + starter * CLUSTER_SIZE; // adresa do VFS do clusteru
            mftf.fragment_count = spoj_len;

            mfti.fragments[l] = mftf;
            l++;

            // prubezne (po castech) vkladam obsah noveho souboru
            text = set_fragment_content(mftf, text);
        }

        // dalsi fragmenty z budou jen prazdne (pro poradek)
        mftf.fragment_start_address = 0;
        mftf.fragment_count = 0;

        // zacinam od jednicky
        for (j = l; j < MFT_FRAG_COUNT; j++){
            mfti.fragments[j] = mftf;
        }

        // aktualizuji bitmapu vsude
        for (j = 0; j < potreba_clusteru; j++){
            ntfs_bitmap[volne_clustery[j]] = 1;
        }
        fseek(fw, bootr->bitmap_start_address, SEEK_SET);
        fwrite(ntfs_bitmap, 4, CLUSTER_COUNT, fw);

        // mfti
        pridej_prvek_mft(volne_uid, mfti);
        mpom = malloc(sizeof(struct mft_item));
        mpom = &mfti;
        DEBUG_PRINT("-- MFTI chci zapsat na adresu %lu\n", sizeof(struct boot_record) + volne_uid * sizeof(struct mft_item));
        fseek(fw, sizeof(struct boot_record) + volne_uid * sizeof(struct mft_item), SEEK_SET);
        fwrite(mpom, sizeof(struct mft_item), 1, fw);

        // odkaz na slozku do nadrazeneho adresare
        if (odkaz == 1) {
            DEBUG_PRINT("-- Zapisuji odkaz na soubor %d do adresare %d\n", volne_uid, cilova_slozka);
            sprintf(pom, "%d", volne_uid);
            append_file_content(cilova_slozka, pom, 1);
        }

        fclose(fw);
    }
}
