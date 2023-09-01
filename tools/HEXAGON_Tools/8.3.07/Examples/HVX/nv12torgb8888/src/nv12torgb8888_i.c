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
/*[     color_NV12toRGB8888                                                ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function converts image from NV12 format to RGB8888 format.   ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     AUG-01-2014                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/
#include <assert.h>
#include <stdlib.h>
#include "hvx.cfg.h"
#if defined(__hexagon__)
#include "hexagon_types.h"
#endif

/* ======================================================================== */
/*  Intrinsic C version.                                                    */
/* ======================================================================== */
void
color_NV12toRGB8888_line(
    unsigned char* __restrict yuv420sp,
    unsigned char* __restrict uv420sp,
    unsigned char* __restrict rgb,
    int                       width,
    int                       stride
)
{
    int x;
    HVX_Vector sConst0xff = Q6_V_vsplat_R(0x00ff00ff);
    HVX_Vector sConst128 = Q6_V_vsplat_R(0x80808080);
    HVX_Vector sConst16 = Q6_V_vsplat_R(0x10101010);
    HVX_Vector sUv, sY0, sY1, sV_833u_400, sR, sG, sB, sGE, sGO, sRE, sRO, sBE, sBO, sIffG, sIBR, sIBR2;
    HVX_VectorPair dUvx2, dY0x2, dY1x2, dY1192a, dY1192b, dY1192c, dU2066v1634, dIffBGR, dIffBGR2;
    HVX_Vector *puv = (HVX_Vector*)uv420sp;
    HVX_Vector *py0 = (HVX_Vector*)yuv420sp;
    HVX_Vector *py1 = (HVX_Vector*)(yuv420sp+stride);
    HVX_Vector *prgb0 = (HVX_Vector*)rgb;
    HVX_Vector *prgb1 = (HVX_Vector*)(rgb+4*stride);
    int const_400_833 = ((-400 << 16) + ((-833) & 0xffff));
    int const1192 = (1192 << 16) + 1192;
    int const2066n1634 = ((2066 << 16) + 1634);
    int const10 = 10;
    int const_1 = -1;

    for (x = 0; x < width; x += VLEN)
    {
        sUv = *puv++;
        sUv = Q6_Vb_vshuff_Vb(sUv);
        dUvx2 = Q6_Wh_vsub_VubVub(sUv, sConst128);
        sV_833u_400 = Q6_Vw_vdmpy_VhRh_sat(Q6_V_lo_W(dUvx2), const_400_833);
        dU2066v1634 = Q6_Ww_vmpy_VhRh(Q6_V_lo_W(dUvx2), const2066n1634);

        sY0 = *py0++;
        sY0 = Q6_Vub_vsub_VubVub_sat(sY0, sConst16);
        dY0x2 = Q6_Wuh_vunpack_Vub(sY0);

        sY1 = *py1++;
        sY1 = Q6_Vub_vsub_VubVub_sat(sY1, sConst16);
        dY1x2 = Q6_Wuh_vunpack_Vub(sY1);

        dY1192a = Q6_Wuw_vmpy_VuhRuh(Q6_V_lo_W(dY0x2), const1192);
        sGE = Q6_Vw_vadd_VwVw(Q6_V_lo_W(dY1192a), sV_833u_400);
        sGO = Q6_Vw_vadd_VwVw(Q6_V_hi_W(dY1192a), sV_833u_400);
        sRE = Q6_Vw_vadd_VwVw(Q6_V_lo_W(dY1192a), Q6_V_lo_W(dU2066v1634));
        sRO = Q6_Vw_vadd_VwVw(Q6_V_hi_W(dY1192a), Q6_V_lo_W(dU2066v1634));
        sBE = Q6_Vw_vadd_VwVw(Q6_V_lo_W(dY1192a), Q6_V_hi_W(dU2066v1634));
        sBO = Q6_Vw_vadd_VwVw(Q6_V_hi_W(dY1192a), Q6_V_hi_W(dU2066v1634));
        sG = Q6_Vuh_vasr_VwVwR_sat(sGO, sGE, const10);
        sR = Q6_Vuh_vasr_VwVwR_sat(sRO, sRE, const10);
        sB = Q6_Vuh_vasr_VwVwR_sat(sBO, sBE, const10);
        sIffG = Q6_Vub_vsat_VhVh(sConst0xff, sG);
        sIBR = Q6_Vub_vsat_VhVh(sB, sR);
        dIffBGR = Q6_W_vshuff_VVR(sIffG, sIBR, const_1);
        *prgb0++ = Q6_V_lo_W(dIffBGR);
        *prgb0++ = Q6_V_hi_W(dIffBGR);

        dY1192c = Q6_Wuw_vmpy_VuhRuh(Q6_V_lo_W(dY1x2), const1192);
        sGE = Q6_Vw_vadd_VwVw(Q6_V_lo_W(dY1192c), sV_833u_400);
        sGO = Q6_Vw_vadd_VwVw(Q6_V_hi_W(dY1192c), sV_833u_400);
        sRE = Q6_Vw_vadd_VwVw(Q6_V_lo_W(dY1192c), Q6_V_lo_W(dU2066v1634));
        sRO = Q6_Vw_vadd_VwVw(Q6_V_hi_W(dY1192c), Q6_V_lo_W(dU2066v1634));
        sBE = Q6_Vw_vadd_VwVw(Q6_V_lo_W(dY1192c), Q6_V_hi_W(dU2066v1634));
        sBO = Q6_Vw_vadd_VwVw(Q6_V_hi_W(dY1192c), Q6_V_hi_W(dU2066v1634));
        sG = Q6_Vuh_vasr_VwVwR_sat(sGO, sGE, const10);
        sR = Q6_Vuh_vasr_VwVwR_sat(sRO, sRE, const10);
        sB = Q6_Vuh_vasr_VwVwR_sat(sBO, sBE, const10);
        sG = Q6_Vh_vmin_VhVh(sG, sConst0xff);
        sR = Q6_Vh_vmin_VhVh(sR, sConst0xff);
        sB = Q6_Vh_vmin_VhVh(sB, sConst0xff);
        sIffG = Q6_Vb_vshuffe_VbVb(sConst0xff, sG);
        sIBR2 = Q6_Vb_vshuffe_VbVb(sB, sR);
        dIffBGR2 = Q6_W_vshuff_VVR(sIffG, sIBR2, const_1);
        *prgb1++ = Q6_V_lo_W(dIffBGR2);
        *prgb1++ = Q6_V_hi_W(dIffBGR2);

        sV_833u_400 = Q6_Vw_vdmpy_VhRh_sat(Q6_V_hi_W(dUvx2), const_400_833);
        dU2066v1634 = Q6_Ww_vmpy_VhRh(Q6_V_hi_W(dUvx2), const2066n1634);

        dY1192b = Q6_Wuw_vmpy_VuhRuh(Q6_V_hi_W(dY0x2), const1192);
        sGE = Q6_Vw_vadd_VwVw(Q6_V_lo_W(dY1192b), sV_833u_400);
        sGO = Q6_Vw_vadd_VwVw(Q6_V_hi_W(dY1192b), sV_833u_400);
        sRE = Q6_Vw_vadd_VwVw(Q6_V_lo_W(dY1192b), Q6_V_lo_W(dU2066v1634));
        sRO = Q6_Vw_vadd_VwVw(Q6_V_hi_W(dY1192b), Q6_V_lo_W(dU2066v1634));
        sBE = Q6_Vw_vadd_VwVw(Q6_V_lo_W(dY1192b), Q6_V_hi_W(dU2066v1634));
        sBO = Q6_Vw_vadd_VwVw(Q6_V_hi_W(dY1192b), Q6_V_hi_W(dU2066v1634));
        sG = Q6_Vuh_vasr_VwVwR_sat(sGO, sGE, const10);
        sR = Q6_Vuh_vasr_VwVwR_sat(sRO, sRE, const10);
        sB = Q6_Vuh_vasr_VwVwR_sat(sBO, sBE, const10);
        sG = Q6_Vh_vmin_VhVh(sG, sConst0xff);
        sR = Q6_Vh_vmin_VhVh(sR, sConst0xff);
        sB = Q6_Vh_vmin_VhVh(sB, sConst0xff);
        sIffG = Q6_Vb_vshuffe_VbVb(sConst0xff, sG);
        sIBR = Q6_Vb_vshuffe_VbVb(sB, sR);
        dIffBGR = Q6_W_vshuff_VVR(sIffG, sIBR, const_1);
        *prgb0++ = Q6_V_lo_W(dIffBGR);
        *prgb0++ = Q6_V_hi_W(dIffBGR);

        dY1192a = Q6_Wuw_vmpy_VuhRuh(Q6_V_hi_W(dY1x2), const1192);
        sGE = Q6_Vw_vadd_VwVw(Q6_V_lo_W(dY1192a), sV_833u_400);
        sGO = Q6_Vw_vadd_VwVw(Q6_V_hi_W(dY1192a), sV_833u_400);
        sRE = Q6_Vw_vadd_VwVw(Q6_V_lo_W(dY1192a), Q6_V_lo_W(dU2066v1634));
        sRO = Q6_Vw_vadd_VwVw(Q6_V_hi_W(dY1192a), Q6_V_lo_W(dU2066v1634));
        sBE = Q6_Vw_vadd_VwVw(Q6_V_lo_W(dY1192a), Q6_V_hi_W(dU2066v1634));
        sBO = Q6_Vw_vadd_VwVw(Q6_V_hi_W(dY1192a), Q6_V_hi_W(dU2066v1634));
        sG = Q6_Vuh_vasr_VwVwR_sat(sGO, sGE, const10);
        sR = Q6_Vuh_vasr_VwVwR_sat(sRO, sRE, const10);
        sB = Q6_Vuh_vasr_VwVwR_sat(sBO, sBE, const10);
        sIffG = Q6_Vub_vsat_VhVh(sConst0xff, sG);
        sIBR = Q6_Vub_vsat_VhVh(sB, sR);
        dIffBGR = Q6_W_vshuff_VVR(sIffG, sIBR, const_1);
        *prgb1++ = Q6_V_lo_W(dIffBGR);
        *prgb1++ = Q6_V_hi_W(dIffBGR);
   }
}

/* ======================================================================== */
void
color_NV12toRGB8888(
unsigned char* __restrict yuv420sp,
unsigned char* __restrict uv420sp,
unsigned char* __restrict rgb,
int                       height,
int                       width,
int                       stride
)
{
    int j;
    unsigned *rgbdst = (unsigned *)rgb;

    for (j = 0; j < height; j += 2)
    {
        color_NV12toRGB8888_line(yuv420sp, uv420sp, (unsigned char *)rgbdst, width, stride);
        yuv420sp += 2 * stride;
        uv420sp += stride;
        rgbdst += 2 * stride;
    }
}
