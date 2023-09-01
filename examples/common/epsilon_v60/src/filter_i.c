/*=====================================================================
               Copyright (c) 2013, 2015 QUALCOMM Technologies Incorporated.
                          All Rights Reserved.
                 QUALCOMM Confidential and Proprietary
======================================================================*/
#include <hexagon_types.h>

static const unsigned short invLUT[128] __attribute__((aligned(128))) = {
0,16384,8192,5461,4096,3276,2730,2340,2048,1820,1638,1489,1365,1260,1170,1092,
1024,963,910,862,819,780,744,712,682,655,630,606,585,564,546,528,
512,496,481,468,455,442,431,420,409,399,390,381,372,364,356,348,
341,334,327,321,315,309,303,297,292,287,282,277,273,268,264,260,
256,252,248,244,240,237,234,230,227,224,221,218,215,212,210,207,
204,202,199,197,195,192,190,188,186,184,182,180,178,176,174,172,
170,168,167,165,163,162,160,159,157,156,154,153,151,150,148,147,
146,144,143,142,141,140,138,137,136,135,134,133,132,131,130,129
};

void epsilonFiltPerRow(
    unsigned char *restrict src,
    int stride,
    int width,
    int threshold,
    unsigned char *restrict dst
)
{
  HVX_VectorPair dSum, dInv, dProd0, dProd1;
  HVX_Vector  prv, nxt, center, p_4, p_3, p_2, p_1, p0, p1, p2, p3, p4; 
  HVX_Vector  thres, count, ZERO, ONE;
  HVX_Vector  ad0, ad1, ad2, ad3, ad4, ad5, ad6, ad7, ad8;
  HVX_Vector  x0, x1, x2, x3, x4, x5, x6, x7, x8, sInvTab0, sInvTab1, out0, out1;
  HVX_VectorPred Q0, Q1, Q2, Q3, Q4;
  HVX_Vector  *pCenter, *pX, *pout;

  int ones = 0x01010101;
  int x, t;

  thres= Q6_V_vsplat_R(Q6_R_vsplatb_R(threshold));
  ONE  = Q6_V_vsplat_R(ones);
  ZERO = Q6_V_vzero();

  sInvTab0 = ((HVX_Vector *)invLUT)[0];
  sInvTab1 = ((HVX_Vector *)invLUT)[1];
  sInvTab0 = Q6_Vh_vshuff_Vh(sInvTab0);
  sInvTab1 = Q6_Vh_vshuff_Vh(sInvTab1);

  pCenter = (HVX_Vector *)src;
  pout    = (HVX_Vector *)dst;

  for (x = 0; x < width/VLEN; x++)
  {
    dSum  = Q6_W_vcombine_VV(ZERO,ZERO);
    count = ZERO;
    center = *pCenter++;

    for (t = -4; t <= 4; t++)
    {
        pX = (HVX_Vector *)(src + t*stride);
        prv = pX[-1];
        p0  = pX[ 0];
        nxt = pX[ 1];

        ad0 = Q6_Vub_vabsdiff_VubVub(p0,center);
        Q0 = Q6_Q_vcmp_gt_VubVub(ad0,thres);
        x0 = Q6_V_vmux_QVV(Q0,ZERO,p0);
        dSum  = Q6_Wh_vmpyacc_WhVubRb(dSum,x0,ones);
        count = Q6_Vb_condacc_QnVbVb(Q0,count,ONE);

        p_1 = Q6_V_vlalign_VVI(p0, prv,1);
        p1  = Q6_V_valign_VVI( nxt,p0, 1);
        p_2 = Q6_V_vlalign_VVI(p0, prv,2);
        p2  = Q6_V_valign_VVI( nxt,p0, 2);

        ad1 = Q6_Vub_vabsdiff_VubVub(p_1,center);
        ad2 = Q6_Vub_vabsdiff_VubVub(p1, center);
        ad3 = Q6_Vub_vabsdiff_VubVub(p_2,center);
        ad4 = Q6_Vub_vabsdiff_VubVub(p2, center);

        Q1 = Q6_Q_vcmp_gt_VubVub(ad1,thres);
        Q2 = Q6_Q_vcmp_gt_VubVub(ad2,thres);
        Q3 = Q6_Q_vcmp_gt_VubVub(ad3,thres);
        Q4 = Q6_Q_vcmp_gt_VubVub(ad4,thres);

        x1 = Q6_V_vmux_QVV(Q1,ZERO,p_1);
        x2 = Q6_V_vmux_QVV(Q2,ZERO,p1 );
        x3 = Q6_V_vmux_QVV(Q3,ZERO,p_2);
        x4 = Q6_V_vmux_QVV(Q4,ZERO,p2 );

        dSum = Q6_Wh_vmpaacc_WhWubRb(dSum,Q6_W_vcombine_VV(x2,x1),ones);
        dSum = Q6_Wh_vmpaacc_WhWubRb(dSum,Q6_W_vcombine_VV(x4,x3),ones);
        count = Q6_Vb_condacc_QnVbVb(Q1,count,ONE);
        count = Q6_Vb_condacc_QnVbVb(Q2,count,ONE);
        count = Q6_Vb_condacc_QnVbVb(Q3,count,ONE);
        count = Q6_Vb_condacc_QnVbVb(Q4,count,ONE);

        p_3 = Q6_V_vlalign_VVI(p0, prv,3);
        p3  = Q6_V_valign_VVI (nxt,p0, 3);
        p_4 = Q6_V_vlalign_VVI(p0, prv,4);
        p4  = Q6_V_valign_VVI( nxt,p0, 4);

        ad5 = Q6_Vub_vabsdiff_VubVub(p_3,center);
        ad6 = Q6_Vub_vabsdiff_VubVub(p3, center);
        ad7 = Q6_Vub_vabsdiff_VubVub(p_4,center);
        ad8 = Q6_Vub_vabsdiff_VubVub(p4, center);

        Q1 = Q6_Q_vcmp_gt_VubVub(ad5,thres);
        Q2 = Q6_Q_vcmp_gt_VubVub(ad6,thres);
        Q3 = Q6_Q_vcmp_gt_VubVub(ad7,thres);
        Q4 = Q6_Q_vcmp_gt_VubVub(ad8,thres);

        x5 = Q6_V_vmux_QVV(Q1,ZERO,p_3);
        x6 = Q6_V_vmux_QVV(Q2,ZERO,p3 );
        x7 = Q6_V_vmux_QVV(Q3,ZERO,p_4);
        x8 = Q6_V_vmux_QVV(Q4,ZERO,p4 );

        dSum = Q6_Wh_vmpaacc_WhWubRb(dSum,Q6_W_vcombine_VV(x6,x5),ones);
        dSum = Q6_Wh_vmpaacc_WhWubRb(dSum,Q6_W_vcombine_VV(x8,x7),ones);
        count = Q6_Vb_condacc_QnVbVb(Q1,count,ONE);
        count = Q6_Vb_condacc_QnVbVb(Q2,count,ONE);
        count = Q6_Vb_condacc_QnVbVb(Q3,count,ONE);
        count = Q6_Vb_condacc_QnVbVb(Q4,count,ONE);
    }
    
    dInv = Q6_Wh_vlut16_VbVhR(count,sInvTab0,0);
    dInv = Q6_Wh_vlut16or_WhVbVhR(dInv,count,sInvTab0,1);
    dInv = Q6_Wh_vlut16or_WhVbVhR(dInv,count,sInvTab0,2);
    dInv = Q6_Wh_vlut16or_WhVbVhR(dInv,count,sInvTab0,3);
    dInv = Q6_Wh_vlut16or_WhVbVhR(dInv,count,sInvTab1,4);
    dInv = Q6_Wh_vlut16or_WhVbVhR(dInv,count,sInvTab1,5);

    dProd0 = Q6_Wuw_vmpy_VuhVuh(Q6_V_lo_W(dSum),Q6_V_lo_W(dInv));
    dProd1 = Q6_Wuw_vmpy_VuhVuh(Q6_V_hi_W(dSum),Q6_V_hi_W(dInv));

    out0 = Q6_Vh_vasr_VwVwR(Q6_V_hi_W(dProd0),Q6_V_lo_W(dProd0),14);
    out1 = Q6_Vh_vasr_VwVwR(Q6_V_hi_W(dProd1),Q6_V_lo_W(dProd1),14);

    *pout++ = Q6_Vb_vshuffe_VbVb(out1,out0);

    src += VLEN;
  } 
}



