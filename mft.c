#include <stdio.h>
#include <stdlib.h>

#include "loader.h"
#include "boot_record.h"
#include "mft.h"

/* Alokuje prvek mft listu - vcetne testovani na dostatek pameti */
MFT_LIST *alokuj_prvek(struct mft_item mfti) {
    MFT_LIST *ml;
    if ((ml = (MFT_LIST *) malloc(sizeof(MFT_LIST))) == NULL) {
        printf("Out of memory - MFT_LIST\n");
    }

    ml->item = &mfti;
    ml->dalsi = NULL;

    return ml;
}

void pridej_prvek(int cluster_id, struct mft_item *mfti) {
    if(mft_list[cluster_id] == NULL){
        mft_list[cluster_id] = mfti;
    }
    else {
        MFT_LIST *mpom;
        mpom = mft_list[cluster_id];
        mft_list[cluster_id] = mfti;
        mft_list[cluster_id]->dalsi = mpom;
    }
}
