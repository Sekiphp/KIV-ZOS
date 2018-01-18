#ifndef HELPER_H
#define HELPER_H

    /* Hlavicky funkci ze souboru ntfs_helpers.c -> komentare jsou tam */
    char* get_cluster_content(int32_t adresa);
    int set_cluster_content(int32_t adresa, char *obsah);

    char* get_fragment_content(struct mft_fragment fragment);

    char* get_file_content(int file_uid);

    int append_file_content(int file_uid, char *append);


    //char* get_cluster_content(int32_t fragment_start_addr, int32_t fragments_count);
    //int set_cluster_content(int32_t cluster_start_addr, char *obsah);

#endif

