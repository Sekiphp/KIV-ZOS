#include <stdio.h>
#include <stdlib.h>

#include "debugger.h"
#include "loader.h"
#include "boot_record.h"
#include "mft.h"

/* Alokuje prvek mft listu - vcetne testovani na dostatek pameti */
MFT_LIST *alokuj_prvek_mft(struct mft_item mfti) {
    MFT_LIST *ml;
    if ((ml = (MFT_LIST *) malloc(sizeof(MFT_LIST))) == NULL) {
        printf("Out of memory - MFT_LIST\n");
    }

    ml->item = mfti;
    ml->dalsi = NULL;

    return ml;
}

/* Prida prvek mft na prislusny index do globalniho mft pole */
void pridej_prvek_mft(int uid, struct mft_item mfti) {
    //printf("Pridavam prvek do mft UID=%d\n", uid);

    MFT_LIST *pom = alokuj_prvek_mft(mfti);

    if(mft_seznam[uid] == NULL){
        mft_seznam[uid] = pom;
    }
    else {
        // vlozime prvek nakonec
        MFT_LIST *mpom = mft_seznam[uid];
        while (mpom != NULL) {
            if (mpom->dalsi == NULL) {
                mpom->dalsi = pom;
                break;
            }

            mpom = mpom->dalsi;
        }
    }
}
