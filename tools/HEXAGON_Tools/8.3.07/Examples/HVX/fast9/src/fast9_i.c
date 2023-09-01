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
#include "hvx.cfg.h"

/* ======================================================================== */
/*  Intrinsic C version of fast9()                                          */
/* ======================================================================== */
int fast9_coord(
    unsigned int        *restrict bitmask,
    unsigned int         num_pixels32,
    int                  xstart,
    short int           *restrict xpos
    )
{
    int i;
    int numcorners = 0;
    HEXAGON_Vect32 BitMasks;
    int bitpos, k, m, x, xoffset;

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
            *xpos++ = x;
            numcorners++;

            BitMasks = Q6_R_clrbit_RR(BitMasks,bitpos);
        }
    }

    return numcorners;
}

/* ======================================================================== */
#if defined(__hexagon__)
#pragma clang optimize off
#endif
void fast9_detect(
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

    HVX_Vector sPixel00, sPixel04, sPixel08;
    HVX_Vector sPixel01, sPixel05, sPixel09;
    HVX_Vector sPixel02, sPixel06, sPixel10, sPixel14;
    HVX_Vector sPixel03, sPixel07, sPixel15;
    HVX_Vector sCn, sCn0, sCn1, sCb, sC_b, sBarrier, sBitMask;
    HVX_Vector sMaskL, sMaskR, sMaskFF;
    HVX_VectorPred Q0, Q1, Q2, Q3;
    HVX_Vector sUp10, sUp11, sDn10, sDn11;
    HVX_Vector sUp20, sUp21, sDn20, sDn21;
    HVX_Vector sUp30, sUp31, sDn30, sDn31;

    HVX_Vector *iptrCn = (HVX_Vector *)(img           );
    HVX_Vector *iptrUp3 = (HVX_Vector *)(img - 3*stride);
    HVX_Vector *iptrDn3 = (HVX_Vector *)(img + 3*stride);
    HVX_Vector *iptrUp2 = (HVX_Vector *)(img - 2*stride);
    HVX_Vector *iptrDn2 = (HVX_Vector *)(img + 2*stride);
    HVX_Vector *iptrUp1 = (HVX_Vector *)(img - 1*stride);
    HVX_Vector *iptrDn1 = (HVX_Vector *)(img + 1*stride);
    HVX_Vector *optr   = (HVX_Vector *)bitmask;

    numpixels = xsize - boundary - 3 - ((boundary-3) & -VLEN);

    sMaskFF = Q6_V_vsplat_R(-1);
    sMaskL = Q6_V_vnot_V(Q6_V_vand_QR(Q6_Q_vsetq_R(boundary-3), 0x01010101));

    int nb = (numpixels>>LOG2VLEN)&7;
    int a = (numpixels&(8*VLEN-1)) == 0 ? 0 : ((-1)<<nb);
    sMaskR = Q6_V_vnot_V(Q6_V_vsplat_R(Q6_R_vsplatb_R(a)));
    sMaskR = Q6_V_vandor_VQR(sMaskR,Q6_Q_vsetq_R(numpixels),Q6_R_vsplatb_R(1<<nb));
    HVX_Vector sZero = Q6_V_vzero();
    HVX_Vector sConst3f = Q6_V_vsplat_R(0x3f3f3f3f);
    HVX_Vector sConst0f = Q6_V_vsplat_R(0xf0f0f0f);

    sBarrier = Q6_V_vsplat_R(Q6_R_vsplatb_R(barrier));
    sUp30 = *iptrUp3++;
    sUp20 = *iptrUp2++;
    sUp10 = *iptrUp1++;
    sCn0 = *iptrCn++;
    sDn10 = *iptrDn1++;
    sDn20 = *iptrDn2++;
    sDn30 = *iptrDn3++;

    num = 8*VLEN;

    unsigned idx0 = 0x01010101;
    unsigned idx1 = 0x02020202;
    unsigned idx2 = 0x04040404;
    unsigned idx3 = 0x08080808;
    unsigned idx4 = 0x10101010;
    unsigned idx5 = 0x20202020;
    unsigned idx6 = 0x40404040;
    unsigned idx7 = 0x80808080;

    for (i = numpixels; i > 0; i-= 8*VLEN)
    {
        HVX_Vector sBrBitL, sBrBitH, sDkBitL, sDkBitH;
        num = (i < num) ? i : num;

        sBitMask = Q6_V_vzero();

        for (j = num; j > 0; j -= VLEN)
        {
#define sPixel12 sCn0
#define sPixel13 sUp10
#define sPixel11 sDn10
            sCn1 = *iptrCn++;
            sCn = Q6_V_valign_VVI(sCn1, sCn0,3);
            sPixel04 = Q6_V_valign_VVI(sCn1, sCn0,6);

            sC_b= Q6_Vub_vsub_VubVub_sat(sCn, sBarrier);
            sCb = Q6_Vub_vadd_VubVub_sat(sCn, sBarrier);

            Q0 = Q6_Q_vcmp_gt_VubVub(sPixel12, sCb);
            Q1 = Q6_Q_vcmp_gt_VubVub(sC_b, sPixel12);
            sBrBitL = Q6_V_vand_QR(Q0, idx1);
            sDkBitL = Q6_V_vand_QR(Q1, idx1);
            sCn0 = sCn1;

            Q2 = Q6_Q_vcmp_gt_VubVub(sPixel04, sCb);
            Q3 = Q6_Q_vcmp_gt_VubVub(sC_b, sPixel04);
            sBrBitL = Q6_V_vandor_VQR(sBrBitL, Q2, idx5);
            sDkBitL = Q6_V_vandor_VQR(sDkBitL, Q3, idx5);

            sUp31 = *iptrUp3++;
            sPixel15 = Q6_V_valign_VVI(sUp31, sUp30,2);
            sPixel00 = Q6_V_valign_VVI(sUp31, sUp30,3);
            sPixel01 = Q6_V_valign_VVI(sUp31, sUp30,4);
            sUp30 = sUp31;

            Q0 = Q6_Q_vcmp_gt_VubVub(sPixel00, sCb);
            Q1 = Q6_Q_vcmp_gt_VubVub(sC_b, sPixel00);
            sBrBitL = Q6_V_vandor_VQR(sBrBitL, Q0, idx7);
            sDkBitL = Q6_V_vandor_VQR(sDkBitL, Q1, idx7);

            Q2 = Q6_Q_vcmp_gt_VubVub(sPixel01, sCb);
            Q3 = Q6_Q_vcmp_gt_VubVub(sC_b, sPixel01);
            sBrBitH = Q6_V_vand_QR(Q2, idx7);
            sDkBitH = Q6_V_vand_QR(Q3, idx7);

            Q0 = Q6_Q_vcmp_gt_VubVub(sPixel15, sCb);
            Q1 = Q6_Q_vcmp_gt_VubVub(sC_b, sPixel15);
            sBrBitH = Q6_V_vandor_VQR(sBrBitH, Q0, idx0);
            sDkBitH = Q6_V_vandor_VQR(sDkBitH, Q1, idx0);

            sUp21 = *iptrUp2++;
            sPixel14 = Q6_V_valign_VVI(sUp21, sUp20,1);
            sPixel02 = Q6_V_valign_VVI(sUp21, sUp20,5);
            sUp20 = sUp21;

            Q2 = Q6_Q_vcmp_gt_VubVub(sPixel14, sCb);
            Q3 = Q6_Q_vcmp_gt_VubVub(sC_b, sPixel14);
            sBrBitL = Q6_V_vandor_VQR(sBrBitL, Q2, idx0);
            sDkBitL = Q6_V_vandor_VQR(sDkBitL, Q3, idx0);

            Q0 = Q6_Q_vcmp_gt_VubVub(sPixel02, sCb);
            Q1 = Q6_Q_vcmp_gt_VubVub(sC_b, sPixel02);
            sBrBitL = Q6_V_vandor_VQR(sBrBitL, Q0, idx6);
            sDkBitL = Q6_V_vandor_VQR(sDkBitL, Q1, idx6);

            sUp11 = *iptrUp1++;
            sPixel03 = Q6_V_valign_VVI(sUp11, sUp10,6);

            Q0 = Q6_Q_vcmp_gt_VubVub(sPixel13, sCb);
            Q1 = Q6_Q_vcmp_gt_VubVub(sC_b, sPixel13);
            sBrBitH = Q6_V_vandor_VQR(sBrBitH, Q0, idx1);
            sDkBitH = Q6_V_vandor_VQR(sDkBitH, Q1, idx1);
            sUp10 = sUp11;

            Q2 = Q6_Q_vcmp_gt_VubVub(sPixel03, sCb);
            Q3 = Q6_Q_vcmp_gt_VubVub(sC_b, sPixel03);
            sBrBitH = Q6_V_vandor_VQR(sBrBitH, Q2, idx6);
            sDkBitH = Q6_V_vandor_VQR(sDkBitH, Q3, idx6);

            sDn11 = *iptrDn1++;
            sPixel05 = Q6_V_valign_VVI(sDn11, sDn10,6);

            Q0 = Q6_Q_vcmp_gt_VubVub(sPixel11, sCb);
            Q1 = Q6_Q_vcmp_gt_VubVub(sC_b, sPixel11);
            sBrBitH = Q6_V_vandor_VQR(sBrBitH, Q0, idx2);
            sDkBitH = Q6_V_vandor_VQR(sDkBitH, Q1, idx2);
            sDn10 = sDn11;

            Q2 = Q6_Q_vcmp_gt_VubVub(sPixel05, sCb);
            Q3 = Q6_Q_vcmp_gt_VubVub(sC_b, sPixel05);
            sBrBitH = Q6_V_vandor_VQR(sBrBitH, Q2, idx5);
            sDkBitH = Q6_V_vandor_VQR(sDkBitH, Q3, idx5);

            sDn21 = *iptrDn2++;
            sPixel10 = Q6_V_valign_VVI(sDn21, sDn20,1);
            sPixel06 = Q6_V_valign_VVI(sDn21, sDn20,5);
            sDn20 = sDn21;

            Q0 = Q6_Q_vcmp_gt_VubVub(sPixel10, sCb);
            Q1 = Q6_Q_vcmp_gt_VubVub(sC_b, sPixel10);
            sBrBitL = Q6_V_vandor_VQR(sBrBitL, Q0, idx2);
            sDkBitL = Q6_V_vandor_VQR(sDkBitL, Q1, idx2);

            Q2 = Q6_Q_vcmp_gt_VubVub(sPixel06, sCb);
            Q3 = Q6_Q_vcmp_gt_VubVub(sC_b, sPixel06);
            sBrBitL = Q6_V_vandor_VQR(sBrBitL, Q2, idx4);
            sDkBitL = Q6_V_vandor_VQR(sDkBitL, Q3, idx4);

            sDn31 = *iptrDn3++;
            sPixel09 = Q6_V_valign_VVI(sDn31, sDn30,2);
            sPixel08 = Q6_V_valign_VVI(sDn31, sDn30,3);
            sPixel07 = Q6_V_valign_VVI(sDn31, sDn30,4);
            sDn30 = sDn31;

            Q0 = Q6_Q_vcmp_gt_VubVub(sPixel09, sCb);
            Q1 = Q6_Q_vcmp_gt_VubVub(sC_b, sPixel09);
            sBrBitH = Q6_V_vandor_VQR(sBrBitH, Q0, idx3);
            sDkBitH = Q6_V_vandor_VQR(sDkBitH, Q1, idx3);

            Q2 = Q6_Q_vcmp_gt_VubVub(sPixel08, sCb);
            Q3 = Q6_Q_vcmp_gt_VubVub(sC_b, sPixel08);
            sBrBitL = Q6_V_vandor_VQR(sBrBitL, Q2, idx3);
            sDkBitL = Q6_V_vandor_VQR(sDkBitL, Q3, idx3);

            Q0 = Q6_Q_vcmp_gt_VubVub(sPixel07, sCb);
            Q1 = Q6_Q_vcmp_gt_VubVub(sC_b, sPixel07);
            sBrBitH = Q6_V_vandor_VQR(sBrBitH, Q0, idx4);
            sDkBitH = Q6_V_vandor_VQR(sDkBitH, Q1, idx4);

            HVX_Vector sBrBit01 = Q6_V_vand_VV(sBrBitH, sBrBitL);
            HVX_Vector sDkBit01 = Q6_V_vand_VV(sDkBitH, sDkBitL);

            HVX_Vector sBrBit23 = Q6_Vb_vadd_VbVb(sBrBit01, sBrBit01);
            HVX_Vector sDkBit23 = Q6_Vb_vadd_VbVb(sDkBit01, sDkBit01);
            Q2 = Q6_Q_vcmp_gt_VbVb(sZero, sBrBit01);
            sBrBit23 = Q6_V_vandor_VQR(sBrBit23, Q2, idx0);
            Q3 = Q6_Q_vcmp_gt_VbVb(sZero, sDkBit01);
            sDkBit23 = Q6_V_vandor_VQR(sDkBit23 , Q3, idx0);
            HVX_Vector sBrBit03 = Q6_V_vand_VV(sBrBit23, sBrBit01);
            HVX_Vector sDkBit03 = Q6_V_vand_VV(sDkBit23, sDkBit01);

            HVX_Vector sBrBit03t = Q6_V_vand_VV(sBrBit03, sConst3f);
            HVX_VectorPair dBrBit47 = Q6_Wb_vshuffoe_VbVb(sBrBit03t, sBrBit03);
            HVX_Vector sBrBit47 = Q6_Vub_vasr_VhVhR_sat(Q6_V_hi_W(dBrBit47), Q6_V_lo_W(dBrBit47), 8-2);

            HVX_Vector sBrBit07 = Q6_V_vand_VV(sBrBit47, sBrBit03);
            HVX_Vector sDkBit03t = Q6_V_vand_VV(sDkBit03, sConst3f);
            HVX_VectorPair dDkBit47 = Q6_Wb_vshuffoe_VbVb(sDkBit03t, sDkBit03);
            HVX_Vector sDkBit47 = Q6_Vub_vasr_VhVhR_sat(Q6_V_hi_W(dDkBit47), Q6_V_lo_W(dDkBit47), 8-2);
            HVX_Vector sDkBit07 = Q6_V_vand_VV(sDkBit47, sDkBit03);

            HVX_Vector sBrBitLt = Q6_V_vand_VV(sBrBitL, sConst0f);
            HVX_VectorPair dBr8 = Q6_Wb_vshuffoe_VbVb(sBrBitLt, sBrBitL);
            HVX_Vector sBrS8 = Q6_Vub_vasr_VhVhR_sat(Q6_V_hi_W(dBr8), Q6_V_lo_W(dBr8), 8-4);
            HVX_VectorPair dBr15 = Q6_Wb_vshuffoe_VbVb(sBrBitH, sBrBitH);
            HVX_Vector sBr15_L = Q6_Vh_vavg_VhVh(Q6_V_lo_W(dBr15), sZero);
            HVX_Vector sBr15_H = Q6_Vh_vavg_VhVh(Q6_V_hi_W(dBr15), sZero);
            HVX_Vector sBrS15 = Q6_Vb_vshuffe_VbVb(sBr15_H, sBr15_L);
            HVX_Vector sBr0815 = Q6_V_vor_VV(sBrS8, sBrS15);
            HVX_Vector sBrIsCorner =  Q6_V_vand_VV(sBrBit07, sBr0815);

            HVX_Vector sDkBitLt = Q6_V_vand_VV(sDkBitL, sConst0f);
            HVX_VectorPair dDk8 = Q6_Wb_vshuffoe_VbVb(sDkBitLt, sDkBitL);
            HVX_Vector sDkS8 = Q6_Vub_vasr_VhVhR_sat(Q6_V_hi_W(dDk8), Q6_V_lo_W(dDk8), 8-4);
            HVX_VectorPair dDk15 = Q6_Wb_vshuffoe_VbVb(sDkBitH, sDkBitH);
            HVX_Vector sDk15_L = Q6_Vh_vavg_VhVh(Q6_V_lo_W(dDk15), sZero);
            HVX_Vector sDk15_H = Q6_Vh_vavg_VhVh(Q6_V_hi_W(dDk15), sZero);
            HVX_Vector sDkS15 = Q6_Vb_vshuffe_VbVb(sDk15_H, sDk15_L);
            HVX_Vector sDk0815 = Q6_V_vor_VV(sDkS8, sDkS15);
            HVX_Vector sDkIsCorner =  Q6_V_vand_VV(sDkBit07, sDk0815);

            HVX_Vector sIsCorner = Q6_V_vor_VV(sBrIsCorner, sDkIsCorner);

            Q2 = Q6_Q_vcmp_gt_VubVub(sIsCorner, sZero);
            idx = Q6_R_rol_RI(idx,1);
            sBitMask = Q6_V_vandor_VQR(sBitMask,Q2,idx);
        }

        sBitMask = Q6_V_vand_VV(sBitMask, sMaskL);
        *optr++ = sBitMask;
        sMaskL = sMaskFF;
    }
    optr--;
    optr[0] = Q6_V_vand_VV(optr[0], sMaskR);
}

/* ======================================================================== */
#if defined(__hexagon__)
#pragma clang optimize on
#endif
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
        num = fast9_coord(bitmask, num_pixels32,  xstart + 3, xpos);
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
