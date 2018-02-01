#ifndef LOADER_H
#define LOADER_H

    #define CLUSTER_COUNT 200 // je i v mft.h; celkovy pocet clusteru v FS
    #define CLUSTER_SIZE 100

    int ntfs_bitmap[CLUSTER_COUNT];
    int32_t pwd;
    struct boot_record *bootr;

    /* Hlavicky funkci ze souboru loader.c -> komentare jsou tam */
    void loader(char filename[]);
    void zaloz_soubor(int cluster_size, int cluster_count, char filename[]);

#endif
