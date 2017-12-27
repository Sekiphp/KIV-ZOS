#ifndef LOADER_H
#define LOADER_H

    #define CLUSTER_COUNT 10
    #define CLUSTER_SIZE 1024

    int ntfs_bitmap[CLUSTER_COUNT];

    /* Hlavicky funkci ze souboru loader.c -> komentare jsou tam */
    void loader(char filename[]);
    int zaloz_soubor(int cluster_size, int cluster_count, char filename[]);

#endif
