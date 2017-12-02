#ifndef MFTF_H
#define MFTF_H

struct mft_fragment {
    int32_t fragment_start_address;     //start adresa
    int32_t fragment_count;             //pocet clusteru ve fragmentu
};

#endif