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
#include "hexagon_types.h"
#include "hvx.cfg.h"
/* ======================================================================== */
/*  Intrinsic C version of bilateral9x9()                                   */
/* ======================================================================== */
void bilateral9x9PerRow(
    unsigned char   *input,
    int              stride,
    int              width,
    unsigned char   *gauss_LUT,
    unsigned char   *range_LUT,
    unsigned char   *output
    )
{
    int i, by, ks;
    HEXAGON_Vect32 c01010101, w00, Gaussk;

    HVX_Vector sPrv, sCur, sNxt, sCentr, sX_k, sXk;
    HVX_Vector sAbsDif0, sAbsDif1, sRange0, sRange1, sWeight0, sWeight1;
    HVX_Vector sSumFiltE0, sSumFiltE2, sSumFiltO1, sSumFiltO3;
    HVX_Vector sDenomE0, sDenomE2, sDenomO1, sDenomO3;
    HVX_Vector sResultE0, sResultE2, sResultO1, sResultO3, sBit;
    HVX_Vector sRangeTab0, sRangeTab1, sRangeTab2, sRangeTab3;
    HVX_VectorPred Q0, Q1, Q2, Q3;
    HVX_VectorPair dRangxGaus0, dRangxGaus1, dPixelxW0, dPixelxW1, dDenomE, dDenomO;
    HVX_VectorPair dSumWeight, dSumFiltE, dSumFiltO;

    HVX_Vector *iptrk;
    HVX_Vector *iptr0 = (HVX_Vector *)input;
    HVX_Vector *outp  = (HVX_Vector *)output;

    c01010101 = 0x01010101;
    w00 = ((int)range_LUT[0]*gauss_LUT[4*9+4]) >> 8;
    w00 = Q6_R_combine_RlRl(w00,w00);

#if LOG2VLEN == 7
    sRangeTab0 = Q6_Vb_vshuff_Vb(((HVX_Vector *)range_LUT)[0]);
    sRangeTab2 = Q6_Vb_vshuff_Vb(((HVX_Vector *)range_LUT)[1]);
    sRangeTab1 = sRangeTab0;
    sRangeTab3 = sRangeTab2;
#else
    sRangeTab0 = Q6_Vb_vshuff_Vb(((HVX_Vector *)range_LUT)[0]);
    sRangeTab1 = Q6_Vb_vshuff_Vb(((HVX_Vector *)range_LUT)[1]);
    sRangeTab2 = Q6_Vb_vshuff_Vb(((HVX_Vector *)range_LUT)[2]);
    sRangeTab3 = Q6_Vb_vshuff_Vb(((HVX_Vector *)range_LUT)[3]);
#endif

    for (i = width; i > 0; i-=VLEN)
    {
        sCentr = *iptr0++;
        dSumFiltE  = Q6_Ww_vmpy_VhRh(Q6_V_lo_W(Q6_Wuh_vzxt_Vub(sCentr)),w00);
        dSumFiltO  = Q6_Ww_vmpy_VhRh(Q6_V_hi_W(Q6_Wuh_vzxt_Vub(sCentr)),w00);
        dSumWeight = Q6_W_vcombine_VV(Q6_V_vsplat_R(w00), Q6_V_vsplat_R(w00));

        for (by = -4; by <= 4; by++)
        {
            iptrk = (HVX_Vector *)(input + by*stride);

            sPrv = iptrk[-1];
            sCur = iptrk[ 0];
            sNxt = iptrk[ 1];

            for (ks = 4; ks > 0; ks--)
            {
                sX_k = Q6_V_vlalign_VVR(sCur,sPrv,ks);
                sXk  = Q6_V_valign_VVR (sNxt,sCur,ks);

                sAbsDif0 = Q6_Vub_vabsdiff_VubVub(sX_k,sCentr);
                sAbsDif1 = Q6_Vub_vabsdiff_VubVub(sXk, sCentr);

                sRange0  = Q6_Vb_vlut32_VbVbR(            sAbsDif0,sRangeTab0,0);
                sRange0  = Q6_Vb_vlut32or_VbVbVbR(sRange0,sAbsDif0,sRangeTab0,1);
                sRange0  = Q6_Vb_vlut32or_VbVbVbR(sRange0,sAbsDif0,sRangeTab1,2);
                sRange0  = Q6_Vb_vlut32or_VbVbVbR(sRange0,sAbsDif0,sRangeTab1,3);
                sRange0  = Q6_Vb_vlut32or_VbVbVbR(sRange0,sAbsDif0,sRangeTab2,4);
                sRange0  = Q6_Vb_vlut32or_VbVbVbR(sRange0,sAbsDif0,sRangeTab2,5);
                sRange0  = Q6_Vb_vlut32or_VbVbVbR(sRange0,sAbsDif0,sRangeTab3,6);
                sRange0  = Q6_Vb_vlut32or_VbVbVbR(sRange0,sAbsDif0,sRangeTab3,7);

                sRange1  = Q6_Vb_vlut32_VbVbR(            sAbsDif1,sRangeTab0,0);
                sRange1  = Q6_Vb_vlut32or_VbVbVbR(sRange1,sAbsDif1,sRangeTab0,1);
                sRange1  = Q6_Vb_vlut32or_VbVbVbR(sRange1,sAbsDif1,sRangeTab1,2);
                sRange1  = Q6_Vb_vlut32or_VbVbVbR(sRange1,sAbsDif1,sRangeTab1,3);
                sRange1  = Q6_Vb_vlut32or_VbVbVbR(sRange1,sAbsDif1,sRangeTab2,4);
                sRange1  = Q6_Vb_vlut32or_VbVbVbR(sRange1,sAbsDif1,sRangeTab2,5);
                sRange1  = Q6_Vb_vlut32or_VbVbVbR(sRange1,sAbsDif1,sRangeTab3,6);
                sRange1  = Q6_Vb_vlut32or_VbVbVbR(sRange1,sAbsDif1,sRangeTab3,7);

                Gaussk = Q6_R_vsplatb_R(gauss_LUT[(by+4)*9+(4-ks)]);
                dRangxGaus0 = Q6_Wuh_vmpy_VubRub(sRange0,Gaussk);
                dRangxGaus1 = Q6_Wuh_vmpy_VubRub(sRange1,Gaussk);

                sWeight0 = Q6_Vb_vshuffo_VbVb(Q6_V_hi_W(dRangxGaus0),Q6_V_lo_W(dRangxGaus0));
                sWeight1 = Q6_Vb_vshuffo_VbVb(Q6_V_hi_W(dRangxGaus1),Q6_V_lo_W(dRangxGaus1));
                dSumWeight = Q6_Wh_vmpaacc_WhWubRb(dSumWeight,Q6_W_vcombine_VV(sWeight1,sWeight0),c01010101);

                dPixelxW0 = Q6_Wuh_vmpy_VubVub(sX_k,sWeight0);
                dPixelxW1 = Q6_Wuh_vmpy_VubVub(sXk ,sWeight1);
                dSumFiltE = Q6_Ww_vadd_WwWw(dSumFiltE, Q6_Ww_vadd_VuhVuh(Q6_V_lo_W(dPixelxW0),Q6_V_lo_W(dPixelxW1)));
                dSumFiltO = Q6_Ww_vadd_WwWw(dSumFiltO, Q6_Ww_vadd_VuhVuh(Q6_V_hi_W(dPixelxW0),Q6_V_hi_W(dPixelxW1)));
            }
        }

        for (ks = 4; ks > 0; ks--)
        {
            sX_k = *(HVX_Vector *)(input - ks*stride);
            sXk  = *(HVX_Vector *)(input + ks*stride);

            sAbsDif0 = Q6_Vub_vabsdiff_VubVub(sX_k,sCentr);
            sAbsDif1 = Q6_Vub_vabsdiff_VubVub(sXk, sCentr);

            sRange0  = Q6_Vb_vlut32_VbVbR(            sAbsDif0,sRangeTab0,0);
            sRange0  = Q6_Vb_vlut32or_VbVbVbR(sRange0,sAbsDif0,sRangeTab0,1);
            sRange0  = Q6_Vb_vlut32or_VbVbVbR(sRange0,sAbsDif0,sRangeTab1,2);
            sRange0  = Q6_Vb_vlut32or_VbVbVbR(sRange0,sAbsDif0,sRangeTab1,3);
            sRange0  = Q6_Vb_vlut32or_VbVbVbR(sRange0,sAbsDif0,sRangeTab2,4);
            sRange0  = Q6_Vb_vlut32or_VbVbVbR(sRange0,sAbsDif0,sRangeTab2,5);
            sRange0  = Q6_Vb_vlut32or_VbVbVbR(sRange0,sAbsDif0,sRangeTab3,6);
            sRange0  = Q6_Vb_vlut32or_VbVbVbR(sRange0,sAbsDif0,sRangeTab3,7);

            sRange1  = Q6_Vb_vlut32_VbVbR(            sAbsDif1,sRangeTab0,0);
            sRange1  = Q6_Vb_vlut32or_VbVbVbR(sRange1,sAbsDif1,sRangeTab0,1);
            sRange1  = Q6_Vb_vlut32or_VbVbVbR(sRange1,sAbsDif1,sRangeTab1,2);
            sRange1  = Q6_Vb_vlut32or_VbVbVbR(sRange1,sAbsDif1,sRangeTab1,3);
            sRange1  = Q6_Vb_vlut32or_VbVbVbR(sRange1,sAbsDif1,sRangeTab2,4);
            sRange1  = Q6_Vb_vlut32or_VbVbVbR(sRange1,sAbsDif1,sRangeTab2,5);
            sRange1  = Q6_Vb_vlut32or_VbVbVbR(sRange1,sAbsDif1,sRangeTab3,6);
            sRange1  = Q6_Vb_vlut32or_VbVbVbR(sRange1,sAbsDif1,sRangeTab3,7);

            Gaussk = Q6_R_vsplatb_R(gauss_LUT[(4-ks)*9+4]);
            dRangxGaus0 = Q6_Wuh_vmpy_VubRub(sRange0,Gaussk);
            dRangxGaus1 = Q6_Wuh_vmpy_VubRub(sRange1,Gaussk);

            sWeight0 = Q6_Vb_vshuffo_VbVb(Q6_V_hi_W(dRangxGaus0),Q6_V_lo_W(dRangxGaus0));
            sWeight1 = Q6_Vb_vshuffo_VbVb(Q6_V_hi_W(dRangxGaus1),Q6_V_lo_W(dRangxGaus1));
            dSumWeight = Q6_Wh_vmpaacc_WhWubRb(dSumWeight,Q6_W_vcombine_VV(sWeight1,sWeight0),c01010101);

            dPixelxW0 = Q6_Wuh_vmpy_VubVub(sX_k,sWeight0);
            dPixelxW1 = Q6_Wuh_vmpy_VubVub(sXk ,sWeight1);
            dSumFiltE = Q6_Ww_vadd_WwWw(dSumFiltE, Q6_Ww_vadd_VuhVuh(Q6_V_lo_W(dPixelxW0),Q6_V_lo_W(dPixelxW1)));
            dSumFiltO = Q6_Ww_vadd_WwWw(dSumFiltO, Q6_Ww_vadd_VuhVuh(Q6_V_hi_W(dPixelxW0),Q6_V_hi_W(dPixelxW1)));
        }

        // divided by weights
        dDenomE = Q6_Ww_vmpy_VhRh(Q6_V_lo_W(dSumWeight),0x00800080);
        dDenomO = Q6_Ww_vmpy_VhRh(Q6_V_hi_W(dSumWeight),0x00800080);

        sDenomE0 = Q6_V_lo_W(dDenomE);
        sDenomE2 = Q6_V_hi_W(dDenomE);
        sDenomO1 = Q6_V_lo_W(dDenomO);
        sDenomO3 = Q6_V_hi_W(dDenomO);

        sSumFiltE0 = Q6_V_lo_W(dSumFiltE);
        sSumFiltE2 = Q6_V_hi_W(dSumFiltE);
        sSumFiltO1 = Q6_V_lo_W(dSumFiltO);
        sSumFiltO3 = Q6_V_hi_W(dSumFiltO);

        sResultE0 = Q6_V_vzero();
        sResultE2 = Q6_V_vzero();
        sResultO1 = Q6_V_vzero();
        sResultO3 = Q6_V_vzero();

        sBit = Q6_V_vsplat_R(0x00800080);

        for (ks = 0; ks < 8; ks++)
        {
            Q0 = Q6_Q_vcmp_gt_VwVw(sDenomE0,sSumFiltE0);
            Q1 = Q6_Q_vcmp_gt_VwVw(sDenomE2,sSumFiltE2);
            Q2 = Q6_Q_vcmp_gt_VwVw(sDenomO1,sSumFiltO1);
            Q3 = Q6_Q_vcmp_gt_VwVw(sDenomO3,sSumFiltO3);

            sResultE0 = Q6_Vw_condacc_QnVwVw(Q0,sResultE0,sBit);
            sResultE2 = Q6_Vw_condacc_QnVwVw(Q1,sResultE2,sBit);
            sResultO1 = Q6_Vw_condacc_QnVwVw(Q2,sResultO1,sBit);
            sResultO3 = Q6_Vw_condacc_QnVwVw(Q3,sResultO3,sBit);

            sSumFiltE0 = Q6_Vw_condnac_QnVwVw(Q0,sSumFiltE0,sDenomE0);
            sSumFiltE2 = Q6_Vw_condnac_QnVwVw(Q1,sSumFiltE2,sDenomE2);
            sSumFiltO1 = Q6_Vw_condnac_QnVwVw(Q2,sSumFiltO1,sDenomO1);
            sSumFiltO3 = Q6_Vw_condnac_QnVwVw(Q3,sSumFiltO3,sDenomO3);

            sDenomE0 = Q6_Vw_vasr_VwR(sDenomE0,1);
            sDenomE2 = Q6_Vw_vasr_VwR(sDenomE2,1);
            sDenomO1 = Q6_Vw_vasr_VwR(sDenomO1,1);
            sDenomO3 = Q6_Vw_vasr_VwR(sDenomO3,1);
            sBit     = Q6_Vw_vasr_VwR(sBit,1);
        }

        *outp++ = Q6_Vb_vshuffe_VbVb(Q6_Vh_vshuffe_VhVh(sResultO3,sResultO1),Q6_Vh_vshuffe_VhVh(sResultE2,sResultE0));
        input += VLEN;
    }
}


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

