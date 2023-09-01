/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


/* 
 * ---------------------------------------------------------------------------
 * AES Cipher Algorithm.
 *  This function is used to generate the lookup table for En/Decrypt functions
 * ---------------------------------------------------------------------------
 */
#include <stdio.h>
#include "typedef.h"
#include "aes.h"


static UWord8 pow_tab[256];
static UWord8 log_tab[256];


static UWord8 FMULT (UWord8 a, UWord8 b)
{
    UWord8 aa = log_tab[a], cc = aa + log_tab[b];
    return pow_tab[cc + (cc < aa ? 1 : 0)];
}

#define GMPY(a,b)    (a && b ? FMULT(a, b) : 0)


void aes_gentabs2(UWord32 *EncryptTAB, UWord32 *DecryptTAB)
{
    UWord32 i;
    UWord8 p, q;
    UWord8 *ISBT = (UWord8 *)(DecryptTAB+256); 

    // log and power tables for GF(2**8) finite field with
    // 0x011b as modular polynomial - the simplest primitive
    // root is 0x03, used here to generate the tables 
    for (i = 0, p = 1; i < 256; ++i) {
       pow_tab[i] = (UWord8) p;
       log_tab[p] = (UWord8) i;

       p ^= (p << 1) ^ (p & 0x80 ? 0x01b : 0);
    }

    log_tab[1] = 0;


    for (i = 0; i < 256; ++i) {
       p = (i ? pow_tab[255 - log_tab[i]] : 0);
       q = ((p >> 7) | (p << 1)) ^ ((p >> 6) | (p << 2));
       p ^= 0x63 ^ q ^ ((q >> 6) | (q << 2));

       EncryptTAB[i] = ((UWord32)GMPY(2, p)      ) |
                       ((UWord32)p          << 8 ) |
                       ((UWord32)p          << 16) |
                       ((UWord32)GMPY(3, p) << 24);

       ISBT[p] = i;
       DecryptTAB[p] = ((UWord32)GMPY(14, i)     )  |
                       ((UWord32)GMPY( 9, i) << 8)  |
                       ((UWord32)GMPY(13, i) << 16) |
                       ((UWord32)GMPY(11, i) << 24);
    }

/*
    // calculate round constants
    for (i = 0, p = 1; i < 10; ++i) {
       RCOtab[i] = p;
       p = (p << 1) ^ (p & 0x80 ? 0x01b : 0);
    }
*/

}

