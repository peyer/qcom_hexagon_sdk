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
/*[     fast9                                                              ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function performs an FAST feature detection. It checks 16     ]*/
/*[ pixels in a circle around the candidate point. If 9 contiguous pixels  ]*/
/*[ are all brighter or darker than the nucleus by a threshold, the pixel  ]*/
/*[ under the nucleus is considered to be a feature.                       ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     DEC-01-2014                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/
#include <stdio.h>
#include <stdlib.h>
#include "hvx.cfg.h"
#if defined(__hexagon__)
#include "hexagon_types.h"
#endif

/* ======================================================================== */
/*  Functions defined in Assembly                                           */
/* ======================================================================== */
int fast9_coord(
    unsigned int        *restrict bitmask,
    unsigned int         num_pixels32,
    int                  xstart,
    short int           *restrict xpos
    );

void fast9_detect(
    unsigned char       *restrict img,
    unsigned int         xsize,
    unsigned int         stride,
    unsigned int         barrier,
    unsigned int        *restrict bitmask,
    unsigned int         boundary
    );

/* ======================================================================== */
void sort( short *a, int n )
{
    int i;
    short temp;

    int update = 1;
    while (update)
    {
        update = 0;
        for (i = 1; i < n; i++)
        {
            if (a[i-1] > a[i])
            {
                update = 1;
                temp  = a[i-1];
                a[i-1]= a[i+0];
                a[i+0]= temp;
            }
        }
    }
}

/* ======================================================================== */
void fast9(
    const unsigned char *restrict im,
    unsigned int         stride,
    unsigned int         xsize,
    unsigned int         ysize,
    unsigned int         barrier,
    unsigned int         border,
    short int           *restrict xy,
    int                  maxnumcorners,
    int                 *numcorners
    )
{
    *numcorners = 0;
    int boundary = border>3 ? border : 3;
    unsigned int xstart = (boundary-3) & -VLEN;
    unsigned int num_pixels = xsize - xstart - boundary;
    num_pixels = (num_pixels+8*VLEN-1)&(-8*VLEN); // roundup to 8*VLEN
    unsigned int num_pixels32 = num_pixels >> 5;

    unsigned int *bitmask = (unsigned int *)memalign(VLEN, num_pixels32*sizeof(unsigned int));
    short        *xpos    = (short        *)malloc(xsize*sizeof(short));

    int y, num, n, k;

    im += xstart;

    for (y = boundary; y < (ysize-boundary); ++y)
    {
        unsigned char* p = (unsigned char *)im + y*stride;

        fast9_detect(p, xsize, stride, barrier, bitmask, boundary);
        num = fast9_coord(bitmask, num_pixels32, xstart + 3, xpos);
        sort(xpos, num);

        k = maxnumcorners - (*numcorners);
        n = (k > num) ? num : k;

        for (k = 0; k < n; k++)
        {
            *(xy++) = xpos[k];
            *(xy++) = y;
        }

        *numcorners += n;

        if (*numcorners >= maxnumcorners)
        {
            break;
        }
    }

    free(xpos   );
    free(bitmask);
}
