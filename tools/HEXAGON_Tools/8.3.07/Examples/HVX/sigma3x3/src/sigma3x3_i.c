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
#include "hexagon_types.h"
#include "hvx.cfg.h"
/* ======================================================================== */
/*  Intrinsic C version of sigma3x3()                                       */
/* ======================================================================== */
const short invLUT[10] __attribute__((aligned(128))) = {
    0,-32768,-16384,-10922,-8192,-6553,-5461,-4681,-4096,-3640
};


/* ======================================================================== */
void sigma3x3PerRow(
    unsigned char   *restrict src,
    int             stride,
    int             width,
    unsigned char   threshold,
    unsigned char   *restrict dst
    )
{
    int i;

    HEXAGON_Vect32 minus1 = -1;

    HVX_Vector sRow0prv, sRow0cur, sRow0nxt, sRow1prv, sRow1cur, sRow1nxt, sRow2prv, sRow2cur, sRow2nxt;
    HVX_Vector sX00, sX01, sX02, sX10, sX12, sX20, sX21, sX22, sCentr;
    HVX_Vector sX00t, sX01t, sX02t, sX10t, sX12t, sX20t, sX21t, sX22t;
    HVX_Vector sAbdif00, sAbdif01, sAbdif02, sAbdif10, sAbdif12, sAbdif20, sAbdif21, sAbdif22;
    HVX_Vector sThres, sInvTab, sCnt, sOutE, sOutO;
    HVX_Vector Zero, One, Two;

    HVX_VectorPair dX21tX01t, dX20tX00t, dX22tX02t, dX12tX10t, dSum, dInv;

    HVX_VectorPred Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Qc;

    HVX_Vector *inp0 = (HVX_Vector *)(src - 1*stride);
    HVX_Vector *inp1 = (HVX_Vector *)(src + 0*stride);
    HVX_Vector *inp2 = (HVX_Vector *)(src + 1*stride);
    HVX_Vector *outp = (HVX_Vector *)dst;

    sInvTab = Q6_Vh_vshuff_Vh(*((HVX_Vector *)invLUT));

    sThres = Q6_V_vsplat_R(Q6_R_vsplatb_R(threshold));
    Zero   = Q6_V_vzero();
    One    = Q6_V_vsplat_R(0x01010101);
    Two    = Q6_V_vsplat_R(0x02020202);

    sRow0prv = Q6_V_vzero();
    sRow1prv = Q6_V_vzero();
    sRow2prv = Q6_V_vzero();

    sRow0cur = *inp0;
    sRow1cur = *inp1;
    sRow2cur = *inp2;

    if (width > VLEN)
    {
        inp0++; inp1++; inp2++;
    }

    for ( i=width; i>0; i-=VLEN )
    {
        sRow0nxt = *inp0;
        sRow1nxt = *inp1;
        sRow2nxt = *inp2;

        sCentr = sRow1cur;
        sX01   = sRow0cur;
        sX21   = sRow2cur;
        dSum = Q6_Wh_vsub_VubVub(Zero,sCentr);

        sAbdif01 = Q6_Vub_vabsdiff_VubVub(sX01,sCentr);
        sAbdif21 = Q6_Vub_vabsdiff_VubVub(sX21,sCentr);
        Q0 = Q6_Q_vcmp_gt_VubVub(sAbdif01,sThres);
        Q1 = Q6_Q_vcmp_gt_VubVub(sAbdif21,sThres);
        sCnt  = Q6_V_vmux_QVV(Q0,One,Two);
        sCnt  = Q6_Vb_condacc_QnVbVb(Q1,sCnt,One);
        sX01t = Q6_V_vmux_QVV(Q0,Zero,sX01);
        sX21t = Q6_V_vmux_QVV(Q1,Zero,sX21);
        dX21tX01t = Q6_W_vcombine_VV(sX21t,sX01t);
        dSum  = Q6_Wh_vmpaacc_WhWubRb(dSum, dX21tX01t,minus1);

        sX00 = Q6_V_vlalign_VVI(sRow0cur,sRow0prv,1);
        sX20 = Q6_V_vlalign_VVI(sRow2cur,sRow2prv,1);
        sAbdif00 = Q6_Vub_vabsdiff_VubVub(sX00,sCentr);
        sAbdif20 = Q6_Vub_vabsdiff_VubVub(sX20,sCentr);
        Q2 = Q6_Q_vcmp_gt_VubVub(sAbdif00,sThres);
        Q3 = Q6_Q_vcmp_gt_VubVub(sAbdif20,sThres);
        sCnt  = Q6_Vb_condacc_QnVbVb(Q2,sCnt,One);
        sCnt  = Q6_Vb_condacc_QnVbVb(Q3,sCnt,One);
        sX00t = Q6_V_vmux_QVV(Q2,Zero,sX00);
        sX20t = Q6_V_vmux_QVV(Q3,Zero,sX20);
        dX20tX00t = Q6_W_vcombine_VV(sX20t,sX00t);
        dSum  = Q6_Wh_vmpaacc_WhWubRb(dSum, dX20tX00t,minus1);

        sX02 = Q6_V_valign_VVI(sRow0nxt,sRow0cur,1);
        sX22 = Q6_V_valign_VVI(sRow2nxt,sRow2cur,1);
        sAbdif02 = Q6_Vub_vabsdiff_VubVub(sX02,sCentr);
        sAbdif22 = Q6_Vub_vabsdiff_VubVub(sX22,sCentr);
        Q4 = Q6_Q_vcmp_gt_VubVub(sAbdif02,sThres);
        Q5 = Q6_Q_vcmp_gt_VubVub(sAbdif22,sThres);
        sCnt  = Q6_Vb_condacc_QnVbVb(Q4,sCnt,One);
        sCnt  = Q6_Vb_condacc_QnVbVb(Q5,sCnt,One);
        sX02t = Q6_V_vmux_QVV(Q4,Zero,sX02);
        sX22t = Q6_V_vmux_QVV(Q5,Zero,sX22);
        dX22tX02t = Q6_W_vcombine_VV(sX22t,sX02t);
        dSum  = Q6_Wh_vmpaacc_WhWubRb(dSum, dX22tX02t,minus1);

        sX10 = Q6_V_vlalign_VVI(sRow1cur,sRow1prv,1);
        sX12 = Q6_V_valign_VVI( sRow1nxt,sRow1cur,1);
        sAbdif10 = Q6_Vub_vabsdiff_VubVub(sX10,sCentr);
        sAbdif12 = Q6_Vub_vabsdiff_VubVub(sX12,sCentr);
        Q6 = Q6_Q_vcmp_gt_VubVub(sAbdif10,sThres);
        Q7 = Q6_Q_vcmp_gt_VubVub(sAbdif12,sThres);
        sCnt  = Q6_Vb_condacc_QnVbVb(Q6,sCnt,One);
        sCnt  = Q6_Vb_condacc_QnVbVb(Q7,sCnt,One);
        sX10t = Q6_V_vmux_QVV(Q6,Zero,sX10);
        sX12t = Q6_V_vmux_QVV(Q7,Zero,sX12);
        dX12tX10t = Q6_W_vcombine_VV(sX12t,sX10t);
        dSum  = Q6_Wh_vmpaacc_WhWubRb(dSum, dX12tX10t,minus1);

        dInv = Q6_Wh_vlut16_VbVhR(sCnt,sInvTab,0);
        sOutE = Q6_Vh_vmpy_VhVh_s1_rnd_sat(Q6_V_lo_W(dSum),Q6_V_lo_W(dInv));
        sOutO = Q6_Vh_vmpy_VhVh_s1_rnd_sat(Q6_V_hi_W(dSum),Q6_V_hi_W(dInv));
        *outp++ = Q6_Vub_vsat_VhVh(sOutO,sOutE);

        sRow0prv = sRow0cur;
        sRow0cur = sRow0nxt;
        sRow2prv = sRow2cur;
        sRow2cur = sRow2nxt;
        sRow1prv = sRow1cur;
        sRow1cur = sRow1nxt;

        if (i > 2*VLEN)
        {
            inp0++; inp1++; inp2++;
        }
    }
}


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


