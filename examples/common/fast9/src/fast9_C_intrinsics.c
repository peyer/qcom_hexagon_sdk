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
#if defined(__hexagon__)
#include "hexagon_types.h"
#endif
#define VLEN 128
#define LOG2VLEN 7

#ifdef __cplusplus
extern "C" {
#endif
/*===========================================================================
    DECLARATIONS
===========================================================================*/
void sort(
    short *array,
    int n
    );

/* ======================================================================== */
/*  Intrinsic C version of fast9()                                          */
/* ======================================================================== */
void fast9_detect_coarse(
    unsigned char       *restrict img,
    unsigned int         xsize,
    unsigned int         stride,
    unsigned int         barrier,
    unsigned int        *restrict bitmask,
    unsigned int         boundary
    )
{
    int i, j, num, numpixels;

    HEXAGON_Vect32  idx = 0x80808080;

    HVX_Vector sPixel00, sPixel04, sPixel08, sPixel12;
    HVX_Vector sPV0, sPV1, sCb, sC_b, sBarrier, sBitMask;
    HVX_Vector sMax00_08, sMin00_08, sMax04_12, sMin04_12, sMinMax, sMaxMin;
    HVX_Vector sMask, sMaskL, sMaskR, sMaskFF;
    HVX_VectorPred Q0, Q1, Q2;

    HVX_Vector *iptrCn = (HVX_Vector *)(img           );
    HVX_Vector *iptrUp = (HVX_Vector *)(img - 3*stride);
    HVX_Vector *iptrDn = (HVX_Vector *)(img + 3*stride);
    HVX_Vector *optr   = (HVX_Vector *)bitmask;

    numpixels = xsize - 2*boundary + (boundary%VLEN);

    sMaskFF = Q6_V_vsplat_R(-1);
    sMaskL  = Q6_V_vnot_V(Q6_V_vand_QR(Q6_Q_vsetq_R(boundary), 0x01010101));

    int nb = (numpixels>>LOG2VLEN)&7;
    int a = (numpixels&(8*VLEN-1)) == 0 ? 0 : ((-1)<<nb);
    sMaskR = Q6_V_vnot_V(Q6_V_vsplat_R(Q6_R_vsplatb_R(a)));
    sMaskR = Q6_V_vandor_VQR(sMaskR,Q6_Q_vsetq_R(numpixels),Q6_R_vsplatb_R(1<<nb));

    sBarrier = Q6_V_vsplat_R(Q6_R_vsplatb_R(barrier));
    sMask = sMaskL;
    sPV0 = iptrCn[-1];
    sPV1 = *iptrCn++;

    num = 8*VLEN;

    for (i = numpixels; i > 0; i-= 8*VLEN)
    {
        num = (i < num) ? i : num;

        sBitMask = Q6_V_vzero();

        for (j = num; j > 0; j -= VLEN)
        {
            sPixel00 = *iptrUp++;
            sPixel08 = *iptrDn++;
            sPixel12 = Q6_V_vlalign_VVI(sPV1,sPV0,3);
            sPV0 = sPV1;
            sPV1 = *iptrCn++;
            sPixel04 = Q6_V_valign_VVI(sPV1,sPV0,3);

            sC_b= Q6_Vub_vsub_VubVub_sat(sPV0,sBarrier);
            sCb = Q6_Vub_vadd_VubVub_sat(sPV0,sBarrier);

            sMax00_08 = Q6_Vub_vmax_VubVub(sPixel00,sPixel08);
            sMin00_08 = Q6_Vub_vmin_VubVub(sPixel00,sPixel08);

            sMax04_12 = Q6_Vub_vmax_VubVub(sPixel04,sPixel12);
            sMin04_12 = Q6_Vub_vmin_VubVub(sPixel04,sPixel12);

            sMinMax = Q6_Vub_vmin_VubVub(sMax00_08,sMax04_12);
            sMaxMin = Q6_Vub_vmax_VubVub(sMin00_08,sMin04_12);

            Q0 = Q6_Q_vcmp_gt_VubVub(sMinMax,sCb);
            Q1 = Q6_Q_vcmp_gt_VubVub(sC_b,sMaxMin);
            Q2 = Q6_Q_or_QQ(Q0,Q1);

            idx = Q6_R_rol_RI(idx,1);
            sBitMask = Q6_V_vandor_VQR(sBitMask,Q2,idx);
        }

        *optr++ = Q6_V_vand_VV(sBitMask,sMask);
        sMask = sMaskFF;
    }
    optr--;
    optr[0] = Q6_V_vand_VV(optr[0],sMaskR);
}

/* ======================================================================== */
int fast9_detect_fine(
    unsigned char       *restrict img,
    unsigned int         num_pixels32,
    unsigned int         stride,
    unsigned int         barrier,
    unsigned int        *restrict bitmask,
    short int           *restrict xpos,
    int                  xstart
    )
{
    int i;
    unsigned char *p = img;
    int numcorners = 0;
    HEXAGON_Vect32 BitMasks, pr;
    HEXAGON_Vect64 PixelRef, Pixel3to12, Pixel11to4, BrightThr, DarkThr, Barriers;
    HEXAGON_Pred P0, P1, P2, P3;
    int bitpos, k, m, x, xoffset;

    Barriers = HEXAGON_V64_CREATE_W(Q6_R_vsplatb_R(barrier), Q6_R_vsplatb_R(barrier));

    for (i = 0; i < num_pixels32; i++)
    {
        BitMasks = *bitmask++;

        while (BitMasks!=0)
        {
            bitpos = Q6_R_ct0_R(BitMasks);

            k = i*32 + bitpos;
            m = k&(8*VLEN-1);
            xoffset = (k&(-8*VLEN))|((m&7)<<LOG2VLEN)|(m>>3);

            x = xstart + xoffset;

            pr = Q6_R_vsplatb_R(p[x]);

            PixelRef   = HEXAGON_V64_CREATE_W(pr, pr);

            Pixel3to12 = HEXAGON_V64_PUT_B7(Pixel3to12,p[x+0*stride-3]);    // load pixel #12
            Pixel11to4 = HEXAGON_V64_PUT_B7(Pixel11to4,p[x+0*stride+3]);    // load pixel #4

            Pixel3to12 = HEXAGON_V64_PUT_B6(Pixel3to12,p[x-1*stride-3]);    // load pixel #13
            Pixel11to4 = HEXAGON_V64_PUT_B6(Pixel11to4,p[x+1*stride+3]);    // load pixel #5

            Pixel3to12 = HEXAGON_V64_PUT_B5(Pixel3to12,p[x-2*stride-2]);    // load pixel #14
            Pixel11to4 = HEXAGON_V64_PUT_B5(Pixel11to4,p[x+2*stride+2]);    // load pixel #6

            Pixel3to12 = HEXAGON_V64_PUT_B4(Pixel3to12,p[x-3*stride-1]);    // load pixel #15
            Pixel11to4 = HEXAGON_V64_PUT_B4(Pixel11to4,p[x+3*stride+1]);    // load pixel #7

            Pixel3to12 = HEXAGON_V64_PUT_B3(Pixel3to12,p[x-3*stride+0]);    // load pixel #0
            Pixel11to4 = HEXAGON_V64_PUT_B3(Pixel11to4,p[x+3*stride+0]);    // load pixel #8

            Pixel3to12 = HEXAGON_V64_PUT_B2(Pixel3to12,p[x-3*stride+1]);    // load pixel #1
            Pixel11to4 = HEXAGON_V64_PUT_B2(Pixel11to4,p[x+3*stride-1]);    // load pixel #9

            Pixel3to12 = HEXAGON_V64_PUT_B1(Pixel3to12,p[x-2*stride+2]);    // load pixel #2
            Pixel11to4 = HEXAGON_V64_PUT_B1(Pixel11to4,p[x+2*stride-2]);    // load pixel #10

            Pixel3to12 = HEXAGON_V64_PUT_B0(Pixel3to12,p[x-1*stride+3]);    // load pixel #3
            Pixel11to4 = HEXAGON_V64_PUT_B0(Pixel11to4,p[x+1*stride-3]);    // load pixel #11

            BrightThr = Q6_P_vaddub_PP_sat(PixelRef,Barriers);
            DarkThr   = Q6_P_vsubub_PP_sat(PixelRef,Barriers);

            P0 = Q6_p_vcmpb_gtu_PP(Pixel11to4,BrightThr);
            P1 = Q6_p_vcmpb_gtu_PP(Pixel3to12,BrightThr);
            P2 = Q6_p_vcmpb_gtu_PP(DarkThr,Pixel11to4);
            P3 = Q6_p_vcmpb_gtu_PP(DarkThr,Pixel3to12);

            if (Q6_p_fastcorner9_pp(P1,P0) || Q6_p_fastcorner9_pp(P3,P2))
            {
                *xpos++ = x;
                numcorners++;
            }

            BitMasks = Q6_R_clrbit_RR(BitMasks,bitpos);
        }
    }

    return numcorners;
}
#ifdef __cplusplus
}
#endif

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
    unsigned int xstart = boundary&(-VLEN);
    unsigned int num_pixels = xsize - xstart - boundary;
    num_pixels = (num_pixels+8*VLEN-1)&(-8*VLEN); // roundup to 8*VLEN
    unsigned int num_pixels32 = num_pixels >> 5;

    unsigned int *bitmask = (unsigned int *)memalign(VLEN, num_pixels32*sizeof(unsigned int));
    short        *xpos    = (short        *)malloc(xsize*sizeof(short));

    int y, num, n, k;

    for (y = boundary; y < (ysize-boundary); ++y)
    {
        unsigned char* p = (unsigned char *)im + y*stride;

        fast9_detect_coarse(p, xsize, stride, barrier, bitmask, boundary);

        num = fast9_detect_fine(p, num_pixels32, stride, barrier, bitmask, xpos, xstart);
        sort(xpos, num);

        k = maxnumcorners - (*numcorners);
        n = ( k > num ) ? num : k;

        for (k = 0; k < n; k++)
        {
            *(xy++) = xpos[k];
            *(xy++) = y;
        }

        *numcorners += n;

        if( *numcorners >= maxnumcorners )
        {
            break;
        }
    }

    free(xpos   );
    free(bitmask);
}


