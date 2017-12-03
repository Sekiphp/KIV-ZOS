#ifndef PARAM_H
#define PARAM_H

/* Globalni promenne */
extern struct boot_record *boot;

typedef struct {
    pthread_mutex_t * mutex;
    int pokus;
  char soubor[50];
} sdilenaPamet;


#endif
