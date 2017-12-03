#ifndef MFT_H
#define MFT_H

#define MFT_FRAG_COUNT = 32;

struct mft_fragment {
    int32_t fragment_start_address;     //start adresa
    int32_t fragment_count;             //pocet clusteru ve fragmentu
};

struct mft_item {
    int32_t uid;                                        //UID polozky, pokud UID = UID_ITEM_FREE, je polozka volna
    int isDirectory;                                    //soubor, nebo adresar (1=adresar, 0=soubor)
    int8_t item_order;                                  //poradi v MFT pri vice souborech, jinak 1
    int8_t item_order_total;                            //celkovy pocet polozek v MFT
    char item_name[12];                                 //8+3 + /0 C/C++ ukoncovaci string znak
    int32_t item_size;                                  //velikost souboru v bytech
    struct mft_fragment fragments[32]; 			//fragmenty souboru - MFT fragments count
};

#endif