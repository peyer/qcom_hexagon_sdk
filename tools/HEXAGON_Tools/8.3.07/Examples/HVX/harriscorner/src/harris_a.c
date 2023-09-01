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
/*[     CornerHarrisu8                                                     ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function performs Harris corner detection.                    ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     JAN-14-2015                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/
#include <stdio.h>
#include <stdlib.h>
#include "hvx.cfg.h"

#define roundup(x,m)            (((x)+(m)-1)&(-(m)))
/* ======================================================================== */
/*  Functions defined in Assembly                                           */
/* ======================================================================== */
void gradients( 
    const unsigned char* __restrict srcImg,
    unsigned int     stride, 
    unsigned int     width, 
    short           *gradxx, 
    short           *gradyy, 
    short           *gradxy
    );

void blur5x5s16Row(
    short           *grads, 
    unsigned int     stride,
    unsigned int     width,
    short           *dst,
    int              flush
    );

void corner_response(   
    short           *blurgradxx, 
    short           *blurgradyy, 
    short           *blurgradxy, 
    unsigned int     width, 
    int             *corner             
    );

void search(     
    int             *corner, 
    unsigned int     flushcorner, 
    unsigned int     stride, 
    unsigned int     width, 
    int              threshold, 
    unsigned int    *masks,
    unsigned int     boundary_l
    );

int get_xpos(
    unsigned int    *masks,
    unsigned int     size, 
    int              xstart, 
    short           *xpos
    );

/* ======================================================================== */
void sort( 
    short *array, 
    int n 
    )
{
    int i, j;
    short temp;
     
    for (i = 1; i < n;++i)
    {
        for (j = 0; j < (n-i); ++j)
        {
            if(array[j] > array[j+1])
            {
                temp      = array[j  ];
                array[j  ]= array[j+1];
                array[j+1]= temp;
            }
        }
    }
}

/* ======================================================================== */
void CornerHarrisu8( 
    const unsigned char* __restrict srcImg,
    unsigned int             width,
    unsigned int             height,
    unsigned int             stride,
    unsigned int             border,
    unsigned int* __restrict xy,
    unsigned int             maxnumcorners,
    unsigned int* __restrict numcorners,
    int                      threshold 
    )
{
    int num_corners  = 0;
    int flushcorner = 0;
    int flushgrads  = 0;
    int responserow = 0;

    //unsigned int boundary = border > 5 ? border : 5;
    unsigned int boundary = 5;

    unsigned int width_t    = roundup(width,VLEN);
    unsigned int x0         = (boundary-2)&(-VLEN/4);
    unsigned int num_pixels = width-boundary-2-x0 + 2*(VLEN/4); // -2: blur; 2*(VLEN/4): srch loop
    unsigned int num_32mask = roundup(num_pixels,8*VLEN) >> 5;

    // Allocate memory
    short *grads     = (short *)memalign(VLEN,3*5*width_t*sizeof(short));   // gxx/gyy/gxy
    short *blurgrads = (short *)memalign(VLEN,3*width_t*sizeof(short));     // blurred gxx/gyy/gxy
    int   *corner    = (int   *)memalign(VLEN,5*width_t*sizeof(int));       // corner response

    unsigned int *masks = (unsigned int *)memalign(VLEN, num_32mask*sizeof(unsigned int)); 
    short *xpos = (short *)malloc(((width+2)/3)*sizeof(short)); 

    //Initializations
    short *gradxx    = grads;
    short *gradyy    = grads + 1*5*width_t;           
    short *gradxy    = grads + 2*5*width_t;   

    short *blurgradxx= blurgrads;
    short *blurgradyy= blurgrads + 1*width_t;
    short *blurgradxy= blurgrads + 2*width_t;

    int y, num, n, k;

    srcImg += stride;

    for (y = 1; y < 5; y++)
    {
        gradients(srcImg, stride, width, gradxx+y*width_t, gradyy+y*width_t, gradxy+y*width_t);
        srcImg += stride;
    }

    // Loop until y reaches it's boundary
    while( y < (height-1) && num_corners < maxnumcorners )
    { 
        int lastrow, rowloc, offset;

        // Calculate a new row of gradients
        lastrow = flushgrads + 4;
        if (lastrow >= 5) lastrow -= 5;

        offset = lastrow * width_t;

        gradients(srcImg, stride, width, gradxx+offset, gradyy+offset, gradxy+offset);
        srcImg += stride;

        // Blurring Ixx, Iyy, Ixy
        blur5x5s16Row(gradxx, width_t, width, blurgradxx, flushgrads);
        blur5x5s16Row(gradyy, width_t, width, blurgradyy, flushgrads);
        blur5x5s16Row(gradxy, width_t, width, blurgradxy, flushgrads);

        // calculate corner response
        rowloc = responserow + flushcorner;
        if (rowloc >= 5) rowloc -= 5;

        corner_response(blurgradxx, blurgradyy, blurgradxy, width, corner + rowloc * width_t); 

        responserow++;

        // if we have a filled 5x5 block, output the corners
        if (responserow == 5)
        {
            // Do the non-max supression and output the Harris corner
            search(corner+x0, flushcorner, width_t, width-boundary-2-x0, threshold, masks, boundary-2-x0);
            num = get_xpos(masks, num_32mask, x0, xpos); 
            sort(xpos, num);

            k = maxnumcorners - num_corners;
            num = ( k > num ) ? num : k ;

            num_corners += num;

            for (k = 0; k < num; k++)
            {
                *(xy++) = xpos[k];
                *(xy++) = y - 4;
            }

            // move 'corner' memory by 1 row by masking ptrs
            if ( ++flushcorner >= 5 ) flushcorner = 0;
            responserow--;
        }

        if (++flushgrads >= 5) flushgrads = 0;
        y++;
    }
    
    *numcorners = num_corners;

    free(xpos);
    free(masks);
    free(corner);
    free(blurgrads);
    free(grads);
}

