#ifndef MFT_H
#define MFT_H

#define MFT_FRAG_COUNT 32
#define UID_ITEM_FREE 0
#define CLUSTER_COUNT 10

// item muze mit 1 az X techto fragmentu
// +---+---+---+---+---+---+---+---+
// | 1 | 2 | X | X | X | 3 | 4 | 5 |
// +---+---+---+---+---+---+---+---+
struct mft_fragment {
    int32_t fragment_start_address;     //start adresa
    int32_t fragment_count;             //pocet clusteru ve fragmentu
};

// jeden soubor muze byt slozen i z vice mft_itemu, ty ale pak maji stejna uid (lisi se item_orderem)
struct mft_item {
    int32_t uid;                                        //UID polozky, pokud UID = UID_ITEM_FREE, je polozka volna
    int isDirectory;                                    //soubor, nebo adresar (1=adresar, 0=soubor)
    int8_t item_order;                                  //poradi v MFT pri vice souborech, jinak 1
    int8_t item_order_total;                            //celkovy pocet polozek v MFT
    char item_name[12];                                 //8+3 + /0 C/C++ ukoncovaci string znak
    int32_t item_size;                                  //velikost souboru v bytech
    struct mft_fragment fragments[MFT_FRAG_COUNT]; 		//fragmenty souboru - MFT fragments count
};

// itemy jsou ulozene ve spojovem seznamu, aby se jeden soubor mohl skladat z vice itemu (2 itemy po 25 frag = 50 fragmentu)
typedef struct mft_list {
    struct mft_item *item;
    struct mft_list *dalsi;
} MFT_LIST;

MFT_LIST *mft_list[CLUSTER_COUNT];

/* hlavicky funkci ze souboru mft.c (komentare se nachazi tam) */
MFT_LIST *alokuj_prvek(struct mft_item mfti);
void pridej_prvek(int cluster_id, struct mft_item mfti);

#endif
