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
/*  Functions defined in Assembly                                           */
/* ======================================================================== */
void histogramPernRow(
    unsigned char   *src,
    int              stride,
    int              width,
    int              height,
    int             *hist
    );


void clearHistogram(
    int		*hist
    );


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

