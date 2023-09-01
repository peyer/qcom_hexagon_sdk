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
/*[     reciprocal                                                         ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function calculates approximation of reciprocal by using      ]*/
/*[     linear interpolation.                                              ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     AUG-04-2016                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/

/* ======================================================================== */
/*  Reference C version of reciprocal().                                    */
/* ======================================================================== */
unsigned short val_table[32] = {
    16384,15887,15420,14979,14563,14169,13797,13443,
    13107,12787,12483,12192,11915,11650,11397,11155,
    10922,10699,10485,10280,10082, 9892, 9709, 9532,
    9362, 9198, 9039, 8886, 8738, 8594, 8456, 8322
};

unsigned short slope_table[32] = {
    31775,29905,28197,26630,25191,23865,22641,21509,
    20460,19485,18579,17734,16946,16209,15520,14873,
    14266,13695,13158,12652,12175,11724,11297,10894,
    10512,10149, 9805, 9478, 9167, 8872, 8590, 8322
};


int norm16(short x)
{
    int l0, l1;
    if(x == 0) return(0);

    for(l0=0; l0<16; l0++) if(!((x<<l0) & 0x8000)) break;
    for(l1=0; l1<16; l1++) if( ((x<<l1) & 0x8000)) break;
    if(l1 > l0) l0 = l1;
    return(l0-1);
}

void reciprocal(
    unsigned short *in,
    unsigned short *recipval,
    short          *recipshft,
    int             width
    )
{
    int i;
    unsigned int x1;
    unsigned short idx, frac, y, slope, t;
    int norm;

    for (i = 0; i<width; i++)
    {
        unsigned short recip_val, recip_shft;
        unsigned short x = in[i];

        if (x != 0)
        {
            norm = norm16(x);

            x1 = x << norm;

            idx  = (x1>>9) & 0x1f;
            frac = x1 & 0x1ff;

            y     = val_table[idx];
            slope = slope_table[idx];
            t     = (slope*frac + (1<<14)) >> 15;

            recip_val  = (unsigned short)(y-t);
            recip_shft = 28 - norm;
        }
        else
        {
            recip_val  = 0;
            recip_shft = 13;
        }

        recipval[i]  = recip_val;
        recipshft[i] = recip_shft;
    }
}

