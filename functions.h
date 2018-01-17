#ifndef FUNC_H
#define FUNC_H

    /* Hlavicky funkci ze souboru functions.c -> komentare jsou tam */
    char* get_mft_item_content(int uid);
    char* get_cluster_content(int32_t fragment_start_addr, int32_t fragments_count);
    int parsuj_pathu(char *patha);
    int zaloz_novou_slozku(int pwd, char *name);
    int get_uid_by_name(char *dir_name, int uid_pwd);
    int append_obsah_souboru(int uid, char *append);

#endif