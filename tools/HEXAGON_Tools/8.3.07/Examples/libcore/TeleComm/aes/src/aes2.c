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
*                   (This implementation requires much less LUT)
*
******************************************************************************
*  SOURCE:          QDSP6 Core Library 
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

static inline 
UWord32 ROTL32 (const UWord32 x, const unsigned bits)
{
	const unsigned n = bits % 32;
	return (x << n) | (x >> (32 - n));
}

/*****************************************************/
/* Function: void aes_encrypt2                       */
/* Descriptions:                                     */
/*    encrypt a block of text                        */
/*****************************************************/
void aes_encrypt2(AES_CTX *ctx, UWord32 *EncryptTAB, uword8_4B_align *in, uword8_4B_align *out)
{
    UWord32 b0[4], b1[4];
    UWord32 *Si, *So, *temp;
    UWord32 *rkey = E_KEY + 4;
    UWord32 *FT = EncryptTAB;
    int nr, i, j;  

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

            So[j] =        FT[ GET_BYTE(Si[ j         ], 0) ]      ^           \
                    ROTL32(FT[ GET_BYTE(Si[(j + 1) & 3], 1) ],  8) ^           \
                    ROTL32(FT[ GET_BYTE(Si[(j + 2) & 3], 2) ], 16) ^           \
                    ROTL32(FT[ GET_BYTE(Si[(j + 3) & 3], 3) ], 24) ^ *rkey++;  
        }

        // swap buffers 
        temp = Si; Si = So; So = temp;
    }

    // last round without MixColumns
    for (j=0; j<4; j++) {

        So[j] = PUT_BYTES( ((UWord8 *)&FT[ GET_BYTE(Si[ j         ], 0) ])[1],            \
                           ((UWord8 *)&FT[ GET_BYTE(Si[(j + 1) & 3], 1) ])[1],            \
                           ((UWord8 *)&FT[ GET_BYTE(Si[(j + 2) & 3], 2) ])[1],            \
                           ((UWord8 *)&FT[ GET_BYTE(Si[(j + 3) & 3], 3) ])[1] ) ^ rkey[j];  
    }

    OUT_UWORD32_LE (out +  0, So[0]);
    OUT_UWORD32_LE (out +  4, So[1]);
    OUT_UWORD32_LE (out +  8, So[2]);
    OUT_UWORD32_LE (out + 12, So[3]);
}


/*****************************************************/
/* Function: void aes_decrypt2                       */
/* Descriptions:                                     */
/*    decrypt a block of text                        */
/*****************************************************/
void aes_decrypt2(AES_CTX *ctx, UWord32 *DecryptTAB, uword8_4B_align *in, uword8_4B_align *out)
{
    UWord32 b0[4], b1[4];
    UWord32 *Si, *So, *temp;
    int key_len = ctx->key_length;
    UWord32 *rkey = D_KEY + key_len + 20;
    UWord32 *IT = DecryptTAB;
    UWord8 *ISBT = (UWord8 *)(DecryptTAB + 256);
    int nr, i, j;

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

            So[j] =        IT[ GET_BYTE(Si[ j         ], 0) ]      ^           \
                    ROTL32(IT[ GET_BYTE(Si[(j + 3) & 3], 1) ],  8) ^           \
                    ROTL32(IT[ GET_BYTE(Si[(j + 2) & 3], 2) ], 16) ^           \
                    ROTL32(IT[ GET_BYTE(Si[(j + 1) & 3], 3) ], 24) ^ rkey[j];  
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


