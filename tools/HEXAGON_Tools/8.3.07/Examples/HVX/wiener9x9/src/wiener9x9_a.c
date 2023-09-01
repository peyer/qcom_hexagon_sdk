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
#include <assert.h>
#include <stdlib.h>
#include "hvx.cfg.h"
#include "wiener9x9.h"
#include "io.h"

/* ======================================================================== */
/*  Functions defined in Assembly                                           */
/* ======================================================================== */
void deltainit(
    unsigned char  *src,
    unsigned short *rgm,
    unsigned       *rgm2,
    int             width,
    int             stride
);

void vertboxfiltervarcomp(
    unsigned char  *src,
    unsigned short *rgm,
    unsigned       *rgm2,
    int             width,
    int             stride
);

void horboxfilter(
    unsigned char  *rgmean,
    unsigned short *rgm,
    int             width
);

void horvarcomp(
    unsigned short *rgvar,
    unsigned       *rgm2,
    int             width,
    unsigned char  *rgmean
);

void reciprocal(
    unsigned short *in,
    unsigned short *recipval,
    short          *recipshft,
    int             width
);

void blending(
    unsigned char  *src,
    unsigned char  *rgmean,
    unsigned short *rgvar,
    short          *rgshft,
    unsigned char  *dst,
    unsigned char   noise,
    int             width
);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void filter_wiener9x9 (
    unsigned char      *out,
    unsigned char      *in,
    int                 width,
    int                 stride,
    int                 height,
    const unsigned char noise)
{
    const int window = WINDOWS;
    const int window_2 = window / 2;
    int y;
    int stride1 = (stride + 2*VLEN-1) & (-2*VLEN);

    unsigned char *input = &in[window_2*stride];
    unsigned char *output = &out[window_2*stride];

    unsigned char *rgmean = (unsigned char*)memalign(VLEN, sizeof(rgmean[0]) * stride1);
    unsigned short *rgvar = (unsigned short*)memalign(VLEN, sizeof(rgvar[0]) * stride1);
    unsigned short *rgval = (unsigned short*)memalign(VLEN, sizeof(rgval[0]) * stride1);
    short *rgshft = (short*)memalign(VLEN, sizeof(rgshft[0]) * stride1);
    unsigned short *m = (unsigned short*)memalign(VLEN, sizeof(m[0]) * stride1);
    unsigned *m2 = (unsigned *)memalign(VLEN, sizeof(m2[0]) * stride1);

    assert(rgmean);
    assert(rgvar);
    assert(rgval);
    assert(rgshft);
    assert(m);
    assert(m2);

    if (window != 9) return;

    deltainit(
        input,
        m,
        m2,
        width,
        stride);

    for (y = window_2; y < height - window_2; y++)
    {
        vertboxfiltervarcomp(input, m, m2, width, stride);
        horboxfilter(rgmean, m, width);
        horvarcomp(rgvar, m2, width, rgmean);
        reciprocal(rgvar, rgval, rgshft, stride1);
        blending(input, rgmean, rgval, rgshft, output, noise,width);

        input += stride;
        output += stride;
    }

    free(rgmean);
    free(rgvar);
    free(rgval);
    free(rgshft);
    free(m);
    free(m2);
}
