#ifndef HELPER_H
#define HELPER_H

    /* Hlavicky funkci ze souboru ntfs_helpers.c -> komentare jsou tam */
    char* get_cluster_content(int32_t adresa);
    int set_cluster_content(int32_t adresa, char *obsah);
    char* get_fragment_content(struct mft_fragment fragment);
    char* get_file_content(int file_uid);
    int set_file_content(int file_uid);
    int update_filesize(int file_uid, int length);
    int append_file_content(int file_uid, char *append);

#endif