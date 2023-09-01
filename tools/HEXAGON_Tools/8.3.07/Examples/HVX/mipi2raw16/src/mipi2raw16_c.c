/* ======================================================================== */
/*  QUALCOMM TECHNOLOGIES, INC.                                             */
/*                                                                          */
/*  HEXAGON HVX Image/Video Processing Library                              */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*          Copyright (c) 2015 QUALCOMM TECHNOLOGIES Incorporated.          */
/*                           All Rights Reserved.                           */
/*                  QUALCOMM Confidential and Proprietary                   */
/* ======================================================================== */

/*[========================================================================]*/
/*[ FUNCTION                                                               ]*/
/*[     Mipi to Raw16                                                      ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function decompresses the packed 10-bit bayer to 16-bit       ]*/
/*[     output.                                                            ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     SEPT-24-2015                                                       ]*/
/*[                                                                        ]*/
/*[========================================================================]*/
#include <assert.h>

/* ======================================================================== */
/*  Reference C version of convertMipiToRaw16().                            */
/* ======================================================================== */
void convertMipiToRaw16(
    unsigned char  *input,
    unsigned        istride,
    unsigned        width,
    unsigned        height,
    unsigned short *output,
    unsigned        ostride
    )
{
    int y, x;

    assert((width&3) == 0);
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x+=4) {
            unsigned char p0 = input[5*x/4 + 0];
            unsigned char p1 = input[5*x/4 + 1];
            unsigned char p2 = input[5*x/4 + 2];
            unsigned char p3 = input[5*x/4 + 3];
            unsigned char lsb = input[5*x/4 + 4];

            output[x+0] = p0 * 4 + ((lsb>>0) & 3);
            output[x+1] = p1 * 4 + ((lsb>>2) & 3);
            output[x+2] = p2 * 4 + ((lsb>>4) & 3);
            output[x+3] = p3 * 4 + ((lsb>>6) & 3);
        }
        input += istride;
        output += ostride;
    }
}
