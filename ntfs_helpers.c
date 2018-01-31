#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debugger.h"
#include "mft.h"
#include "ntfs_helpers.h"
#include "loader.h"
#include "shell_functions.h"
#include "boot_record.h"
#include "functions.h"
#include "parametr.h"

extern int pwd;
extern char output_file[100];
extern int ntfs_bitmap[]; // v loader.c

/*
    Ziska data z jednoho clusteru
    @param adresa Adresa pocatku datoveho bloku
    @return Obsah bloku
*/
char* get_cluster_content(int32_t adresa) {
    FILE *fr;
    char *ret;

    ret = (char*) malloc(CLUSTER_SIZE);

    fr = fopen(output_file, "rb");
    if (fr != NULL) {
        fseek(fr, adresa, SEEK_SET);
        fread(ret, sizeof(char), CLUSTER_SIZE, fr);

        fclose(fr);
    }

    //DEBUG_PRINT("/%s/ -> strlen(ret)=%d\n", ret, strlen(ret));

    return ret;
}

/*
    Prepise obsah clusteru
    @param adresa Adresa pocatku datoveho bloku
    @param obsah Novy obsah bloku
    @return Zaporne hodnoty jsou chybne
*/
int set_cluster_content(int32_t adresa, char *obsah) {
    FILE *f;

    f = fopen(output_file, "r+b");
    if (f != NULL) {
        fseek(f, adresa, SEEK_SET);
        fwrite(obsah, 1, CLUSTER_SIZE, f);

        fclose(f);
        return 1;
    }

    return -1;
}

/*
    Smaze data ve vsech clusterech patricich k danemu souboru a uklidi po danem souboru
    @param file_uid UID souboru ke smazani
*/
void delete_file(int file_uid) {
    MFT_LIST* mft_itemy;
    struct mft_item mfti;
    struct mft_fragment mftf;
    int j;

    // najdu vsechny mfti
    if (mft_seznam[file_uid] != NULL){
        mft_itemy = mft_seznam[file_uid];

        // iteruji mfti
        while (mft_itemy != NULL){
            mfti = mft_itemy->item;

            // najdu vsechny mftf
            for (j = 0; j < MFT_FRAG_COUNT; j++){
                mftf = mfti.fragments[j];

                if (mftf.fragment_start_address != 0 && mftf.fragment_count > 0) {
                    // prepisu data - smazu clustery v souboru
                    clear_fragment_content(mftf);

                    // vymazu bitmapu (virtualne i v souboru)
                    clear_bitmap(mftf);
                }
            }

            // prehodim se na dalsi prvek
            mft_itemy = mft_itemy->dalsi;
        }

        // vycistim mft (virtualne i v souboru)
        clear_mft(file_uid);
    }
}

/*
    Smaze zaznam z mtf
    @param file_uid
*/
void clear_mft(int file_uid) {
    FILE *fw;
    int i, adresa;
    char obsah[sizeof(struct mft_item)];

    // vynuluji z listu (virtualne)
    mft_seznam[file_uid] = NULL;

    fw = fopen(output_file, "r+b");
    if(fw != NULL) {
        for(i = 0; i < CLUSTER_COUNT; i++) {
            if (i == file_uid) {
                // prepisu mfti prazdnem
                adresa = sizeof(struct boot_record) + sizeof(struct mft_item) * file_uid;
                memset(obsah, 0, sizeof(struct mft_item));
                DEBUG_PRINT("-- MFTI chci zapisovat na adresu %u\n", adresa);
                fseek(fw, adresa, SEEK_SET);
                fwrite(obsah, 1, sizeof(struct mft_item), fw);
            }
        }

        fclose(fw);
    }

    DEBUG_PRINT("clear mft success\n");
}

/*
    Ziska obsah vsech fragmentu patricich do clusteru
    @param fragment Struktura fragmentu, kterou chceme cist
    @return Obsah celeho fragmentu
*/
char* get_fragment_content(struct mft_fragment fragment) {
    int adresa, bloku, i;
    char *ret;

    adresa = fragment.fragment_start_address;
    bloku = fragment.fragment_count;
    ret = (char*) malloc(bloku * CLUSTER_SIZE);
    strcpy(ret, "");

    if (adresa != 0) {
        for (i = 0; i < bloku; i++) {
            strcat(ret, get_cluster_content(adresa));

            adresa += CLUSTER_SIZE;
        }
    }

    //ret[strlen(ret)] = '\0';

    return ret;
}

/*
    Naplni clustery z daneho fragmentu
    @param fragment Informace o fragmentu
    @param zbyvajici_obsah Obsah pro naplneni
    @return Vratim zbytek strungu, ktery se sem nevesel
*/
char* set_fragment_content(struct mft_fragment fragment, char *zbyvajici_obsah) {
    int i;
    int adresa = fragment.fragment_start_address;

    //DEBUG_PRINT("-- %s\n", zbyvajici_obsah);

    for (i = 0; i < fragment.fragment_count; i++) {
        DEBUG_PRINT("ZAPISUJI BLOK %d z %d na adresu %d, zbyva obsahu %zd\n", i+1, fragment.fragment_count, adresa, strlen(zbyvajici_obsah));

        set_cluster_content(adresa, zbyvajici_obsah);

        adresa += CLUSTER_SIZE;
        zbyvajici_obsah += CLUSTER_SIZE;
    }

    return zbyvajici_obsah;
}

/*
    Vynuluje obsah zadaneho fragmentu
    @param fragment Struktura fragmentu, ktery chceme prepsat
*/
void clear_fragment_content(struct mft_fragment fragment) {
    int adresa, bloku, i;
    char obsah[CLUSTER_SIZE];

    adresa = fragment.fragment_start_address;
    bloku = fragment.fragment_count;

    memset(obsah, 0, CLUSTER_SIZE);

    if (adresa != 0) {
        for (i = 0; i < bloku; i++) {
            set_cluster_content(adresa, obsah);

            adresa += CLUSTER_SIZE;
        }
    }
}

/*
    Vynuluje obsah zadaneho fragmentu
    @param fragment Struktura fragmentu, ktey chceme prepsat
*/
void clear_bitmap(struct mft_fragment fragment) {
    int index_s, index_e, i;
    FILE *fw;

    // podle adresy pozname ID clusteru
    index_s = (fragment.fragment_start_address - bootr->data_start_address) / CLUSTER_SIZE;
    index_e = index_s + fragment.fragment_count;

    DEBUG_PRINT("-- Index bitmapy pro vynulovani je %d\n", index_s);

    // updatuju virtualni bitmapu
    for (i = index_s; i < index_e; i++) {
        ntfs_bitmap[i] = 0;
    }

    // prepisu celou bitmapu v souboru
    fw = fopen(output_file, "r+b");
    if(fw != NULL){
        DEBUG_PRINT("-- Bitmapu chci zapisovat na adresu %u\n", bootr->bitmap_start_address);
        fseek(fw, bootr->bitmap_start_address, SEEK_SET);
        fwrite(ntfs_bitmap, 4, CLUSTER_COUNT, fw);

        fclose(fw);
    }
}

/*
    Ziska obsah celeho souboru - precte si vsechny udaje z MFTLISTU (MFTI, MFTF)
    @param file_uid UID souboru, ktery chceme cist
    @return Obsah celeho souboru
*/
char* get_file_content(int file_uid) {
    int i, j;
    char *ret;
    MFT_LIST* mft_itemy;
    struct mft_item mfti;
    struct mft_fragment mftf;

    // existuje takovy item v MFT
    if (mft_seznam[file_uid] != NULL){
        mft_itemy = mft_seznam[file_uid];

        //DEBUG_PRINT("stoji za zminku\n");

        // projedeme vsechny itemy pro dane UID souboru
        // bylo by dobre si pak z tech itemu nejak sesortit fragmenty dle adres
        // zacneme iterovar pres ->dalsi
        i = 0;
        while (mft_itemy != NULL){
            mfti = mft_itemy->item;
            i++;

            // alokujeme si celou velikost dle MFT
            if (i == 1) {
                ret = (char*) malloc(((mfti.item_size / CLUSTER_SIZE) + 1) * CLUSTER_SIZE);
                strcpy(ret, "");
            }

            DEBUG_PRINT("[%d] Nacteny item s UID=%d ma nazev %s\n", i, mfti.uid, mfti.item_name);

            // precteme vsechny fragmenty z daneho mft itemu (je jich: MFT_FRAG_COUNT)
            for (j = 0; j < MFT_FRAG_COUNT; j++){
                mftf = mfti.fragments[j];

                if (mftf.fragment_start_address != 0 && mftf.fragment_count > 0) {
                    DEBUG_PRINT("-- Fragment %d ze souboru s UID %d, start=%d, count=%d\n", j, mfti.uid, mftf.fragment_start_address, mftf.fragment_count);

                    char *fragc = get_fragment_content(mftf);

                    //DEBUG_PRINT("get_fragment_content(mftf)=%s\n", fragc);
                    strncat(ret, fragc, strlen(fragc));
                }
            }

            // prehodim se na dalsi prvek
            mft_itemy = mft_itemy->dalsi;
        }
    }

    //ret[strlen(ret)] = '\0';

    return ret;
}

/*
    Upravi danemu souboru velikost
    @param file_uid Unikatni cislo souboru
    @param length Delka pro zapis do MFTI souboru
*/
int update_filesize(int file_uid, int length){
    FILE *fw;
    struct mft_item *mpom;
    int mft_size = sizeof(struct mft_item);

    fw = fopen(output_file, "r+b");
    if (fw != NULL) {
        mpom = malloc(mft_size);

        // aktualizuji virtualni MFT
        mft_seznam[file_uid]->item.item_size = length;

        // zapisu mft
        mpom = &mft_seznam[file_uid]->item;
        fseek(fw, bootr->mft_start_address + file_uid * mft_size, SEEK_SET);
        fwrite(mpom, mft_size, 1, fw);

        fclose(fw);

        return 0;
    }

    return -1;
}

/*
    Pripoji na konec souboru dalsi data
    @param file_uid UID souboru
    @param append Retezec pro pripojeni nakonec souboru
    @param dir Je slozka nebo ne
*/
int append_file_content(int file_uid, char *append, int dir){
    int i, j, adresa, delka, mftf_adr;
    char *ret;
    MFT_LIST* mft_itemy;
    FILE *fw;

    ret = (char *) malloc(CLUSTER_SIZE);
    //char *soucasny_obsah = get_file_content(file_uid);

    //printf("Soucasny obsah souboru je: %s a ma delku %zd --- \n", soucasny_obsah, strlen(soucasny_obsah));
    DEBUG_PRINT("Chci appendnout %zd znaku: %s\n", strlen(append), append);

    fw = fopen(output_file, "r+b");
    if (fw != NULL) {
        // musim si vypocitat adresu, kam budu zapisovat
        adresa = 0;
        if (mft_seznam[file_uid] != NULL){
            mft_itemy = mft_seznam[file_uid];

            // projedeme vsechny itemy pro dane UID souboru
            // zacneme iterovar pres ->dalsi
            i = 0;
            while (mft_itemy != NULL){
                i++;
                DEBUG_PRINT("-- [%d] Nacteny item s UID=%d ma nazev %s\n", i, mft_itemy->item.uid, mft_itemy->item.item_name);

                // precteme vsechny fragmenty z daneho mft itemu (maximalne je jich: MFT_FRAG_COUNT)
                for (j = 0; j < MFT_FRAG_COUNT; j++){
                    mftf_adr = mft_itemy->item.fragments[j].fragment_start_address;

                    // najdu si posledni fragment s adresou
                    if (mftf_adr > 0 && mft_itemy->item.fragments[j].fragment_count > 0) {
                        DEBUG_PRINT("-- MFTF adr = %d, %d\n", mftf_adr, mft_itemy->item.fragments[j].fragment_count);
                        adresa = mftf_adr;
                    }
                }

                // prehodim se na dalsi prvek
                mft_itemy = mft_itemy->dalsi;
            }
        }

        if (adresa != 0){
            // nactu obsah daneho clusteru
            strcpy(ret, get_cluster_content(adresa));
            DEBUG_PRINT("OBSAH = %s\n", ret);

            // pripojim k nemu co potrebuji
            if (dir == 1)
                strcat(ret, "\n"); // tohle plati jen pro slozky

            strcat(ret, append);
            delka = strlen(ret);

            // zapisu
            fseek(fw, adresa, SEEK_SET);
            fwrite(ret, 1, delka, fw);

            // zaktualizuji virtualni mft i mft v souboru
            update_filesize(file_uid, delka);

            DEBUG_PRINT("Dokoncuji editaci clusteru /%s/; strlen=%d\n", ret, delka);
            //DEBUG_PRINT("=========== VYSLEDEK ==========\n%s\n========== /VYSLEDEK ==========\n", get_file_content(file_uid));
        }
        else {
            return -1;
        }

        fclose(fw);
    }

    //free((void *) ret);

    return 1;
}

/*
    Zmeni obsah souboru - resi nafukovani a smrstovani souboru
    @param file_uid Soubor pro editaci
    @param text Cely (novy) obsah
    @param filename Nazev souboru
    @param puvodni_uid Puvodni UID souboru pokud ma byt zachovano
 */
void edit_file_content(int file_uid, char *text, char *filename, int puvodni_uid){
    DEBUG_PRINT("EDIT FILE (int file_uid=%d, char *text=%s, char *filename=%s, int puvodni_uid=%d)\n", file_uid, text, filename, puvodni_uid);

    delete_file(file_uid);
    vytvor_soubor(pwd, filename, text, puvodni_uid, 1, 0);
}

/*
    Vytvori MFT info o souboru (jak na disku tak virtualne)
*/
int vytvor_soubor_v_mft(FILE *fw, int volne_uid, char *filename, char *text, struct mft_fragment fpom[], int fpom_size, int is_dir) {
    int i, j, k, l, adresa_mfti;
    struct mft_item *mpom, *mff;

    mff = malloc(sizeof(struct mft_item));

    // vypocitam si skutecny pocet fragmentu
    int pocet_fragu = 0;
    int prvku = (fpom_size / sizeof(struct mft_fragment));
    DEBUG_PRINT("PRVKU=%d\n", prvku);
    for (i = 0; i < prvku; i++) {
        DEBUG_PRINT("OVERUJI start=%d, count=%d\n", fpom[i].fragment_start_address, fpom[i].fragment_count);
        if (fpom[i].fragment_start_address != -1) {
            pocet_fragu++;
        }
    }

    int potreba_mfti = (pocet_fragu / MFT_FRAG_COUNT) + (pocet_fragu % MFT_FRAG_COUNT);
    int sizeof_mft_item = sizeof(struct mft_item);

    DEBUG_PRINT("potreba_mfti=(%d / %d) + (%d mod %d) = %d\n", pocet_fragu, MFT_FRAG_COUNT, pocet_fragu, MFT_FRAG_COUNT, potreba_mfti);

    // vytvorim mfti pole o spravne velikosti
    k = 0;
    struct mft_item mfti[potreba_mfti];
    for (i = 0; i < potreba_mfti; i++) {
        mfti[i].uid = volne_uid;
        mfti[i].isDirectory = is_dir;
        mfti[i].item_order = i + 1;
        mfti[i].item_order_total = potreba_mfti;
        strcpy(mfti[i].item_name, filename);
        mfti[i].item_size = strlen(text);

        // kazdemu z tech prvku napushuju fragmenty co to pujde
        for (j = 0; j < MFT_FRAG_COUNT; j++) {
            mfti[i].fragments[j] = fpom[k];

            // vkladani textu souboru
            //printf("////%d////\n", fpom[k].fragment_count);
            text = set_fragment_content(fpom[k], text);
            k++;
        }

        // pridam ho virtualne
        pridej_prvek_mft(volne_uid, mfti[i]);

        // pridam ho do souboru
        mpom = malloc(sizeof(struct mft_item));
        mpom = &mfti[i];

        // musim najit adresu v MFT bloku v souboru
        for (l = 0; l < CLUSTER_COUNT; l++) {
            adresa_mfti = bootr->mft_start_address + (l * sizeof_mft_item);

            fseek(fw, adresa_mfti, SEEK_SET);
            fread(mff, sizeof_mft_item, 1, fw);

            DEBUG_PRINT("if (%d == %d) or (%s == '')\n", mff->uid, UID_ITEM_FREE, mff->item_name);
            if (mff->uid == UID_ITEM_FREE || strcmp(mff->item_name, "") == 0) {
                DEBUG_PRINT("-- MFTI chci zapsat na adresu %d\n", adresa_mfti);

                fseek(fw, adresa_mfti, SEEK_SET);
                fwrite(mpom, sizeof(struct mft_item), 1, fw);

                break;
            }
        }
    }

    return 1;
}
