#ifndef PARAM_H
#define PARAM_H

    /* Hlavicky funkci ze souboru loader.c -> komentare jsou tam */
typedef struct {
    pthread_mutex_t * mutex;
    int pokus;
  char soubor[50];
} sdilenaPamet;


#endif
