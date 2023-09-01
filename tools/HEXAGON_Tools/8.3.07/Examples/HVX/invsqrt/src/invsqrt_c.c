/* ======================================================================== */
/*  QUALCOMM TECHNOLOGIES, INC.                                             */
/*                                                                          */
/*  HEXAGON HVX Image/Video Processing Library                              */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*          Copyright (c) 2014 QUALCOMM TECHNOLOGIES Incorporated.          */
/*                           All Rights Reserved.                           */
/*                  QUALCOMM Confidential and Proprietary                   */
/* ======================================================================== */

/*[========================================================================]*/
/*[ FUNCTION                                                               ]*/
/*[     invsqrt                                                            ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function computes 1 / squareroot(x) using interpolation.      ]*/
/*[     Input is an unsigned 16 bits integer.                              ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     AUG-09-2016                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/
#include "invsqrt.h"

/* ======================================================================== */
/*  Reference C version.                                                    */
/* ======================================================================== */
int clz(short x)
{
    int l0;
    if(x == 0) return(16);

    for(l0=0; l0<16; l0++) if((x<<l0) & 0x8000) break;
    return(l0);
}

void invsqrt(
    unsigned short *input,
    unsigned short *sqrt_recip_shft,
    unsigned short *sqrt_recip_val,
    unsigned        width
    )
{
    static unsigned short val_table[24] = {
        32768,30893,29308,27944,26754,25705,24770,23930,
        23170,22478,21845,21262,20724,20224,19759,19325,
        18918,18536,18176,17836,17515,17210,16921,16646
    };

    static unsigned short slope_table[24] = {
        29985,25366,21823,19035,16794,14961,13439,12158,
        11069,10133, 9322, 8614, 7991, 7440, 6949, 6510,
         6116, 5759, 5436, 5143, 4874, 4628, 4403, 4195
    };

    unsigned t1, x, i;
    unsigned short idx, frac, y, slope, t3;
    int     shift_nbits;

    for (i = 0; i<width; i++)
    {
        x = input[i];
        if (x != 0) 
        {
            shift_nbits = clz(x)&-2;
            t1 = x << shift_nbits;

            idx = (t1>>11) - 8;
            frac = t1 & 0x7ff;

            y = val_table[idx];
            slope = slope_table[idx];
            t3 = (slope * frac + (1<<14)) >> 15;

            sqrt_recip_val[i]  = y - t3;
            sqrt_recip_shft[i] = 22 - (shift_nbits>>1);
        }
        else
        {
            sqrt_recip_val[i]  = 0;
            sqrt_recip_shft[i] = 14;
        }
    }
}

