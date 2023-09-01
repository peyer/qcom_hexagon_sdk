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
/*[     invsqrt                                                            ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function computes 1 / squareroot(x) using interpolation.      ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     AUG-10-2016                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/
#include "hexagon_types.h"
#include "hvx.cfg.h"

/* ======================================================================== */
/*  Intrinsic C version.                                                    */
/* ======================================================================== */
void invsqrt(
    unsigned short *restrict input,
    unsigned short *restrict sqrt_recip_shft,
    unsigned short *restrict sqrt_recip_val,
    unsigned        width
    )
{
    static unsigned short InvSqrtTABLE[64] __attribute__((aligned(VLEN))) = {
        // slope
            0,    0,    0,    0,    0,    0,    0,    0,
        29985,25366,21823,19035,16794,14961,13439,12158,
        11069,10133, 9322, 8614, 7991, 7440, 6949, 6510,
         6116, 5759, 5436, 5143, 4874, 4628, 4403, 4195,
        // value
            0,    0,    0,    0,    0,    0,    0,    0,
        32768,30893,29308,27944,26754,25705,24770,23930,
        23170,22478,21845,21262,20724,20224,19759,19325,
        18918,18536,18176,17836,17515,17210,16921,16646
    };

    int i;
    HVX_Vector sIn0, sIn1, sX0, sX1, sX0s, sX1s, sClz0, sClz1, sFrac0, sFrac1;
    HVX_Vector sIdxv, sIdxs, sSlopeL, sSlopeH, sValueL, sValueH, sT0, sT1;
    HVX_Vector sC2c, sC20, sMask7FF, sMaskFFFE;

    HVX_VectorPair dY, dSlope, dInvSqrtTab;

    HVX_Vector *iptr  = (HVX_Vector *)input;
    HVX_Vector *optrV = (HVX_Vector *)sqrt_recip_val;
    HVX_Vector *optrS = (HVX_Vector *)sqrt_recip_shft;

    sMask7FF = Q6_V_vsplat_R(0x07FF07FF);
    sC2c     = Q6_V_vsplat_R(0x002C002C);
    sC20     = Q6_V_vsplat_R(0x20202020);
    sMaskFFFE= Q6_V_vsplat_R(0xFFFEFFFE);

#if LOG2VLEN==7
    dInvSqrtTab  = Q6_Wuh_vzxt_Vub(*(HVX_Vector *)InvSqrtTABLE);
#else
    dInvSqrtTab  = Q6_Wb_vshuffoe_VbVb(((HVX_Vector *)InvSqrtTABLE)[1], ((HVX_Vector *)InvSqrtTABLE)[0]);
#endif

    for ( i=width; i>0; i-=VLEN )
    {
        sIn0 = *iptr++;
        sIn1 = *iptr++;

        sClz0 = Q6_V_vand_VV(Q6_Vuh_vcl0_Vuh(sIn0),sMaskFFFE);
        sClz1 = Q6_V_vand_VV(Q6_Vuh_vcl0_Vuh(sIn1),sMaskFFFE);

        sX0 = Q6_Vh_vasl_VhVh(sIn0, sClz0);
        sX1 = Q6_Vh_vasl_VhVh(sIn1, sClz1);

        sFrac0 = Q6_V_vand_VV(sX0,sMask7FF);
        sFrac1 = Q6_V_vand_VV(sX1,sMask7FF);

        sX0s = Q6_Vuh_vlsr_VuhR(sX0,3);
        sX1s = Q6_Vuh_vlsr_VuhR(sX1,3);

        sIdxs = Q6_Vb_vshuffo_VbVb(sX1s,sX0s);
        sIdxv = Q6_V_vor_VV(sIdxs,sC20);

        sSlopeL = Q6_Vb_vlut32_VbVbR(sIdxs,Q6_V_lo_W(dInvSqrtTab),0);
        sSlopeH = Q6_Vb_vlut32_VbVbR(sIdxs,Q6_V_hi_W(dInvSqrtTab),0);
        dSlope = Q6_Wb_vshuffoe_VbVb(sSlopeH,sSlopeL);

        sValueL = Q6_Vb_vlut32_VbVbR(sIdxv,Q6_V_lo_W(dInvSqrtTab),1);
        sValueH = Q6_Vb_vlut32_VbVbR(sIdxv,Q6_V_hi_W(dInvSqrtTab),1);
        dY = Q6_Wb_vshuffoe_VbVb(sValueH,sValueL);

        sT0 = Q6_Vh_vmpy_VhVh_s1_rnd_sat(Q6_V_lo_W(dSlope),sFrac0);
        sT1 = Q6_Vh_vmpy_VhVh_s1_rnd_sat(Q6_V_hi_W(dSlope),sFrac1);

        *optrS++ = Q6_Vh_vnavg_VhVh(sC2c,sClz0);
        *optrS++ = Q6_Vh_vnavg_VhVh(sC2c,sClz1);

        *optrV++ = Q6_Vh_vsub_VhVh(Q6_V_lo_W(dY),sT0);
        *optrV++ = Q6_Vh_vsub_VhVh(Q6_V_hi_W(dY),sT1);
    }
}

