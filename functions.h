#ifndef FUNC_H
#define FUNC_H

    /* Hlavicky funkci ze souboru functions.c -> komentare jsou tam */
    int get_volne_uid();
    int parsuj_pathu(char *patha, int cd);
    int zaloz_novou_slozku(int pwd, char *name);
    int get_uid_by_name(char *dir_name, int uid_pwd);
    int is_name_unique(char *newname, int uid_pwd);
    void ls_printer(int uid);
    int is_empty_dir(int file_uid);
    char* read_file_from_pc(char *pc_soubor);
    void vytvor_soubor(int cilova_slozka, char *filename, char *text, int puvodni_uid, int is_dir, int odkaz);
    int get_backlink(int uid_pwd);
    void *kontrola_konzistence(void *arg);

#endif
