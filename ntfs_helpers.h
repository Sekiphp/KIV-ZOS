#ifndef HELPER_H
#define HELPER_H

    /* Hlavicky funkci ze souboru ntfs_helpers.c -> komentare jsou tam */
    char* get_cluster_content(int32_t adresa);
    int set_cluster_content(int32_t adresa, char *obsah);
    void clear_bitmap(struct mft_fragment fragment);
    void clear_mft(int file_uid);
    void clear_fragment_content(struct mft_fragment fragment);
    char* get_fragment_content(struct mft_fragment fragment);
    char* set_fragment_content(struct mft_fragment fragment, char *zbyvajici_obsah);
    char* get_file_content(int file_uid);
    void delete_file(int file_uid);
    int update_filesize(int file_uid, int length);
    int append_file_content(int file_uid, char *append, int dir);
    void edit_file_content(int file_uid, char *text, char *filename, int puvodni_uid);
    int vytvor_soubor_v_mft(FILE *fw, int volne_uid, char *filename, char *text, struct mft_fragment fpom[], int fpom_size, int is_dir);

#endif