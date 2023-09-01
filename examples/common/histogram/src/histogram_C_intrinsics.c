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
/*[     histogram                                                          ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function takes an 8-bit image block and returns the histogram ]*/
/*[     of 256 32-bit bins.                                                ]*/
/*[                                                                        ]*/
/*[========================================================================]*/
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif
/* ======================================================================== */
/*  Functions defined in Assembly                                           */
/* ======================================================================== */
void histogramPernRow(
    unsigned char   *src,
    int              stride,
    int              width,
    int              height,
    int             *hist
    )
{
    int i, j;
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            hist[src[i*stride+j]]++;
        }
    }
}


/* ======================================================================== */
void clearHistogram(
    int		*hist
    )
{
    int i;
    for (i = 0; i < 256; i++)
        hist[i] = 0;
}


#ifdef __cplusplus
}
#endif
/* ======================================================================== */
void histogram(
    unsigned char   *src,
    int              stride,
    int              width,
    int              height,
    int             *hist
    )
{
    int i, k, n;

    clearHistogram(hist);

    //  Consideration on n
    // - Reduce overhead of histogramPernRow
    // - MUST have width*n < 2^15
    // - able to data prefetch

    n = 8192/width;

    for (i=0; i<height; i+=n)
    {
        k = (height - i) > n ? n : (height-i);
        histogramPernRow(src, stride, width, k, hist);
        src += n*stride;
    }
}

