#ifndef BOOTR_H
#define BOOTR_H

// obsahuje informace o NTFS
struct boot_record {
    char signature[9];              //login autora FS
    char volume_descriptor[251];    //popis vygenerovaného FS
    int32_t disk_size;              //celkova velikost VFS
    int32_t cluster_size;           //velikost clusteru
    int32_t cluster_count;          //pocet clusteru
    int32_t mft_start_address;      //adresa pocatku mft
    int32_t bitmap_start_address;   //adresa pocatku bitmapy
    int32_t data_start_address;     //adresa pocatku datovych bloku
    int32_t mft_max_fragment_count; //maximalni pocet fragmentu v jednom zaznamu v mft (pozor, ne souboru)
                                    // stejne jako   MFT_FRAGMENTS_COUNT
                                    // pokud by bylo 1, tak jeden soubor muze mit max 1 mft_item
};

#endif