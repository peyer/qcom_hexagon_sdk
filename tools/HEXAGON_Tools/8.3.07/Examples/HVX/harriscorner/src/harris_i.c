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
#include "hexagon_types.h"
#include "hexagon_protos.h"
#include "hvx.cfg.h"

#define roundup(x,m)            (((x)+(m)-1)&(-(m)))
/* ======================================================================== */
/*  Intrinsic version of CornerHarrisu8()                                   */
/* ======================================================================== */
void gradients( 
    const unsigned char *restrict srcImg,
    unsigned int         stride, 
    unsigned int         width, 
    short               *restrict gradxx, 
    short               *restrict gradyy, 
    short               *restrict gradxy
    )
{
    int i;
    HVX_Vector  sVx0, sVx1, sX_1, sX1, sIx, sIy;
    HVX_VectorPair dIxx, dIxy, dIyy;

    HVX_Vector *iptrC = (HVX_Vector *)(srcImg         );
    HVX_Vector *iptr0 = (HVX_Vector *)(srcImg - stride);
    HVX_Vector *iptr1 = (HVX_Vector *)(srcImg + stride);

    HVX_Vector *optrxx = (HVX_Vector *)gradxx;
    HVX_Vector *optryy = (HVX_Vector *)gradyy;
    HVX_Vector *optrxy = (HVX_Vector *)gradxy;

    sVx0 = Q6_V_vzero();
    sVx1 = *iptrC++;

    for (i = width; i > 0; i -= VLEN)
    {
        sX_1 = Q6_V_vlalign_VVI(sVx1,sVx0,1);

        sVx0 = sVx1;
        sVx1 = *iptrC++;
        sX1 =  Q6_V_valign_VVI(sVx1,sVx0,1);

        sIx = Q6_Vb_vnavg_VubVub(sX1,sX_1);
        sIy = Q6_Vb_vnavg_VubVub(*iptr1++, *iptr0++);

        dIxx = Q6_Wh_vmpy_VbVb(sIx,sIx);
        dIxx = Q6_W_vshuff_VVR(Q6_V_hi_W(dIxx),Q6_V_lo_W(dIxx),-2);
        *optrxx++ = Q6_V_lo_W(dIxx);
        *optrxx++ = Q6_V_hi_W(dIxx);

        dIyy = Q6_Wh_vmpy_VbVb(sIy,sIy);
        dIyy = Q6_W_vshuff_VVR(Q6_V_hi_W(dIyy),Q6_V_lo_W(dIyy),-2);
        *optryy++ = Q6_V_lo_W(dIyy);
        *optryy++ = Q6_V_hi_W(dIyy);

        dIxy = Q6_Wh_vmpy_VbVb(sIx,sIy);
        dIxy = Q6_W_vshuff_VVR(Q6_V_hi_W(dIxy),Q6_V_lo_W(dIxy),-2);
        *optrxy++ = Q6_V_lo_W(dIxy);
        *optrxy++ = Q6_V_hi_W(dIxy);
    }
}

/* ======================================================================== */
void blur5x5s16Row(
    short           *restrict grads, 
    unsigned int     stride,
    unsigned int     width,
    short           *restrict dst,
    int              flush
    )
{
    int i;

    HVX_Vector sLine0, sLine1, sLine2, sLine3, sLine4;
    HVX_Vector sVsum0, sVsum1, sVsum2, sVsum3, sVsum4, sVsum5;
    HVX_Vector sVsum1a3, sVsum2a4, sSumE, sSumO;

    HVX_VectorPair dVsumv0, dVsumv1;

    HVX_Vector *iptr0 = (HVX_Vector *)(grads +   flush      *stride);
    HVX_Vector *iptr1 = (HVX_Vector *)(grads + ((flush+1)%5)*stride);
    HVX_Vector *iptr2 = (HVX_Vector *)(grads + ((flush+2)%5)*stride);
    HVX_Vector *iptr3 = (HVX_Vector *)(grads + ((flush+3)%5)*stride);
    HVX_Vector *iptr4 = (HVX_Vector *)(grads + ((flush+4)%5)*stride);
    HVX_Vector *optr  = (HVX_Vector *)dst;

    dVsumv0 = Q6_W_vcombine_VV(Q6_V_vzero(),Q6_V_vzero());
    dVsumv1 = dVsumv0;

    sLine0 = *iptr0++;
    sLine1 = *iptr1++;
    sLine2 = *iptr2++;
    sLine3 = *iptr3++;
    sLine4 = *iptr4++;

    dVsumv1 = Q6_Ww_vadd_VhVh(sLine0,sLine4);
    dVsumv1 = Q6_Ww_vmpyacc_WwVhRh_sat(dVsumv1,sLine2,0x00060006);
    dVsumv1 = Q6_Ww_vmpaacc_WwWhRb(dVsumv1,Q6_W_vcombine_VV(sLine3,sLine1),0x04040404);

    for (i=width; i>0; i-=(VLEN/2))
    {
        sLine0 = *iptr0++;
        sLine1 = *iptr1++;
        sLine2 = *iptr2++;
        sLine3 = *iptr3++;
        sLine4 = *iptr4++;

        dVsumv0 = dVsumv1;
        dVsumv1 = Q6_Ww_vadd_VhVh(sLine0,sLine4);
        dVsumv1 = Q6_Ww_vmpyacc_WwVhRh_sat(dVsumv1,sLine2,0x00060006);
        dVsumv1 = Q6_Ww_vmpaacc_WwWhRb(dVsumv1,Q6_W_vcombine_VV(sLine3,sLine1),0x04040404);

        sVsum0 = Q6_V_lo_W(dVsumv0);
        sVsum1 = Q6_V_hi_W(dVsumv0);
        sVsum2 = Q6_V_valign_VVI( Q6_V_lo_W(dVsumv1),Q6_V_lo_W(dVsumv0),4);
        sVsum3 = Q6_V_valign_VVI( Q6_V_hi_W(dVsumv1),Q6_V_hi_W(dVsumv0),4);
        sVsum4 = Q6_V_valign_VVR( Q6_V_lo_W(dVsumv1),Q6_V_lo_W(dVsumv0),8);
        sVsum5 = Q6_V_valign_VVR( Q6_V_hi_W(dVsumv1),Q6_V_hi_W(dVsumv0),8);

        sVsum1a3 = Q6_Vw_vadd_VwVw(sVsum1,sVsum3);
        sVsum2a4 = Q6_Vw_vadd_VwVw(sVsum2,sVsum4);

        sSumE = Q6_Vw_vadd_VwVw(sVsum0,sVsum4);
        sSumE = Q6_Vw_vmpyiacc_VwVwRb(sSumE,sVsum2,0x06060606);
        sSumE = Q6_Vw_vmpyiacc_VwVwRb(sSumE,sVsum1a3,0x04040404);
        sSumO = Q6_Vw_vadd_VwVw(sVsum1,sVsum5);
        sSumO = Q6_Vw_vmpyiacc_VwVwRb(sSumO,sVsum3,0x06060606);
        sSumO = Q6_Vw_vmpyiacc_VwVwRb(sSumO,sVsum2a4,0x04040404);

        *optr++ = Q6_Vh_vasr_VwVwR(sSumO,sSumE,8);
    }
}

/* ======================================================================== */
void corner_response(   
    short           *restrict blurgradxx, 
    short           *restrict blurgradyy, 
    short           *restrict blurgradxy, 
    unsigned int     width, 
    int             *restrict corner             
    )
{
    int i;

    HVX_Vector sIxx, sIxy, sIyy, sTrace;
    HVX_VectorPair dIxxIyy, dIxyIxy, dDet, dTrace2, dResponse;

    HVX_Vector *iptrxx = (HVX_Vector *)blurgradxx;
    HVX_Vector *iptryy = (HVX_Vector *)blurgradyy;
    HVX_Vector *iptrxy = (HVX_Vector *)blurgradxy;
    HVX_Vector *optr   = (HVX_Vector *)corner;

    for (i=width; i>0; i-=(VLEN/2))
    {
        sIxx = *iptrxx++;
        sIyy = *iptryy++;
        sIxy = *iptrxy++;

        dIxxIyy = Q6_Ww_vmpy_VhVh(sIxx,sIyy);
        dIxyIxy = Q6_Ww_vmpy_VhVh(sIxy,sIxy);
        dDet    = Q6_Ww_vsub_WwWw(dIxxIyy,dIxyIxy);

        sTrace  = Q6_Vh_vadd_VhVh(sIxx,sIyy);
        dTrace2 = Q6_Ww_vmpy_VhVh(sTrace,sTrace);
        dTrace2 = Q6_W_vcombine_VV(Q6_Vw_vasr_VwR(Q6_V_hi_W(dTrace2),4),Q6_Vw_vasr_VwR(Q6_V_lo_W(dTrace2),4));

        dResponse = Q6_Ww_vsub_WwWw(dDet,dTrace2);
        dResponse = Q6_W_vshuff_VVR(Q6_V_hi_W(dResponse),Q6_V_lo_W(dResponse),-4);
        *optr++ = Q6_V_lo_W(dResponse);
        *optr++ = Q6_V_hi_W(dResponse);
    }
}

/* ======================================================================== */
void search(     
    int             *restrict corner, 
    unsigned int     flushcorner, 
    unsigned int     stride, 
    unsigned int     width, 
    int              threshold, 
    unsigned int    *restrict masks,
    unsigned int     boundary_l
    )
{
    int i, j, num;

    HEXAGON_Vect32  idx = 0x80000000;

    HVX_Vector sThreshold, sBitMask, sLine2, sVal, sMax, sMaxTemp;
    HVX_Vector sVmax4, sVmax5cur, sVmax5nxt, sVmax5_0, sVmax5_1, sVmax5_3, sVmax5_4; 
    HVX_Vector sMask, sMaskL, sMaskR, sMaskFF, sZero;
    HVX_VectorPred Q0;

    HVX_Vector *iptr0 = (HVX_Vector *)(corner +   flushcorner      *stride);
    HVX_Vector *iptr1 = (HVX_Vector *)(corner + ((flushcorner+1)%5)*stride); 
    HVX_Vector *iptr2 = (HVX_Vector *)(corner + ((flushcorner+2)%5)*stride); 
    HVX_Vector *iptr3 = (HVX_Vector *)(corner + ((flushcorner+3)%5)*stride); 
    HVX_Vector *iptr4 = (HVX_Vector *)(corner + ((flushcorner+4)%5)*stride); 
    HVX_Vector *optr  = (HVX_Vector *)masks;

    sThreshold = Q6_V_vsplat_R(threshold);
    sMaskFF = Q6_V_vsplat_R(-1);
    sZero   = Q6_V_vzero();

    // Boundary mask
    Q0 = Q6_Q_vcmp_eq_VwVw(Q6_V_vlalign_VVR(sZero,sMaskFF,boundary_l*4),sMaskFF);
    sMaskL = Q6_V_vand_QR(Q0,1);
    sMaskL = Q6_V_vnot_V(sMaskL);

    int nb = (width>>(LOG2VLEN-2))%32;
    int a = (width&(8*VLEN-1)) == 0 ? 0 : ((-1)<<nb);
    sMaskR = Q6_V_vnot_V(Q6_V_vsplat_R(a));
    Q0 = Q6_Q_vcmp_eq_VwVw(Q6_V_vlalign_VVR(sZero,sMaskFF,(4*width)%VLEN),sMaskFF);
    sMaskR = Q6_V_vandor_VQR(sMaskR,Q0,1<<nb);

    // Initialization
    sVmax4 = Q6_Vw_vmax_VwVw(*iptr0++,*iptr1++);
    sVmax4 = Q6_Vw_vmax_VwVw(sVmax4,*iptr3++);
    sVmax4 = Q6_Vw_vmax_VwVw(sVmax4,*iptr4++);

    sLine2 = *iptr2++;
    sVmax5cur = Q6_V_vzero();
    sVmax5nxt = Q6_Vw_vmax_VwVw(sVmax4,sLine2);

    sMask = sMaskL;
    num = 8*VLEN;

    for (i = width; i > 0; i-= 8*VLEN)
    {
        num = (i < num) ? i : num;

        sBitMask = Q6_V_vzero();

        for (j = num; j > 0; j -= VLEN/4)
        {
            sVal = sLine2;
            sLine2 = *iptr2++;

            sVmax5_0 = Q6_V_vlalign_VVR(sVmax5nxt,sVmax5cur,8);
            sVmax5_1 = Q6_V_vlalign_VVI(sVmax5nxt,sVmax5cur,4);

            sMaxTemp = Q6_Vw_vmax_VwVw(sVmax4,sThreshold);

            sVmax4 = Q6_Vw_vmax_VwVw(*iptr0++,*iptr1++);
            sVmax4 = Q6_Vw_vmax_VwVw(sVmax4,*iptr3++);
            sVmax4 = Q6_Vw_vmax_VwVw(sVmax4,*iptr4++);

            sVmax5cur = sVmax5nxt;
            sVmax5nxt = Q6_Vw_vmax_VwVw(sVmax4,sLine2);

            sVmax5_3 = Q6_V_valign_VVI(sVmax5nxt,sVmax5cur,4);
            sVmax5_4 = Q6_V_valign_VVR(sVmax5nxt,sVmax5cur,8);

            sMax = Q6_Vw_vmax_VwVw(sMaxTemp,sVmax5_0);
            sMax = Q6_Vw_vmax_VwVw(sMax,sVmax5_1);
            sMax = Q6_Vw_vmax_VwVw(sMax,sVmax5_3);
            sMax = Q6_Vw_vmax_VwVw(sMax,sVmax5_4);

            Q0 = Q6_Q_vcmp_gt_VwVw(sVal,sMax);

            idx = Q6_R_rol_RI(idx,1);
            sBitMask = Q6_V_vandor_VQR(sBitMask,Q0,idx);
        }

        *optr++ = Q6_V_vand_VV(sBitMask,sMask);
        sMask = sMaskFF;
    }

    optr--;
    optr[0] = Q6_V_vand_VV(optr[0],sMaskR);
}

/* ======================================================================== */
int get_xpos( 
    unsigned int    *masks,
    unsigned int     size, 
    int              xstart,
    short           *xpos
    )
{

   unsigned int w, bitpos;
   int k, x0;
   int numcorners = 0;

   for( k = 0; k < size; k++ )
   {
      w = masks[k];

      x0 = ((k/(VLEN/4))<<(LOG2VLEN+3)) + (k%(VLEN/4)) + xstart + 2;

      while( w != 0 )
      {
          bitpos = Q6_R_ct0_R(w);

          w ^= (1<<bitpos);

          *(xpos++) = x0 + bitpos*(VLEN/4);
          numcorners++;
      }
   }

   return numcorners;
}

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
    unsigned int num_pixels = width-boundary-2-x0; // -2: blur
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

