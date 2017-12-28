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
/*
    ml->item = mfti;
    ml->dalsi = NULL;
*/
    return ml;
}

void pridej_prvek(int uid, struct mft_item mfti) {
    printf("Pridavam prvek do mft UID=%d\n", uid);
    MFT_LIST *pom;
    pom = (MFT_LIST *) malloc(sizeof(MFT_LIST));
    pom->item = mfti;
    pom->dalsi = NULL;

    if(mft_seznam[uid] == NULL){
	pom->ij = 55;
        mft_seznam[uid] = pom;
    }
    else {/*
        MFT_LIST *mpom;
        mpom = mft_seznam[cluster_id];
        pom->dalsi = mpom;
	pom->ij = 666;*/
    }
}
