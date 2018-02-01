#ifndef MFT_H
#define MFT_H

#define MFT_FRAG_COUNT 10
#define UID_ITEM_FREE -1
#define CLUSTER_COUNT 200 // je i v loader.h; celkovy pocet clusteru v FS

// item muze mit 1 az X techto fragmentu
// +---+---+---+---+---+---+---+---+
// | 1 | 2 | X | X | X | 3 | 4 | 5 |
// +---+---+---+---+---+---+---+---+
struct mft_fragment {
    int32_t fragment_start_address;     //start adresa
    int32_t fragment_count;             //pocet clusteru ve fragmentu
} MFT_FRAGMENT;

// jeden soubor muze byt slozen i z vice mft_itemu, ty ale pak maji stejna uid (lisi se item_orderem)
struct mft_item {
    int uid;                                        //UID polozky, pokud UID = UID_ITEM_FREE, je polozka volna
    int isDirectory;                                    //soubor, nebo adresar (1=adresar, 0=soubor)
    int8_t item_order;                                  //poradi v MFT pri vice souborech, jinak 1
    int8_t item_order_total;                            //celkovy pocet polozek v MFT
    char item_name[12];                                 //8+3 + /0 C/C++ ukoncovaci string znak
    int32_t item_size;                                  //velikost souboru v bytech
    struct mft_fragment fragments[MFT_FRAG_COUNT]; 		//fragmenty souboru - MFT fragments count
} MFT_ITEM;

// itemy jsou ulozene ve spojovem seznamu, aby se jeden soubor mohl skladat z vice itemu (2 itemy po 25 frag = 50 fragmentu)
typedef struct mft_list {
    struct mft_item item; // k nested prvkum pristupuji pres tecky
    struct mft_list *dalsi;
} MFT_LIST;

MFT_LIST *mft_seznam[CLUSTER_COUNT];

/* hlavicky funkci ze souboru mft.c (komentare se nachazi tam) */
MFT_LIST *alokuj_prvek_mft(struct mft_item mfti);
void pridej_prvek_mft(int uid, struct mft_item mfti);

#endif
