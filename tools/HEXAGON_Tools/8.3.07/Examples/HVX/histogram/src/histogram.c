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
/*[     histogram                                                          ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function takes an 8-bit image block and returns the histogram ]*/
/*[     of 256 32-bit bins.                                                ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     AUG-01-2014                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/

/* ======================================================================== */
/*  Reference C version of histogram()                                      */
/* ======================================================================== */
void histogram(
    unsigned char   *src,
    int              stride,
    int              width,
    int              height,
    int             *hist
    )
{
    int i, j;

    for (i=0; i<256; i++)
    {
        hist[i] = 0;
    }


    for(i = 0; i < height; i++)
    {
        for(j = 0; j < width; j++ )
        {
            hist[src[i*stride+j]]++;
        }
    }
}

