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
/*[     fcvNCCPatchOnSquare8x8u8                                           ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function performs ncc using 8x8 template and 18x18 ROI        ]*/
/*[     in an image. Total search points are 11x11.                        ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     AUG-01-2014                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/

//==============================================================================
// Include Files
//==============================================================================
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#if defined(__hexagon__)
#include "hexagon_types.h"
#endif

#include "hvx.cfg.h"
#include "ncc.h"

//==============================================================================
// Macro
//==============================================================================
#define PATCH_SIZE 8
#define PATCH_CENTER (PATCH_SIZE/2)
#define PATCH_NUM_PIXELS 64
#define PATCH_NUM_PIXELS_SHIFT 6   // 2^6 = 64

#define MAX_SEARCH_RADIUS 5
#define SEARCH_SIZE (2*MAX_SEARCH_RADIUS+1)
#define AREA_SIZE (SEARCH_SIZE+PATCH_SIZE)
#define AREA_CENTER (AREA_SIZE/2)

#define DONT_USE_LOW_VARIANCE 0

#if LOG2VLEN == 6
typedef long HEXAGON_Vect512_UN __attribute__((__vector_size__(64)))__attribute__((aligned(4)));
#define vmem(A)  *((HEXAGON_Vect512*)(A))
#if defined(__hexagon__)
#define vmemu(A) *((HEXAGON_Vect512_UN*)(A))
#endif
#else
typedef long HEXAGON_Vect1024_UN __attribute__((__vector_size__(128)))__attribute__((aligned(4)));
#define vmem(A)  *((HEXAGON_Vect1024*)(A))
#if defined(__hexagon__)
#define vmemu(A) *((HEXAGON_Vect1024_UN*)(A))
#endif
#endif

//==============================================================================
// Declarations
//==============================================================================
static int ncc_initloc[VLEN/sizeof(int)] __attribute__((aligned(VLEN))) =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
#if VLEN == 128
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
#endif
};

static unsigned char ncc_ctrl03[VLEN] __attribute__((aligned(VLEN))) =
{
    0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0x07, 0x05, 0x0e, 0x0e, 0x0a, 0x0a, 0x09, 0x0b, 0x09, 0x0f,
    0x1c, 0x14, 0x14, 0x14, 0x17, 0x11, 0x13, 0x11, 0x12, 0x12, 0x16, 0x1e, 0x1d, 0x1f, 0x1d, 0x1b,
    0x28, 0x28, 0x28, 0x28, 0x2b, 0x25, 0x27, 0x25, 0x2e, 0x2e, 0x22, 0x22, 0x21, 0x23, 0x21, 0x2f,
    0x24, 0x24, 0x24, 0x34, 0x3f, 0x39, 0x3b, 0x39, 0x3a, 0x3a, 0x3e, 0x3e, 0x3d, 0x3f, 0x35, 0x33,
#if LOG2VLEN == 7
    0x40, 0x40, 0x40, 0x40, 0x43, 0x45, 0x47, 0x45, 0x4e, 0x4e, 0x4a, 0x4a, 0x49, 0x4b, 0x49, 0x4f,
    0x5c, 0x54, 0x54, 0x54, 0x57, 0x51, 0x53, 0x51, 0x52, 0x52, 0x56, 0x5e, 0x5d, 0x5f, 0x5d, 0x5b,
    0x68, 0x68, 0x68, 0x68, 0x6b, 0x65, 0x67, 0x65, 0x6e, 0x6e, 0x62, 0x62, 0x61, 0x63, 0x61, 0x6f,
    0x64, 0x64, 0x64, 0x74, 0x7f, 0x79, 0x7b, 0x79, 0x7a, 0x7a, 0x7e, 0x7e, 0x7d, 0x7f, 0x75, 0x73
#endif
};

static unsigned char ncc_ctrl47[VLEN] __attribute__((aligned(VLEN))) =
{
    0x0c, 0x04, 0x04, 0x04, 0x07, 0x01, 0x03, 0x01, 0x02, 0x02, 0x06, 0x0e, 0x0d, 0x0f, 0x0d, 0x0b,
    0x18, 0x18, 0x18, 0x18, 0x1b, 0x15, 0x17, 0x15, 0x1e, 0x1e, 0x12, 0x12, 0x11, 0x13, 0x11, 0x1f,
    0x34, 0x34, 0x34, 0x24, 0x2f, 0x29, 0x2b, 0x29, 0x2a, 0x2a, 0x2e, 0x2e, 0x2d, 0x2f, 0x25, 0x23,
    0x20, 0x20, 0x20, 0x20, 0x23, 0x25, 0x27, 0x35, 0x3e, 0x3e, 0x3a, 0x3a, 0x39, 0x3b, 0x39, 0x3f,
#if LOG2VLEN == 7
    0x4c, 0x44, 0x44, 0x44, 0x47, 0x41, 0x43, 0x41, 0x42, 0x42, 0x46, 0x4e, 0x4d, 0x4f, 0x4d, 0x4b,
    0x58, 0x58, 0x58, 0x58, 0x5b, 0x55, 0x57, 0x55, 0x5e, 0x5e, 0x52, 0x52, 0x51, 0x53, 0x51, 0x5f,
    0x74, 0x74, 0x74, 0x64, 0x6f, 0x69, 0x6b, 0x69, 0x6a, 0x6a, 0x6e, 0x6e, 0x6d, 0x6f, 0x65, 0x63,
    0x60, 0x60, 0x60, 0x60, 0x63, 0x65, 0x67, 0x75, 0x7e, 0x7e, 0x7a, 0x7a, 0x79, 0x7b, 0x79, 0x7f
#endif
};

//==============================================================================
// Functions
//==============================================================================

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void
integrate64u8(
    const unsigned char* __restrict pxls,
    unsigned short* psum,
    unsigned int* psum2
)
{
    HEXAGON_Vect sum = 0, sum2 = 0;
    HEXAGON_Vect *input = (HEXAGON_Vect *)pxls;
    assert(pxls && psum && psum2);
    int i;

    for (i = 0; i < 8; i++)
    {
        HEXAGON_Vect val = input[i];
        sum = Q6_P_vraddubacc_PP(sum, val, val);
        sum2 = Q6_P_vrmpybuacc_PP(sum2, val, val);
    }
    *psum = (HEXAGON_V64_GET_W0(sum) + HEXAGON_V64_GET_W1(sum))/2;
    *psum2 =HEXAGON_V64_GET_W0(sum2) + HEXAGON_V64_GET_W1(sum2);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void computecrossvar(
    unsigned char * ptch8x8,
    unsigned char * img,
    short stride,
    unsigned short patchSum,
    int *noms,
    int *denoms
    )
{
#if LOG2VLEN == 6
    HVX_Vector sLine0, sLine1, sLine2, sLine3, sLine4, sLine5, sLine6, sLine7, sOld03, sOld47;
    HVX_Vector sSum, sSum2, sCross0, sCross1, sPatchsumxsum, sSum0, sSum1, sVar, sSumxsum, sOldsum2, sOldsum2_03, sOldsum2_47, sCross;
    HVX_VectorPair dPatchsumxsum, dSumxsum;
    HVX_Vector sLine0_47, sLine1_47, sLine2_47, sLine3_47, sLine4_47, sLine5_47, sLine6_47, sLine7_47;
    HVX_Vector sLine0_03, sLine1_03, sLine2_03, sLine3_03, sLine4_03, sLine5_03, sLine6_03, sLine7_03, sCtrl03, sCtrl47, sZero;
    HVX_VectorPred Q0;
    int one = 0x01010101;
    int t0_03, t0_47, t1_03, t1_47, t2_03, t2_47, t3_03, t3_47, t4_03, t4_47, t5_03, t5_47, t6_03, t6_47, t7_03, t7_47;
    int i;

    t0_03 = *(int *)&ptch8x8[ 0*4];
    t0_47 = *(int *)&ptch8x8[ 1*4];
    t1_03 = *(int *)&ptch8x8[ 2*4];
    t1_47 = *(int *)&ptch8x8[ 3*4];
    t2_03 = *(int *)&ptch8x8[ 4*4];
    t2_47 = *(int *)&ptch8x8[ 5*4];
    t3_03 = *(int *)&ptch8x8[ 6*4];
    t3_47 = *(int *)&ptch8x8[ 7*4];
    t4_03 = *(int *)&ptch8x8[ 8*4];
    t4_47 = *(int *)&ptch8x8[ 9*4];
    t5_03 = *(int *)&ptch8x8[10*4];
    t5_47 = *(int *)&ptch8x8[11*4];
    t6_03 = *(int *)&ptch8x8[12*4];
    t6_47 = *(int *)&ptch8x8[13*4];
    t7_03 = *(int *)&ptch8x8[14*4];
    t7_47 = *(int *)&ptch8x8[15*4];

    sZero = Q6_V_vzero();
    sCtrl03 = *(HVX_Vector *)ncc_ctrl03;
    sCtrl47 = *(HVX_Vector *)ncc_ctrl47;
    Q0 = Q6_Q_vsetq_R(11*4);
    vmem(noms+128-VLEN/4) = sZero;

    sLine0 = vmemu(img); img += stride;
    sLine0_03 = Q6_V_vdelta_VV(sLine0, sCtrl03);
    sLine0_47 = Q6_V_vdelta_VV(sLine0, sCtrl47);
    sSum2 = Q6_Vuw_vrmpy_VubVub(sLine0_03, sLine0_03);
    sSum2 = Q6_Vuw_vrmpyacc_VuwVubVub(sSum2, sLine0_47, sLine0_47);
    sSum = Q6_Vuw_vrmpy_VubRub(sLine0_03, one);
    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine0_47, one);

    sLine1 = vmemu(img); img += stride;
    sLine1_03 = Q6_V_vdelta_VV(sLine1, sCtrl03);
    sLine1_47 = Q6_V_vdelta_VV(sLine1, sCtrl47);
    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine1_03, one);
    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine1_47, one);
    sSum0 = Q6_Vuw_vrmpy_VubVub(sLine1_03, sLine1_03);
    sSum1 = Q6_Vuw_vrmpy_VubVub(sLine1_47, sLine1_47);
    sSum0 = Q6_Vw_vadd_VwVw(sSum0, sSum1);
    sSum2 = Q6_Vw_vadd_VwVw(sSum2, sSum0);

    sLine2 = vmemu(img); img += stride;
    sLine2_03 = Q6_V_vdelta_VV(sLine2, sCtrl03);
    sLine2_47 = Q6_V_vdelta_VV(sLine2, sCtrl47);
    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine2_03, one);
    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine2_47, one);
    sSum0 = Q6_Vuw_vrmpy_VubVub(sLine2_03, sLine2_03);
    sSum1 = Q6_Vuw_vrmpy_VubVub(sLine2_47, sLine2_47);
    sSum0 = Q6_Vw_vadd_VwVw(sSum0, sSum1);
    sSum2 = Q6_Vw_vadd_VwVw(sSum2, sSum0);

    sLine3 = vmemu(img); img += stride;
    sLine3_03 = Q6_V_vdelta_VV(sLine3, sCtrl03);
    sLine3_47 = Q6_V_vdelta_VV(sLine3, sCtrl47);
    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine3_03, one);
    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine3_47, one);
    sSum0 = Q6_Vuw_vrmpy_VubVub(sLine3_03, sLine3_03);
    sSum1 = Q6_Vuw_vrmpy_VubVub(sLine3_47, sLine3_47);
    sSum0 = Q6_Vw_vadd_VwVw(sSum0, sSum1);
    sSum2 = Q6_Vw_vadd_VwVw(sSum2, sSum0);

    sLine4 = vmemu(img); img += stride;
    sLine4_03 = Q6_V_vdelta_VV(sLine4, sCtrl03);
    sLine4_47 = Q6_V_vdelta_VV(sLine4, sCtrl47);
    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine4_03, one);
    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine4_47, one);
    sSum0 = Q6_Vuw_vrmpy_VubVub(sLine4_03, sLine4_03);
    sSum1 = Q6_Vuw_vrmpy_VubVub(sLine4_47, sLine4_47);
    sSum0 = Q6_Vw_vadd_VwVw(sSum0, sSum1);
    sSum2 = Q6_Vw_vadd_VwVw(sSum2, sSum0);

    sLine5 = vmemu(img); img += stride;
    sLine5_03 = Q6_V_vdelta_VV(sLine5, sCtrl03);
    sLine5_47 = Q6_V_vdelta_VV(sLine5, sCtrl47);
    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine5_03, one);
    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine5_47, one);
    sSum0 = Q6_Vuw_vrmpy_VubVub(sLine5_03, sLine5_03);
    sSum1 = Q6_Vuw_vrmpy_VubVub(sLine5_47, sLine5_47);
    sSum0 = Q6_Vw_vadd_VwVw(sSum0, sSum1);
    sSum2 = Q6_Vw_vadd_VwVw(sSum2, sSum0);

    sLine6 = vmemu(img); img += stride;
    sLine6_03 = Q6_V_vdelta_VV(sLine6, sCtrl03);
    sLine6_47 = Q6_V_vdelta_VV(sLine6, sCtrl47);
    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine6_03, one);
    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine6_47, one);
    sSum0 = Q6_Vuw_vrmpy_VubVub(sLine6_03, sLine6_03);
    sSum1 = Q6_Vuw_vrmpy_VubVub(sLine6_47, sLine6_47);
    sSum0 = Q6_Vw_vadd_VwVw(sSum0, sSum1);
    sSum2 = Q6_Vw_vadd_VwVw(sSum2, sSum0);

    for (i = 0; i < SEARCH_SIZE; i++)
    {
        sLine7 = vmemu(img); img += stride;
        sLine7_03 = Q6_V_vdelta_VV(sLine7, sCtrl03);
        sLine7_47 = Q6_V_vdelta_VV(sLine7, sCtrl47);

        sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine7_03, one);
        sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine7_47, one);
        sSum0 = Q6_Vuw_vrmpy_VubVub(sLine7_03, sLine7_03);
        sSum1 = Q6_Vuw_vrmpy_VubVub(sLine7_47, sLine7_47);
        sSum0 = Q6_Vw_vadd_VwVw(sSum0, sSum1);
        sSum2 = Q6_Vw_vadd_VwVw(sSum2, sSum0);

        sCross0 = Q6_Vuw_vrmpy_VubRub(sLine0_03, t0_03);
        sCross1 = Q6_Vuw_vrmpy_VubRub(sLine0_47, t0_47);
        sCross0 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross0, sLine1_03, t1_03);
        sCross1 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross1, sLine1_47, t1_47);
        sCross0 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross0, sLine2_03, t2_03);
        sCross1 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross1, sLine2_47, t2_47);
        sCross0 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross0, sLine3_03, t3_03);
        sCross1 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross1, sLine3_47, t3_47);
        sCross0 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross0, sLine4_03, t4_03);
        sCross1 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross1, sLine4_47, t4_47);
        sCross0 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross0, sLine5_03, t5_03);
        sCross1 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross1, sLine5_47, t5_47);
        sCross0 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross0, sLine6_03, t6_03);
        sCross1 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross1, sLine6_47, t6_47);
        sCross0 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross0, sLine7_03, t7_03);
        sCross1 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross1, sLine7_47, t7_47);
        sCross0 = Q6_Vw_vadd_VwVw(sCross0, sCross1);

        dPatchsumxsum = Q6_Wuw_vmpy_VuhRuh(sSum, patchSum);
        sPatchsumxsum = Q6_Vw_vasr_VwR(Q6_V_lo_W(dPatchsumxsum), 6);
        dSumxsum = Q6_Wuw_vmpy_VuhVuh(sSum, sSum);
        sSumxsum = Q6_Vw_vasr_VwR(Q6_V_lo_W(dSumxsum), 6);
        sCross = Q6_Vw_vsub_VwVw(sCross0, sPatchsumxsum);
        sCross = Q6_Vw_vmax_VwVw(sCross, sZero);
        sVar = Q6_Vw_vsub_VwVw(sSum2, sSumxsum);

        sOld03 = sLine0_03;
        sOld47 = sLine0_47;
        sLine0_03 = sLine1_03;
        sLine0_47 = sLine1_47;
        sLine1_03 = sLine2_03;
        sLine1_47 = sLine2_47;
        sLine2_03 = sLine3_03;
        sLine2_47 = sLine3_47;
        sLine3_03 = sLine4_03;
        sLine3_47 = sLine4_47;
        sLine4_03 = sLine5_03;
        sLine4_47 = sLine5_47;
        sLine5_03 = sLine6_03;
        sLine5_47 = sLine6_47;
        sLine6_03 = sLine7_03;
        sLine6_47 = sLine7_47;

        sOldsum2_03 = Q6_Vuw_vrmpy_VubVub(sOld03, sOld03);
        sOldsum2_47 = Q6_Vuw_vrmpy_VubVub(sOld47, sOld47);
        sOldsum2 = Q6_Vw_vadd_VwVw(sOldsum2_03, sOldsum2_47);
        sSum2 = Q6_Vw_vsub_VwVw(sSum2, sOldsum2);

        sSum = Q6_Vw_vrmpyacc_VwVubRb(sSum, sOld03, -1);
        sSum = Q6_Vw_vrmpyacc_VwVubRb(sSum, sOld47, -1);

        sCross = Q6_V_vmux_QVV(Q0, sCross, sZero);
        sVar = Q6_V_vmux_QVV(Q0, sVar, sZero);

        vmemu(noms) = sCross; noms += SEARCH_SIZE;
        vmemu(denoms) = sVar; denoms += SEARCH_SIZE;
    }
#else
    HVX_Vector sLine0, sLine1, sLine2, sLine3, sLine4, sLine5, sLine6, sLine7, sOld03, sOld47, sOldOld03, sOldOld47;
    HVX_Vector sSum, sSum2, sCross0, sCross1, sPatchsumxsum, sSum0, sSum1, sVar, sVar1, sSumxsum, sOldsum2, sOldsum2_03, sOldsum2_47, sCross;
    HVX_VectorPair dPatchsumxsum, dSumxsum;
    HVX_Vector sLine0_47, sLine1_47, sLine2_47, sLine3_47, sLine4_47, sLine5_47, sLine6_47, sLine7_47;
    HVX_Vector sLine0_03, sLine1_03, sLine2_03, sLine3_03, sLine4_03, sLine5_03, sLine6_03, sLine7_03, sCtrl03, sCtrl47, sZero;
    HVX_Vector sLine8_03, sLine8_47, sLine8;
    HVX_VectorPred Q0, Q1;
    int one = 0x01010101;
    int t0_03, t0_47, t1_03, t1_47, t2_03, t2_47, t3_03, t3_47, t4_03, t4_47, t5_03, t5_47, t6_03, t6_47, t7_03, t7_47;
    int i;

    t0_03 = *(int *)&ptch8x8[ 0*4];
    t0_47 = *(int *)&ptch8x8[ 1*4];
    t1_03 = *(int *)&ptch8x8[ 2*4];
    t1_47 = *(int *)&ptch8x8[ 3*4];
    t2_03 = *(int *)&ptch8x8[ 4*4];
    t2_47 = *(int *)&ptch8x8[ 5*4];
    t3_03 = *(int *)&ptch8x8[ 6*4];
    t3_47 = *(int *)&ptch8x8[ 7*4];
    t4_03 = *(int *)&ptch8x8[ 8*4];
    t4_47 = *(int *)&ptch8x8[ 9*4];
    t5_03 = *(int *)&ptch8x8[10*4];
    t5_47 = *(int *)&ptch8x8[11*4];
    t6_03 = *(int *)&ptch8x8[12*4];
    t6_47 = *(int *)&ptch8x8[13*4];
    t7_03 = *(int *)&ptch8x8[14*4];
    t7_47 = *(int *)&ptch8x8[15*4];

    sZero = Q6_V_vzero();
    sCtrl03 = *(HVX_Vector *)ncc_ctrl03;
    sCtrl47 = *(HVX_Vector *)ncc_ctrl47;
    Q0 = Q6_Q_vsetq_R(11*4);
    Q1 = Q6_Q_vsetq_R(VLEN/2);
    vmem(noms+128-VLEN/4) = sZero;

    sLine0 = vmemu(img); img += stride;
    sLine0_03 = Q6_V_vdelta_VV(sLine0, sCtrl03);
    sLine0_47 = Q6_V_vdelta_VV(sLine0, sCtrl47);
    sLine1 = vmemu(img); img += stride;
    sLine1_03 = Q6_V_vdelta_VV(sLine1, sCtrl03);
    sLine1_47 = Q6_V_vdelta_VV(sLine1, sCtrl47);
    sLine0_03 = Q6_V_vmux_QVV(Q1, sLine0_03, sLine1_03);
    sLine0_47 = Q6_V_vmux_QVV(Q1, sLine0_47, sLine1_47);

    sSum2 = Q6_Vuw_vrmpy_VubVub(sLine0_03, sLine0_03);
    sSum2 = Q6_Vuw_vrmpyacc_VuwVubVub(sSum2, sLine0_47, sLine0_47);
    sSum = Q6_Vuw_vrmpy_VubRub(sLine0_03, one);
    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine0_47, one);

    sLine2 = vmemu(img); img += stride;
    sLine2_03 = Q6_V_vdelta_VV(sLine2, sCtrl03);
    sLine2_47 = Q6_V_vdelta_VV(sLine2, sCtrl47);
    sLine1_03 = Q6_V_vmux_QVV(Q1, sLine1_03, sLine2_03);
    sLine1_47 = Q6_V_vmux_QVV(Q1, sLine1_47, sLine2_47);

    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine1_03, one);
    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine1_47, one);
    sSum0 = Q6_Vuw_vrmpy_VubVub(sLine1_03, sLine1_03);
    sSum1 = Q6_Vuw_vrmpy_VubVub(sLine1_47, sLine1_47);
    sSum0 = Q6_Vw_vadd_VwVw(sSum0, sSum1);
    sSum2 = Q6_Vw_vadd_VwVw(sSum2, sSum0);

    sLine3 = vmemu(img); img += stride;
    sLine3_03 = Q6_V_vdelta_VV(sLine3, sCtrl03);
    sLine3_47 = Q6_V_vdelta_VV(sLine3, sCtrl47);
    sLine2_03 = Q6_V_vmux_QVV(Q1, sLine2_03, sLine3_03);
    sLine2_47 = Q6_V_vmux_QVV(Q1, sLine2_47, sLine3_47);

    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine2_03, one);
    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine2_47, one);
    sSum0 = Q6_Vuw_vrmpy_VubVub(sLine2_03, sLine2_03);
    sSum1 = Q6_Vuw_vrmpy_VubVub(sLine2_47, sLine2_47);
    sSum0 = Q6_Vw_vadd_VwVw(sSum0, sSum1);
    sSum2 = Q6_Vw_vadd_VwVw(sSum2, sSum0);

    sLine4 = vmemu(img); img += stride;
    sLine4_03 = Q6_V_vdelta_VV(sLine4, sCtrl03);
    sLine4_47 = Q6_V_vdelta_VV(sLine4, sCtrl47);
    sLine3_03 = Q6_V_vmux_QVV(Q1, sLine3_03, sLine4_03);
    sLine3_47 = Q6_V_vmux_QVV(Q1, sLine3_47, sLine4_47);

    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine3_03, one);
    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine3_47, one);
    sSum0 = Q6_Vuw_vrmpy_VubVub(sLine3_03, sLine3_03);
    sSum1 = Q6_Vuw_vrmpy_VubVub(sLine3_47, sLine3_47);
    sSum0 = Q6_Vw_vadd_VwVw(sSum0, sSum1);
    sSum2 = Q6_Vw_vadd_VwVw(sSum2, sSum0);

    sLine5 = vmemu(img); img += stride;
    sLine5_03 = Q6_V_vdelta_VV(sLine5, sCtrl03);
    sLine5_47 = Q6_V_vdelta_VV(sLine5, sCtrl47);
    sLine4_03 = Q6_V_vmux_QVV(Q1, sLine4_03, sLine5_03);
    sLine4_47 = Q6_V_vmux_QVV(Q1, sLine4_47, sLine5_47);

    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine4_03, one);
    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine4_47, one);
    sSum0 = Q6_Vuw_vrmpy_VubVub(sLine4_03, sLine4_03);
    sSum1 = Q6_Vuw_vrmpy_VubVub(sLine4_47, sLine4_47);
    sSum0 = Q6_Vw_vadd_VwVw(sSum0, sSum1);
    sSum2 = Q6_Vw_vadd_VwVw(sSum2, sSum0);

    sLine6 = vmemu(img); img += stride;
    sLine6_03 = Q6_V_vdelta_VV(sLine6, sCtrl03);
    sLine6_47 = Q6_V_vdelta_VV(sLine6, sCtrl47);
    sLine5_03 = Q6_V_vmux_QVV(Q1, sLine5_03, sLine6_03);
    sLine5_47 = Q6_V_vmux_QVV(Q1, sLine5_47, sLine6_47);

    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine5_03, one);
    sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine5_47, one);
    sSum0 = Q6_Vuw_vrmpy_VubVub(sLine5_03, sLine5_03);
    sSum1 = Q6_Vuw_vrmpy_VubVub(sLine5_47, sLine5_47);
    sSum0 = Q6_Vw_vadd_VwVw(sSum0, sSum1);
    sSum2 = Q6_Vw_vadd_VwVw(sSum2, sSum0);


    for (i = 0; i < SEARCH_SIZE; i+=2)
    {
        sLine7 = vmemu(img); img += stride;
        sLine7_03 = Q6_V_vdelta_VV(sLine7, sCtrl03);
        sLine7_47 = Q6_V_vdelta_VV(sLine7, sCtrl47);
        sLine6_03 = Q6_V_vmux_QVV(Q1, sLine6_03, sLine7_03);
        sLine6_47 = Q6_V_vmux_QVV(Q1, sLine6_47, sLine7_47);

        sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine6_03, one);
        sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine6_47, one);
        sSum0 = Q6_Vuw_vrmpy_VubVub(sLine6_03, sLine6_03);
        sSum1 = Q6_Vuw_vrmpy_VubVub(sLine6_47, sLine6_47);
        sSum0 = Q6_Vw_vadd_VwVw(sSum0, sSum1);
        sSum2 = Q6_Vw_vadd_VwVw(sSum2, sSum0);

        if (i != SEARCH_SIZE-1) {
            sLine8 = vmemu(img); img += stride;
            sLine8_03 = Q6_V_vdelta_VV(sLine8, sCtrl03);
            sLine8_47 = Q6_V_vdelta_VV(sLine8, sCtrl47);
            sLine7_03 = Q6_V_vmux_QVV(Q1, sLine7_03, sLine8_03);
            sLine7_47 = Q6_V_vmux_QVV(Q1, sLine7_47, sLine8_47);
        }

        sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine7_03, one);
        sSum = Q6_Vuw_vrmpyacc_VuwVubRub(sSum, sLine7_47, one);
        sSum0 = Q6_Vuw_vrmpy_VubVub(sLine7_03, sLine7_03);
        sSum1 = Q6_Vuw_vrmpy_VubVub(sLine7_47, sLine7_47);
        sSum0 = Q6_Vw_vadd_VwVw(sSum0, sSum1);
        sSum2 = Q6_Vw_vadd_VwVw(sSum2, sSum0);

        sCross0 = Q6_Vuw_vrmpy_VubRub(sLine0_03, t0_03);
        sCross1 = Q6_Vuw_vrmpy_VubRub(sLine0_47, t0_47);
        sCross0 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross0, sLine1_03, t1_03);
        sCross1 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross1, sLine1_47, t1_47);
        sCross0 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross0, sLine2_03, t2_03);
        sCross1 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross1, sLine2_47, t2_47);
        sCross0 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross0, sLine3_03, t3_03);
        sCross1 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross1, sLine3_47, t3_47);
        sCross0 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross0, sLine4_03, t4_03);
        sCross1 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross1, sLine4_47, t4_47);
        sCross0 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross0, sLine5_03, t5_03);
        sCross1 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross1, sLine5_47, t5_47);
        sCross0 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross0, sLine6_03, t6_03);
        sCross1 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross1, sLine6_47, t6_47);
        sCross0 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross0, sLine7_03, t7_03);
        sCross1 = Q6_Vuw_vrmpyacc_VuwVubRub(sCross1, sLine7_47, t7_47);
        sCross0 = Q6_Vw_vadd_VwVw(sCross0, sCross1);

        dPatchsumxsum = Q6_Wuw_vmpy_VuhRuh(sSum, patchSum);
        sPatchsumxsum = Q6_Vw_vasr_VwR(Q6_V_lo_W(dPatchsumxsum), 6);
        dSumxsum = Q6_Wuw_vmpy_VuhVuh(sSum, sSum);
        sSumxsum = Q6_Vw_vasr_VwR(Q6_V_lo_W(dSumxsum), 6);
        sCross = Q6_Vw_vsub_VwVw(sCross0, sPatchsumxsum);
        sCross = Q6_Vw_vmax_VwVw(sCross, sZero);
        sVar = Q6_Vw_vsub_VwVw(sSum2, sSumxsum);

        sOldOld03 = sLine0_03;
        sOldOld47 = sLine0_47;
        sOld03 = sLine1_03;
        sOld47 = sLine1_47;
        sLine0_03 = sLine2_03;
        sLine0_47 = sLine2_47;
        sLine1_03 = sLine3_03;
        sLine1_47 = sLine3_47;
        sLine2_03 = sLine4_03;
        sLine2_47 = sLine4_47;
        sLine3_03 = sLine5_03;
        sLine3_47 = sLine5_47;
        sLine4_03 = sLine6_03;
        sLine4_47 = sLine6_47;
        sLine5_03 = sLine7_03;
        sLine5_47 = sLine7_47;
        sLine6_03 = sLine8_03;
        sLine6_47 = sLine8_47;

        sOldsum2_03 = Q6_Vuw_vrmpy_VubVub(sOldOld03, sOldOld03);
        sOldsum2_47 = Q6_Vuw_vrmpy_VubVub(sOldOld47, sOldOld47);
        sOldsum2 = Q6_Vw_vadd_VwVw(sOldsum2_03, sOldsum2_47);
        sSum2 = Q6_Vw_vsub_VwVw(sSum2, sOldsum2);
        sOldsum2_03 = Q6_Vuw_vrmpy_VubVub(sOld03, sOld03);
        sOldsum2_47 = Q6_Vuw_vrmpy_VubVub(sOld47, sOld47);
        sOldsum2 = Q6_Vw_vadd_VwVw(sOldsum2_03, sOldsum2_47);
        sSum2 = Q6_Vw_vsub_VwVw(sSum2, sOldsum2);

        sSum = Q6_Vw_vrmpyacc_VwVubRb(sSum, sOldOld03, -1);
        sSum = Q6_Vw_vrmpyacc_VwVubRb(sSum, sOldOld47, -1);
        sSum = Q6_Vw_vrmpyacc_VwVubRb(sSum, sOld03, -1);
        sSum = Q6_Vw_vrmpyacc_VwVubRb(sSum, sOld47, -1);

        sCross1 = Q6_V_vmux_QVV(Q0, sCross, sZero);
        sVar1 = Q6_V_vmux_QVV(Q0, sVar, sZero);
        vmemu(noms) = sCross1; noms += SEARCH_SIZE;
        vmemu(denoms) = sVar1; denoms += SEARCH_SIZE;

        if (i != SEARCH_SIZE-1) {
            sCross = Q6_V_valign_VVR(sZero, sCross, VLEN/2);
            sVar = Q6_V_valign_VVR(sZero, sVar, VLEN/2);
            sCross = Q6_V_vmux_QVV(Q0, sCross, sZero);
            sVar = Q6_V_vmux_QVV(Q0, sVar, sZero);
            vmemu(noms) = sCross; noms += SEARCH_SIZE;
            vmemu(denoms) = sVar; denoms += SEARCH_SIZE;
        }
    }
#endif
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int searchbestncc(
    int * nom,
    int * denom
)
{
    int i, shft1, nn, loc;
    HVX_Vector *pnom   = (HVX_Vector *)nom;
    HVX_Vector *pdenom = (HVX_Vector *)denom;
    HVX_Vector sBestL32nn, sBestH32nn, sBestd, sBestloc, sBestn;
    HVX_Vector sN, sD, sL32nn, sH32nn, sZero, sCurloc, sLocInc;
    HVX_Vector sHnnpLeft, sMnnpLeft, sLnnpLeft;
    HVX_Vector sHnnpRight, sMnnpRight, sLnnpRight;
    HVX_VectorPred Q0, Q1, Q2, Q3;
    HVX_Vector sMask, sL32nnM, sBestL32nnM;
    int loopcount = (SEARCH_SIZE*SEARCH_SIZE + VLEN/4-1)/(VLEN/4);

    sZero = Q6_V_vzero();
    sBestd = Q6_V_vsplat_R(1);
    sBestL32nn = sZero;
    sBestH32nn = sZero;
    sBestn = sZero;
    sBestloc = sZero;
    sCurloc = *(HVX_Vector*)&ncc_initloc[0];
    sLocInc = Q6_V_vsplat_R(VLEN/4);
    sMask = Q6_V_vsplat_R(0x7fffffff);

    sCurloc = Q6_Vw_vsub_VwVw(sCurloc, sLocInc);

    shft1 = VLEN/4/2;
    for (i = 0; i < loopcount + LOG2VLEN - 2; i++)
    {
        if (i < loopcount)
        {
            sN = pnom[i];
            sD = pdenom[i];
            sL32nn = Q6_Vw_vmpyieo_VhVh(sN, sN);
            sL32nn = Q6_Vw_vmpyieacc_VwVwVuh(sL32nn, sN, sN);
            sH32nn = Q6_Vw_vmpye_VwVuh(sN, sN);
            sH32nn = Q6_Vw_vmpyoacc_VwVwVh_s1_sat_shift(sH32nn, sN, sN);
            sH32nn = Q6_Vw_vavg_VwVw(sH32nn, sZero);
            sCurloc = Q6_Vw_vadd_VwVw(sCurloc, sLocInc);
        }
        else
        {
            int shft = shft1 * 4;
            sH32nn = Q6_V_valign_VVR(sZero, sBestH32nn, shft);
            sL32nn = Q6_V_valign_VVR(sZero, sBestL32nn, shft);
            sD = Q6_V_valign_VVR(sZero, sBestd, shft);
            sCurloc = Q6_V_valign_VVR(sZero, sBestloc, shft);
            shft1 = shft1>>1;
        }

        sHnnpLeft = Q6_Vw_vmpye_VwVuh(sBestd, sH32nn);
        sHnnpLeft = Q6_Vuw_vlsr_VuwR(sHnnpLeft, 16);
        sHnnpRight = Q6_Vw_vmpye_VwVuh(sD, sBestH32nn);
        sHnnpRight = Q6_Vuw_vlsr_VuwR(sHnnpRight, 16);
        Q0 = Q6_Q_vcmp_gt_VuwVuw(sHnnpLeft, sHnnpRight);
        Q1 = Q6_Q_vcmp_eq_VwVw(sHnnpLeft, sHnnpRight);
        Q3 = Q6_Q_vcmp_eq_VwVw(sHnnpLeft, sHnnpRight);
        sMnnpLeft = Q6_Vw_vmpye_VwVuh(sBestd, sL32nn);
        sL32nnM = Q6_V_vand_VV(sL32nn, sMask);
        Q2 = Q6_Q_vcmp_eq_VwVw(sL32nn, sL32nnM);
        sMnnpLeft = Q6_Vw_vmpyoacc_VwVwVh_s1_sat_shift(sMnnpLeft, sBestd, sL32nnM);
        sMnnpLeft = Q6_Vw_condacc_QnVwVw(Q2, sMnnpLeft, sBestd);
        sMnnpLeft = Q6_Vw_vavg_VwVw(sMnnpLeft, sZero);
        sMnnpLeft = Q6_Vw_vmpyieacc_VwVwVuh(sMnnpLeft, sBestd, sH32nn);
        sMnnpRight = Q6_Vw_vmpye_VwVuh(sD, sBestL32nn);
        sBestL32nnM = Q6_V_vand_VV(sBestL32nn, sMask);
        Q2 = Q6_Q_vcmp_eq_VwVw(sBestL32nn, sBestL32nnM);
        sMnnpRight = Q6_Vw_vmpyoacc_VwVwVh_s1_sat_shift(sMnnpRight, sD, sBestL32nnM);
        sMnnpRight = Q6_Vw_condacc_QnVwVw(Q2, sMnnpRight, sD);
        sMnnpRight = Q6_Vw_vavg_VwVw(sMnnpRight, sZero);
        sMnnpRight = Q6_Vw_vmpyieacc_VwVwVuh(sMnnpRight, sD, sBestH32nn);
        Q3 = Q6_Q_vcmp_gtand_QVuwVuw(Q3, sMnnpLeft, sMnnpRight);
        Q1 = Q6_Q_vcmp_eqand_QVwVw(Q1, sMnnpLeft, sMnnpRight);
        Q0 = Q6_Q_or_QQ(Q3, Q0);
        sLnnpLeft = Q6_Vw_vmpyieo_VhVh(sBestd, sL32nn);
        sLnnpLeft = Q6_Vw_vmpyieacc_VwVwVuh(sLnnpLeft, sBestd, sL32nn);
        sLnnpRight = Q6_Vw_vmpyieo_VhVh(sD, sBestL32nn);
        sLnnpRight = Q6_Vw_vmpyieacc_VwVwVuh(sLnnpRight, sD, sBestL32nn);
        Q1 = Q6_Q_vcmp_gtand_QVuwVuw(Q1, sLnnpLeft, sLnnpRight);
        Q0 = Q6_Q_or_QQ(Q0, Q1);
        sBestL32nn = Q6_V_vmux_QVV(Q0, sL32nn, sBestL32nn);
        sBestH32nn = Q6_V_vmux_QVV(Q0, sH32nn, sBestH32nn);
        sBestd = Q6_V_vmux_QVV(Q0, sD, sBestd);
        sBestloc = Q6_V_vmux_QVV(Q0, sCurloc, sBestloc);
    }

    sBestL32nn = Q6_V_vor_VV(sBestH32nn, sBestL32nn);
    nn = Q6_R_vextract_VR(sBestL32nn, 0);
    loc = Q6_R_vextract_VR(sBestloc, 0);

    if (nn == 0)
    {
        return -3;
    }
    else
    {
        return ((loc/SEARCH_SIZE)<<16) + (loc%SEARCH_SIZE);
    }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
short
calcNCC(
    int                  denomPatch,
    int                  nom,
    int                  denom,
    int                  filterLowVariance,
    float                r
)
{
    float ncc;
    short iNCC;

    // filterLowVariance == 0 means: don't use filterLowwVariance value
    // if filterLowVarince != 0, use the value to filter low variance values
    if ((filterLowVariance != 0)  && (denom <= filterLowVariance))
        return -127;

    ncc = 128.f * (float)nom / (float)sqrt( (float)denom * (float)denomPatch ) ;

    iNCC = ncc > 0 ? (short)(ncc+r) : (short)(ncc-r);
    assert( (iNCC >= -128) && (iNCC <= 128) );

    return iNCC;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int
fcvNCCPatchOnSquare8x8u8(
    const unsigned char* __restrict ptch8x8,
    const unsigned char* __restrict img,
    unsigned short            imgW,
    unsigned short            imgH,
    unsigned short            srchX,
    unsigned short            srchY,
    unsigned short            srchW,
    int                       filterLowVariance,
    unsigned short*           bestX,
    unsigned short*           bestY,
    unsigned int*             bestNCC,
    int                       doSubPixel,
    float*                    subX,
    float*                    subY,
    unsigned char*            scratch
)
{
    unsigned int patchSum2;
    unsigned short patchSum, nSide = srchW >> 1;
    float r = .5;
    unsigned short areaExt = nSide + PATCH_CENTER;
    short xLT, yLT, bstNCC;
    int denomPatch, xBest, yBest, ind, best, bX, bY, bestFound;
    int *noms, *denoms;

    assert( ptch8x8 && img && bestX && bestY && bestNCC  &&
        imgW && imgH && srchX && srchY && srchW && scratch);
    *bestX = *bestY = 0;
    *bestNCC = 0;

    if( nSide > AREA_CENTER ) {
        return 1;
    }

    if( (srchX < areaExt) || (srchX > imgW-areaExt) || (srchY < areaExt) || (srchY > imgH-areaExt) ) {
        return 2;
    }

    noms = (int*)scratch;
    denoms = noms + (11*11+VLEN/4-1)/(VLEN/4)*(VLEN/4)+(VLEN/4);

    bX    = 0;
    bY    = 0;
    bstNCC= 0;

    xLT = (short)(srchX - areaExt);
    yLT = (short)(srchY - areaExt);

    integrate64u8( ptch8x8, &patchSum, &patchSum2);

    denomPatch = patchSum2 - ((patchSum* patchSum)>>PATCH_NUM_PIXELS_SHIFT);

    if (filterLowVariance != 0)
    {
        if (denomPatch < filterLowVariance)
        {
            *bestNCC = (unsigned int)-127;
            free(noms);
            return 4;
        }
    }

    computecrossvar((unsigned char*)ptch8x8, (unsigned char*)(img+xLT+(yLT)*imgW), imgW, patchSum, noms, denoms);

    bestFound = 0;
    best = searchbestncc(noms, denoms);
    bY = best >> 16;
    bX = best & 0xffff;

    if( best < 0 )  {
        *bestX = 0;
        *bestY = 0;
        return -best;

    } else {
        *bestX = (short)bX+srchX-5;
        *bestY = (short)bY+srchY-5;
    }

    xBest = *bestX - srchX;
    yBest = *bestY - srchY;


    ind = SEARCH_SIZE*(yBest + nSide)+xBest + nSide;
    bstNCC = calcNCC(denomPatch, noms[ind], denoms[ind], filterLowVariance, r);

    *bestNCC = bstNCC;
    if ((filterLowVariance != 0) && (bstNCC == -127))   {
        bestFound = 5;
    } else {
        bestFound = 0;
    }

    // Note: Removed sub-pixel
    assert(doSubPixel == 0);
    return bestFound;
}
