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
/*[     Gaussian7x7u8                                                      ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function applies a 7x7 Gaussian filter to a image.            ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     OCT-23-2014                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/
#include "gaussian7x7_asm.h"
#include "hexagon_types.h"
#include "hexagon_protos.h"         // part of Q6 tools, contains intrinsics definitions

/* ======================================================================== */
/*  Intrinsic C version of Gaussian7x7u8().                                 */
/* ======================================================================== */
void Gaussian7x7u8PerRow(
    unsigned char **pSrc,
    int            width,
    unsigned char *dst,
    int VLEN
    )
{
#if (__HEXAGON_ARCH__ >= 60)
    int i;
    HEXAGON_Vect32 c0c1c0c1,c2c3c2c3,c1c1c1c1,c2c2c2c2,c3c3c3c3;

    HVX_Vector sLine0, sLine1, sLine2, sLine3, sLine4, sLine5, sLine6;
    HVX_Vector sXV0e, sXV0o, sXV1e, sXV1o;
    HVX_Vector sX_1, sX_2, sX_3, sX0, sX1, sX2, sX3, sX4; 
    HVX_Vector sX1X_1, sX2X_2, sX3X_3, sX2X0, sX3X_1, sX4X_2;
    HVX_Vector sOut02, sOut13;
    HVX_VectorPair dVsum, dSum02, dSum13;

    HVX_Vector *iptr0 = (HVX_Vector *)(pSrc[0]);
    HVX_Vector *iptr1 = (HVX_Vector *)(pSrc[1]);
    HVX_Vector *iptr2 = (HVX_Vector *)(pSrc[2]);
    HVX_Vector *iptr3 = (HVX_Vector *)(pSrc[3]);
    HVX_Vector *iptr4 = (HVX_Vector *)(pSrc[4]);
    HVX_Vector *iptr5 = (HVX_Vector *)(pSrc[5]);
    HVX_Vector *iptr6 = (HVX_Vector *)(pSrc[6]);

    HVX_Vector *optr  = (HVX_Vector *)dst;

    c0c1c0c1 = 0x01060106;
    c2c3c2c3 = 0x0F140F14;
    c1c1c1c1 = 0x06060606;
    c2c2c2c2 = 0x0F0F0F0F;
    c3c3c3c3 = 0x14141414;

    sXV0e = Q6_V_vzero();
    sXV0o = Q6_V_vzero();

    sLine0 = *iptr0++;
    sLine1 = *iptr1++;
    sLine2 = *iptr2++;
    sLine3 = *iptr3++;
    sLine4 = *iptr4++;
    sLine5 = *iptr5++;
    sLine6 = *iptr6++;

    dVsum = Q6_Wh_vadd_VubVub(sLine0,sLine6);
    dVsum = Q6_Wh_vmpaacc_WhWubRb(dVsum,Q6_W_vcombine_VV(sLine1,sLine5),c1c1c1c1);
    dVsum = Q6_Wh_vmpaacc_WhWubRb(dVsum,Q6_W_vcombine_VV(sLine2,sLine4),c2c2c2c2);
    dVsum = Q6_Wh_vmpyacc_WhVubRb(dVsum,sLine3,c3c3c3c3);


    sXV1e = Q6_V_lo_W(dVsum);
    sXV1o = Q6_V_hi_W(dVsum);

    for ( i=width; i>VLEN; i-=VLEN )
    {
        sLine0 = *iptr0++;
        sLine1 = *iptr1++;
        sLine2 = *iptr2++;
        sLine3 = *iptr3++;
        sLine4 = *iptr4++;
        sLine5 = *iptr5++;
        sLine6 = *iptr6++;

        dVsum = Q6_Wh_vadd_VubVub(sLine0,sLine6);
        dVsum = Q6_Wh_vmpaacc_WhWubRb(dVsum,Q6_W_vcombine_VV(sLine1,sLine5),c1c1c1c1);
        dVsum = Q6_Wh_vmpaacc_WhWubRb(dVsum,Q6_W_vcombine_VV(sLine2,sLine4),c2c2c2c2);
        dVsum = Q6_Wh_vmpyacc_WhVubRb(dVsum,sLine3,c3c3c3c3);

        sX_1 = Q6_V_vlalign_VVI(sXV1o,sXV0o,2);
        sX_2 = Q6_V_vlalign_VVI(sXV1e,sXV0e,2);
        sX_3 = Q6_V_vlalign_VVI(sXV1o,sXV0o,4);

        sXV0e = sXV1e;
        sXV0o = sXV1o;
        sXV1e = Q6_V_lo_W(dVsum);
        sXV1o = Q6_V_hi_W(dVsum);

        sX0 = sXV0e;
        sX1 = sXV0o;
        sX2 = Q6_V_valign_VVI(sXV1e,sXV0e,2);
        sX3 = Q6_V_valign_VVI(sXV1o,sXV0o,2);
        sX4 = Q6_V_valign_VVI(sXV1e,sXV0e,4);

        sX1X_1 = Q6_Vh_vadd_VhVh(sX1,sX_1);
        sX2X_2 = Q6_Vh_vadd_VhVh(sX2,sX_2);
        sX3X_3 = Q6_Vh_vadd_VhVh(sX3,sX_3);

        sX2X0  = Q6_Vh_vadd_VhVh(sX2,sX0);
        sX3X_1 = Q6_Vh_vadd_VhVh(sX3,sX_1);
        sX4X_2 = Q6_Vh_vadd_VhVh(sX4,sX_2);

        dSum02 = Q6_Ww_vmpa_WhRb(Q6_W_vcombine_VV(sX1X_1,sX0),c2c3c2c3);
        dSum02 = Q6_Ww_vmpaacc_WwWhRb(dSum02,Q6_W_vcombine_VV(sX3X_3,sX2X_2),c0c1c0c1);
        dSum13 = Q6_Ww_vmpa_WhRb(Q6_W_vcombine_VV(sX2X0,sX1),c2c3c2c3);
        dSum13 = Q6_Ww_vmpaacc_WwWhRb(dSum13,Q6_W_vcombine_VV(sX4X_2,sX3X_1),c0c1c0c1);

        sOut02 = Q6_Vh_vasr_VwVwR(Q6_V_hi_W(dSum02),Q6_V_lo_W(dSum02),12);
        sOut13 = Q6_Vh_vasr_VwVwR(Q6_V_hi_W(dSum13),Q6_V_lo_W(dSum13),12);

        *optr++ = Q6_Vb_vshuffe_VbVb(sOut13,sOut02);
    }


    {   
        sX_1 = Q6_V_vlalign_VVI(sXV1o,sXV0o,2);
        sX_2 = Q6_V_vlalign_VVI(sXV1e,sXV0e,2);
        sX_3 = Q6_V_vlalign_VVI(sXV1o,sXV0o,4);

        sXV0e = sXV1e;
        sXV0o = sXV1o;
        sXV1e = Q6_V_lo_W(dVsum);
        sXV1o = Q6_V_hi_W(dVsum);

        sX0 = sXV0e;
        sX1 = sXV0o;
        sX2 = Q6_V_valign_VVI(sXV1e,sXV0e,2);
        sX3 = Q6_V_valign_VVI(sXV1o,sXV0o,2);
        sX4 = Q6_V_valign_VVI(sXV1e,sXV0e,4);

        sX1X_1 = Q6_Vh_vadd_VhVh(sX1,sX_1);
        sX2X_2 = Q6_Vh_vadd_VhVh(sX2,sX_2);
        sX3X_3 = Q6_Vh_vadd_VhVh(sX3,sX_3);

        sX2X0  = Q6_Vh_vadd_VhVh(sX2,sX0);
        sX3X_1 = Q6_Vh_vadd_VhVh(sX3,sX_1);
        sX4X_2 = Q6_Vh_vadd_VhVh(sX4,sX_2);

        dSum02 = Q6_Ww_vmpa_WhRb(Q6_W_vcombine_VV(sX1X_1,sX0),c2c3c2c3);
        dSum02 = Q6_Ww_vmpaacc_WwWhRb(dSum02,Q6_W_vcombine_VV(sX3X_3,sX2X_2),c0c1c0c1);
        dSum13 = Q6_Ww_vmpa_WhRb(Q6_W_vcombine_VV(sX2X0,sX1),c2c3c2c3);
        dSum13 = Q6_Ww_vmpaacc_WwWhRb(dSum13,Q6_W_vcombine_VV(sX4X_2,sX3X_1),c0c1c0c1);

        sOut02 = Q6_Vh_vasr_VwVwR(Q6_V_hi_W(dSum02),Q6_V_lo_W(dSum02),12);
        sOut13 = Q6_Vh_vasr_VwVwR(Q6_V_hi_W(dSum13),Q6_V_lo_W(dSum13),12);

        *optr++ = Q6_Vb_vshuffe_VbVb(sOut13,sOut02);
    }
#endif
}
