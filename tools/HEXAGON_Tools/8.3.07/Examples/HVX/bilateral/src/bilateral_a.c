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
/*[     bilateral9x9                                                       ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function applies a 9x9 bilateral filter to a image.           ]*/
/*[ The intensity value at each pixel in an image is replaced by a weighted]*/
/*[ average of intensity vaules from nearby pixels.                        ]*/
/*[     It is widely used for image smoothing with edge-perserving.        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     AUG-01-2014                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/

/* ======================================================================== */
/*  Functions defined in Assembly                                           */
/* ======================================================================== */
void bilateral9x9PerRow(
    unsigned char   *input,
    int              stride,
    int              width,
    unsigned char   *gauss_LUT,
    unsigned char   *range_LUT,
    unsigned char   *output
    );


/* ======================================================================== */
void bilateral9x9(
    unsigned char   *input,
    int             stride,
    int             width,
    int             height,
    unsigned char   *gauss_LUT,
    unsigned char   *range_LUT,
    unsigned char   *output
    )
{
    int y;
    unsigned char *inp  = input  + 4*stride;
    unsigned char *outp = output + 4*stride;

    for(y = 4; y < (height-4); y++)
    {
        bilateral9x9PerRow( inp, stride, width, gauss_LUT, range_LUT, outp);
        inp  += stride;
        outp += stride;
    }

    return;
}

