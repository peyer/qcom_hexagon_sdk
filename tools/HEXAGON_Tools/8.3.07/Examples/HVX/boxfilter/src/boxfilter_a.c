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
/*[     boxfilter11x11                                                     ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function performs 11x11 box filter on a image.                ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     AUG-25-2016                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/
#include <stdlib.h>
#include "hexagon_types.h"
#include "hvx.cfg.h"

#define max_t   Q6_R_max_RR
#define min_t   Q6_R_min_RR
/* ======================================================================== */
/*  Functions defined in Assembly                                           */
/* ======================================================================== */
void Box11perRow(
    unsigned char   *inp0,
    unsigned char   *inp1,
    int             width,
    unsigned short  *sump,
    unsigned char   *outp
    );

void Vmemset(
    void            *src,
    unsigned char   value,
    unsigned int    size
    );

/* ======================================================================== */
/*  Constants for Assembly                                                  */
/* ======================================================================== */
const unsigned char splatBidx0control[128] __attribute__((aligned(128))) = {
   0x00,0x01,0x02,0x02,0x04,0x04,0x04,0x04,
   0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
   0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
   0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
   0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
   0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
   0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
   0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
   0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
   0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
   0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
   0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
   0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
   0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
   0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
   0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40
};


/* ======================================================================== */
void boxfilter11x11(
    unsigned char *restrict inp,
    int            stride,
    int            width,
    int            height,
    unsigned char *restrict outp
    )
{
    int i, k;

    int width_t = (width + VLEN-1)&(-VLEN);

    unsigned short *sums = (unsigned short *)memalign(VLEN, width_t*sizeof(sums[0]));
    unsigned char  *zero = (unsigned char  *)memalign(VLEN, width_t*sizeof(zero[0]));

    unsigned char *src0, *src1;
    unsigned char *output = outp;

    Vmemset( sums, 0, width_t*sizeof(sums[0]) );
    Vmemset( zero, 0, width_t*sizeof(zero[0]) );

    for (k = -5; k <= 5; k++)
    {
        src1 = inp + min_t(max_t(k, 0), height-1)*stride;

        Box11perRow( zero, src1, width, sums, output );
    }

    for (i = 1; i< height; i++)
    {
        src0 = inp + max_t(i-6, 0       )*stride;
        src1 = inp + min_t(i+5, height-1)*stride;
        output+= stride;

        Box11perRow( src0, src1, width, sums, output );
   }

   free(sums);
   free(zero);
}



