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
/*[     sigma9x9                                                           ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function performs 9x9 sigma filtering on an image block       ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     OCT-21-2014                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/
#include "hexagon_types.h"
#include "hvx.cfg.h"
/* ======================================================================== */
/*  Intrinsic C version of sigma9x9()                                       */
/* ======================================================================== */
const short invLUT[128] __attribute__((aligned(128))) = {
    0,-32768,-16384,-10923,-8192,-6554,-5461,-4681,-4096,-3641,-3277,-2979,-2731,-2521,-2341,-2185,
    -2048,-1928,-1820,-1725,-1638,-1560,-1489,-1425,-1365,-1311,-1260,-1214,-1170,-1130,-1092,-1057,
    -1024,-993,-964,-936,-910,-886,-862,-840,-819,-799,-780,-762,-745,-728,-712,-697,
    -683,-669,-655,-643,-630,-618,-607,-596,-585,-575,-565,-555,-546,-537,-529,-520,
    -512,-504,-496,-489,-482,-475,-468,-462,-455,-449,-443,-437,-431,-426,-420,-415,
    -410,-405,-400,-395,-390,-386,-381,-377,-372,-368,-364,-360,-356,-352,-349,-345,
    -341,-338,-334,-331,-328,-324,-321,-318,-315,-312,-309,-306,-303,-301,-298,-295,
    -293,-290,-287,-285,-282,-280,-278,-275,-273,-271,-269,-266,-264,-262,-260,-258,
};


/* ======================================================================== */
void sigma9x9PerRow(
    unsigned char *restrict src,
    int             stride,
    int             width,
    unsigned char   threshold,
    unsigned char *restrict dst
    )
{
    HVX_VectorPair dSum, dInv, dProd0, dProd1;
    HVX_Vector  prv, nxt, center, p_4, p_3, p_2, p_1, p0, p1, p2, p3, p4;
    HVX_Vector  thres, count, ZERO, ONE;
    HVX_Vector  ad0, ad1, ad2, ad3, ad4, ad5, ad6, ad7, ad8;
    HVX_Vector  x0, x1, x2, x3, x4, x5, x6, x7, x8, sInvTab0, sInvTab1, sInvTab2, out0, out1;
    HVX_VectorPred Q0, Q1, Q2, Q3, Q4;
    HVX_Vector  *pCenter, *pX, *pout;

    int i, t;

    thres= Q6_V_vsplat_R(Q6_R_vsplatb_R(threshold));
    ONE  = Q6_V_vsplat_R(0x01010101);
    ZERO = Q6_V_vzero();

    sInvTab0 = ((HVX_Vector *)invLUT)[0];
    sInvTab0 = Q6_Vh_vshuff_Vh(sInvTab0);
#if LOG2VLEN==6
    sInvTab1 = ((HVX_Vector *)invLUT)[1];
    sInvTab2 = ((HVX_Vector *)invLUT)[2];
    sInvTab1 = Q6_Vh_vshuff_Vh(sInvTab1);
    sInvTab2 = Q6_Vh_vshuff_Vh(sInvTab2);
#else
    sInvTab1 = sInvTab0;
    sInvTab2 = ((HVX_Vector *)invLUT)[1];
    sInvTab2 = Q6_Vh_vshuff_Vh(sInvTab2);
#endif

    pCenter = (HVX_Vector *)src;
    pout    = (HVX_Vector *)dst;

    for ( i=width; i>0; i-=VLEN )
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
            dSum  = Q6_Wh_vmpyacc_WhVubRb(dSum,x0,-1);
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

            dSum = Q6_Wh_vmpaacc_WhWubRb(dSum,Q6_W_vcombine_VV(x2,x1),-1);
            dSum = Q6_Wh_vmpaacc_WhWubRb(dSum,Q6_W_vcombine_VV(x4,x3),-1);
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

            dSum = Q6_Wh_vmpaacc_WhWubRb(dSum,Q6_W_vcombine_VV(x6,x5),-1);
            dSum = Q6_Wh_vmpaacc_WhWubRb(dSum,Q6_W_vcombine_VV(x8,x7),-1);
            count = Q6_Vb_condacc_QnVbVb(Q1,count,ONE);
            count = Q6_Vb_condacc_QnVbVb(Q2,count,ONE);
            count = Q6_Vb_condacc_QnVbVb(Q3,count,ONE);
            count = Q6_Vb_condacc_QnVbVb(Q4,count,ONE);
        }

        dInv = Q6_Wh_vlut16_VbVhR(count,sInvTab0,0);
        dInv = Q6_Wh_vlut16or_WhVbVhR(dInv,count,sInvTab0,1);
        dInv = Q6_Wh_vlut16or_WhVbVhR(dInv,count,sInvTab1,2);
        dInv = Q6_Wh_vlut16or_WhVbVhR(dInv,count,sInvTab1,3);
        dInv = Q6_Wh_vlut16or_WhVbVhR(dInv,count,sInvTab2,4);
        dInv = Q6_Wh_vlut16or_WhVbVhR(dInv,count,sInvTab2,5);

        out0 = Q6_Vh_vmpy_VhVh_s1_rnd_sat(Q6_V_lo_W(dSum),Q6_V_lo_W(dInv));
        out1 = Q6_Vh_vmpy_VhVh_s1_rnd_sat(Q6_V_hi_W(dSum),Q6_V_hi_W(dInv));

        *pout++ = Q6_Vb_vshuffe_VbVb(out1,out0);

        src += VLEN;
    }
}


/* ======================================================================== */
void sigma9x9(
    unsigned char   *src,
    int             stride,
    int             width,
    int             height,
    unsigned char   threshold,
    unsigned char   *dst
    )
{
    int  y;

    unsigned char *inp  = src + 4*stride;
    unsigned char *outp = dst + 4*stride;

    for (y = 4; y < height - 4; y++)
    {
        sigma9x9PerRow(inp, stride, width, threshold, outp);

        inp += stride;
        outp+= stride;
    }
}


