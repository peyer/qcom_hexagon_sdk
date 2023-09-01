/*****************************************************************************
*
*  Qualcomm Proprietary
*  Copyright (c) 2007  Qualcomm Incorporated.  All rights reserved.
*
*  All data and information contained in or disclosed by this document are
*  confidential and proprietary information of Qualcomm Incorporated, and
*  all rights therein are expressly reserved. By accepting this material,
*  the recipient agrees that this material and the information contained
*  therein are held in confidence and in trust and will not be used,
*  copied, reproduced in whole or in part, nor its contents revealed in
*  any manner to others without the express written permission of Qualcomm
*  param_incorporated.
*
*  This technology was exported from the United States in accordance with
*  the Export Administration Regulations. Diversion contrary to U.S. law
*  prohibited.
*
******************************************************************************
*  FILENAME:        aes.c
*  Authors:         MZ
*
******************************************************************************
*  DESCRIPTIONS:    AES Cipher Algorithm
*
******************************************************************************
*  SOURCE:          Hexagon Core Library 
*****************************************************************************/
#include <stdio.h>
#include "typedef.h"
#include "aes.h"

#define E_KEY ctx->E
#define D_KEY ctx->D

#define GET_UWORD32_LE(x)        (*(const UWord32 *)(x))
#define OUT_UWORD32_LE(to, from) (*(UWord32 *)(to) = (from))

#define GET_BYTE(x,n)            ((UWord8)((x)>>(8*(n))))

#define PUT_BYTES(b0,b1,b2,b3)   ((UWord32)(b0)      |  \
                                  ((UWord32)(b1)<< 8)|  \
                                  ((UWord32)(b2)<<16)|  \
                                  ((UWord32)(b3)<<24))


#define SubRotW(x)                                        \
    ( PUT_BYTES( ((UWord8 *)&FT[(UWord8)(x >>  8)])[1],  \
                 ((UWord8 *)&FT[(UWord8)(x >> 16)])[1],  \
                 ((UWord8 *)&FT[(UWord8)(x >> 24)])[1],  \
                 ((UWord8 *)&FT[(UWord8)(x      )])[1])    )


#define XTIME4(a) (((a) & 0x7F7F7F7F) << 1) ^ ((((a) & 0x80808080) >> 7) * 0x1B)

static inline 
UWord32 ROTL32 (const UWord32 x, const unsigned bits)
{
	const unsigned n = bits % 32;
	return (x << n) | (x >> (32 - n));
}

/*****************************************************/
/* Function: void aes_setkey                         */
/* Descriptions:                                     */
/*    key schedule from the user supplied key        */
/*****************************************************/
static UWord32 RCOtab[10] = {
  1,2,4,8,16,32,64,128,27,54
};

int aes_setkey(uword8_4B_align *in_key, unsigned int key_len, UWord32 *EncryptTAB, AES_CTX *ctx)
{
    UWord32 e, ex, ex2, ex3;
    UWord32 *FT = EncryptTAB;
    int i;

    if (key_len != 16 && key_len != 24 && key_len != 32) {
          return 1;
    }

    ctx->key_length = key_len;

    for (i=0; i< (key_len>>2); i++) {
       E_KEY[i] = GET_UWORD32_LE (in_key + 4*i );
    }

    switch (key_len) {
    case 16:
       for (i = 0; i < 10; ++i) {
          E_KEY[4 * i + 4] = SubRotW(E_KEY[4 * i + 3]) ^ RCOtab[i] \
                                              ^ E_KEY[4 * i    ] ;    
          E_KEY[4 * i + 5] = E_KEY[4 * i + 4] ^ E_KEY[4 * i + 1];
          E_KEY[4 * i + 6] = E_KEY[4 * i + 5] ^ E_KEY[4 * i + 2];
          E_KEY[4 * i + 7] = E_KEY[4 * i + 6] ^ E_KEY[4 * i + 3];

       }
       break;

    case 24:
       for (i = 0; i < 8; ++i) {
          E_KEY[6 * i + 6 ] = SubRotW(E_KEY[6 * i + 5]) ^ RCOtab[i] \
                                                ^ E_KEY[6 * i    ] ;    
          E_KEY[6 * i + 7 ] = E_KEY[6 * i + 6 ] ^ E_KEY[6 * i + 1];
          E_KEY[6 * i + 8 ] = E_KEY[6 * i + 7 ] ^ E_KEY[6 * i + 2];
          E_KEY[6 * i + 9 ] = E_KEY[6 * i + 8 ] ^ E_KEY[6 * i + 3];
          E_KEY[6 * i + 10] = E_KEY[6 * i + 9 ] ^ E_KEY[6 * i + 4];
          E_KEY[6 * i + 11] = E_KEY[6 * i + 10] ^ E_KEY[6 * i + 5];
       }
       break;

    case 32:
       for (i = 0; i < 7; ++i) {
          E_KEY[8 * i + 8 ] = SubRotW(E_KEY[8 * i + 7]) ^ RCOtab[i] \
                                                ^ E_KEY[8 * i    ] ;    
          E_KEY[8 * i + 9 ] = E_KEY[8 * i + 8 ] ^ E_KEY[8 * i + 1];
          E_KEY[8 * i + 10] = E_KEY[8 * i + 9 ] ^ E_KEY[8 * i + 2];
          E_KEY[8 * i + 11] = E_KEY[8 * i + 10] ^ E_KEY[8 * i + 3];
          E_KEY[8 * i + 12] = SubRotW(ROTL32(E_KEY[8 * i + 11],8))   \
                                                ^ E_KEY[8 * i + 4] ;    
          E_KEY[8 * i + 13] = E_KEY[8 * i + 12] ^ E_KEY[8 * i + 5];
          E_KEY[8 * i + 14] = E_KEY[8 * i + 13] ^ E_KEY[8 * i + 6];
          E_KEY[8 * i + 15] = E_KEY[8 * i + 14] ^ E_KEY[8 * i + 7];
       }
       break;
    }

    // Key expansion for Equivalent Inverse Cipher
    D_KEY[0] = E_KEY[0];
    D_KEY[1] = E_KEY[1];
    D_KEY[2] = E_KEY[2];
    D_KEY[3] = E_KEY[3];

    for (i = 4; i < key_len + 24; ++i) {

         // InvMixColumns(E_KEY[i])
         e    = E_KEY[i];        
         ex   = XTIME4(e  );        
         ex2  = XTIME4(ex );        
         ex3  = XTIME4(ex2);        
         D_KEY[i] =         (    ex ^ ex2 ^ ex3)      ^
                     ROTL32((e ^ ex       ^ ex3), 24) ^
                     ROTL32((e      ^ ex2 ^ ex3), 16) ^ 
                     ROTL32((e            ^ ex3), 8 ); 
    }

    return 0;
}


/*****************************************************/
/* Function: void aes_encrypt                        */
/* Descriptions:                                     */
/*    encrypt a block of text                        */
/*****************************************************/
void aes_encrypt(AES_CTX *ctx, UWord32 *EncryptTAB, uword8_4B_align *in, uword8_4B_align *out)
{
    UWord32 b0[4], b1[4];
    UWord32 *Si, *So, *temp;
    UWord32 *rkey = E_KEY + 4;
    UWord32 *FT0, *FT1, *FT2, *FT3;
    int nr, i, j;  

    FT0 = EncryptTAB;
    FT1 = EncryptTAB + 1*256;
    FT2 = EncryptTAB + 2*256;
    FT3 = EncryptTAB + 3*256;

    b0[0] = GET_UWORD32_LE (in + 0 ) ^ E_KEY[0];
    b0[1] = GET_UWORD32_LE (in + 4 ) ^ E_KEY[1];
    b0[2] = GET_UWORD32_LE (in + 8 ) ^ E_KEY[2];
    b0[3] = GET_UWORD32_LE (in + 12) ^ E_KEY[3];

    if (ctx->key_length > 24) {
        nr = 14;
    }
    else if (ctx->key_length > 16) {
        nr = 12;
    }
    else {
        nr = 10;
    }

    Si = b0;
    So = b1;

    for (i=0; i<nr-1; i++) {

        for (j=0; j<4; j++) {

            So[j] = FT0[ GET_BYTE(Si[ j         ], 0) ] ^           \
                    FT1[ GET_BYTE(Si[(j + 1) & 3], 1) ] ^           \
                    FT2[ GET_BYTE(Si[(j + 2) & 3], 2) ] ^           \
                    FT3[ GET_BYTE(Si[(j + 3) & 3], 3) ] ^ *rkey++;  
        }

        // swap buffers 
        temp = Si; Si = So; So = temp;
    }

    // last round without MixColumns
    for (j=0; j<4; j++) {

        So[j] = PUT_BYTES( ((UWord8 *)&FT0[ GET_BYTE(Si[ j         ], 0) ])[1],            \
                           ((UWord8 *)&FT0[ GET_BYTE(Si[(j + 1) & 3], 1) ])[1],            \
                           ((UWord8 *)&FT0[ GET_BYTE(Si[(j + 2) & 3], 2) ])[1],            \
                           ((UWord8 *)&FT0[ GET_BYTE(Si[(j + 3) & 3], 3) ])[1] ) ^ rkey[j];  
    }

    OUT_UWORD32_LE (out +  0, So[0]);
    OUT_UWORD32_LE (out +  4, So[1]);
    OUT_UWORD32_LE (out +  8, So[2]);
    OUT_UWORD32_LE (out + 12, So[3]);
}


/*****************************************************/
/* Function: void aes_decrypt                        */
/* Descriptions:                                     */
/*    decrypt a block of text                        */
/*****************************************************/
void aes_decrypt(AES_CTX *ctx, UWord32 *DecryptTAB, uword8_4B_align *in, uword8_4B_align *out)
{
    UWord32 b0[4], b1[4];
    UWord32 *Si, *So, *temp;
    int key_len = ctx->key_length;
    UWord32 *rkey = D_KEY + key_len + 20;
    UWord32 *IT0, *IT1, *IT2, *IT3;
    UWord8 *ISBT = (UWord8 *)(DecryptTAB + 4*256);
    int nr, i, j;

    IT0 = DecryptTAB;
    IT1 = DecryptTAB + 1*256;
    IT2 = DecryptTAB + 2*256;
    IT3 = DecryptTAB + 3*256;

    b0[0] = GET_UWORD32_LE (in + 0 ) ^ E_KEY[key_len + 24];
    b0[1] = GET_UWORD32_LE (in + 4 ) ^ E_KEY[key_len + 25];
    b0[2] = GET_UWORD32_LE (in + 8 ) ^ E_KEY[key_len + 26];
    b0[3] = GET_UWORD32_LE (in + 12) ^ E_KEY[key_len + 27];

    if (key_len > 24) {
        nr = 14;
    }
    else if (key_len > 16) {
        nr = 12;
    }
    else {
        nr = 10;
    }

    Si = b0;
    So = b1;

    for (i=0; i<nr-1; i++) {

        for (j=0; j<4; j++) {

            So[j] = IT0[ GET_BYTE(Si[ j         ], 0) ] ^           \
                    IT1[ GET_BYTE(Si[(j + 3) & 3], 1) ] ^           \
                    IT2[ GET_BYTE(Si[(j + 2) & 3], 2) ] ^           \
                    IT3[ GET_BYTE(Si[(j + 1) & 3], 3) ] ^ rkey[j];  
        }
        rkey -= 4;

        // swap buffers 
        temp = Si; Si = So; So = temp;
    }

    // last round without MixColumns
    for (j=0; j<4; j++) {

        So[j] = PUT_BYTES( ISBT[ GET_BYTE(Si[ j         ], 0) ],            \
                           ISBT[ GET_BYTE(Si[(j + 3) & 3], 1) ],            \
                           ISBT[ GET_BYTE(Si[(j + 2) & 3], 2) ],            \
                           ISBT[ GET_BYTE(Si[(j + 1) & 3], 3) ] ) ^ rkey[j];  
    }

    OUT_UWORD32_LE (out +  0, So[0]);
    OUT_UWORD32_LE (out +  4, So[1]);
    OUT_UWORD32_LE (out +  8, So[2]);
    OUT_UWORD32_LE (out + 12, So[3]);
}

