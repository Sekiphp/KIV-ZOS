#ifndef SHELL_FUNC_H
#define SHELL_FUNC_H

    /* Hlavicky funkci ze souboru shell_functions.c -> komentare jsou tam */
    char* get_mft_item_content(int32_t uid);
    char* get_cluster_content(int32_t fragment_start_addr, int32_t fragments_count);
    int parsuj_pathu(char *patha);

    void func_cp(char *cmd);
    void func_mv(char *cmd);
    void func_rm(char *cmd);
    void func_mkdir(char *cmd);
    void func_rmdir(char *cmd);
    void func_ls(char *cmd);
    void func_cat(char *cmd);
    void func_cd(char *cmd);
    void func_pwd(char *cmd);
    void func_info(char *cmd);
    void func_incp(char *cmd);
    void func_outcp(char *cmd);
    void func_load(char *cmd);

#endif
