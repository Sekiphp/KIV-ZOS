#ifndef SHELL_FUNC_H
#define SHELL_FUNC_H

    /* Hlavicky funkci ze souboru shell_functions.c -> komentare jsou tam */
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
