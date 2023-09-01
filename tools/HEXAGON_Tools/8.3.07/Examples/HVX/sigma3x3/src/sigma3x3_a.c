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
/*[     sigma3x3                                                           ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function performs 3x3 sigma filtering on an image block       ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     AUG-01-2014                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/

/* ======================================================================== */
const short invLUT[10] __attribute__((aligned(128))) = {
    0,-32768,-16384,-10922,-8192,-6553,-5461,-4681,-4096,-3640
};


/* ======================================================================== */
/*  Functions defined in Assembly                                           */
/* ======================================================================== */
void sigma3x3PerRow(
    unsigned char   *src,
    int             stride,
    int             width,
    unsigned char   threshold,
    unsigned char   *dst
    );


/* ======================================================================== */
void sigma3x3(
    unsigned char   *src,
    int             stride,
    int             width,
    int             height,
    unsigned char   threshold,
    unsigned char   *dst
    )
{
    int  y;

    unsigned char *inp  = src + stride;
    unsigned char *outp = dst + stride;

    for (y = 1; y < height - 1; y++)
    {
        sigma3x3PerRow(inp, stride, width, threshold, outp);

        inp += stride;
        outp+= stride;
    }
}

