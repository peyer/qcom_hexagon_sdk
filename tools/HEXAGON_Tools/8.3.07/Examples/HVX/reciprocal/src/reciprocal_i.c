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
/*[     reciprocal                                                         ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function calculates approximation of reciprocal by using      ]*/
/*[     linear interpolation.                                              ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     AUG-04-2016                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/
#include "hexagon_types.h"
#include "hvx.cfg.h"

/* ======================================================================== */
/*  Intrinsic C version of reciprocal().                                    */
/* ======================================================================== */
unsigned short RecipTABLE[64] __attribute__((aligned(VLEN))) = {
    // slope
    31775,29905,28197,26630,25191,23865,22641,21509,
    20460,19485,18579,17734,16946,16209,15520,14873,
    14266,13695,13158,12652,12175,11724,11297,10894,
    10512,10149, 9805, 9478, 9167, 8872, 8590, 8322,
    // value
    16384,15887,15420,14979,14563,14169,13797,13443,
    13107,12787,12483,12192,11915,11650,11397,11155,
    10922,10699,10485,10280,10082, 9892, 9709, 9532,
    9362, 9198, 9039, 8886, 8738, 8594, 8456, 8322
};


void reciprocal(
    unsigned short *in,
    unsigned short *restrict recipval,
    short          *restrict recipshft,
    int             width
    )
{
    int i;
    HVX_Vector sIn0, sIn1, sX0, sX1, sNorm0, sNorm1, sFrac0, sFrac1;
    HVX_Vector sIdxv, sIdxs, sSlopeL, sSlopeH, sValueL, sValueH, sT0, sT1;
    HVX_Vector sC1c, sMask1FF, sC20, sC40;

    HVX_VectorPair dY, dSlope, dRecipTab;

    HVX_Vector *iptr  = (HVX_Vector *)in;
    HVX_Vector *optrV = (HVX_Vector *)recipval;
    HVX_Vector *optrS = (HVX_Vector *)recipshft;

    sMask1FF = Q6_V_vsplat_R(0x01FF01FF);
    sC1c     = Q6_V_vsplat_R(0x001C001C);
    sC20     = Q6_V_vsplat_R(0x20202020);
    sC40     = Q6_V_vsplat_R(0x40404040);

#if LOG2VLEN==7
    dRecipTab  = Q6_Wuh_vzxt_Vub(*(HVX_Vector *)RecipTABLE);
#else
    dRecipTab  = Q6_Wb_vshuffoe_VbVb(((HVX_Vector *)RecipTABLE)[1], ((HVX_Vector *)RecipTABLE)[0]);
#endif

    for ( i=width; i>0; i-=VLEN )
    {
        sIn0 = *iptr++;
        sIn1 = *iptr++;

        sNorm0 = Q6_Vh_vnormamt_Vh(sIn0);
        sNorm1 = Q6_Vh_vnormamt_Vh(sIn1);

        sX0 = Q6_Vh_vasl_VhVh(sIn0, sNorm0);
        sX1 = Q6_Vh_vasl_VhVh(sIn1, sNorm1);

        sFrac0 = Q6_V_vand_VV(sX0,sMask1FF);
        sFrac1 = Q6_V_vand_VV(sX1,sMask1FF);

        sIdxs = Q6_Vb_vshuffo_VbVb(sX1,sX0);
        sIdxs = Q6_Vb_vnavg_VubVub(sIdxs,sC40);
        sIdxv = Q6_V_vor_VV(sIdxs,sC20);

        sSlopeL = Q6_Vb_vlut32_VbVbR(sIdxs,Q6_V_lo_W(dRecipTab),0);
        sSlopeH = Q6_Vb_vlut32_VbVbR(sIdxs,Q6_V_hi_W(dRecipTab),0);
        dSlope = Q6_Wb_vshuffoe_VbVb(sSlopeH,sSlopeL);

        sValueL = Q6_Vb_vlut32_VbVbR(sIdxv,Q6_V_lo_W(dRecipTab),1);
        sValueH = Q6_Vb_vlut32_VbVbR(sIdxv,Q6_V_hi_W(dRecipTab),1);
        dY = Q6_Wb_vshuffoe_VbVb(sValueH,sValueL);

        sT0 = Q6_Vh_vmpy_VhVh_s1_rnd_sat(Q6_V_lo_W(dSlope),sFrac0);
        sT1 = Q6_Vh_vmpy_VhVh_s1_rnd_sat(Q6_V_hi_W(dSlope),sFrac1);

        *optrS++ = Q6_Vh_vsub_VhVh(sC1c,sNorm0);
        *optrS++ = Q6_Vh_vsub_VhVh(sC1c,sNorm1);

        *optrV++ = Q6_Vh_vsub_VhVh(Q6_V_lo_W(dY),sT0);
        *optrV++ = Q6_Vh_vsub_VhVh(Q6_V_hi_W(dY),sT1);
    }
}

