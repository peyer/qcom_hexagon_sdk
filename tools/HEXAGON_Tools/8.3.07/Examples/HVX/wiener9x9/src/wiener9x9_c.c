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
/*[     wiener9x9                                                          ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function applies a 9x9 kernel of Wiener filter to an image.   ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     AUG-01-2014                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/
#include "wiener9x9.h"

/* ======================================================================== */
/*  Reference C version of wiener9x9().                                     */
/* ======================================================================== */
void reciprocal_c(
    unsigned short *in,
    unsigned short *recipval,
    short          *recipshft,
    int width
)
{
    int i;
    unsigned int t1;
    unsigned short shft, t2, idx, frac, y, slope, t4;
    static unsigned short val_table[32] =
    {
        4096,    3971,   3855,   3744,   3640,   3542,   3449,   3360,
        3276,    3196,   3120,   3048,   2978,   2912,   2849,   2788,
        2730,    2674,   2621,   2570,   2520,   2473,   2427,   2383,
        2340,    2299,   2259,   2221,   2184,   2148,   2114,   2080
    };
    static unsigned short slope_table[32] =
    {
        125,     116,    111,    104,    98,     93,     89,     84,
        80,      76,     72,     70,     66,     63,     61,     58,
        56,      53,     51,     50,     47,     46,     44,     43,
        41,      40,     38,     37,     36,     34,     34,     32
    };

    for (i = 0; i<width; i++)
    {
        unsigned short recip_val, recip_shft;
        unsigned short x = in[i];

        if (x != 0)
        {
            if      (x >> 14) shft = 14;
            else if (x >> 13) shft = 13;
            else if (x >> 12) shft = 12;
            else if (x >> 11) shft = 11;
            else if (x >> 10) shft = 10;
            else if (x >>  9) shft =  9;
            else if (x >>  8) shft =  8;
            else if (x >>  7) shft =  7;
            else if (x >>  6) shft =  6;
            else if (x >>  5) shft =  5;
            else if (x >>  4) shft =  4;
            else if (x >>  3) shft =  3;
            else if (x >>  2) shft =  2;
            else if (x >>  1) shft =  1;
            else              shft =  0;

            t1     = x << 10;
            t2     = t1 >> shft;
            idx    = (t2 >> 5) & 0x1f;
            frac   = t2 & 0x1f;

            y      = val_table[idx];
            slope  = slope_table[idx];
            t4     = (slope*frac + (1<<4)) >> 5;

            recip_val  = (unsigned short) (y-t4);
            recip_shft = shft;
        }
        else
        {
            recip_val  = 0;
            recip_shft = 0;
        }

        recipval[i] = recip_val;
        recipshft[i] = recip_shft;
    }
}


/* ======================================================================== */
#define BIT 7
#define min(a,b) (a < b? a:b)
#define max(a,b) (a > b? a:b)

void filter_wiener9x9 (
    unsigned char      *out,
    unsigned char      *in,
    int                 width,
    int                 stride,
    int                 height,
    const unsigned char noise
)
{
    const int window = WINDOWS;
    const int window_2 = window / 2;
    int x, y, s, t, r;

    for (y = window_2; y < height - window_2; y++)
    {
        for (x = window_2; x < width - window_2; x++)
        {
            int var;
            unsigned char mean;
            unsigned m2 = 0;
            unsigned m = 0;
            unsigned short var1, recipval;
            short recipshft;
            for (t = -window_2; t <= window_2; t++)
            {
                for (s = -window_2; s <= window_2; s++)
                {
                    m += in[(y+t)*stride+x+s];
                    m2 += in[(y+t)*stride+x+s] * in[(y+t)*stride+x+s];
                }
            }
            mean = ((m * 809 + 0x8000) >> 16);
            var = ((m2 * 809 + 0x8000) >> 16);

            var -= mean*mean;
            var = max(0,var);
            var1 = min(var, 0x7fff);

            reciprocal_c(&var1, &recipval, &recipshft, 1);

            if (var1)
            {
                r = (noise * recipval) >> (12 + recipshft - BIT);
            }
            else
            {
                r = (1<<BIT);
            }
            r = min(r, 1<<BIT);

            out[y*stride+x] = in[y*stride+x] + (((mean - in[y*stride+x]) * r + (1<<(BIT-1))) >> BIT);
        }
        for (x = 0; x < window_2; x++)
        {
            out[y*stride+x] = in[y*stride+x];
            out[(y+1)*stride-1-x] = in[(y+1)*stride-1-x];
        }
    }
}
