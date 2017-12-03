#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "loader.h"
#include "boot_record.h"
#include "mft.h"

/* Nacte NTFS ze souboru */
void *create_example(void *arg){
    printf("CREATE EXAMPLE starting...\n");
int a[10];
int i;
char str[5], pomocny[11];


FILE *file = fopen("output", "wb");
if(file != NULL){

// zapisu boot record
	struct boot_record *br = malloc(sizeof(struct boot_record));
	strcpy(br->signature, "Hubacek");
	strcpy(br->volume_descriptor, "Puvodni vytvoreni 3.12.2017");
	br->disk_size = 1024 * 10;
	br->cluster_size = 1024;
	br->cluster_count = 50;
	br->mft_start_address = 288;
	br->bitmap_start_address = 2840 + 288;
	br->data_start_address = 2840 + 288 + 40;
	br->mft_max_fragment_count = 1;

	fwrite(br, sizeof(struct boot_record), 1, file);
	free((void *) br);

// zapisu mft
	for(i = 0; i < 10; i++){
	sprintf(str, "%d", i);

	struct mft_item *mfti = malloc(sizeof(struct mft_item));
	struct mft_fragment mftf;

	mftf.fragment_start_address = 288 + 2840 + 40;
	mftf.fragment_count = 1;


	mfti->uid = 1;
	mfti->isDirectory = 0;
	mfti->item_order = 1;
	mfti->item_order_total = 1;
	strcpy(pomocny, "");
	strcat(pomocny, "soubor");
	strcat(pomocny, str);
	strcat(pomocny, ".txt\0");
	strcpy(mfti->item_name, pomocny);
	mfti->item_size = 100;
	mfti->fragments[0] = mftf;

	fwrite(mfti, sizeof(struct mft_item), 1, file);

	free((void *) mfti);
//	free((void *) mftf);
	}

    fclose(file);
}
printf("zapsano %lu bajtu\n", sizeof(struct boot_record));



struct boot_record *br2=malloc(sizeof(struct boot_record));
    FILE * file2= fopen("output", "rb");
    if (file2 != NULL) {
        fread(br2, sizeof(struct boot_record), 1, file2);
        fclose(file2);
    }
    printf("%s\n",br2->signature);


printf("MFT item: %lu bajtu\n", sizeof(struct mft_item));
printf("MFT fragment: %lu bajtu\n", sizeof(struct mft_fragment));
printf(": 32t: %lu , 8t: %lu , int: %lu :\n", sizeof(int32_t), sizeof(int8_t), sizeof(int));
printf("a %lu", sizeof(a));/*

struct mft_item {
    int32_t uid;                                        //UID polozky, pokud UID = UID_ITEM_FREE, je polozka volna
    int isDirectory;                                    //soubor, nebo adresar (1=adresar, 0=soubor)
    int8_t item_order;                                  //poradi v MFT pri vice souborech, jinak 1
    int8_t item_order_total;                            //celkovy pocet polozek v MFT
    char item_name[12];                                 //8+3 + /0 C/C++ ukoncovaci string znak
    int32_t item_size;                                  //velikost souboru v bytech
    struct mft_fragment fragmentskk[32];                //fragmenty souboru - MFT fragments count
};
*/



    printf("LOADER ending\n");
    return NULL;
}
