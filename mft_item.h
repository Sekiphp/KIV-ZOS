#ifndef MFTI_H
#define MFTI_H

const int32_t MFT_FRAGMENTS_COUNT = 32;

struct mft_item {
    int32_t uid;                                        //UID polozky, pokud UID = UID_ITEM_FREE, je polozka volna
    bool isDirectory;                                   //soubor, nebo adresar
    int8_t item_order;                                  //poradi v MFT pri vice souborech, jinak 1
    int8_t item_order_total;                            //celkovy pocet polozek v MFT
    char item_name[12];                                 //8+3 + /0 C/C++ ukoncovaci string znak
    int32_t item_size;                                  //velikost souboru v bytech
    struct mft_fragment fragments[MFT_FRAGMENTS_COUNT]; //fragmenty souboru
};

#endif