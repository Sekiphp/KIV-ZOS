#include <stdio.h>
#include <stdlib.h>

#include "loader.h"
#include "boot_record.h"

/* Alokuje prvek mft listu - vcetne testovani na dostatek pameti */
MFT_LIST *alokuj_prvek(struct mft_item) {
    MFT_LIST *ml;
    if ((ml = (MFT_LIST *) malloc(sizeof(MFT_LIST))) == NULL) {
        printf("Out of memory - MFT_LIST\n");
    }

    ml->item = mft_item;
    ml->dalsi = NULL;

    return ml;
}

void pridej_prvek(struct mft_item) {
    if(mft_list == NULL){
        mft_list = alokuj_hranu(i, j, w);
    }
    else {
        MFT_LIST *mpom;
        mpom = mft_list;
        mft_list = alokuj_prvek(mft_item);
        mft_list->dalsi = mpom;
    }
}