#ifndef LOADER_H
#define LOADER_H

    /* Hlavicky funkci ze souboru loader.c -> komentare jsou tam */
    void *loader(void * arg);
    int zaloz_soubor(int cluster_size, int cluster_count, char filename[]);

#endif
