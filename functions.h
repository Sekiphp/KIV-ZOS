#ifndef FUNC_H
#define FUNC_H

    /* Hlavicky funkci ze souboru functions.c -> komentare jsou tam */
    int parsuj_pathu(char *patha);
    int zaloz_novou_slozku(int pwd, char *name);
    int get_uid_by_name(char *dir_name, int uid_pwd);
    void ls_printer(char *p_c);
    void ls(int uid);
    void vytvor_soubor_z_pc(int cilova_slozka, char *pc_soubor);

#endif
