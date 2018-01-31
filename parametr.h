#ifndef PARAM_H
#define PARAM_H

/* Globalni promenne */
extern struct boot_record *boot;
extern char output_file[100];

typedef struct {
    pthread_mutex_t * mutex;
    int zpracovany_cluster;
} sdilenaPamet;


#endif
