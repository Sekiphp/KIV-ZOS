#ifndef PARAM_H
#define PARAM_H

/* Globalni promenne */
extern struct boot_record *boot;
extern char output_file[100];

/* Sdilena pamet mezi vlakny kontroly konzistence */
typedef struct {
    pthread_mutex_t * mutex;
    int zpracovany_cluster;
} sdilenaPamet;


#endif
