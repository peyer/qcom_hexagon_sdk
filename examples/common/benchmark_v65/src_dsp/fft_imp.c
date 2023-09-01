/**=============================================================================

@file
   fft_imp.cpp

@brief
   implementation file for gaussian7x7 RPC interface.

Copyright (c) 2017, 2019 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

//==============================================================================
// Include Files
//==============================================================================

// enable message outputs for profiling by defining _DEBUG and including HAP_farf.h
#ifndef _DEBUG
#define _DEBUG
#endif
#include "HAP_farf.h"
#undef FARF_HIGH
#define FARF_HIGH 1

#include <stdlib.h>
#include <string.h>

// profile DSP execution time (without RPC overhead) via HAP_perf api's.
#include "HAP_perf.h"

#if (__HEXAGON_ARCH__ >= 65)
#include "HAP_vtcm_mgr.h"
#else
static void* HAP_request_VTCM(int a, int b) {return 0;}
static void HAP_release_VTCM(void *a)   {}
#endif

#include "AEEStdErr.h"

// gaussian7x7 includes
#include "benchmark_asm.h"
#include "benchmark.h"

#include "dspCV_worker.h"
#include "dspCV_hvx.h"

#include "qprintf.h"

#include "hexagon_protos.h"
#include "hexagon_types.h"

#include "qurt_barrier.h"

#include "HAP_compute_res.h"

/*===========================================================================
    DEFINITIONS
===========================================================================*/
#define PROFILING_ON

#define L2FETCH(ADDR, REG)   asm("  l2fetch(%0, %1)\n"  :: "r" (ADDR), "r" (REG)    );

#define VLEN 128
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

typedef long HEXAGON_Vect1024_UN __attribute__((__vector_size__(128)))__attribute__((aligned(4)));
#define vmemu(A) *((HEXAGON_Vect1024_UN*)(A))

typedef union
{
    HVX_VectorPair VV;
    struct
    {
        HVX_Vector l;
        HVX_Vector h;
    } V;
} HVX_VP;

#define V1_0 V1__0.VV
#define V3_2 V3__2.VV
#define V5_4 V5__4.VV
#define V7_6 V7__6.VV
#define V9_8 V9__8.VV
#define V11_10 V11__10.VV
#define V13_12 V13__12.VV
#define V15_14 V15__14.VV
#define V17_16 V17__16.VV
#define V19_18 V19__18.VV
#define V21_20 V21__20.VV
#define V23_22 V23__22.VV
#define V25_24 V25__24.VV
#define V27_26 V27__26.VV
#define V29_28 V29__28.VV
#define V31_30 V31__30.VV

#define V0 V1__0.V.l
#define V1 V1__0.V.h
#define V2 V3__2.V.l
#define V3 V3__2.V.h
#define V4 V5__4.V.l
#define V5 V5__4.V.h
#define V6 V7__6.V.l
#define V7 V7__6.V.h
#define V8 V9__8.V.l
#define V9 V9__8.V.h
#define V10 V11__10.V.l
#define V11 V11__10.V.h
#define V12 V13__12.V.l
#define V13 V13__12.V.h
#define V14 V15__14.V.l
#define V15 V15__14.V.h
#define V16 V17__16.V.l
#define V17 V17__16.V.h
#define V18 V19__18.V.l
#define V19 V19__18.V.h
#define V20 V21__20.V.l
#define V21 V21__20.V.h
#define V22 V23__22.V.l
#define V23 V23__22.V.h
#define V24 V25__24.V.l
#define V25 V25__24.V.h
#define V26 V27__26.V.l
#define V27 V27__26.V.h
#define V28 V29__28.V.l
#define V29 V29__28.V.h
#define V30 V31__30.V.l
#define V31 V31__30.V.h

#define DECLARE_HVX_REGISTERS \
        HVX_VP V1__0, V3__2, V5__4, V7__6, V9__8, V11__10, V13__12, V15__14; \
        HVX_VP V17__16, V19__18, V21__20, V23__22, V25__24, V27__26, V29__28, V31__30;


#define V_CPLX_MULT_16_16(V0inR, V0inI, V1inR, V1inI, VoutR, VoutI, Vscratch) \
    VoutR = Q6_Vh_vmpy_VhVh_s1_rnd_sat(V0inR, V1inR); \
    Vscratch = Q6_Vh_vmpy_VhVh_s1_rnd_sat(V0inI, V1inI); \
    VoutR = Q6_Vh_vsub_VhVh_sat(VoutR, Vscratch); \
    VoutI = Q6_Vh_vmpy_VhVh_s1_rnd_sat(V0inR, V1inI); \
    Vscratch = Q6_Vh_vmpy_VhVh_s1_rnd_sat(V0inI, V1inR); \
    VoutI = Q6_Vh_vadd_VhVh_sat(VoutI, Vscratch);

#define V_CPLX_MULT_32_16(V0inR, V0inI, V1inR, V1inI, VoutR, VoutI, Vscratch) \
    VoutR = Q6_Vw_vmpyo_VwVh_s1_rnd_sat(V0inR, V1inR); \
    Vscratch = Q6_Vw_vmpyo_VwVh_s1_rnd_sat(V0inI, V1inI); \
    VoutR = Q6_Vw_vsub_VwVw_sat(VoutR, Vscratch); \
    VoutI = Q6_Vw_vmpyo_VwVh_s1_rnd_sat(V0inR, V1inI); \
    Vscratch = Q6_Vw_vmpyo_VwVh_s1_rnd_sat(V0inI, V1inR); \
    VoutI = Q6_Vw_vadd_VwVw_sat(VoutI, Vscratch);

    // version not requiring scratch but corrupting inputs
#define V_CPLX_MULT_32_16_2(V0inR, V0inI, V1inR, V1inI, VoutR, VoutI) \
    VoutR = Q6_Vw_vmpyo_VwVh_s1_rnd_sat(V0inR, V1inR); \
    VoutI = Q6_Vw_vmpyo_VwVh_s1_rnd_sat(V0inR, V1inI); \
    V0inR = Q6_Vw_vmpyo_VwVh_s1_rnd_sat(V0inI, V1inI); \
    VoutR = Q6_Vw_vsub_VwVw_sat(VoutR, V0inR); \
    V0inR = Q6_Vw_vmpyo_VwVh_s1_rnd_sat(V0inI, V1inR); \
    VoutI = Q6_Vw_vadd_VwVw_sat(VoutI, V0inR);


/*===========================================================================
    DECLARATIONS
===========================================================================*/

// length 1024 Twiddle factors - 16-bit.
#define FFTSIZE 1024
// Wb = W[br(i)]. Split into Real and Imaginary
// Wa = W[br[(2*i+1)]. Split into Real and Imaginary
// Wc = W[br(i)+br(2*i+1)]. Split into Real and Imaginary
static const int16_t Twiddles[FFTSIZE * 3 / 2] __attribute__ ((aligned (128))) =
{
    // Re[Wb] (0..255)
    0x7fff, 0x0000, 0x5a82, 0xa57e, 0x7642, 0xcf05, 0x30fc, 0x89bf, 0x7d8a, 0xe708, 0x471d, 0x9593, 0x6a6e, 0xb8e4, 0x18f9, 0x8276, 0x7f62, 0xf375, 0x5134, 0x9d0e, 0x70e3, 0xc3aa, 0x2528, 0x8583, 0x7a7d, 0xdad8, 0x3c57, 0x8f1e, 0x62f2, 0xaecd, 0x0c8c, 0x809e, 0x7fd9, 0xf9b9, 0x55f6, 0xa129, 0x73b6, 0xc946, 0x2b1f, 0x877c, 0x7c2a, 0xe0e7, 0x41ce, 0x9236, 0x66d0, 0xb3c1, 0x12c8, 0x8163, 0x7e9d, 0xed38, 0x4c40, 0x9931, 0x6dca, 0xbe32, 0x1f1a, 0x83d7, 0x7885, 0xd4e1, 0x36ba, 0x8c4b, 0x5ed7, 0xaa0b, 0x0648, 0x8028, 0x7ff6, 0xfcdc, 0x5843, 0xa34c, 0x7505, 0xcc22, 0x2e11, 0x8894, 0x7ce4, 0xe3f5, 0x447b, 0x93dc, 0x68a7, 0xb64c, 0x15e2, 0x81e3, 0x7f0a, 0xf055, 0x4ec0, 0x9b18, 0x6f5f, 0xc0e9, 0x2224, 0x84a3, 0x798a, 0xd7da, 0x398d, 0x8dab, 0x60ec, 0xac65, 0x096b, 0x8059, 0x7fa7, 0xf696, 0x539b, 0x9f14, 0x7255, 0xc674, 0x2827, 0x8676, 0x7b5d, 0xdddd, 0x3f17, 0x90a1, 0x64e9, 0xb141, 0x0fab, 0x80f7, 0x7e1e, 0xea1e, 0x49b4, 0x975a, 0x6c24, 0xbb86, 0x1c0c, 0x831d, 0x776c, 0xd1ef, 0x33df, 0x8afc, 0x5cb4, 0xa7be, 0x0324, 0x800a, 0x7ffe, 0xfe6e, 0x5964, 0xa463, 0x75a6, 0xcd92, 0x2f87, 0x8927, 0x7d3a, 0xe57e, 0x45cd, 0x94b6, 0x698c, 0xb797, 0x176e, 0x822a, 0x7f38, 0xf1e5, 0x4ffb, 0x9c11, 0x7023, 0xc248, 0x23a7, 0x8511, 0x7a06, 0xd958, 0x3af3, 0x8e62, 0x61f1, 0xad97, 0x0afb, 0x8079, 0x7fc2, 0xf827, 0x54ca, 0xa01d, 0x7308, 0xc7dc, 0x29a4, 0x86f7, 0x7bc6, 0xdf61, 0x4074, 0x916a, 0x65de, 0xb27f, 0x113a, 0x812b, 0x7e60, 0xebab, 0x4afb, 0x9843, 0x6cf9, 0xbcdb, 0x1d93, 0x8377, 0x77fb, 0xd368, 0x354e, 0x8ba1, 0x5dc8, 0xa8e3, 0x04b6, 0x8017, 0x7fea, 0xfb4a, 0x571e, 0xa239, 0x7460, 0xcab3, 0x2c99, 0x8806, 0x7c89, 0xe26d, 0x4326, 0x9307, 0x67bd, 0xb505, 0x1455, 0x81a1, 0x7ed6, 0xeec7, 0x4d81, 0x9a23, 0x6e97, 0xbf8d, 0x209f, 0x843b, 0x790a, 0xd65d, 0x3825, 0x8cf9, 0x5fe4, 0xab36, 0x07d9, 0x803e, 0x7f87, 0xf505, 0x5269, 0x9e0f, 0x719e, 0xc50e, 0x26a8, 0x85fb, 0x7aef, 0xdc5a, 0x3db8, 0x8fdd, 0x63ef, 0xb005, 0x0e1c, 0x80c8, 0x7dd6, 0xe893, 0x486a, 0x9674, 0x6b4b, 0xba33, 0x1a83, 0x82c7, 0x76d9, 0xd079, 0x326e, 0x8a5b, 0x5b9d, 0xa69c, 0x0192, 0x8003,
    // Im[Wb] (256..511)
    0x0000, 0x8000, 0xa57e, 0xa57e, 0xcf05, 0x89bf, 0x89bf, 0xcf05, 0xe708, 0x8276, 0x9593, 0xb8e4, 0xb8e4, 0x9593, 0x8276, 0xe708, 0xf375, 0x809e, 0x9d0e, 0xaecd, 0xc3aa, 0x8f1e, 0x8583, 0xdad8, 0xdad8, 0x8583, 0x8f1e, 0xc3aa, 0xaecd, 0x9d0e, 0x809e, 0xf375, 0xf9b9, 0x8028, 0xa129, 0xaa0b, 0xc946, 0x8c4b, 0x877c, 0xd4e1, 0xe0e7, 0x83d7, 0x9236, 0xbe32, 0xb3c1, 0x9931, 0x8163, 0xed38, 0xed38, 0x8163, 0x9931, 0xb3c1, 0xbe32, 0x9236, 0x83d7, 0xe0e7, 0xd4e1, 0x877c, 0x8c4b, 0xc946, 0xaa0b, 0xa129, 0x8028, 0xf9b9, 0xfcdc, 0x800a, 0xa34c, 0xa7be, 0xcc22, 0x8afc, 0x8894, 0xd1ef, 0xe3f5, 0x831d, 0x93dc, 0xbb86, 0xb64c, 0x975a, 0x81e3, 0xea1e, 0xf055, 0x80f7, 0x9b18, 0xb141, 0xc0e9, 0x90a1, 0x84a3, 0xdddd, 0xd7da, 0x8676, 0x8dab, 0xc674, 0xac65, 0x9f14, 0x8059, 0xf696, 0xf696, 0x8059, 0x9f14, 0xac65, 0xc674, 0x8dab, 0x8676, 0xd7da, 0xdddd, 0x84a3, 0x90a1, 0xc0e9, 0xb141, 0x9b18, 0x80f7, 0xf055, 0xea1e, 0x81e3, 0x975a, 0xb64c, 0xbb86, 0x93dc, 0x831d, 0xe3f5, 0xd1ef, 0x8894, 0x8afc, 0xcc22, 0xa7be, 0xa34c, 0x800a, 0xfcdc, 0xfe6e, 0x8003, 0xa463, 0xa69c, 0xcd92, 0x8a5b, 0x8927, 0xd079, 0xe57e, 0x82c7, 0x94b6, 0xba33, 0xb797, 0x9674, 0x822a, 0xe893, 0xf1e5, 0x80c8, 0x9c11, 0xb005, 0xc248, 0x8fdd, 0x8511, 0xdc5a, 0xd958, 0x85fb, 0x8e62, 0xc50e, 0xad97, 0x9e0f, 0x8079, 0xf505, 0xf827, 0x803e, 0xa01d, 0xab36, 0xc7dc, 0x8cf9, 0x86f7, 0xd65d, 0xdf61, 0x843b, 0x916a, 0xbf8d, 0xb27f, 0x9a23, 0x812b, 0xeec7, 0xebab, 0x81a1, 0x9843, 0xb505, 0xbcdb, 0x9307, 0x8377, 0xe26d, 0xd368, 0x8806, 0x8ba1, 0xcab3, 0xa8e3, 0xa239, 0x8017, 0xfb4a, 0xfb4a, 0x8017, 0xa239, 0xa8e3, 0xcab3, 0x8ba1, 0x8806, 0xd368, 0xe26d, 0x8377, 0x9307, 0xbcdb, 0xb505, 0x9843, 0x81a1, 0xebab, 0xeec7, 0x812b, 0x9a23, 0xb27f, 0xbf8d, 0x916a, 0x843b, 0xdf61, 0xd65d, 0x86f7, 0x8cf9, 0xc7dc, 0xab36, 0xa01d, 0x803e, 0xf827, 0xf505, 0x8079, 0x9e0f, 0xad97, 0xc50e, 0x8e62, 0x85fb, 0xd958, 0xdc5a, 0x8511, 0x8fdd, 0xc248, 0xb005, 0x9c11, 0x80c8, 0xf1e5, 0xe893, 0x822a, 0x9674, 0xb797, 0xba33, 0x94b6, 0x82c7, 0xe57e, 0xd079, 0x8927, 0x8a5b, 0xcd92, 0xa69c, 0xa463, 0x8003, 0xfe6e,
    // Re[Wa] (512..767)
    0x0000, 0xa57e, 0xcf05, 0x89bf, 0xe708, 0x9593, 0xb8e4, 0x8276, 0xf375, 0x9d0e, 0xc3aa, 0x8583, 0xdad8, 0x8f1e, 0xaecd, 0x809e, 0xf9b9, 0xa129, 0xc946, 0x877c, 0xe0e7, 0x9236, 0xb3c1, 0x8163, 0xed38, 0x9931, 0xbe32, 0x83d7, 0xd4e1, 0x8c4b, 0xaa0b, 0x8028, 0xfcdc, 0xa34c, 0xcc22, 0x8894, 0xe3f5, 0x93dc, 0xb64c, 0x81e3, 0xf055, 0x9b18, 0xc0e9, 0x84a3, 0xd7da, 0x8dab, 0xac65, 0x8059, 0xf696, 0x9f14, 0xc674, 0x8676, 0xdddd, 0x90a1, 0xb141, 0x80f7, 0xea1e, 0x975a, 0xbb86, 0x831d, 0xd1ef, 0x8afc, 0xa7be, 0x800a, 0xfe6e, 0xa463, 0xcd92, 0x8927, 0xe57e, 0x94b6, 0xb797, 0x822a, 0xf1e5, 0x9c11, 0xc248, 0x8511, 0xd958, 0x8e62, 0xad97, 0x8079, 0xf827, 0xa01d, 0xc7dc, 0x86f7, 0xdf61, 0x916a, 0xb27f, 0x812b, 0xebab, 0x9843, 0xbcdb, 0x8377, 0xd368, 0x8ba1, 0xa8e3, 0x8017, 0xfb4a, 0xa239, 0xcab3, 0x8806, 0xe26d, 0x9307, 0xb505, 0x81a1, 0xeec7, 0x9a23, 0xbf8d, 0x843b, 0xd65d, 0x8cf9, 0xab36, 0x803e, 0xf505, 0x9e0f, 0xc50e, 0x85fb, 0xdc5a, 0x8fdd, 0xb005, 0x80c8, 0xe893, 0x9674, 0xba33, 0x82c7, 0xd079, 0x8a5b, 0xa69c, 0x8003, 0xff37, 0xa4f0, 0xce4b, 0x8972, 0xe643, 0x9524, 0xb83d, 0x8250, 0xf2ad, 0x9c8f, 0xc2f9, 0x854a, 0xda18, 0x8ebf, 0xae32, 0x808b, 0xf8f0, 0xa0a2, 0xc891, 0x8739, 0xe024, 0x91d0, 0xb31f, 0x8146, 0xec72, 0x98ba, 0xbd86, 0x83a6, 0xd424, 0x8bf5, 0xa976, 0x801f, 0xfc13, 0xa2c2, 0xcb6a, 0x884c, 0xe331, 0x9371, 0xb5a8, 0x81c1, 0xef8e, 0x9a9d, 0xc03b, 0x846e, 0xd71b, 0x8d51, 0xabcd, 0x804b, 0xf5cd, 0x9e91, 0xc5c0, 0x8638, 0xdd1b, 0x903f, 0xb0a2, 0x80df, 0xe958, 0x96e7, 0xbadc, 0x82f1, 0xd134, 0x8aab, 0xa72c, 0x8006, 0xfda5, 0xa3d7, 0xccda, 0x88dd, 0xe4b9, 0x9448, 0xb6f1, 0x8206, 0xf11d, 0x9b94, 0xc198, 0x84da, 0xd899, 0x8e06, 0xacfe, 0x8069, 0xf75e, 0x9f98, 0xc728, 0x86b6, 0xde9f, 0x9105, 0xb1df, 0x8110, 0xeae5, 0x97ce, 0xbc30, 0x8349, 0xd2ab, 0x8b4e, 0xa850, 0x8010, 0xfa81, 0xa1b0, 0xc9fc, 0x87c0, 0xe1aa, 0x929e, 0xb462, 0x8181, 0xedff, 0x99a9, 0xbedf, 0x8408, 0xd59f, 0x8ca1, 0xaaa0, 0x8032, 0xf43d, 0x9d8f, 0xc45b, 0x85be, 0xdb99, 0x8f7d, 0xaf69, 0x80b3, 0xe7cd, 0x9603, 0xb98b, 0x829e, 0xcfbf, 0x8a0c, 0xa60d, 0x8001,
    // Im[Wa] (768..1023)
    0x8000, 0xa57e, 0x89bf, 0xcf05, 0x8276, 0xb8e4, 0x9593, 0xe708, 0x809e, 0xaecd, 0x8f1e, 0xdad8, 0x8583, 0xc3aa, 0x9d0e, 0xf375, 0x8028, 0xaa0b, 0x8c4b, 0xd4e1, 0x83d7, 0xbe32, 0x9931, 0xed38, 0x8163, 0xb3c1, 0x9236, 0xe0e7, 0x877c, 0xc946, 0xa129, 0xf9b9, 0x800a, 0xa7be, 0x8afc, 0xd1ef, 0x831d, 0xbb86, 0x975a, 0xea1e, 0x80f7, 0xb141, 0x90a1, 0xdddd, 0x8676, 0xc674, 0x9f14, 0xf696, 0x8059, 0xac65, 0x8dab, 0xd7da, 0x84a3, 0xc0e9, 0x9b18, 0xf055, 0x81e3, 0xb64c, 0x93dc, 0xe3f5, 0x8894, 0xcc22, 0xa34c, 0xfcdc, 0x8003, 0xa69c, 0x8a5b, 0xd079, 0x82c7, 0xba33, 0x9674, 0xe893, 0x80c8, 0xb005, 0x8fdd, 0xdc5a, 0x85fb, 0xc50e, 0x9e0f, 0xf505, 0x803e, 0xab36, 0x8cf9, 0xd65d, 0x843b, 0xbf8d, 0x9a23, 0xeec7, 0x81a1, 0xb505, 0x9307, 0xe26d, 0x8806, 0xcab3, 0xa239, 0xfb4a, 0x8017, 0xa8e3, 0x8ba1, 0xd368, 0x8377, 0xbcdb, 0x9843, 0xebab, 0x812b, 0xb27f, 0x916a, 0xdf61, 0x86f7, 0xc7dc, 0xa01d, 0xf827, 0x8079, 0xad97, 0x8e62, 0xd958, 0x8511, 0xc248, 0x9c11, 0xf1e5, 0x822a, 0xb797, 0x94b6, 0xe57e, 0x8927, 0xcd92, 0xa463, 0xfe6e, 0x8001, 0xa60d, 0x8a0c, 0xcfbf, 0x829e, 0xb98b, 0x9603, 0xe7cd, 0x80b3, 0xaf69, 0x8f7d, 0xdb99, 0x85be, 0xc45b, 0x9d8f, 0xf43d, 0x8032, 0xaaa0, 0x8ca1, 0xd59f, 0x8408, 0xbedf, 0x99a9, 0xedff, 0x8181, 0xb462, 0x929e, 0xe1aa, 0x87c0, 0xc9fc, 0xa1b0, 0xfa81, 0x8010, 0xa850, 0x8b4e, 0xd2ab, 0x8349, 0xbc30, 0x97ce, 0xeae5, 0x8110, 0xb1df, 0x9105, 0xde9f, 0x86b6, 0xc728, 0x9f98, 0xf75e, 0x8069, 0xacfe, 0x8e06, 0xd899, 0x84da, 0xc198, 0x9b94, 0xf11d, 0x8206, 0xb6f1, 0x9448, 0xe4b9, 0x88dd, 0xccda, 0xa3d7, 0xfda5, 0x8006, 0xa72c, 0x8aab, 0xd134, 0x82f1, 0xbadc, 0x96e7, 0xe958, 0x80df, 0xb0a2, 0x903f, 0xdd1b, 0x8638, 0xc5c0, 0x9e91, 0xf5cd, 0x804b, 0xabcd, 0x8d51, 0xd71b, 0x846e, 0xc03b, 0x9a9d, 0xef8e, 0x81c1, 0xb5a8, 0x9371, 0xe331, 0x884c, 0xcb6a, 0xa2c2, 0xfc13, 0x801f, 0xa976, 0x8bf5, 0xd424, 0x83a6, 0xbd86, 0x98ba, 0xec72, 0x8146, 0xb31f, 0x91d0, 0xe024, 0x8739, 0xc891, 0xa0a2, 0xf8f0, 0x808b, 0xae32, 0x8ebf, 0xda18, 0x854a, 0xc2f9, 0x9c8f, 0xf2ad, 0x8250, 0xb83d, 0x9524, 0xe643, 0x8972, 0xce4b, 0xa4f0, 0xff37,
    // Re[Wc] (1024..1279)
    0x0000, 0xa57e, 0x89bf, 0x30fc, 0xb8e4, 0xe708, 0x8276, 0x6a6e, 0xdad8, 0xc3aa, 0x809e, 0x5134, 0x9d0e, 0x0c8c, 0x8f1e, 0x7a7d, 0xed38, 0xb3c1, 0x83d7, 0x41ce, 0xaa0b, 0xf9b9, 0x877c, 0x73b6, 0xc946, 0xd4e1, 0x8028, 0x5ed7, 0x9236, 0x1f1a, 0x9931, 0x7e9d, 0xf696, 0xac65, 0x8676, 0x398d, 0xb141, 0xf055, 0x84a3, 0x6f5f, 0xd1ef, 0xcc22, 0x800a, 0x5843, 0x975a, 0x15e2, 0x93dc, 0x7ce4, 0xe3f5, 0xbb86, 0x81e3, 0x49b4, 0xa34c, 0x0324, 0x8afc, 0x776c, 0xc0e9, 0xdddd, 0x80f7, 0x64e9, 0x8dab, 0x2827, 0x9f14, 0x7fa7, 0xfb4a, 0xa8e3, 0x8806, 0x354e, 0xb505, 0xebab, 0x8377, 0x6cf9, 0xd65d, 0xc7dc, 0x803e, 0x54ca, 0x9a23, 0x113a, 0x916a, 0x7bc6, 0xe893, 0xb797, 0x82c7, 0x45cd, 0xa69c, 0xfe6e, 0x8927, 0x75a6, 0xc50e, 0xd958, 0x8079, 0x61f1, 0x8fdd, 0x23a7, 0x9c11, 0x7f38, 0xf1e5, 0xb005, 0x8511, 0x3db8, 0xad97, 0xf505, 0x85fb, 0x719e, 0xcd92, 0xd079, 0x8003, 0x5b9d, 0x94b6, 0x1a83, 0x9674, 0x7dd6, 0xdf61, 0xbf8d, 0x812b, 0x4d81, 0xa01d, 0x07d9, 0x8cf9, 0x790a, 0xbcdb, 0xe26d, 0x81a1, 0x67bd, 0x8ba1, 0x2c99, 0xa239, 0x7fea, 0xfda5, 0xa72c, 0x88dd, 0x3327, 0xb6f1, 0xe958, 0x82f1, 0x6bb8, 0xd899, 0xc5c0, 0x8069, 0x5303, 0x9b94, 0x0ee4, 0x903f, 0x7b27, 0xeae5, 0xb5a8, 0x8349, 0x43d1, 0xa850, 0xfc13, 0x884c, 0x74b3, 0xc728, 0xd71b, 0x804b, 0x6068, 0x9105, 0x2162, 0x9a9d, 0x7ef0, 0xf43d, 0xae32, 0x85be, 0x3ba5, 0xaf69, 0xf2ad, 0x854a, 0x7083, 0xcfbf, 0xce4b, 0x8001, 0x59f4, 0x9603, 0x1833, 0x9524, 0x7d63, 0xe1aa, 0xbd86, 0x8181, 0x4b9e, 0xa1b0, 0x057f, 0x8bf5, 0x7840, 0xbedf, 0xe024, 0x8146, 0x6657, 0x8ca1, 0x2a62, 0xa0a2, 0x7fce, 0xf8f0, 0xaaa0, 0x8739, 0x3770, 0xb31f, 0xedff, 0x8408, 0x6e31, 0xd424, 0xc9fc, 0x801f, 0x568a, 0x98ba, 0x138f, 0x929e, 0x7c5a, 0xe643, 0xb98b, 0x8250, 0x47c4, 0xa4f0, 0x00c9, 0x8a0c, 0x768e, 0xc2f9, 0xdb99, 0x80b3, 0x6371, 0x8ebf, 0x25e8, 0x9d8f, 0x7f75, 0xef8e, 0xb1df, 0x846e, 0x3fc6, 0xabcd, 0xf75e, 0x86b6, 0x72af, 0xcb6a, 0xd2ab, 0x8010, 0x5d3e, 0x9371, 0x1cd0, 0x97ce, 0x7e3f, 0xdd1b, 0xc198, 0x80df, 0x4f5e, 0x9e91, 0x0a33, 0x8e06, 0x79c9, 0xbadc, 0xe4b9, 0x8206, 0x691a, 0x8aab, 0x2ecc, 0xa3d7, 0x7ffa,
    // Im[Wc] (1280..1535)
    0x8000, 0x5a82, 0xcf05, 0x7642, 0x9593, 0x7d8a, 0x18f9, 0x471d, 0x8583, 0x70e3, 0xf375, 0x62f2, 0xaecd, 0x7f62, 0x3c57, 0x2528, 0x8163, 0x66d0, 0xe0e7, 0x6dca, 0xa129, 0x7fd9, 0x2b1f, 0x36ba, 0x8c4b, 0x7885, 0x0648, 0x55f6, 0xbe32, 0x7c2a, 0x4c40, 0x12c8, 0x8059, 0x60ec, 0xd7da, 0x7255, 0x9b18, 0x7f0a, 0x2224, 0x3f17, 0x8894, 0x7505, 0xfcdc, 0x5cb4, 0xb64c, 0x7e1e, 0x447b, 0x1c0c, 0x831d, 0x6c24, 0xea1e, 0x68a7, 0xa7be, 0x7ff6, 0x33df, 0x2e11, 0x90a1, 0x7b5d, 0x0fab, 0x4ec0, 0xc674, 0x798a, 0x539b, 0x096b, 0x8017, 0x5dc8, 0xd368, 0x7460, 0x9843, 0x7e60, 0x1d93, 0x4326, 0x86f7, 0x7308, 0xf827, 0x5fe4, 0xb27f, 0x7ed6, 0x4074, 0x209f, 0x822a, 0x698c, 0xe57e, 0x6b4b, 0xa463, 0x7ffe, 0x2f87, 0x326e, 0x8e62, 0x7a06, 0x0afb, 0x5269, 0xc248, 0x7aef, 0x4ffb, 0x0e1c, 0x80c8, 0x63ef, 0xdc5a, 0x7023, 0x9e0f, 0x7f87, 0x26a8, 0x3af3, 0x8a5b, 0x76d9, 0x0192, 0x5964, 0xba33, 0x7d3a, 0x486a, 0x176e, 0x843b, 0x6e97, 0xeec7, 0x65de, 0xab36, 0x7fc2, 0x3825, 0x29a4, 0x9307, 0x7c89, 0x1455, 0x4afb, 0xcab3, 0x77fb, 0x571e, 0x04b6, 0x8006, 0x5c29, 0xd134, 0x7556, 0x96e7, 0x7dfb, 0x1b47, 0x4524, 0x8638, 0x71fa, 0xf5cd, 0x616f, 0xb0a2, 0x7f22, 0x3e68, 0x22e5, 0x81c1, 0x6832, 0xe331, 0x6c8f, 0xa2c2, 0x7ff1, 0x2d55, 0x3497, 0x8d51, 0x794a, 0x08a2, 0x5433, 0xc03b, 0x7b92, 0x4e21, 0x1073, 0x808b, 0x6272, 0xda18, 0x7141, 0x9c8f, 0x7f4e, 0x2467, 0x3d08, 0x8972, 0x75f4, 0xff37, 0x5b10, 0xb83d, 0x7db1, 0x4675, 0x19be, 0x83a6, 0x6d62, 0xec72, 0x6747, 0xa976, 0x7fe2, 0x3604, 0x2bdc, 0x91d0, 0x7bf9, 0x1201, 0x4ce1, 0xc891, 0x78c8, 0x5560, 0x0711, 0x8032, 0x5f5e, 0xd59f, 0x735f, 0x99a9, 0x7eba, 0x1fdd, 0x4121, 0x87c0, 0x740b, 0xfa81, 0x5e50, 0xb462, 0x7e7f, 0x427a, 0x1e57, 0x829e, 0x6add, 0xe7cd, 0x69fd, 0xa60d, 0x7fff, 0x31b5, 0x3042, 0x8f7d, 0x7ab7, 0x0d54, 0x5098, 0xc45b, 0x7a42, 0x51cf, 0x0bc4, 0x8110, 0x6564, 0xde9f, 0x6efb, 0x9f98, 0x7fb5, 0x28e5, 0x38d9, 0x8b4e, 0x77b4, 0x03ed, 0x57b1, 0xbc30, 0x7cb7, 0x4a58, 0x151c, 0x84da, 0x6fc2, 0xf11d, 0x646c, 0xacfe, 0x7f98, 0x3a40, 0x2768, 0x9448, 0x7d0f, 0x16a8, 0x490f, 0xccda, 0x7723, 0x58d4, 0x025b
};

static const int16_t Twiddles_stage2[FFTSIZE * 3 / 2 / 2] __attribute__ ((aligned (128))) =
{
    0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x5a82, 0x5a82, 0x5a82, 0x5a82, 0xa57e, 0xa57e, 0xa57e, 0xa57e, 0x7642, 0x7642, 0x7642, 0x7642, 0xcf05, 0xcf05, 0xcf05, 0xcf05, 0x30fc, 0x30fc, 0x30fc, 0x30fc, 0x89bf, 0x89bf, 0x89bf, 0x89bf, 0x7d8a, 0x7d8a, 0x7d8a, 0x7d8a, 0xe708, 0xe708, 0xe708, 0xe708, 0x471d, 0x471d, 0x471d, 0x471d, 0x9593, 0x9593, 0x9593, 0x9593, 0x6a6e, 0x6a6e, 0x6a6e, 0x6a6e, 0xb8e4, 0xb8e4, 0xb8e4, 0xb8e4, 0x18f9, 0x18f9, 0x18f9, 0x18f9, 0x8276, 0x8276, 0x8276, 0x8276, 0x7f62, 0x7f62, 0x7f62, 0x7f62, 0xf375, 0xf375, 0xf375, 0xf375, 0x5134, 0x5134, 0x5134, 0x5134, 0x9d0e, 0x9d0e, 0x9d0e, 0x9d0e, 0x70e3, 0x70e3, 0x70e3, 0x70e3, 0xc3aa, 0xc3aa, 0xc3aa, 0xc3aa, 0x2528, 0x2528, 0x2528, 0x2528, 0x8583, 0x8583, 0x8583, 0x8583, 0x7a7d, 0x7a7d, 0x7a7d, 0x7a7d, 0xdad8, 0xdad8, 0xdad8, 0xdad8, 0x3c57, 0x3c57, 0x3c57, 0x3c57, 0x8f1e, 0x8f1e, 0x8f1e, 0x8f1e, 0x62f2, 0x62f2, 0x62f2, 0x62f2, 0xaecd, 0xaecd, 0xaecd, 0xaecd, 0x0c8c, 0x0c8c, 0x0c8c, 0x0c8c, 0x809e, 0x809e, 0x809e, 0x809e,
    0x0000, 0x0000, 0x0000, 0x0000, 0x8000, 0x8000, 0x8000, 0x8000, 0xa57e, 0xa57e, 0xa57e, 0xa57e, 0xa57e, 0xa57e, 0xa57e, 0xa57e, 0xcf05, 0xcf05, 0xcf05, 0xcf05, 0x89bf, 0x89bf, 0x89bf, 0x89bf, 0x89bf, 0x89bf, 0x89bf, 0x89bf, 0xcf05, 0xcf05, 0xcf05, 0xcf05, 0xe708, 0xe708, 0xe708, 0xe708, 0x8276, 0x8276, 0x8276, 0x8276, 0x9593, 0x9593, 0x9593, 0x9593, 0xb8e4, 0xb8e4, 0xb8e4, 0xb8e4, 0xb8e4, 0xb8e4, 0xb8e4, 0xb8e4, 0x9593, 0x9593, 0x9593, 0x9593, 0x8276, 0x8276, 0x8276, 0x8276, 0xe708, 0xe708, 0xe708, 0xe708, 0xf375, 0xf375, 0xf375, 0xf375, 0x809e, 0x809e, 0x809e, 0x809e, 0x9d0e, 0x9d0e, 0x9d0e, 0x9d0e, 0xaecd, 0xaecd, 0xaecd, 0xaecd, 0xc3aa, 0xc3aa, 0xc3aa, 0xc3aa, 0x8f1e, 0x8f1e, 0x8f1e, 0x8f1e, 0x8583, 0x8583, 0x8583, 0x8583, 0xdad8, 0xdad8, 0xdad8, 0xdad8, 0xdad8, 0xdad8, 0xdad8, 0xdad8, 0x8583, 0x8583, 0x8583, 0x8583, 0x8f1e, 0x8f1e, 0x8f1e, 0x8f1e, 0xc3aa, 0xc3aa, 0xc3aa, 0xc3aa, 0xaecd, 0xaecd, 0xaecd, 0xaecd, 0x9d0e, 0x9d0e, 0x9d0e, 0x9d0e, 0x809e, 0x809e, 0x809e, 0x809e, 0xf375, 0xf375, 0xf375, 0xf375,
    0x0000, 0x0000, 0x0000, 0x0000, 0xa57e, 0xa57e, 0xa57e, 0xa57e, 0xcf05, 0xcf05, 0xcf05, 0xcf05, 0x89bf, 0x89bf, 0x89bf, 0x89bf, 0xe708, 0xe708, 0xe708, 0xe708, 0x9593, 0x9593, 0x9593, 0x9593, 0xb8e4, 0xb8e4, 0xb8e4, 0xb8e4, 0x8276, 0x8276, 0x8276, 0x8276, 0xf375, 0xf375, 0xf375, 0xf375, 0x9d0e, 0x9d0e, 0x9d0e, 0x9d0e, 0xc3aa, 0xc3aa, 0xc3aa, 0xc3aa, 0x8583, 0x8583, 0x8583, 0x8583, 0xdad8, 0xdad8, 0xdad8, 0xdad8, 0x8f1e, 0x8f1e, 0x8f1e, 0x8f1e, 0xaecd, 0xaecd, 0xaecd, 0xaecd, 0x809e, 0x809e, 0x809e, 0x809e, 0xf9b9, 0xf9b9, 0xf9b9, 0xf9b9, 0xa129, 0xa129, 0xa129, 0xa129, 0xc946, 0xc946, 0xc946, 0xc946, 0x877c, 0x877c, 0x877c, 0x877c, 0xe0e7, 0xe0e7, 0xe0e7, 0xe0e7, 0x9236, 0x9236, 0x9236, 0x9236, 0xb3c1, 0xb3c1, 0xb3c1, 0xb3c1, 0x8163, 0x8163, 0x8163, 0x8163, 0xed38, 0xed38, 0xed38, 0xed38, 0x9931, 0x9931, 0x9931, 0x9931, 0xbe32, 0xbe32, 0xbe32, 0xbe32, 0x83d7, 0x83d7, 0x83d7, 0x83d7, 0xd4e1, 0xd4e1, 0xd4e1, 0xd4e1, 0x8c4b, 0x8c4b, 0x8c4b, 0x8c4b, 0xaa0b, 0xaa0b, 0xaa0b, 0xaa0b, 0x8028, 0x8028, 0x8028, 0x8028,
    0x8000, 0x8000, 0x8000, 0x8000, 0xa57e, 0xa57e, 0xa57e, 0xa57e, 0x89bf, 0x89bf, 0x89bf, 0x89bf, 0xcf05, 0xcf05, 0xcf05, 0xcf05, 0x8276, 0x8276, 0x8276, 0x8276, 0xb8e4, 0xb8e4, 0xb8e4, 0xb8e4, 0x9593, 0x9593, 0x9593, 0x9593, 0xe708, 0xe708, 0xe708, 0xe708, 0x809e, 0x809e, 0x809e, 0x809e, 0xaecd, 0xaecd, 0xaecd, 0xaecd, 0x8f1e, 0x8f1e, 0x8f1e, 0x8f1e, 0xdad8, 0xdad8, 0xdad8, 0xdad8, 0x8583, 0x8583, 0x8583, 0x8583, 0xc3aa, 0xc3aa, 0xc3aa, 0xc3aa, 0x9d0e, 0x9d0e, 0x9d0e, 0x9d0e, 0xf375, 0xf375, 0xf375, 0xf375, 0x8028, 0x8028, 0x8028, 0x8028, 0xaa0b, 0xaa0b, 0xaa0b, 0xaa0b, 0x8c4b, 0x8c4b, 0x8c4b, 0x8c4b, 0xd4e1, 0xd4e1, 0xd4e1, 0xd4e1, 0x83d7, 0x83d7, 0x83d7, 0x83d7, 0xbe32, 0xbe32, 0xbe32, 0xbe32, 0x9931, 0x9931, 0x9931, 0x9931, 0xed38, 0xed38, 0xed38, 0xed38, 0x8163, 0x8163, 0x8163, 0x8163, 0xb3c1, 0xb3c1, 0xb3c1, 0xb3c1, 0x9236, 0x9236, 0x9236, 0x9236, 0xe0e7, 0xe0e7, 0xe0e7, 0xe0e7, 0x877c, 0x877c, 0x877c, 0x877c, 0xc946, 0xc946, 0xc946, 0xc946, 0xa129, 0xa129, 0xa129, 0xa129, 0xf9b9, 0xf9b9, 0xf9b9, 0xf9b9,
    0x0000, 0x0000, 0x0000, 0x0000, 0xa57e, 0xa57e, 0xa57e, 0xa57e, 0x89bf, 0x89bf, 0x89bf, 0x89bf, 0x30fc, 0x30fc, 0x30fc, 0x30fc, 0xb8e4, 0xb8e4, 0xb8e4, 0xb8e4, 0xe708, 0xe708, 0xe708, 0xe708, 0x8276, 0x8276, 0x8276, 0x8276, 0x6a6e, 0x6a6e, 0x6a6e, 0x6a6e, 0xdad8, 0xdad8, 0xdad8, 0xdad8, 0xc3aa, 0xc3aa, 0xc3aa, 0xc3aa, 0x809e, 0x809e, 0x809e, 0x809e, 0x5134, 0x5134, 0x5134, 0x5134, 0x9d0e, 0x9d0e, 0x9d0e, 0x9d0e, 0x0c8c, 0x0c8c, 0x0c8c, 0x0c8c, 0x8f1e, 0x8f1e, 0x8f1e, 0x8f1e, 0x7a7d, 0x7a7d, 0x7a7d, 0x7a7d, 0xed38, 0xed38, 0xed38, 0xed38, 0xb3c1, 0xb3c1, 0xb3c1, 0xb3c1, 0x83d7, 0x83d7, 0x83d7, 0x83d7, 0x41ce, 0x41ce, 0x41ce, 0x41ce, 0xaa0b, 0xaa0b, 0xaa0b, 0xaa0b, 0xf9b9, 0xf9b9, 0xf9b9, 0xf9b9, 0x877c, 0x877c, 0x877c, 0x877c, 0x73b6, 0x73b6, 0x73b6, 0x73b6, 0xc946, 0xc946, 0xc946, 0xc946, 0xd4e1, 0xd4e1, 0xd4e1, 0xd4e1, 0x8028, 0x8028, 0x8028, 0x8028, 0x5ed7, 0x5ed7, 0x5ed7, 0x5ed7, 0x9236, 0x9236, 0x9236, 0x9236, 0x1f1a, 0x1f1a, 0x1f1a, 0x1f1a, 0x9931, 0x9931, 0x9931, 0x9931, 0x7e9d, 0x7e9d, 0x7e9d, 0x7e9d,
    0x8000, 0x8000, 0x8000, 0x8000, 0x5a82, 0x5a82, 0x5a82, 0x5a82, 0xcf05, 0xcf05, 0xcf05, 0xcf05, 0x7642, 0x7642, 0x7642, 0x7642, 0x9593, 0x9593, 0x9593, 0x9593, 0x7d8a, 0x7d8a, 0x7d8a, 0x7d8a, 0x18f9, 0x18f9, 0x18f9, 0x18f9, 0x471d, 0x471d, 0x471d, 0x471d, 0x8583, 0x8583, 0x8583, 0x8583, 0x70e3, 0x70e3, 0x70e3, 0x70e3, 0xf375, 0xf375, 0xf375, 0xf375, 0x62f2, 0x62f2, 0x62f2, 0x62f2, 0xaecd, 0xaecd, 0xaecd, 0xaecd, 0x7f62, 0x7f62, 0x7f62, 0x7f62, 0x3c57, 0x3c57, 0x3c57, 0x3c57, 0x2528, 0x2528, 0x2528, 0x2528, 0x8163, 0x8163, 0x8163, 0x8163, 0x66d0, 0x66d0, 0x66d0, 0x66d0, 0xe0e7, 0xe0e7, 0xe0e7, 0xe0e7, 0x6dca, 0x6dca, 0x6dca, 0x6dca, 0xa129, 0xa129, 0xa129, 0xa129, 0x7fd9, 0x7fd9, 0x7fd9, 0x7fd9, 0x2b1f, 0x2b1f, 0x2b1f, 0x2b1f, 0x36ba, 0x36ba, 0x36ba, 0x36ba, 0x8c4b, 0x8c4b, 0x8c4b, 0x8c4b, 0x7885, 0x7885, 0x7885, 0x7885, 0x0648, 0x0648, 0x0648, 0x0648, 0x55f6, 0x55f6, 0x55f6, 0x55f6, 0xbe32, 0xbe32, 0xbe32, 0xbe32, 0x7c2a, 0x7c2a, 0x7c2a, 0x7c2a, 0x4c40, 0x4c40, 0x4c40, 0x4c40, 0x12c8, 0x12c8, 0x12c8, 0x12c8
};

static const int16_t Twiddles_stage3[FFTSIZE * 3 / 2 / 2] __attribute__ ((aligned (128))) =
{
    0x7fff, 0x5a82, 0x7fff, 0x5a82, 0x7fff, 0x5a82, 0x7fff, 0x5a82, 0x7fff, 0x5a82, 0x7fff, 0x5a82, 0x7fff, 0x5a82, 0x7fff, 0x5a82, 0x7fff, 0x5a82, 0x7fff, 0x5a82, 0x7fff, 0x5a82, 0x7fff, 0x5a82, 0x7fff, 0x5a82, 0x7fff, 0x5a82, 0x7fff, 0x5a82, 0x7fff, 0x5a82, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x7642, 0x30fc, 0x7642, 0x30fc, 0x7642, 0x30fc, 0x7642, 0x30fc, 0x7642, 0x30fc, 0x7642, 0x30fc, 0x7642, 0x30fc, 0x7642, 0x30fc, 0x7642, 0x30fc, 0x7642, 0x30fc, 0x7642, 0x30fc, 0x7642, 0x30fc, 0x7642, 0x30fc, 0x7642, 0x30fc, 0x7642, 0x30fc, 0x7642, 0x30fc, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf,
    0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x0000, 0xa57e, 0x8000, 0xa57e, 0x8000, 0xa57e, 0x8000, 0xa57e, 0x8000, 0xa57e, 0x8000, 0xa57e, 0x8000, 0xa57e, 0x8000, 0xa57e, 0x8000, 0xa57e, 0x8000, 0xa57e, 0x8000, 0xa57e, 0x8000, 0xa57e, 0x8000, 0xa57e, 0x8000, 0xa57e, 0x8000, 0xa57e, 0x8000, 0xa57e, 0x8000, 0xa57e, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05, 0x89bf, 0xcf05,
    0x0000, 0xcf05, 0x0000, 0xcf05, 0x0000, 0xcf05, 0x0000, 0xcf05, 0x0000, 0xcf05, 0x0000, 0xcf05, 0x0000, 0xcf05, 0x0000, 0xcf05, 0x0000, 0xcf05, 0x0000, 0xcf05, 0x0000, 0xcf05, 0x0000, 0xcf05, 0x0000, 0xcf05, 0x0000, 0xcf05, 0x0000, 0xcf05, 0x0000, 0xcf05, 0xa57e, 0x89bf, 0xa57e, 0x89bf, 0xa57e, 0x89bf, 0xa57e, 0x89bf, 0xa57e, 0x89bf, 0xa57e, 0x89bf, 0xa57e, 0x89bf, 0xa57e, 0x89bf, 0xa57e, 0x89bf, 0xa57e, 0x89bf, 0xa57e, 0x89bf, 0xa57e, 0x89bf, 0xa57e, 0x89bf, 0xa57e, 0x89bf, 0xa57e, 0x89bf, 0xa57e, 0x89bf, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276,
    0x8000, 0x89bf, 0x8000, 0x89bf, 0x8000, 0x89bf, 0x8000, 0x89bf, 0x8000, 0x89bf, 0x8000, 0x89bf, 0x8000, 0x89bf, 0x8000, 0x89bf, 0x8000, 0x89bf, 0x8000, 0x89bf, 0x8000, 0x89bf, 0x8000, 0x89bf, 0x8000, 0x89bf, 0x8000, 0x89bf, 0x8000, 0x89bf, 0x8000, 0x89bf, 0xa57e, 0xcf05, 0xa57e, 0xcf05, 0xa57e, 0xcf05, 0xa57e, 0xcf05, 0xa57e, 0xcf05, 0xa57e, 0xcf05, 0xa57e, 0xcf05, 0xa57e, 0xcf05, 0xa57e, 0xcf05, 0xa57e, 0xcf05, 0xa57e, 0xcf05, 0xa57e, 0xcf05, 0xa57e, 0xcf05, 0xa57e, 0xcf05, 0xa57e, 0xcf05, 0xa57e, 0xcf05, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0x8276, 0x9593, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708, 0xb8e4, 0xe708,
    0x0000, 0x89bf, 0x0000, 0x89bf, 0x0000, 0x89bf, 0x0000, 0x89bf, 0x0000, 0x89bf, 0x0000, 0x89bf, 0x0000, 0x89bf, 0x0000, 0x89bf, 0x0000, 0x89bf, 0x0000, 0x89bf, 0x0000, 0x89bf, 0x0000, 0x89bf, 0x0000, 0x89bf, 0x0000, 0x89bf, 0x0000, 0x89bf, 0x0000, 0x89bf, 0xa57e, 0x30fc, 0xa57e, 0x30fc, 0xa57e, 0x30fc, 0xa57e, 0x30fc, 0xa57e, 0x30fc, 0xa57e, 0x30fc, 0xa57e, 0x30fc, 0xa57e, 0x30fc, 0xa57e, 0x30fc, 0xa57e, 0x30fc, 0xa57e, 0x30fc, 0xa57e, 0x30fc, 0xa57e, 0x30fc, 0xa57e, 0x30fc, 0xa57e, 0x30fc, 0xa57e, 0x30fc, 0xb8e4, 0x8276, 0xb8e4, 0x8276, 0xb8e4, 0x8276, 0xb8e4, 0x8276, 0xb8e4, 0x8276, 0xb8e4, 0x8276, 0xb8e4, 0x8276, 0xb8e4, 0x8276, 0xb8e4, 0x8276, 0xb8e4, 0x8276, 0xb8e4, 0x8276, 0xb8e4, 0x8276, 0xb8e4, 0x8276, 0xb8e4, 0x8276, 0xb8e4, 0x8276, 0xb8e4, 0x8276, 0xe708, 0x6a6e, 0xe708, 0x6a6e, 0xe708, 0x6a6e, 0xe708, 0x6a6e, 0xe708, 0x6a6e, 0xe708, 0x6a6e, 0xe708, 0x6a6e, 0xe708, 0x6a6e, 0xe708, 0x6a6e, 0xe708, 0x6a6e, 0xe708, 0x6a6e, 0xe708, 0x6a6e, 0xe708, 0x6a6e, 0xe708, 0x6a6e, 0xe708, 0x6a6e, 0xe708, 0x6a6e,
    0x8000, 0xcf05, 0x8000, 0xcf05, 0x8000, 0xcf05, 0x8000, 0xcf05, 0x8000, 0xcf05, 0x8000, 0xcf05, 0x8000, 0xcf05, 0x8000, 0xcf05, 0x8000, 0xcf05, 0x8000, 0xcf05, 0x8000, 0xcf05, 0x8000, 0xcf05, 0x8000, 0xcf05, 0x8000, 0xcf05, 0x8000, 0xcf05, 0x8000, 0xcf05, 0x5a82, 0x7642, 0x5a82, 0x7642, 0x5a82, 0x7642, 0x5a82, 0x7642, 0x5a82, 0x7642, 0x5a82, 0x7642, 0x5a82, 0x7642, 0x5a82, 0x7642, 0x5a82, 0x7642, 0x5a82, 0x7642, 0x5a82, 0x7642, 0x5a82, 0x7642, 0x5a82, 0x7642, 0x5a82, 0x7642, 0x5a82, 0x7642, 0x5a82, 0x7642, 0x9593, 0x18f9, 0x9593, 0x18f9, 0x9593, 0x18f9, 0x9593, 0x18f9, 0x9593, 0x18f9, 0x9593, 0x18f9, 0x9593, 0x18f9, 0x9593, 0x18f9, 0x9593, 0x18f9, 0x9593, 0x18f9, 0x9593, 0x18f9, 0x9593, 0x18f9, 0x9593, 0x18f9, 0x9593, 0x18f9, 0x9593, 0x18f9, 0x9593, 0x18f9, 0x7d8a, 0x471d, 0x7d8a, 0x471d, 0x7d8a, 0x471d, 0x7d8a, 0x471d, 0x7d8a, 0x471d, 0x7d8a, 0x471d, 0x7d8a, 0x471d, 0x7d8a, 0x471d, 0x7d8a, 0x471d, 0x7d8a, 0x471d, 0x7d8a, 0x471d, 0x7d8a, 0x471d, 0x7d8a, 0x471d, 0x7d8a, 0x471d, 0x7d8a, 0x471d, 0x7d8a, 0x471d
};

//Second Twiddle Factor Table used in "Last Butterfly" function
//
// j*W[2*pi*k/N], k = 0,1,2,...N/4]
//
static const int32_t WTwiddles[FFTSIZE / 4 + 1] __attribute__ ((aligned (128))) =
{
 0x7FFF0000, 0x7FFE00C9, 0x7FFD0192, 0x7FF9025B, 0x7FF50324, 0x7FF003ED, 0x7FE904B6, 0x7FE1057F, 0x7FD80648, 0x7FCD0711, 0x7FC107D9, 0x7FB408A2, 0x7FA6096A, 0x7F970A33, 0x7F860AFB, 0x7F740BC4, 0x7F610C8C, 0x7F4D0D54, 0x7F370E1C, 0x7F210EE3, 0x7F090FAB, 0x7EEF1072, 0x7ED5113A, 0x7EB91201, 0x7E9C12C8, 0x7E7E138F, 0x7E5F1455, 0x7E3E151C, 0x7E1D15E2, 0x7DFA16A8, 0x7DD5176E, 0x7DB01833, 0x7D8918F9, 0x7D6219BE, 0x7D391A82, 0x7D0E1B47, 0x7CE31C0B, 0x7CB61CCF, 0x7C881D93, 0x7C591E57, 0x7C291F1A, 0x7BF81FDD, 0x7BC5209F, 0x7B912161, 0x7B5C2223, 0x7B2622E5, 0x7AEE23A6, 0x7AB62467, 0x7A7C2528, 0x7A4125E8, 0x7A0526A8, 0x79C82767, 0x79892826, 0x794A28E5, 0x790929A3, 0x78C72A61, 0x78842B1F, 0x783F2BDC, 0x77FA2C99, 0x77B32D55, 0x776B2E11, 0x77222ECC, 0x76D82F87, 0x768D3041, 0x764130FB, 0x75F331B5, 0x75A5326E, 0x75553326, 0x750433DF, 0x74B23496, 0x745F354D, 0x740A3604, 0x73B536BA, 0x735E376F, 0x73073824, 0x72AE38D9, 0x7254398C, 0x71F93A40, 0x719D3AF2, 0x71403BA5, 0x70E23C56, 0x70833D07, 0x70223DB8, 0x6FC13E68, 0x6F5E3F17, 0x6EFB3FC5, 0x6E964073, 0x6E304121, 0x6DC941CE, 0x6D61427A, 0x6CF84325, 0x6C8E43D0, 0x6C23447A, 0x6BB74524, 0x6B4A45CD, 0x6ADC4675, 0x6A6D471C, 0x69FD47C3, 0x698B4869, 0x6919490F, 0x68A649B4, 0x68324A58, 0x67BC4AFB, 0x67464B9D, 0x66CF4C3F, 0x66564CE0, 0x65DD4D81, 0x65634E20, 0x64E84EBF, 0x646C4F5D, 0x63EE4FFB, 0x63705097, 0x62F15133, 0x627151CE, 0x61F05268, 0x616E5302, 0x60EB539B, 0x60685432, 0x5FE354C9, 0x5F5D5560, 0x5ED755F5, 0x5E4F568A, 0x5DC7571D, 0x5D3E57B0, 0x5CB35842, 0x5C2858D3, 0x5B9C5964, 0x5B0F59F3, 0x5A825A82, 0x59F35B0F, 0x59645B9C, 0x58D35C28, 0x58425CB3, 0x57B05D3E, 0x571D5DC7, 0x568A5E4F, 0x55F55ED7, 0x55605F5D, 0x54C95FE3, 0x54326068, 0x539B60EB, 0x5302616E, 0x526861F0, 0x51CE6271, 0x513362F1, 0x50976370, 0x4FFB63EE, 0x4F5D646C, 0x4EBF64E8, 0x4E206563, 0x4D8165DD, 0x4CE06656, 0x4C3F66CF, 0x4B9D6746, 0x4AFB67BC, 0x4A586832, 0x49B468A6, 0x490F6919, 0x4869698B, 0x47C369FD, 0x471C6A6D, 0x46756ADC, 0x45CD6B4A, 0x45246BB7, 0x447A6C23, 0x43D06C8E, 0x43256CF8, 0x427A6D61, 0x41CE6DC9, 0x41216E30, 0x40736E96, 0x3FC56EFB, 0x3F176F5E, 0x3E686FC1, 0x3DB87022, 0x3D077083, 0x3C5670E2, 0x3BA57140, 0x3AF2719D, 0x3A4071F9, 0x398C7254, 0x38D972AE, 0x38247307, 0x376F735E, 0x36BA73B5, 0x3604740A, 0x354D745F, 0x349674B2, 0x33DF7504, 0x33267555, 0x326E75A5, 0x31B575F3, 0x30FB7641, 0x3041768D, 0x2F8776D8, 0x2ECC7722, 0x2E11776B, 0x2D5577B3, 0x2C9977FA, 0x2BDC783F, 0x2B1F7884, 0x2A6178C7, 0x29A37909, 0x28E5794A, 0x28267989, 0x276779C8, 0x26A87A05, 0x25E87A41, 0x25287A7C, 0x24677AB6, 0x23A67AEE, 0x22E57B26, 0x22237B5C, 0x21617B91, 0x209F7BC5, 0x1FDD7BF8, 0x1F1A7C29, 0x1E577C59, 0x1D937C88, 0x1CCF7CB6, 0x1C0B7CE3, 0x1B477D0E, 0x1A827D39, 0x19BE7D62, 0x18F97D89, 0x18337DB0, 0x176E7DD5, 0x16A87DFA, 0x15E27E1D, 0x151C7E3E, 0x14557E5F, 0x138F7E7E, 0x12C87E9C, 0x12017EB9, 0x113A7ED5, 0x10727EEF, 0x0FAB7F09, 0x0EE37F21, 0x0E1C7F37, 0x0D547F4D, 0x0C8C7F61, 0x0BC47F74, 0x0AFB7F86, 0x0A337F97, 0x096A7FA6, 0x08A27FB4, 0x07D97FC1, 0x07117FCD, 0x06487FD8, 0x057F7FE1, 0x04B67FE9, 0x03ED7FF0, 0x03247FF5, 0x025B7FF9, 0x01927FFD, 0x00C97FFE, 0x00007FFF 
};

static const int16_t brev1024[1024] = {0,512,256,768,128,640,384,896,64,576,320,832,192,704,448,960,32,544,288,800,160,672,416,928,96,608,352,864,224,736,480,992,16,528,272,784,144,656,400,912,80,592,336,848,208,720,464,976,48,560,304,816,176,688,432,944,112,624,368,880,240,752,496,1008,8,520,264,776,136,648,392,904,72,584,328,840,200,712,456,968,40,552,296,808,168,680,424,936,104,616,360,872,232,744,488,1000,24,536,280,792,152,664,408,920,88,600,344,856,216,728,472,984,56,568,312,824,184,696,440,952,120,632,376,888,248,760,504,1016,4,516,260,772,132,644,388,900,68,580,324,836,196,708,452,964,36,548,292,804,164,676,420,932,100,612,356,868,228,740,484,996,20,532,276,788,148,660,404,916,84,596,340,852,212,724,468,980,52,564,308,820,180,692,436,948,116,628,372,884,244,756,500,1012,12,524,268,780,140,652,396,908,76,588,332,844,204,716,460,972,44,556,300,812,172,684,428,940,108,620,364,876,236,748,492,1004,28,540,284,796,156,668,412,924,92,604,348,860,220,732,476,988,60,572,316,828,188,700,444,956,124,636,380,892,252,764,508,1020,2,514,258,770,130,642,386,898,66,578,322,834,194,706,450,962,34,546,290,802,162,674,418,930,98,610,354,866,226,738,482,994,18,530,274,786,146,658,402,914,82,594,338,850,210,722,466,978,50,562,306,818,178,690,434,946,114,626,370,882,242,754,498,1010,10,522,266,778,138,650,394,906,74,586,330,842,202,714,458,970,42,554,298,810,170,682,426,938,106,618,362,874,234,746,490,1002,26,538,282,794,154,666,410,922,90,602,346,858,218,730,474,986,58,570,314,826,186,698,442,954,122,634,378,890,250,762,506,1018,6,518,262,774,134,646,390,902,70,582,326,838,198,710,454,966,38,550,294,806,166,678,422,934,102,614,358,870,230,742,486,998,22,534,278,790,150,662,406,918,86,598,342,854,214,726,470,982,54,566,310,822,182,694,438,950,118,630,374,886,246,758,502,1014,14,526,270,782,142,654,398,910,78,590,334,846,206,718,462,974,46,558,302,814,174,686,430,942,110,622,366,878,238,750,494,1006,30,542,286,798,158,670,414,926,94,606,350,862,222,734,478,990,62,574,318,830,190,702,446,958,126,638,382,894,254,766,510,1022,1,513,257,769,129,641,385,897,65,577,321,833,193,705,449,961,33,545,289,801,161,673,417,929,97,609,353,865,225,737,481,993,17,529,273,785,145,657,401,913,81,593,337,849,209,721,465,977,49,561,305,817,177,689,433,945,113,625,369,881,241,753,497,1009,9,521,265,777,137,649,393,905,73,585,329,841,201,713,457,969,41,553,297,809,169,681,425,937,105,617,361,873,233,745,489,1001,25,537,281,793,153,665,409,921,89,601,345,857,217,729,473,985,57,569,313,825,185,697,441,953,121,633,377,889,249,761,505,1017,5,517,261,773,133,645,389,901,69,581,325,837,197,709,453,965,37,549,293,805,165,677,421,933,101,613,357,869,229,741,485,997,21,533,277,789,149,661,405,917,85,597,341,853,213,725,469,981,53,565,309,821,181,693,437,949,117,629,373,885,245,757,501,1013,13,525,269,781,141,653,397,909,77,589,333,845,205,717,461,973,45,557,301,813,173,685,429,941,109,621,365,877,237,749,493,1005,29,541,285,797,157,669,413,925,93,605,349,861,221,733,477,989,61,573,317,829,189,701,445,957,125,637,381,893,253,765,509,1021,3,515,259,771,131,643,387,899,67,579,323,835,195,707,451,963,35,547,291,803,163,675,419,931,99,611,355,867,227,739,483,995,19,531,275,787,147,659,403,915,83,595,339,851,211,723,467,979,51,563,307,819,179,691,435,947,115,627,371,883,243,755,499,1011,11,523,267,779,139,651,395,907,75,587,331,843,203,715,459,971,43,555,299,811,171,683,427,939,107,619,363,875,235,747,491,1003,27,539,283,795,155,667,411,923,91,603,347,859,219,731,475,987,59,571,315,827,187,699,443,955,123,635,379,891,251,763,507,1019,7,519,263,775,135,647,391,903,71,583,327,839,199,711,455,967,39,551,295,807,167,679,423,935,103,615,359,871,231,743,487,999,23,535,279,791,151,663,407,919,87,599,343,855,215,727,471,983,55,567,311,823,183,695,439,951,119,631,375,887,247,759,503,1015,15,527,271,783,143,655,399,911,79,591,335,847,207,719,463,975,47,559,303,815,175,687,431,943,111,623,367,879,239,751,495,1007,31,543,287,799,159,671,415,927,95,607,351,863,223,735,479,991,63,575,319,831,191,703,447,959,127,639,383,895,255,767,511,1023};
/*===========================================================================
    TYPEDEF
===========================================================================*/

// multi-threading context structures
typedef struct
{
    dspCV_synctoken_t *token;
    unsigned int workerCount;
    unsigned int jobCount;
    uint8_t *src;
    uint32_t srcWidth;
    uint32_t srcHeight;
    uint32_t srcStride;
    int32_t *dst;
    uint32_t dstStride;
    int32_t pPrecision;
    HVX_Vector *scratchBuf;
    qurt_barrier_t       barrier;
} fcvFFTu8_callback_t;

/*===========================================================================
    LOCAL FUNCTION
===========================================================================*/
// callback for multi-threaded horizontal FFT's
static void fcvFFTu8_horiz_callback(void* data)
{
    fcvFFTu8_callback_t *dptr = (fcvFFTu8_callback_t*) data;

#if (__HEXAGON_ARCH__ < 65)
    int lockResult = dspCV_hvx_lock(DSPCV_HVX_MODE_128B, 1);
    if (128 != lockResult) 
    {
        FARF(HIGH,"Error, could not acquire HVX!");
        dspCV_worker_pool_synctoken_jobdone(dptr->token); // release multi-threading job token
        return;
    }
#endif

    uint8_t *src = dptr->src;
    //constants for FFTSIZE
    // Re-interpret srcWidth of 1024 real values as 512 complex values, hence srcWidth = 512.
    const uint32_t srcWidth = FFTSIZE/2;
    const uint32_t srcHeight = FFTSIZE;

    const uint32_t srcStride = dptr->srcStride;
    const int32_t precision = dptr->pPrecision;
    const int32_t scalefactor = Q6_R_vsplatb_R(1 << precision);
    unsigned int workerCount = dspCV_atomic_inc_return(&(dptr->workerCount)) - 1;

    // if VTCM is allocated, use for intermediate storage
    HVX_Vector *scratch = dptr->scratchBuf;
    scratch += (workerCount * 2 * (FFTSIZE/(VLEN/sizeof(int32_t))));

    // set up L2 fetch regs
    uint64_t L2FETCH_REGISTER = (1ULL <<48) | ((uint64_t)srcStride<<32) | ((uint64_t)srcWidth<<16) | 1;

    // loop until no more horizontal strips to process
    while (1)
    {
        // atomically add 1 to the job count to claim a job.
        unsigned int jobCount = dspCV_atomic_inc_return(&(dptr->jobCount)) - 1;
        // if all horizontal strips have been claimed for processing, break out and exit the callback
        if (jobCount >= srcHeight)
            break;

        if (0) L2FETCH(src + 4*srcStride, L2FETCH_REGISTER);

        // Declare HVX register aliases
        DECLARE_HVX_REGISTERS;

        HVX_Vector *vsrc = (HVX_Vector*)&src[srcStride * jobCount];
        V0 = *vsrc++;
        V1 = *vsrc++;
        V4 = *vsrc++;
        V5 = *vsrc++;
        V2 = *vsrc++;
        V3 = *vsrc++;
        V6 = *vsrc++;
        V7 = *vsrc++;

        // Perform partial bit-reversal on 16-bit complex elements.
        for(int i = -2; i >= -64; i += i)
        {
            V1_0 = Q6_W_vshuff_VVR(V1, V0, i); // brev[i*4 + 0], i = 64..127 : 0..63
            V3_2 = Q6_W_vshuff_VVR(V3, V2, i); // brev[i*4 + 1], i = 64..127 : 0..63
            V5_4 = Q6_W_vshuff_VVR(V5, V4, i); // brev[i*4 + 2], i = 64..127 : 0..63
            V7_6 = Q6_W_vshuff_VVR(V7, V6, i); // brev[i*4 + 3], i = 64..127 : 0..63
        }

        // multiply by scale factor (and widen to 16 bits)
        V11_10 = Q6_Wh_vmpy_VubRb(V1,scalefactor);    // Im(brev[i*4 + 0]) : Re(brev[i*4 + 0]), i = 64..127
        V1_0 = Q6_Wh_vmpy_VubRb(V0,scalefactor);      // Im(brev[i*4 + 0]) : Re(brev[i*4 + 0]), i = 0..63
        V13_12 = Q6_Wh_vmpy_VubRb(V3,scalefactor);    // Im(brev[i*4 + 1]) : Re(brev[i*4 + 1]), i = 64..127
        V3_2 = Q6_Wh_vmpy_VubRb(V2,scalefactor);      // Im(brev[i*4 + 1]) : Re(brev[i*4 + 1]), i = 0..63
        V15_14 = Q6_Wh_vmpy_VubRb(V5,scalefactor);    // Im(brev[i*4 + 2]) : Re(brev[i*4 + 2]), i = 64..127
        V5_4 = Q6_Wh_vmpy_VubRb(V4,scalefactor);      // Im(brev[i*4 + 2]) : Re(brev[i*4 + 2]), i = 0..63
        V17_16 = Q6_Wh_vmpy_VubRb(V7,scalefactor);    // Im(brev[i*4 + 3]) : Re(brev[i*4 + 3]), i = 64..127
        V7_6 = Q6_Wh_vmpy_VubRb(V6,scalefactor);      // Im(brev[i*4 + 3]) : Re(brev[i*4 + 3]), i = 0..63

        // FFT Stage 1 for N/2 (i.e. 512)
        V21_20 = Q6_Wh_vadd_WhWh_sat(V1_0, V3_2);      // a = x0 + x1 (lo)
        V30 = Q6_Vh_vsub_VhVh_sat(V3, V1);             // Re(b) = Im(x1) - Im(x0) (lo)
        V31 = Q6_Vh_vsub_VhVh_sat(V0, V2);             // Im(b) = Re(x0) - Re(x1) (lo)
        V1_0 = Q6_Wh_vadd_WhWh_sat(V11_10, V13_12);    // a = x0 + x1 (hi)
        V2 = Q6_Vh_vsub_VhVh_sat(V13, V11);            // Re(b) = Im(x1) - Im(x0) (hi)
        V3 = Q6_Vh_vsub_VhVh_sat(V10, V12);            // Im(b) = Re(x0) - Re(x1) (hi)

        V11_10 = Q6_Wh_vadd_WhWh_sat(V5_4, V7_6);     // c = x2 + x3 (lo)
        V5_4 = Q6_Wh_vsub_WhWh_sat(V5_4, V7_6);       // d = x2 - x3 (lo)
        V13_12 = Q6_Wh_vadd_WhWh_sat(V15_14, V17_16); // c = x2 + x3 (hi)
        V7_6 = Q6_Wh_vsub_WhWh_sat(V15_14, V17_16);   // d = x2 - x3 (hi)

        V15_14 = Q6_Wh_vadd_WhWh_sat(V21_20, V11_10); // X0 = a + c (lo)
        V11_10 = Q6_Wh_vsub_WhWh_sat(V21_20, V11_10); // X2 = a - c (lo)
        V21_20 = Q6_Wh_vadd_WhWh_sat(V1_0, V13_12);   // X0 = a + c (hi)
        V1_0 = Q6_Wh_vsub_WhWh_sat(V1_0, V13_12);     // X2 = a - c (hi)

        V13_12 = Q6_Wh_vadd_WhWh_sat(V31_30, V5_4);   // X1 = b + d (lo)
        V5_4 = Q6_Wh_vsub_WhWh_sat(V31_30, V5_4);     // X3 = b - d (lo)
        V31_30 = Q6_Wh_vadd_WhWh_sat(V3_2, V7_6);     // X1 = b + d (hi)
        V3_2 = Q6_Wh_vsub_WhWh_sat(V3_2, V7_6);       // X3 = b - d (hi)

        // load and apply twiddles
        HVX_Vector *twid = (HVX_Vector *) Twiddles;
        V6 = twid[0];         // Re(Wb[0..63])
        V7 = twid[1];         // Re(Wb[64..127)
        V8 = twid[4];         // Im(Wb[0..63])
        V9 = twid[5];         // Im(Wb[64..127)
        V16 = twid[8];        // Re(Wa[0..63])
        V17 = twid[9];        // Re(Wa[64..127)
        V18 = twid[12];       // Im(Wa[0..63])
        V19 = twid[13];       // Im(Wa[64..127)
        V22 = twid[16];       // Re(Wc[0..63])
        V23 = twid[17];       // Re(Wc[64..127)
        V24 = twid[20];       // Im(Wc[0..63])
        V25 = twid[21];       // Im(Wc[64..127)

        V_CPLX_MULT_16_16(V12, V13, V16, V18, V26, V27, V28); // X1 = X1 * Wa (lo)   (Im : Re)
        V_CPLX_MULT_16_16(V30, V31, V17, V19, V12, V13, V28); // X1 = X1 * Wa (hi)   (Im : Re)
        V_CPLX_MULT_16_16(V10, V11, V6, V8, V30, V31, V28);   // X2 = X2 * Wb (lo)   (Im : Re)
        V_CPLX_MULT_16_16(V0, V1, V7, V9, V10, V11, V28);     // X2 = X2 * Wb (hi)   (Im : Re)
        V_CPLX_MULT_16_16(V4, V5, V22, V24, V0, V1, V28);     // X3 = X3 * Wc (lo)   (Im : Re)
        V_CPLX_MULT_16_16(V2, V3, V23, V25, V4, V5, V28);     // X3 = X3 * Wc (hi)   (Im : Re)

        // stage 2
        // rearrange for 2nd butterfly - shuffle X3 & X2 (2 bytes)
        V3_2 = Q6_W_vshuff_VVR(V0, V30, 2);     // real lo
        V29_28 = Q6_W_vshuff_VVR(V1, V31, 2);   // imag lo
        V31_30 = Q6_W_vshuff_VVR(V4, V10, 2);   // real hi
        V5_4 = Q6_W_vshuff_VVR(V5, V11, 2);     // imag hi
        // shuffle X1 & X0 (2 bytes)
        V1_0 = Q6_W_vshuff_VVR(V26, V14, 2);    // real lo
        V27_26 = Q6_W_vshuff_VVR(V27, V15, 2);  // imag lo
        V11_10 = Q6_W_vshuff_VVR(V12, V20, 2);  // real hi
        V21_20 = Q6_W_vshuff_VVR(V13, V21, 2);  // imag hi
        // shuffle (4 bytes)
        V13_12 = Q6_W_vshuff_VVR(V2, V0, 4);         // X2:X0 real lo
        V3_2 = Q6_W_vshuff_VVR(V3, V1, 4);           // X3:X1 real lo
        V1_0 = Q6_W_vshuff_VVR(V28, V26, 4);         // X2:X0 imag lo
        V29_28 = Q6_W_vshuff_VVR(V29, V27, 4);       // X3:X1 imag lo
        V27_26 = Q6_W_vshuff_VVR(V30, V10, 4);       // X2:X0 real hi
        V15_14 = Q6_W_vshuff_VVR(V31, V11, 4);       // X3:X1 real hi
        V11_10 = Q6_W_vshuff_VVR(V4, V20, 4);        // X2:X0 imag hi
        V5_4 = Q6_W_vshuff_VVR(V5, V21, 4);          // X3:X1 imag hi

        V20 = Q6_Vh_vadd_VhVh_sat(V12, V2);          // Re(a) = Re(x0) + Re(x1) (lo)
        V21 = Q6_Vh_vadd_VhVh_sat(V0, V28);          // Im(a) = Im(x0) + Im(x1) (lo)
        V30 = Q6_Vh_vsub_VhVh_sat(V28, V0);          // Re(b) = Im(x1) - Im(x0) (lo)
        V31 = Q6_Vh_vsub_VhVh_sat(V12, V2);          // Im(b) = Re(x0) - Re(x1) (lo)
        V0 = Q6_Vh_vadd_VhVh_sat(V26, V14);          // Re(a) = Re(x0) + Re(x1) (hi)
        V2 = Q6_Vh_vsub_VhVh_sat(V4, V10);           // Re(b) = Im(x1) - Im(x0) (hi)
        V12 = Q6_Vh_vadd_VhVh_sat(V27, V15);         // Re(c) = Re(x2) + Re(x3) (hi)
        V26 = Q6_Vh_vsub_VhVh_sat(V26, V14);         // Im(b) = Re(x0) - Re(x1) (hi)
        V14 = Q6_Vh_vadd_VhVh_sat(V10, V4);          // Im(a) = Im(x0) + Im(x1) (hi)
        V4 = Q6_Vh_vsub_VhVh_sat(V13, V3);           // Re(d) = Re(x2) - Re(x3) (lo)
        V10 = Q6_Vh_vadd_VhVh_sat(V13, V3);          // Re(c) = Re(x2) + Re(x3) (lo)
        V3 = V26;                                    // Im(b) = Re(x0) - Re(x1) (hi)
        V13 = Q6_Vh_vadd_VhVh_sat(V11, V5);          // Im(c) = Im(x2) + Im(x3) (hi)
        V26 = Q6_Vh_vsub_VhVh_sat(V27, V15);         // Re(d) = Re(x2) - Re(x3) (hi)
        V27 = Q6_Vh_vsub_VhVh_sat(V11, V5);          // Im(d) = Im(x2) - Im(x3) (hi)
        V11 = Q6_Vh_vadd_VhVh_sat(V1, V29);          // Im(c) = Im(x2) + Im(x3) (lo)
        V5 = Q6_Vh_vsub_VhVh_sat(V1, V29);           // Im(d) = Im(x2) - Im(x3) (lo)
        V1 = V14;                                    // Im(a) = Im(x0) + Im(x1) (hi)

        V15_14 = Q6_Wh_vadd_WhWh_sat(V21_20, V11_10); // X0 = a + c (lo)
        V11_10 = Q6_Wh_vsub_WhWh_sat(V21_20, V11_10); // X2 = a - c (lo)
        V21_20 = Q6_Wh_vadd_WhWh_sat(V1_0, V13_12);   // X0 = a + c (hi)
        V1_0 = Q6_Wh_vsub_WhWh_sat(V1_0, V13_12);     // X2 = a - c (hi)

        V13_12 = Q6_Wh_vadd_WhWh_sat(V31_30, V5_4);   // X1 = b + d (lo)
        V5_4 = Q6_Wh_vsub_WhWh_sat(V31_30, V5_4);     // X3 = b - d (lo)
        V31_30 = Q6_Wh_vadd_WhWh_sat(V3_2, V27_26);   // X1 = b + d (hi)
        V3_2 = Q6_Wh_vsub_WhWh_sat(V3_2, V27_26);     // X3 = b - d (hi)

        // load and apply twiddles
        twid = (HVX_Vector *) Twiddles_stage2;
        V6 = *twid++;       // Re(Wb) (lo)
        V8 = *twid++;       // Re(Wb) (hi)
        V7 = *twid++;       // Im(Wb) (lo)
        V9 = *twid++;       // Im(Wb) (hi)
        V16 = *twid++;      // Re(Wa) (lo)
        V18 = *twid++;      // Re(Wa) (hi)
        V17 = *twid++;      // Im(Wa) (lo)
        V19 = *twid++;      // Im(Wa) (hi)
        V22 = *twid++;      // Re(Wc) (lo)
        V24 = *twid++;      // Re(Wc) (hi)
        V23 = *twid++;      // Im(Wc) (lo)
        V25 = *twid++;      // Im(Wc) (hi)

        V_CPLX_MULT_16_16(V12, V13, V16, V17, V26, V27, V28); // X1 = X1 * Wa (lo)   (Im : Re)
        V_CPLX_MULT_16_16(V30, V31, V18, V19, V12, V13, V28); // X1 = X1 * Wa (hi)   (Im : Re)
        V_CPLX_MULT_16_16(V10, V11, V6, V7, V30, V31, V28);   // X2 = X2 * Wb (lo)   (Im : Re)
        V_CPLX_MULT_16_16(V0, V1, V8, V9, V10, V11, V28);     // X2 = X2 * Wb (hi)   (Im : Re)
        V_CPLX_MULT_16_16(V4, V5, V22, V23, V0, V1, V28);     // X3 = X3 * Wc (lo)   (Im : Re)
        V_CPLX_MULT_16_16(V2, V3, V24, V25, V4, V5, V28);     // X3 = X3 * Wc (hi)   (Im : Re)

        // stage 3
        // rearrange for 3nd butterfly - shuffle X3 & X2 (8 bytes)
        V3_2 = Q6_W_vshuff_VVR(V0, V30, 8);           // real lo
        V29_28 = Q6_W_vshuff_VVR(V1, V31, 8);         // imag lo
        V31_30 = Q6_W_vshuff_VVR(V4, V10, 8);         // real hi
        V5_4 = Q6_W_vshuff_VVR(V5, V11, 8);           // imag hi
        // shuffle X1 & X0 (8 bytes)
        V1_0 = Q6_W_vshuff_VVR(V26, V14, 8);          // real lo
        V27_26 = Q6_W_vshuff_VVR(V27, V15, 8);        // imag lo
        V11_10 = Q6_W_vshuff_VVR(V12, V20, 8);        // real hi
        V21_20 = Q6_W_vshuff_VVR(V13, V21, 8);        // imag hi
        // shuffle (16 bytes)
        V13_12 = Q6_W_vshuff_VVR(V2, V0, 16);         // X2:X0 real lo
        V3_2 = Q6_W_vshuff_VVR(V3, V1, 16);           // X3:X1 real lo
        V1_0 = Q6_W_vshuff_VVR(V28, V26, 16);         // X2:X0 imag lo
        V29_28 = Q6_W_vshuff_VVR(V29, V27, 16);       // X3:X1 imag lo
        V27_26 = Q6_W_vshuff_VVR(V30, V10, 16);       // X2:X0 real hi
        V15_14 = Q6_W_vshuff_VVR(V31, V11, 16);       // X3:X1 real hi
        V11_10 = Q6_W_vshuff_VVR(V4, V20, 16);        // X2:X0 imag hi
        V5_4 = Q6_W_vshuff_VVR(V5, V21, 16);          // X3:X1 imag hi

        // At this point, need to start using 32 bits for X0...X3.

        V7_6 = Q6_Ww_vunpack_Vh(V13);                 // X2[1] : X2[0] real
        V13_12 = Q6_Ww_vunpack_Vh(V12);               // X0[1] : X0[0] real
        V9_8 = Q6_Ww_vunpack_Vh(V3);                  // X3[1] : X3[0] real
        V3_2 = Q6_Ww_vunpack_Vh(V2);                  // X1[1] : X1[0] real
        V17_16 = Q6_Ww_vadd_WwWw_sat(V13_12, V3_2);   // Re(a[1] : a[0]) = Re(x0) + Re(x1)
        V3_2 = Q6_Ww_vsub_WwWw_sat(V13_12, V3_2);     // Im(b[1] : b[0]) = Re(x0) - Re(x1)
        V13_12 = Q6_Ww_vadd_WwWw_sat(V7_6, V9_8);     // Re(c[1] : c[0]) = Re(x2) + Re(x3)
        V7_6 = Q6_Ww_vsub_WwWw_sat(V7_6, V9_8);       // Re(d[1] : d[0]) = Re(x2) - Re(x3)
        V9_8 = Q6_Ww_vadd_WwWw_sat(V17_16, V13_12);   // Re(X0[1]:X0[0]) = Re(a) + Re(c)
        V17_16 = Q6_Ww_vsub_WwWw_sat(V17_16, V13_12); // Re(X2[1]:X2[0]) = Re(a) - Re(c)

        V13_12 = Q6_Ww_vunpack_Vh(V0);                // X0[1] : X0[0] imag
        V1_0 = Q6_Ww_vunpack_Vh(V1);                  // X2[1] : X2[0] imag
        V21_20 = Q6_Ww_vunpack_Vh(V28);               // X1[1] : X1[0] imag
        V29_28 = Q6_Ww_vunpack_Vh(V29);               // X3[1] : X3[0] imag
        V31_30 = Q6_Ww_vadd_WwWw_sat(V13_12, V21_20); // Im(a[1] : a[0]) = Im(x0) + Im(x1)
        V21_20 = Q6_Ww_vsub_WwWw_sat(V21_20, V13_12); // Re(b[1] : b[0]) = Im(x1) - Im(x0)
        V13_12 = Q6_Ww_vadd_WwWw_sat(V1_0, V29_28);   // Im(c[1] : c[0]) = Im(x2) + Im(x3)
        V1_0 = Q6_Ww_vsub_WwWw_sat(V1_0, V29_28);     // Im(d[1] : d[0]) = Im(x2) - Im(x3)
        V29_28 = Q6_Ww_vadd_WwWw_sat(V31_30, V13_12); // Im(X0[1]:X0[0]) = Im(a) + Im(c)
        V13_12 = Q6_Ww_vsub_WwWw_sat(V31_30, V13_12); // Im(X2[1]:X2[0]) = Im(a) - Im(c)
        V31_30 = Q6_Ww_vadd_WwWw_sat(V21_20, V7_6);   // Re(X1[1]:X1[0]) = Re(b) + Re(d)
        V7_6 = Q6_Ww_vsub_WwWw_sat(V21_20, V7_6);     // Re(X3[1]:X3[0]) = Re(b) - Re(d)
        V21_20 = Q6_Ww_vadd_WwWw_sat(V3_2, V1_0);     // Im(X1[1]:X1[0]) = Im(b) + Im(d)
        V3_2 = Q6_Ww_vsub_WwWw_sat(V3_2, V1_0);       // Im(X3[1]:X3[0]) = Im(b) - Im(d)

        // free up some registers
        scratch[0] = V8;                              // Re(X0[0])
        scratch[1] = V9;                              // Re(X0[1])
        scratch[2] = V28;                             // Im(X0[0])
        scratch[3] = V29;                             // Im(X0[1])

        // Apply twiddles
        twid = (HVX_Vector *) Twiddles_stage3;
        V0 = twid[0];                           // Re(Wb[1:0]) (shuffled)
        V1 = twid[2];                           // Im(Wb[1:0]) (shuffled)
        V_CPLX_MULT_32_16(V17, V13, V0, V1, V8, V9, V28); // X2[1] = X2 * Wb (lo)   (Im : Re)
        scratch[4] = V8;                        // Re(X2[1])
        scratch[5] = V9;                        // Im(X2[1])
        V0 = Q6_Vh_vshuffe_VhVh(V0,V0);         // copy Re(Wb[0]) to odd elements for 32x16 instruction
        V1 = Q6_Vh_vshuffe_VhVh(V1,V1);         // copy Im(Wb[0]) to odd elements for 32x16 instruction
        V_CPLX_MULT_32_16(V16, V12, V0, V1, V8, V9, V28); // X2[0] = X2 * Wb (lo)   (Im : Re)
        V0 = twid[4];                           // Re(Wa[1:0]) (shuffled)
        V1 = twid[6];                           // Im(Wa[1:0]) (shuffled)
        V_CPLX_MULT_32_16(V31, V21, V0, V1, V16, V17, V28); // X1[1] = X1 * Wa (lo)   (Im : Re)
        V0 = Q6_Vh_vshuffe_VhVh(V0,V0);         // copy Re(Wa[0]) to odd elements for 32x16 instruction
        V1 = Q6_Vh_vshuffe_VhVh(V1,V1);         // copy Im(Wa[0]) to odd elements for 32x16 instruction
        V_CPLX_MULT_32_16(V30, V20, V0, V1, V12, V13, V28); // X1[0] = X1 * Wa (lo)   (Im : Re)
        V0 = twid[8];                           // Re(Wc[1:0]) (shuffled)
        V1 = twid[10];                          // Im(Wc[1:0]) (shuffled)
        V_CPLX_MULT_32_16(V7, V3, V0, V1, V20, V21, V28); // X3[1] = X3 * Wc (lo)   (Im : Re)
        V0 = Q6_Vh_vshuffe_VhVh(V0,V0);         // copy Re(Wc[0]) to odd elements for 32x16 instruction
        V1 = Q6_Vh_vshuffe_VhVh(V1,V1);         // copy Im(Wc[0]) to odd elements for 32x16 instruction
        V_CPLX_MULT_32_16(V6, V2, V0, V1, V30, V31, V28); // X3[0] = X3 * Wc (lo)   (Im : Re)

        V1_0 = Q6_Ww_vunpack_Vh(V27);                 // X2[3] : X2[2] real
        V27_26 = Q6_Ww_vunpack_Vh(V26);               // X0[3] : X0[2] real
        V3_2 = Q6_Ww_vunpack_Vh(V15);                 // X3[3] : X3[2] real
        V15_14 = Q6_Ww_vunpack_Vh(V14);               // X1[3] : X1[2] real
        V7_6 = Q6_Ww_vadd_WwWw_sat(V27_26, V15_14);   // Re(a[3] : a[2]) = Re(x0) + Re(x1)
        V27_26 = Q6_Ww_vsub_WwWw_sat(V27_26, V15_14); // Im(b[3] : b[2]) = Re(x0) - Re(x1)
        V15_14 = Q6_Ww_vadd_WwWw_sat(V1_0, V3_2);     // Re(c[3] : c[2]) = Re(x2) + Re(x3)
        V3_2 = Q6_Ww_vsub_WwWw_sat(V1_0, V3_2);       // Re(d[3] : d[2]) = Re(x2) - Re(x3)
        V1_0 = Q6_Ww_vadd_WwWw_sat(V7_6, V15_14);     // Re(X0[3]:X0[2]) = Re(a) + Re(c)
        V7_6 = Q6_Ww_vsub_WwWw_sat(V7_6, V15_14);     // Re(X2[3]:X2[2]) = Re(a) - Re(c)

        V23_22 = Q6_Ww_vunpack_Vh(V10);               // X0[3] : X0[2] imag
        V11_10 = Q6_Ww_vunpack_Vh(V11);               // X2[3] : X2[2] imag
        V19_18 = Q6_Ww_vunpack_Vh(V4);                // X1[3] : X1[2] imag
        V5_4 = Q6_Ww_vunpack_Vh(V5);                  // X3[3] : X3[2] imag
        V25_24 = Q6_Ww_vadd_WwWw_sat(V19_18, V23_22); // Im(a[3] : a[2]) = Im(x0) + Im(x1)
        V19_18 = Q6_Ww_vsub_WwWw_sat(V19_18, V23_22); // Re(b[3] : b[2]) = Im(x1) - Im(x0)
        V23_22 = Q6_Ww_vadd_WwWw_sat(V11_10, V5_4);   // Im(c[3] : c[2]) = Im(x2) + Im(x3)
        V5_4 = Q6_Ww_vsub_WwWw_sat(V11_10, V5_4);     // Im(d[3] : d[2]) = Im(x2) - Im(x3)
        V11_10 = Q6_Ww_vadd_WwWw_sat(V25_24, V23_22); // Im(X0[3]:X0[2]) = Im(a) + Im(c)
        V25_24 = Q6_Ww_vsub_WwWw_sat(V25_24, V23_22); // Im(X2[3]:X2[2]) = Im(a) - Im(c)
        V23_22 = Q6_Ww_vadd_WwWw_sat(V19_18, V3_2);   // Re(X1[3]:X1[2]) = Re(b) + Re(d)
        V3_2 = Q6_Ww_vsub_WwWw_sat(V19_18, V3_2);     // Re(X3[3]:X3[2]) = Re(b) - Re(d)
        V19_18 = Q6_Ww_vadd_WwWw_sat(V27_26, V5_4);   // Im(X1[3]:X1[2]) = Im(b) + Im(d)
        V5_4 = Q6_Ww_vsub_WwWw_sat(V27_26, V5_4);     // Im(X3[3]:X3[2]) = Im(b) - Im(d)

        // free up some registers
        scratch[6] = V0;                              // Re(X0[2])
        scratch[7] = V1;                              // Re(X0[3])

        // Apply twiddles
        V0 = twid[1];                           // Re(Wb[3:2]) (shuffled)
        V1 = twid[3];                           // Im(Wb[3:2]) (shuffled)
        V_CPLX_MULT_32_16(V7, V25, V0, V1, V26, V27, V28); // X2[3] = X2 * Wb (Im : Re)
        V0 = Q6_Vh_vshuffe_VhVh(V0,V0);         // copy Re(Wb[2]) to odd elements for 32x16 instruction
        V1 = Q6_Vh_vshuffe_VhVh(V1,V1);         // copy Im(Wb[2]) to odd elements for 32x16 instruction
        V_CPLX_MULT_32_16(V6, V24, V0, V1, V14, V15, V28); // X2[2] = X2 * Wb (Im : Re)
        V0 = twid[5];                           // Re(Wa[3:2]) (shuffled)
        V1 = twid[7];                           // Im(Wa[3:2]) (shuffled)
        V_CPLX_MULT_32_16(V23, V19, V0, V1, V6, V7, V28); // X1[3] = X1 * Wa (Im : Re)
        V0 = Q6_Vh_vshuffe_VhVh(V0,V0);         // copy Re(Wa[2]) to odd elements for 32x16 instruction
        V1 = Q6_Vh_vshuffe_VhVh(V1,V1);         // copy Im(Wa[2]) to odd elements for 32x16 instruction
        V_CPLX_MULT_32_16(V22, V18, V0, V1, V24, V25, V28); // X1[2] = X1 * Wa (Im : Re)
        V0 = twid[9];                           // Re(Wc[3:2]) (shuffled)
        V1 = twid[11];                          // Im(Wc[3:2]) (shuffled)
        V_CPLX_MULT_32_16(V3, V5, V0, V1, V22, V23, V28); // X3[3] = X3 * Wc (Im : Re)
        V0 = Q6_Vh_vshuffe_VhVh(V0,V0);         // copy Re(Wc[2]) to odd elements for 32x16 instruction
        V1 = Q6_Vh_vshuffe_VhVh(V1,V1);         // copy Im(Wc[2]) to odd elements for 32x16 instruction
        V_CPLX_MULT_32_16(V2, V4, V0, V1, V18, V19, V28); // X3[2] = X3 * Wc (Im : Re)

        // stage 4
        // rearrange for 3nd butterfly - shuffle X3 & X2 (64 bytes)
        V5_4 = Q6_W_vshuff_VVR(V22, V26, 64);       // real (X3[3] : X2[3])
        V23_22 = Q6_W_vshuff_VVR(V23, V27, 64);     // imag (X3[3] : X2[3])
        V27_26 = Q6_W_vshuff_VVR(V18, V14, 64);     // real (X3[2] : X2[2])
        V19_18 = Q6_W_vshuff_VVR(V19, V15, 64);     // imag (X3[2] : X2[2])
        V28 = scratch[4];
        V15_14 = Q6_W_vshuff_VVR(V20, V28, 64);     // real (X3[1] : X2[1])
        V29 = scratch[5];
        V21_20 = Q6_W_vshuff_VVR(V21, V29, 64);     // imag (X3[1] : X2[1])
        V3_2 = Q6_W_vshuff_VVR(V30, V8, 64);        // real (X3[0] : X2[0])
        V9_8 = Q6_W_vshuff_VVR(V31, V9, 64);        // imag (X3[0] : X2[0])
        // shuffle X1 & X0 (64 bytes)
        V28 = scratch[7];
        V31_30 = Q6_W_vshuff_VVR(V6, V28, 64);      // real (X1[3] : X0[3])
        V1_0 = Q6_W_vshuff_VVR(V7, V11, 64);     // imag (X1[3] : X0[3])
        V28 = scratch[6];
        V7_6 = Q6_W_vshuff_VVR(V24, V28, 64);     // real (X1[2] : X0[2])
        scratch[6] = V6;                             // real (X0[2])
        scratch[7] = V7;                             // real (X1[2])
        V25_24 = Q6_W_vshuff_VVR(V25, V10, 64);     // imag (X1[2] : X0[2])
        V28 = scratch[1];
        V29 = scratch[3];
        V11_10 = Q6_W_vshuff_VVR(V16, V28, 64);     // real (X1[1] : X0[1])
        V17_16 = Q6_W_vshuff_VVR(V17, V29, 64);     // imag (X1[1] : X0[1])
        V28 = scratch[0];
        V29 = scratch[2];
        V7_6 = Q6_W_vshuff_VVR(V12, V28, 64);        // real (X1[0] : X0[0])
        V13_12 = Q6_W_vshuff_VVR(V13, V29, 64);        // imag (X1[0] : X0[0])
        // Shuffling 128 bytes means just re-interpreting which vectors are which X values.
        //Re(x0[0]) --> Re(x0[0]) = V6
        //Re(x2[0]) --> Re(x0[1]) = V2
        //Re(x0[2]) --> Re(x0[2]) = scratch[6]
        //Re(x2[2]) --> Re(x0[3]) = V26
        //Re(x1[0]) --> Re(x1[0]) = V7
        //Re(x3[0]) --> Re(x1[1]) = V3
        //Re(x1[2]) --> Re(x1[2]) = scratch[7]
        //Re(x3[2]) --> Re(x1[3]) = V27
        //Re(x0[1]) --> Re(x2[0]) = V10
        //Re(x2[1]) --> Re(x2[1]) = V14
        //Re(x0[3]) --> Re(x2[2]) = V30
        //Re(x2[3]) --> Re(x2[3]) = V4
        //Re(x1[1]) --> Re(x3[0]) = V11
        //Re(x3[1]) --> Re(x3[1]) = V15
        //Re(x1[3]) --> Re(x3[2]) = V31
        //Re(x3[3]) --> Re(x3[3]) = V5

        //Im(x0[0]) --> Im(x0[0]) = V12
        //Im(x2[0]) --> Im(x0[1]) = V8
        //Im(x0[2]) --> Im(x0[2]) = V24
        //Im(x2[2]) --> Im(x0[3]) = V18
        //Im(x1[0]) --> Im(x1[0]) = V13
        //Im(x3[0]) --> Im(x1[1]) = V9
        //Im(x1[2]) --> Im(x1[2]) = V25
        //Im(x3[2]) --> Im(x1[3]) = V19
        //Im(x0[1]) --> Im(x2[0]) = V16
        //Im(x2[1]) --> Im(x2[1]) = V20
        //Im(x0[3]) --> Im(x2[2]) = V0  (scratch[2])
        //Im(x2[3]) --> Im(x2[3]) = V22 (scratch[0])
        //Im(x1[1]) --> Im(x3[0]) = V17
        //Im(x3[1]) --> Im(x3[1]) = V21
        //Im(x1[3]) --> Im(x3[2]) = V1  (scratch[3])
        //Im(x3[3]) --> Im(x3[3]) = V23 (scratch[1]) 

        scratch[0] = V22;
        scratch[1] = V23;
        scratch[2] = V0;
        scratch[3] = V1;

        V28 = Q6_Vw_vadd_VwVw_sat(V6, V7);      // Re(a[0]) = Re(x0[0]) + Re(x1[0])
        V29 = Q6_Vw_vadd_VwVw_sat(V2, V3);      // Re(a[1]) = Re(x0[1]) + Re(x1[1])
        V0 = Q6_Vw_vsub_VwVw_sat(V6, V7);       // Im(b[0]) = Re(x0[0]) - Re(x1[0])
        V1 = Q6_Vw_vsub_VwVw_sat(V2, V3);       // Im(b[1]) = Re(x0[1]) - Re(x1[1])

        V6 = scratch[6];
        V7 = scratch[7];
        V2 = Q6_Vw_vadd_VwVw_sat(V6, V7);     // Re(a[2]) = Re(x0[2]) + Re(x1[2])
        V3 = Q6_Vw_vadd_VwVw_sat(V26, V27);     // Re(a[3]) = Re(x0[3]) + Re(x1[3])
        V6 = Q6_Vw_vsub_VwVw_sat(V6, V7);     // Im(b[2]) = Re(x0[2]) - Re(x1[2])
        V7 = Q6_Vw_vsub_VwVw_sat(V26, V27);     // Im(b[3]) = Re(x0[3]) - Re(x1[3])

        V26 = Q6_Vw_vadd_VwVw_sat(V10, V11);     // Re(c[0]) = Re(x2[0]) + Re(x3[0])
        V27 = Q6_Vw_vadd_VwVw_sat(V14, V15);      // Re(c[1]) = Re(x2[1]) + Re(x3[1])
        V23_22 = Q6_Ww_vadd_WwWw_sat(V29_28, V27_26); // Re(X0[1]:X0[0]) = Re(a) + Re(c)
        scratch[4] = V22;                         // Re(X0[0])
        scratch[5] = V23;                         // Re(X0[1])
        V27_26 = Q6_Ww_vsub_WwWw_sat(V29_28, V27_26); // Re(X2[1]:X2[0]) = Re(a) - Re(c)
        V28 = Q6_Vw_vsub_VwVw_sat(V10, V11);     // Re(d[0]) = Re(x2[0]) - Re(x3[0]) 
        V29 = Q6_Vw_vsub_VwVw_sat(V14, V15);     // Re(d[1]) = Re(x2[1]) - Re(x3[1])
        V10 = Q6_Vw_vadd_VwVw_sat(V30, V31);      // Re(c[2]) = Re(x2[2]) + Re(x3[2])
        V11 = Q6_Vw_vadd_VwVw_sat(V4, V5);      // Re(c[3]) = Re(x2[3]) + Re(x3[3])
        V30 = Q6_Vw_vsub_VwVw_sat(V30, V31);     // Re(d[2]) = Re(x2[2]) - Re(x3[2]) 
        V31 = Q6_Vw_vsub_VwVw_sat(V4, V5);     // Re(d[3]) = Re(x2[3]) - Re(x3[3])
        V23_22 = Q6_Ww_vadd_WwWw_sat(V3_2, V11_10);// Re(X0[3]:X0[2]) = Re(a) + Re(c)
        scratch[6] = V22;                         // Re(X0[2])
        scratch[7] = V23;                         // Re(X0[3])
        V3_2 = Q6_Ww_vsub_WwWw_sat(V3_2, V11_10);// Re(X2[3]:X2[2]) = Re(a) - Re(c)

        V4 = Q6_Vw_vsub_VwVw_sat(V13, V12);      // Re(b[0]) = Im(x1[0]) - Im(x0[0])
        V5 = Q6_Vw_vsub_VwVw_sat(V9, V8);        // Re(b[1]) = Im(x1[1]) - Im(x0[1])
        V12 = Q6_Vw_vadd_VwVw_sat(V12, V13);      // Im(a[0]) = Im(x0[0]) + Im(x1[0])
        V13 = Q6_Vw_vadd_VwVw_sat(V8, V9);       // Im(a[1]) = Im(x0[1]) + Im(x1[1])
        V15_14 = Q6_Ww_vadd_WwWw_sat(V5_4, V29_28); // Re(X1[1]:X1[0]) = Re(b) + Re(d)
        V5_4 = Q6_Ww_vsub_WwWw_sat(V5_4, V29_28);   // Re(X3[1]:X3[0]) = Re(b) - Re(d)
        V28 = Q6_Vw_vsub_VwVw_sat(V25, V24);     // Re(b[2]) = Im(x1[2]) - Im(x0[2])
        V29 = Q6_Vw_vsub_VwVw_sat(V19, V18);      // Re(b[3]) = Im(x1[3]) - Im(x0[3])
        V8 = Q6_Vw_vadd_VwVw_sat(V24, V25);      // Im(a[2]) = Im(x0[2]) + Im(x1[2])
        V9 = Q6_Vw_vadd_VwVw_sat(V18, V19);       // Im(a[3]) = Im(x0[3]) + Im(x1[3])
        V23_22 = Q6_Ww_vadd_WwWw_sat(V29_28, V31_30); // Re(X1[3]:X1[2]) = Re(b) + Re(d)
        V29_28 = Q6_Ww_vsub_WwWw_sat(V29_28, V31_30);   // Re(X3[3]:X3[2]) = Re(b) - Re(d)

        V24 = Q6_Vw_vadd_VwVw_sat(V16, V17);        // Im(c[0]) = Im(x2[0]) + Im(x3[0])
        V25 = Q6_Vw_vadd_VwVw_sat(V20, V21);        // Im(c[1]) = Im(x2[1]) + Im(x3[1])
        V30 = Q6_Vw_vsub_VwVw_sat(V16, V17);     // Im(d[0]) = Im(x2[0]) - Im(x2[2]) 
        V31 = Q6_Vw_vsub_VwVw_sat(V20, V21);      // Im(d[1]) = Im(x2[1]) - Im(x2[3])
        V21_20 = Q6_Ww_vadd_WwWw_sat(V13_12, V25_24); // Im(X0[1]:X0[0]) = Im(a) + Im(c)
        V10 = scratch[2];    // Im(x2[2])
        V11 = scratch[3];    // Im(x3[2])
        V18 = scratch[0];    // Im(x2[3])
        V19 = scratch[1];    // Im(x3[3])
        scratch[0] = V20;                         // Im(X0[0])
        scratch[1] = V21;                         // Im(X0[1])
        V21_20 = Q6_Ww_vsub_WwWw_sat(V13_12, V25_24); // Im(X2[1]:X2[0]) = Im(a) - Im(c)
        V12 = Q6_Vw_vadd_VwVw_sat(V10, V11);        // Im(c[2]) = Im(x2[2]) + Im(x3[2])
        V13 = Q6_Vw_vadd_VwVw_sat(V18, V19);        // Im(c[3]) = Im(x2[3]) + Im(x3[3])
        V24 = Q6_Vw_vsub_VwVw_sat(V10, V11);        // Im(d[2]) = Im(x2[2]) - Im(x3[2])
        V25 = Q6_Vw_vsub_VwVw_sat(V18, V19);        // Im(d[3]) = Im(x3[1]) - Im(x3[3])
        
        V19_18 = Q6_Ww_vadd_WwWw_sat(V1_0, V31_30);     // Im(X1[1]:X1[0]) = Im(b) + Im(d)
        V1_0 = Q6_Ww_vsub_WwWw_sat(V1_0, V31_30);       // Im(X3[1]:X3[0]) = Im(b) - Im(d)
        V11_10 = Q6_Ww_vadd_WwWw_sat(V9_8, V13_12); // Im(X0[3]:X0[2]) = Im(a) + Im(c)
        scratch[2] = V10;                         // Im(X0[2])
        scratch[3] = V11;                         // Im(X0[3])
        V31_30 = Q6_Ww_vsub_WwWw_sat(V9_8, V13_12); // Im(X2[3]:X2[2]) = Im(a) - Im(c)
        V13_12 = Q6_Ww_vadd_WwWw_sat(V7_6, V25_24);     // Im(X1[3]:X1[2]) = Im(b) + Im(d)
        V7_6 = Q6_Ww_vsub_WwWw_sat(V7_6, V25_24);       // Im(X3[3]:X3[2]) = Im(b) - Im(d)

        // Apply twiddles
        
        // In this stage, we only need 2 twiddles and most are +/-1 or 0.
        // Re[Wb] = 0x7fff, 0x0000 (1, 0)
        // Im[Wb] = 0x0000, 0x8000 (1, -1)
        // Re[Wa] = 0x0000, 0xa57e (0, ..)
        // Im[Wa] = 0x8000, 0xa57e (-1, ..)
        // Re[Wc] = 0x0000, 0xa57e (0, ..)
        // Im[Wc] = 0x8000, 0x5a82 (-1, ..)
        // will treat 0x7fff as 1 and 0x8000 as -1, and avoid full 
        // multiplies where possible due to all the +/-1 and 0 twiddles.

        V8 = Q6_Vh_vsplat_R(0xa57e);        // twidA
        V9 = Q6_Vh_vsplat_R(0x5a82);        // twidB
        V16 = Q6_V_vzero();                 // 0
        V17 = Q6_V_vzero();                 // 0 
        
        // V26 = Re(X2[0]) = Re(X2[0]) * 1
        // V27 = Re(X2[1]) = Re(X2[1]) * 1
        // V20 = Im(X2[0]) = Im(X2[0]) * 1
        // V21 = Im(X2[1]) = Im(X2[1]) * 1
        // V18 = Re(X1[0]) = -Im(X1[0]) * -1
        // V19 = Re(X1[1]) = -Im(X1[1]) * -1
        V15_14 = Q6_Ww_vsub_WwWw_sat(V17_16, V15_14); // Im(X1[1:0]) = -Re(X1[1:0])
        // V0 = Re(X3[0]) = -Im(X3[0]) * -1
        // V1 = Re(X3[1]) = -Im(X3[1]) * -1
        V5_4 = Q6_Ww_vsub_WwWw_sat(V17_16, V5_4); // Im(X3[1:0]) = -Re(X3[1:0])
        // V30 = Re(X2[2]) = -Im(X2[2]) * -1
        // V31 = Re(X2[3]) = -Im(X2[3]) * -1
        V3_2 = Q6_Ww_vsub_WwWw_sat(V17_16, V3_2); // Im(X2[3:2]) = -Re(X2[3:2])
        V_CPLX_MULT_32_16(V22, V12, V8, V8, V16, V24, V25); // X1[2] = X1[2] * Wa (Im : Re)
        V_CPLX_MULT_32_16(V23, V13, V8, V8, V17, V25, V10); // X1[3] = X1[3] * Wa (Im : Re)
        V_CPLX_MULT_32_16(V28, V6, V8, V9, V12, V22, V10); // X3[2] = X3[2] * Wa (Im : Re)
        V_CPLX_MULT_32_16(V29, V7, V8, V9, V13, V23, V10); // X3[3] = X3[3] * Wa (Im : Re)

        // Final stage (radix-2)
        // re-interpret for shuffling
        // Re(X0[1] : X0[0]) --> Re(x0[1] : x0[0]) = scratch[5:4]
        // Re(X1[1] : X1[0]) --> Re(x0[3] : x0[2]) = V19_18
        // Re(X2[1] : X2[0]) --> Re(x0[5] : x0[4]) = V27_26
        // Re(X3[1] : X3[0]) --> Re(x0[7] : x0[6]) = V1_0
        // Re(X0[3] : X0[2]) --> Re(x1[1] : x1[0]) = scratch[7:6]
        // Re(X1[3] : X1[2]) --> Re(x1[3] : x1[2]) = V17_16
        // Re(X2[3] : X2[2]) --> Re(x1[5] : x1[4]) = V31_30
        // Re(X3[3] : X3[2]) --> Re(x1[7] : x1[6]) = V13_12
        // Im(X0[1] : X0[0]) --> Im(x0[1] : x0[0]) = scratch[1:0]
        // Im(X1[1] : X1[0]) --> Im(x0[3] : x0[2]) = 15_14
        // Im(X2[1] : X2[0]) --> Im(x0[5] : x0[4]) = V21_20
        // Im(X3[1] : X3[0]) --> Im(x0[7] : x0[6]) = V5_4
        // Im(X0[3] : X0[2]) --> Im(x1[1] : x1[0]) = scratch[3:2]
        // Im(X1[3] : X1[2]) --> Im(x1[3] : x1[2]) = V25_24
        // Im(X2[3] : X2[2]) --> Im(x1[5] : x1[4]) = V3_2
        // Im(X3[3] : X3[2]) --> Im(x1[7] : x1[6]) = V23_22
        
        
        V10 = scratch[4];      
        V11 = scratch[5];      
        V28 = scratch[6];      
        V29 = scratch[7];      
        V9_8 = Q6_Ww_vadd_WwWw_sat(V11_10, V29_28);     // Re(X0[1] : X0[0])  (scratch[1:0])
        V29_28 = Q6_Ww_vsub_WwWw_sat(V11_10, V29_28);   // Re(X1[1] : X1[0])
        V11_10 = Q6_Ww_vadd_WwWw_sat(V19_18, V17_16);   // Re(X0[3] : X0[2])
        V17_16 = Q6_Ww_vsub_WwWw_sat(V19_18, V17_16);   // Re(X1[3] : X1[2])
        V19_18 = Q6_Ww_vadd_WwWw_sat(V27_26, V31_30);   // Re(X0[5] : X0[4])
        V31_30 = Q6_Ww_vsub_WwWw_sat(V27_26, V31_30);   // Re(X1[5] : X1[4])
        V27_26 = Q6_Ww_vadd_WwWw_sat(V1_0, V13_12);     // Re(X0[7] : X0[6])
        V13_12 = Q6_Ww_vsub_WwWw_sat(V1_0, V13_12);     // Re(X1[7] : X1[6])
        V6 = scratch[0];
        V7 = scratch[1];
        scratch[0] = V8;                                // Re(X0[0])
        scratch[1] = V9;                                // Re(X0[1])
        V8 = scratch[2];
        V9 = scratch[3];

        V1_0 = Q6_Ww_vadd_WwWw_sat(V7_6, V9_8);         // Im(X0[1] : X0[0])
        V7_6 = Q6_Ww_vsub_WwWw_sat(V7_6, V9_8);         // Im(X1[1] : X1[0])        
        V9_8 = Q6_Ww_vadd_WwWw_sat(V15_14, V25_24);     // Im(X0[3] : X0[2]) (scratch[9:8])
        V15_14 = Q6_Ww_vsub_WwWw_sat(V15_14, V25_24);   // Im(X1[3] : X1[2]) (scratch[5:4])
        V25_24 = Q6_Ww_vadd_WwWw_sat(V21_20, V3_2);     // Im(X0[5] : X0[4]) (scratch[3:2])
        V3_2 = Q6_Ww_vsub_WwWw_sat(V21_20, V3_2);       // Im(X1[5] : X1[4]) (scratch[7:6])
        V21_20 = Q6_Ww_vadd_WwWw_sat(V5_4, V23_22);   // Im(X0[7] : X0[6])   
        V23_22 = Q6_Ww_vsub_WwWw_sat(V5_4, V23_22);   // Im(X1[7] : X1[6])

        // Last Butterfly
        // output rows in bit-reverse order.
        HVX_Vector* vout = (HVX_Vector*) ((uint8_t*)dptr->dst + brev1024[jobCount]*dptr->dstStride);

        HVX_Vector *wtwid = (HVX_Vector*)WTwiddles;
        HVX_VectorPred Q0 = Q6_Q_vsetq_R(4);    // for masking first word in a vector.
        scratch[2] = V24;
        scratch[3] = V25;
        scratch[4] = V14;
        scratch[5] = V15;
        scratch[6] = V2;
        scratch[7] = V3;
        V25 = *wtwid++;                       // Im(W) in odd halfwords
        V24 = Q6_Vh_vshuffe_VhVh(V25, V25);   // Re(W) in odd halfwords
        V14 = Q6_Vb_vsplat_R(0x7C);           // VDELTA control for reversing 4-byte words in a vector

        V3 = Q6_V_vdelta_VV(V13, V14);        // reverse(Re(X1[7]))
        V14 = Q6_V_vdelta_VV(V23, V14);       // reverse(Im(X1[7]))
        V13 = Q6_V_vlalign_VVI(V3, V3, 4);    // align(reverse(Re(X1[7])))
        V23 = Q6_V_vlalign_VVI(V14, V14, 4);  // align(reverse(Im(X1[7])))
        scratch[8] = V8;                                // Im(X0[2])
        scratch[9] = V9;                                // Im(X0[3])
        V8 = scratch[0];
        V2 = Q6_Vw_vadd_VwVw_sat(V8, V0);     // Re(X[0]) + Im(X[0])
        V9 = Q6_Vw_vsub_VwVw_sat(V8, V0);     // Re(X[0]) - Im(X[0])
        vout[16] = V9;                        // Re(X[N/2] = Re(X[0]) - Im(X[0])
        V9 = Q6_Vw_vavg_VwVw(V8, V13);        // Re(X) = AVG(Re(X0[0]), reverse(Re(X1[7])))
        V13 = Q6_Vw_vnavg_VwVw(V8, V13);      // Re(Y) = AVG(Re(X0[0]), -reverse(Re(X1[7])))
        V8 = Q6_Vw_vnavg_VwVw(V0, V23);       // Im(X) = AVG(Im(X0[0]), -reverse(Im(X1[7])))
        V23 = Q6_Vw_vavg_VwVw(V0, V23);       // Im(Y) = AVG(Im(X0[0]), reverse(Im(X1[7])))
        V_CPLX_MULT_32_16_2(V13, V23, V24, V25, V0, V15);  // Y = L_cmult32x16(Y, Wtwid[i]);
        V13 = Q6_Vw_vsub_VwVw_sat(V9, V0);    // Re(X) = Re(X) - Re(Y)
        V23 = Q6_Vw_vsub_VwVw_sat(V8, V15);   // Im(X) = Im(X) - Im(Y)
        V13 = Q6_V_vmux_QVV(Q0, V2, V13);     // replace Re(X) first word with input Re(X[0]) + Im(X[0])
        V2 = Q6_V_vzero();
        V23 = Q6_V_vmux_QVV(Q0, V2, V23);     // replace Im(X) first word with 0
        V9 = Q6_Vw_vadd_VwVw_sat(V9, V0);    // Re(X) = Re(X) + Re(Y)
        V0 = Q6_Vb_vsplat_R(0x7C);            // VDELTA control for reversing 4-byte words in a vector
        V8 = Q6_Vw_vadd_VwVw_sat(V8, V15);   // Im(X) = Im(X) + Im(Y)
        V8 = Q6_Vw_vsub_VwVw_sat(V2, V8);   // Im(X) = -Im(X)
        V8 = Q6_V_vdelta_VV(V8, V0);        // reverse(Im(X))
        V9 = Q6_V_vdelta_VV(V9, V0);        // reverse(Re(X))

        vout[0]  = V13;                       // store Re(X[0])
        vout[32] = V23;                       // store Im(X[0])

        V25 = *wtwid++;                       // Im(W) in odd halfwords
        V24 = Q6_Vh_vshuffe_VhVh(V25, V25);   // Re(W) in odd halfwords
        V12 = Q6_V_vdelta_VV(V12, V0);        // reverse(Re(X1[6]))
        V22 = Q6_V_vdelta_VV(V22, V0);        // reverse(Im(X1[6]))
        V2 = Q6_V_vlalign_VVI(V12, V3, 4);    // align(reverse(Re(X1[6])))
        V3 = Q6_V_vlalign_VVI(V22, V14, 4);   // align(reverse(Im(X1[6])))
        V23 = scratch[1];
        V14 = Q6_Vw_vavg_VwVw(V23, V2);       // Re(X) = AVG(Re(X0[1]), reverse(Re(X1[6])))
        V23 = Q6_Vw_vnavg_VwVw(V23, V2);      // Re(Y) = AVG(Re(X0[1]), -reverse(Re(X1[6])))
        V15 = Q6_Vw_vnavg_VwVw(V1, V3);       // Im(X) = AVG(Im(X0[1]), -reverse(Im(X1[6])))
        V1 = Q6_Vw_vavg_VwVw(V1, V3);         // Im(Y) = AVG(Im(X0[1]), reverse(Im(X1[6])))
        V_CPLX_MULT_32_16_2(V23, V1, V24, V25, V2, V3);  // Y = L_cmult32x16(Y, Wtwid[i]);
        V25_24 = Q6_Ww_vsub_WwWw_sat(V15_14, V3_2);        // X = X - Y  (Im : Re)
        vout[1]  = V24;                       // store Re(X)
        vout[33] = V25;                       // store Im(X)
        V25_24 = Q6_Ww_vadd_WwWw_sat(V15_14, V3_2);        // X = X + Y  (Im : Re)
        V2 = Q6_V_vzero();
        V25 = Q6_Vw_vsub_VwVw_sat(V2, V25);   // Im(X) = -Im(X)
        V24 = Q6_V_vdelta_VV(V24, V0);        // reverse(Re(X))
        V25 = Q6_V_vdelta_VV(V25, V0);        // reverse(Im(X))
        V9 = Q6_V_vlalign_VVI(V9, V24, 4);
        V8 = Q6_V_vlalign_VVI(V8, V25, 4);
        vout[15] = V9;                        // store Re(X[15])
        vout[47] = V8;                        // store Im(X[15])

        V9 = *wtwid++;                       // Im(W) in odd halfwords
        V8 = Q6_Vh_vshuffe_VhVh(V9, V9);      // Re(W) in odd halfwords
        V14 = Q6_V_vdelta_VV(V31, V0);        // reverse(Re(X1[5]))
        V15 = scratch[7];
        V15 = Q6_V_vdelta_VV(V15, V0);        // reverse(Im(X1[5]))
        V12 = Q6_V_vlalign_VVI(V14, V12, 4);    // align(reverse(Re(X1[5])))
        V13 = Q6_V_vlalign_VVI(V15, V22, 4);   // align(reverse(Im(X1[5])))
        V22 = Q6_Vw_vavg_VwVw(V10, V12);       // Re(X) = AVG(Re(X0[2]), reverse(Re(X1[5])))
        V3 = Q6_Vw_vnavg_VwVw(V10, V12);      // Re(Y) = AVG(Re(X0[2]), -reverse(Re(X1[5])))
        V31 = scratch[8];
        V23 = Q6_Vw_vnavg_VwVw(V31, V13);       // Im(X) = AVG(Im(X0[2]), -reverse(Im(X1[5])))
        V31 = Q6_Vw_vavg_VwVw(V31, V13);         // Im(Y) = AVG(Im(X0[2]), reverse(Im(X1[5])))
        V_CPLX_MULT_32_16_2(V3, V31, V8, V9, V12, V13);  // Y = L_cmult32x16(Y, Wtwid[i]);
        V9_8 = Q6_Ww_vsub_WwWw_sat(V23_22, V13_12);        // X = X - Y  (Im : Re)
        vout[2]  = V8;                       // store Re(X)
        vout[34] = V9;                       // store Im(X)
        V9_8 = Q6_Ww_vadd_WwWw_sat(V23_22, V13_12);        // X = X + Y  (Im : Re)
        V9 = Q6_Vw_vsub_VwVw_sat(V2, V9);   // Im(X) = -Im(X)
        V8 = Q6_V_vdelta_VV(V8, V0);        // reverse(Re(X))
        V9 = Q6_V_vdelta_VV(V9, V0);        // reverse(Im(X))
        V24 = Q6_V_vlalign_VVI(V24, V8, 4);
        V25 = Q6_V_vlalign_VVI(V25, V9, 4);
        vout[14] = V24;                        // store Re(X[14])
        vout[46] = V25;                        // store Im(X[14])

        V25 = *wtwid++;                       // Im(W) in odd halfwords
        V24 = Q6_Vh_vshuffe_VhVh(V25, V25);      // Re(W) in odd halfwords
        V12 = Q6_V_vdelta_VV(V30, V0);        // reverse(Re(X1[4]))
        V13 = scratch[6];
        V13 = Q6_V_vdelta_VV(V13, V0);        // reverse(Im(X1[4]))
        V14 = Q6_V_vlalign_VVI(V12, V14, 4);    // align(reverse(Re(X1[4])))
        V15 = Q6_V_vlalign_VVI(V13, V15, 4);   // align(reverse(Im(X1[4])))
        V30 = Q6_Vw_vavg_VwVw(V11, V14);       // Re(X) = AVG(Re(X0[3]), reverse(Re(X1[4])))
        V22 = Q6_Vw_vnavg_VwVw(V11, V14);      // Re(Y) = AVG(Re(X0[3]), -reverse(Re(X1[4])))
        V14 = scratch[9];
        V31 = Q6_Vw_vnavg_VwVw(V14, V15);       // Im(X) = AVG(Im(X0[3]), -reverse(Im(X1[4])))
        V23 = Q6_Vw_vavg_VwVw(V14, V15);         // Im(Y) = AVG(Im(X0[3]), reverse(Im(X1[4])))
        V_CPLX_MULT_32_16_2(V22, V23, V24, V25, V14, V15);  // Y = L_cmult32x16(Y, Wtwid[i]);
        V25_24 = Q6_Ww_vsub_WwWw_sat(V31_30, V15_14);        // X = X - Y  (Im : Re)
        vout[3]  = V24;                       // store Re(X)
        vout[35] = V25;                       // store Im(X)
        V25_24 = Q6_Ww_vadd_WwWw_sat(V31_30, V15_14);        // X = X + Y  (Im : Re)
        V25 = Q6_Vw_vsub_VwVw_sat(V2, V25);   // Im(X) = -Im(X)
        V24 = Q6_V_vdelta_VV(V24, V0);        // reverse(Re(X))
        V25 = Q6_V_vdelta_VV(V25, V0);        // reverse(Im(X))
        V8 = Q6_V_vlalign_VVI(V8, V24, 4);
        V9 = Q6_V_vlalign_VVI(V9, V25, 4);
        vout[13] = V8;                        // store Re(X[13])
        vout[45] = V9;                        // store Im(X[13])

        V9 = *wtwid++;                       // Im(W) in odd halfwords
        V8 = Q6_Vh_vshuffe_VhVh(V9, V9);      // Re(W) in odd halfwords
        V22 = Q6_V_vdelta_VV(V17, V0);        // reverse(Re(X1[3]))
        V23 = scratch[5];
        V23 = Q6_V_vdelta_VV(V23, V0);        // reverse(Im(X1[3]))
        V14 = Q6_V_vlalign_VVI(V22, V12, 4);    // align(reverse(Re(X1[3])))
        V15 = Q6_V_vlalign_VVI(V23, V13, 4);   // align(reverse(Im(X1[3])))
        V30 = Q6_Vw_vavg_VwVw(V18, V14);       // Re(X) = AVG(Re(X0[4]), reverse(Re(X1[3])))
        V12 = Q6_Vw_vnavg_VwVw(V18, V14);      // Re(Y) = AVG(Re(X0[4]), -reverse(Re(X1[3])))
        V18 = scratch[2];
        V31 = Q6_Vw_vnavg_VwVw(V18, V15);       // Im(X) = AVG(Im(X0[4]), -reverse(Im(X1[3])))
        V13 = Q6_Vw_vavg_VwVw(V18, V15);         // Im(Y) = AVG(Im(X0[4]), reverse(Im(X1[3])))
        V_CPLX_MULT_32_16_2(V12, V13, V8, V9, V14, V15);  // Y = L_cmult32x16(Y, Wtwid[i]);
        V9_8 = Q6_Ww_vsub_WwWw_sat(V31_30, V15_14);        // X = X - Y  (Im : Re)
        vout[4]  = V8;                       // store Re(X)
        vout[36] = V9;                       // store Im(X)
        V9_8 = Q6_Ww_vadd_WwWw_sat(V31_30, V15_14);        // X = X + Y  (Im : Re)
        V9 = Q6_Vw_vsub_VwVw_sat(V2, V9);   // Im(X) = -Im(X)
        V8 = Q6_V_vdelta_VV(V8, V0);        // reverse(Re(X))
        V9 = Q6_V_vdelta_VV(V9, V0);        // reverse(Im(X))
        V24 = Q6_V_vlalign_VVI(V24, V8, 4);
        V25 = Q6_V_vlalign_VVI(V25, V9, 4);
        vout[12] = V24;                        // store Re(X[12])
        vout[44] = V25;                        // store Im(X[12])

        V25 = *wtwid++;                       // Im(W) in odd halfwords
        V24 = Q6_Vh_vshuffe_VhVh(V25, V25);      // Re(W) in odd halfwords
        V12 = Q6_V_vdelta_VV(V16, V0);        // reverse(Re(X1[2]))
        V13 = scratch[4];
        V13 = Q6_V_vdelta_VV(V13, V0);        // reverse(Im(X1[2]))
        V14 = Q6_V_vlalign_VVI(V12, V22, 4);    // align(reverse(Re(X1[2])))
        V15 = Q6_V_vlalign_VVI(V13, V23, 4);   // align(reverse(Im(X1[2])))
        V30 = Q6_Vw_vavg_VwVw(V19, V14);       // Re(X) = AVG(Re(X0[5]), reverse(Re(X1[2])))
        V22 = Q6_Vw_vnavg_VwVw(V19, V14);      // Re(Y) = AVG(Re(X0[5]), -reverse(Re(X1[2])))
        V18 = scratch[3];
        V31 = Q6_Vw_vnavg_VwVw(V18, V15);       // Im(X) = AVG(Im(X0[5]), -reverse(Im(X1[2])))
        V23 = Q6_Vw_vavg_VwVw(V18, V15);         // Im(Y) = AVG(Im(X0[5]), reverse(Im(X1[2])))
        V_CPLX_MULT_32_16_2(V22, V23, V24, V25, V14, V15);  // Y = L_cmult32x16(Y, Wtwid[i]);
        V25_24 = Q6_Ww_vsub_WwWw_sat(V31_30, V15_14);        // X = X - Y  (Im : Re)
        vout[5]  = V24;                       // store Re(X)
        vout[37] = V25;                       // store Im(X)
        V25_24 = Q6_Ww_vadd_WwWw_sat(V31_30, V15_14);        // X = X + Y  (Im : Re)
        V25 = Q6_Vw_vsub_VwVw_sat(V2, V25);   // Im(X) = -Im(X)
        V24 = Q6_V_vdelta_VV(V24, V0);        // reverse(Re(X))
        V25 = Q6_V_vdelta_VV(V25, V0);        // reverse(Im(X))
        V8 = Q6_V_vlalign_VVI(V8, V24, 4);
        V9 = Q6_V_vlalign_VVI(V9, V25, 4);
        vout[11] = V8;                        // store Re(X[11])
        vout[43] = V9;                        // store Im(X[11])

        V9 = *wtwid++;                       // Im(W) in odd halfwords
        V8 = Q6_Vh_vshuffe_VhVh(V9, V9);      // Re(W) in odd halfwords
        V22 = Q6_V_vdelta_VV(V29, V0);        // reverse(Re(X1[1]))
        V23 = Q6_V_vdelta_VV(V7, V0);        // reverse(Im(X1[1]))
        V14 = Q6_V_vlalign_VVI(V22, V12, 4);    // align(reverse(Re(X1[1])))
        V15 = Q6_V_vlalign_VVI(V23, V13, 4);   // align(reverse(Im(X1[1])))
        V30 = Q6_Vw_vavg_VwVw(V26, V14);       // Re(X) = AVG(Re(X0[6]), reverse(Re(X1[1])))
        V12 = Q6_Vw_vnavg_VwVw(V26, V14);      // Re(Y) = AVG(Re(X0[6]), -reverse(Re(X1[1])))
        V31 = Q6_Vw_vnavg_VwVw(V20, V15);       // Im(X) = AVG(Im(X0[6]), -reverse(Im(X1[1])))
        V13 = Q6_Vw_vavg_VwVw(V20, V15);         // Im(Y) = AVG(Im(X0[6]), reverse(Im(X1[1])))
        V_CPLX_MULT_32_16_2(V12, V13, V8, V9, V14, V15);  // Y = L_cmult32x16(Y, Wtwid[i]);
        V9_8 = Q6_Ww_vsub_WwWw_sat(V31_30, V15_14);        // X = X - Y  (Im : Re)
        vout[6]  = V8;                       // store Re(X)
        vout[38] = V9;                       // store Im(X)
        V9_8 = Q6_Ww_vadd_WwWw_sat(V31_30, V15_14);        // X = X + Y  (Im : Re)
        V9 = Q6_Vw_vsub_VwVw_sat(V2, V9);   // Im(X) = -Im(X)
        V8 = Q6_V_vdelta_VV(V8, V0);        // reverse(Re(X))
        V9 = Q6_V_vdelta_VV(V9, V0);        // reverse(Im(X))
        V24 = Q6_V_vlalign_VVI(V24, V8, 4);
        V25 = Q6_V_vlalign_VVI(V25, V9, 4);
        vout[10] = V24;                        // store Re(X[10])
        vout[42] = V25;                        // store Im(X[10])

        V25 = *wtwid++;                       // Im(W) in odd halfwords
        V24 = Q6_Vh_vshuffe_VhVh(V25, V25);      // Re(W) in odd halfwords
        V12 = Q6_V_vdelta_VV(V28, V0);        // reverse(Re(X1[0]))
        V13 = Q6_V_vdelta_VV(V6, V0);        // reverse(Im(X1[0]))
        V14 = Q6_V_vlalign_VVI(V12, V22, 4);    // align(reverse(Re(X1[0])))
        V15 = Q6_V_vlalign_VVI(V13, V23, 4);   // align(reverse(Im(X1[0])))
        V30 = Q6_Vw_vavg_VwVw(V27, V14);       // Re(X) = AVG(Re(X0[7]), reverse(Re(X1[0])))
        V22 = Q6_Vw_vnavg_VwVw(V27, V14);      // Re(Y) = AVG(Re(X0[7]), -reverse(Re(X1[0])))
        V31 = Q6_Vw_vnavg_VwVw(V21, V15);       // Im(X) = AVG(Im(X0[7]), -reverse(Im(X1[0])))
        V23 = Q6_Vw_vavg_VwVw(V21, V15);         // Im(Y) = AVG(Im(X0[7]), reverse(Im(X1[0])))
        V_CPLX_MULT_32_16_2(V22, V23, V24, V25, V14, V15);  // Y = L_cmult32x16(Y, Wtwid[i]);
        V25_24 = Q6_Ww_vsub_WwWw_sat(V31_30, V15_14);        // X = X - Y  (Im : Re)
        vout[7]  = V24;                       // store Re(X)
        vout[39] = V25;                       // store Im(X)
        V25_24 = Q6_Ww_vadd_WwWw_sat(V31_30, V15_14);        // X = X + Y  (Im : Re)
        V25 = Q6_Vw_vsub_VwVw_sat(V2, V25);   // Im(X) = -Im(X)
        V24 = Q6_V_vdelta_VV(V24, V0);        // reverse(Re(X))
        V25 = Q6_V_vdelta_VV(V25, V0);        // reverse(Im(X))
        V8 = Q6_V_vlalign_VVI(V8, V24, 4);
        V9 = Q6_V_vlalign_VVI(V9, V25, 4);
        vout[9] = V8;                        // store Re(X[9])
        vout[41] = V9;                        // store Im(X[9])
        
        V9 = *wtwid++;                       // Im(W) in odd halfwords
        V8 = Q6_Vh_vshuffe_VhVh(V9, V9);      // Re(W) in odd halfwords
        V22 = Q6_V_vdelta_VV(V27, V0);        // reverse(Re(X0[7]))
        V23 = Q6_V_vdelta_VV(V21, V0);        // reverse(Im(X0[7]))
        V14 = Q6_V_vlalign_VVI(V22, V12, 4);    // align(reverse(Re(X0[7])))
        V15 = Q6_V_vlalign_VVI(V23, V13, 4);   // align(reverse(Im(X0[7])))
        V30 = Q6_Vw_vavg_VwVw(V28, V14);       // Re(X) = AVG(Re(X1[0]), reverse(Re(X0[7])))
        V12 = Q6_Vw_vnavg_VwVw(V28, V14);      // Re(Y) = AVG(Re(X1[0]), -reverse(Re(X0[7])))
        V31 = Q6_Vw_vnavg_VwVw(V6, V15);       // Im(X) = AVG(Im(X1[0]), -reverse(Im(X0[1])))
        V13 = Q6_Vw_vavg_VwVw(V6, V15);         // Im(Y) = AVG(Im(X1[0]), reverse(Im(X0[1])))
        V_CPLX_MULT_32_16_2(V12, V13, V8, V9, V14, V15);  // Y = L_cmult32x16(Y, Wtwid[i]);
        V9_8 = Q6_Ww_vadd_WwWw_sat(V31_30, V15_14);        // X = X + Y  (Im : Re)
        V9 = Q6_Vw_vsub_VwVw_sat(V2, V9);   // Im(X) = -Im(X)
        V8 = Q6_V_vdelta_VV(V8, V0);        // reverse(Re(X))
        V9 = Q6_V_vdelta_VV(V9, V0);        // reverse(Im(X))
        V24 = Q6_V_vlalign_VVI(V24, V8, 4);
        V25 = Q6_V_vlalign_VVI(V25, V9, 4);
        vout[8] = V24;                        // store Re(X[9])
        vout[40] = V25;                        // store Im(X[9])
        vout[48] = V2;                          // Im(X[N/2]) = 0
    }
#if (__HEXAGON_ARCH__ < 65)
    dspCV_hvx_unlock();
#endif
    dspCV_worker_pool_synctoken_jobdone(dptr->token); // release multi-threading job token
}

// callback for multi-threaded horizontal FFT's
// Assumes exactly 2 worker threads!
static void fcvFFTu8_vert_callback(void* data)
{
    fcvFFTu8_callback_t *dptr = (fcvFFTu8_callback_t*) data;

#if (__HEXAGON_ARCH__ < 65)
    int lockResult = dspCV_hvx_lock(DSPCV_HVX_MODE_128B, 1);
    if (128 != lockResult) 
    {
        FARF(HIGH,"Error, could not acquire HVX!");
        dspCV_worker_pool_synctoken_jobdone(dptr->token); // release multi-threading job token
        return;
    }
#endif

    //constants for FFTSIZE
    const uint32_t srcWidth = FFTSIZE;
    const uint32_t srcHeight = FFTSIZE;
    const uint32_t vecColumns = (srcWidth / 2) / (VLEN / sizeof(int32_t));
    const uint32_t rowsPerWorker = srcHeight / 2;

    const uint32_t dstStride = dptr->dstStride;
    unsigned int workerCount = dspCV_atomic_inc_return(&(dptr->workerCount)) - 1;

    // Declare HVX register aliases
    DECLARE_HVX_REGISTERS;

    V31 = Q6_Vb_vsplat_R(0x7C);           // VDELTA control for reversing 4-byte words in a vector
    V30 = Q6_V_vzero();
    const HVX_VectorPred Q0 = Q6_Q_vsetq_R(4);    // for masking first word in a vector.

    // prefetch real & imaginary by using stride/2 and height*2
    const uint64_t L2FETCH_REGISTER = (1ULL <<48) | ((uint64_t)dptr->dstStride<<(32-1)) | ((uint64_t)128<<16) | 8;
    for (int col = 0; col <= vecColumns; col++)
    {
        HVX_Vector *vsrc = (HVX_Vector*)(dptr->dst + (workerCount * rowsPerWorker) * dstStride / sizeof(dptr->dst[0])) + col;
        HVX_Vector *vdst = dptr->scratchBuf + 2*(workerCount * rowsPerWorker);
        uint32_t srcStrideVec = dptr->dstStride / VLEN;
        uint32_t imagOffset = 32;
        
        // do all but the last radix-4 stages in a loop. One thread on top half, other thread on bottom.
        for (int k1 = 1, k2 = srcHeight / 4; k1 * 4 < rowsPerWorker; k1 = k1 * 4, k2 = k2 / 4)
        {
            if (k1==1) L2FETCH(vsrc, L2FETCH_REGISTER);
            const int16_t *twid = (workerCount ? Twiddles + k2 / 2 : Twiddles);
            const int16_t *WbReal = twid, *WbImag = twid + 256;
            const int16_t *WaReal = twid + 512, *WaImag = twid + 768;
            const int16_t *WcReal = twid + 1024, *WcImag = twid + 1280;
            for (int i = 0; i < k2 / 2; i++)
            {
                V20 = Q6_Vh_vsplat_R(*WbReal++);
                V21 = Q6_Vh_vsplat_R(*WbImag++);
                V22 = Q6_Vh_vsplat_R(*WaReal++);
                V23 = Q6_Vh_vsplat_R(*WaImag++);
                V24 = Q6_Vh_vsplat_R(*WcReal++);
                V25 = Q6_Vh_vsplat_R(*WcImag++);

                HVX_Vector *vsrc0 = vsrc + i * 4 * k1 * srcStrideVec;
                HVX_Vector *vsrc1 = vsrc0 + k1 * srcStrideVec;
                HVX_Vector *vsrc2 = vsrc1 + k1 * srcStrideVec;
                HVX_Vector *vsrc3 = vsrc2 + k1 * srcStrideVec;
                
                HVX_Vector *vdst0 = vdst + i * 4 * k1 * 2;
                HVX_Vector *vdst1 = vdst0 + k1 * 2;
                HVX_Vector *vdst2 = vdst1 + k1 * 2;
                HVX_Vector *vdst3 = vdst2 + k1 * 2;
                for (int j = 0; j < k1; j++)
                {
                    
                    V0 = vsrc0[0];             // Re(x[0])
                    V1 = vsrc0[imagOffset];    // Im(x[0])
                    V2 = vsrc1[0];             // Re(x[1])
                    V3 = vsrc1[imagOffset];    // Im(x[1])
                    V4 = vsrc2[0];             // Re(x[2])
                    V5 = vsrc2[imagOffset];    // Im(x[2])
                    V6 = vsrc3[0];             // Re(x[3])
                    V7 = vsrc3[imagOffset];    // Im(x[3])
                    
                    vsrc0 += srcStrideVec;
                    vsrc1 += srcStrideVec;
                    vsrc2 += srcStrideVec;
                    vsrc3 += srcStrideVec;

                    V9_8 = Q6_Ww_vadd_WwWw_sat(V1_0, V3_2);        // a = x[0] + x[1]  (Im : Re)
                    V10 = Q6_Vw_vsub_VwVw_sat(V3, V1);              // b = j * (x[0] - x[1])  (Im : Re)
                    V11 = Q6_Vw_vsub_VwVw_sat(V0, V2);              // b = j * (x[0] - x[1])  (Im : Re)

                    V3_2 = Q6_Ww_vadd_WwWw_sat(V5_4, V7_6);        // c = x[2] + x[3]  (Im : Re)
                    V5_4 = Q6_Ww_vsub_WwWw_sat(V5_4, V7_6);        // d = x[2] - x[3]  (Im : Re)
                    
                    V7_6 = Q6_Ww_vadd_WwWw_sat(V9_8, V3_2);        // x[0] = a + c  (Im : Re)
                    *vdst0++ = V6;
                    *vdst0++ = V7;

                    V3_2 = Q6_Ww_vsub_WwWw_sat(V9_8, V3_2);        // x[2] = a - c  (Im : Re)
                    V9_8 = Q6_Ww_vadd_WwWw_sat(V11_10, V5_4);        // x[1] = b + d  (Im : Re)
                    V1_0 = Q6_Ww_vsub_WwWw_sat(V11_10, V5_4);        // x[3] = b - d  (Im : Re)

                    V_CPLX_MULT_32_16_2(V8, V9, V22, V23, V4, V5); // x[1] * Wa
                    *vdst1++ = V4;
                    *vdst1++ = V5;
                    
                    V_CPLX_MULT_32_16_2(V2, V3, V20, V21, V8, V9); // x[2] * Wb
                    *vdst2++ = V8;
                    *vdst2++ = V9;
                    
                    V_CPLX_MULT_32_16_2(V0, V1, V24, V25, V2, V3); // x[3] * Wc
                    *vdst3++ = V2;
                    *vdst3++ = V3;
                }
            }
            // for the rest of the iterations, perform in-place on the scratch (hopefully VTCM)
            vsrc = vdst;
            srcStrideVec = 2;
            imagOffset = 1;
            
        }
        // sync worker threads before starting last butterfly.
        qurt_barrier_wait(&dptr->barrier);
        vsrc = dptr->scratchBuf;
        vdst = (HVX_Vector*)(dptr->dst);
        const uint32_t dstStrideVec = dptr->dstStride / VLEN;
        const uint32_t srcOffsetVec = (srcHeight / 4) * srcStrideVec;
        const uint32_t dstOffsetVec = (srcHeight / 4) * dstStrideVec;

        // handle even rows on worker 0 and odd on worker 1
        HVX_Vector *vsrc0 = vsrc + workerCount * srcStrideVec;
        HVX_Vector *vsrc1 = vsrc0 + srcOffsetVec;
        HVX_Vector *vsrc2 = vsrc1 + srcOffsetVec;
        HVX_Vector *vsrc3 = vsrc2 + srcOffsetVec;
        
        HVX_Vector *vdst0 = vdst + workerCount * dstStrideVec;
        HVX_Vector *vdst1 = vdst0 + dstOffsetVec;
        HVX_Vector *vdst2 = vdst1 + dstOffsetVec;
        HVX_Vector *vdst3 = vdst2 + dstOffsetVec;
        
        HVX_Vector *vdst0Conj = vdst + (srcHeight - workerCount) * dstStrideVec;
        HVX_Vector *vdst1Conj = vdst0Conj - dstOffsetVec;
        HVX_Vector *vdst2Conj = vdst1Conj - dstOffsetVec;
        HVX_Vector *vdst3Conj = vdst2Conj - dstOffsetVec;

        if (col == 0) for (int j = workerCount; j < srcHeight / 4; j+=2)
        {
            V0 = vsrc0[0];             // Re(x[0])
            V1 = vsrc0[imagOffset];    // Im(x[0])
            V2 = vsrc1[0];             // Re(x[1])
            V3 = vsrc1[imagOffset];    // Im(x[1])
            V4 = vsrc2[0];             // Re(x[2])
            V5 = vsrc2[imagOffset];    // Im(x[2])
            V6 = vsrc3[0];             // Re(x[3])
            V7 = vsrc3[imagOffset];    // Im(x[3])
         
            vsrc0 += 2*srcStrideVec;
            vsrc1 += 2*srcStrideVec;
            vsrc2 += 2*srcStrideVec;
            vsrc3 += 2*srcStrideVec;

            V9_8 = Q6_Ww_vadd_WwWw_sat(V1_0, V3_2);        // a = x[0] + x[1]  (Im : Re)
            V11_10 = Q6_Ww_vsub_WwWw_sat(V1_0, V3_2);      // b = x[0] - x[1]  (Im : Re)

            V3_2 = Q6_Ww_vadd_WwWw_sat(V5_4, V7_6);        // c = x[2] + x[3]  (Im : Re)
            V0 = Q6_Vw_vsub_VwVw_sat(V7, V5);              // d = j * (x[2] - x[3])  (Im : Re)
            V1 = Q6_Vw_vsub_VwVw_sat(V4, V6);              // d = j * (x[2] - x[3])  (Im : Re)
            
            V7_6 = Q6_Ww_vadd_WwWw_sat(V9_8, V3_2);        // x[0] = a + c  (Im : Re)
            vdst0[col] = V6;
            vdst0[col+32] = V7;
            // conjugate symmetry
            V7 = Q6_Vw_vsub_VwVw_sat(V30, V7);   // Im(X) = -Im(X)
            V6 = Q6_V_vdelta_VV(V6, V31);       // reverse(Re(X))
            V7 = Q6_V_vdelta_VV(V7, V31);       // reverse(-Im(X))
            HVX_Vector *v0ptr = (j == 0 ? vdst0 : vdst0Conj);
            V6 = Q6_V_vlalign_VVI(V6, V30, 4);
            V7 = Q6_V_vlalign_VVI(V7, V30, 4);
            v0ptr[31] = V6;
            v0ptr[63] = V7;
            vdst0 += 2*dstStrideVec;
            vdst0Conj -= 2*dstStrideVec;


            V3_2 = Q6_Ww_vsub_WwWw_sat(V9_8, V3_2);        // x[2] = a - c  (Im : Re)
            vdst2[col] = V2;
            vdst2[col+32] = V3;
            // conjugate symmetry
            V3 = Q6_Vw_vsub_VwVw_sat(V30, V3);   // Im(X) = -Im(X)
            V2 = Q6_V_vdelta_VV(V2, V31);       // reverse(Re(X))
            V3 = Q6_V_vdelta_VV(V3, V31);       // reverse(-Im(X))
            V2 = Q6_V_vlalign_VVI(V2, V30, 4);
            V3 = Q6_V_vlalign_VVI(V3, V30, 4);
            vdst2Conj[31] = V2;
            vdst2Conj[63] = V3;
            vdst2 += 2*dstStrideVec;
            vdst2Conj -= 2*dstStrideVec;

            V9_8 = Q6_Ww_vsub_WwWw_sat(V11_10, V1_0);        // x[1] = b - d  (Im : Re)
            vdst1[col] = V8;
            vdst1[col+32] = V9;
            // conjugate symmetry
            V9 = Q6_Vw_vsub_VwVw_sat(V30, V9);   // Im(X) = -Im(X)
            V8 = Q6_V_vdelta_VV(V8, V31);       // reverse(Re(X))
            V9 = Q6_V_vdelta_VV(V9, V31);       // reverse(-Im(X))
            V8 = Q6_V_vlalign_VVI(V8, V30, 4);
            V9 = Q6_V_vlalign_VVI(V9, V30, 4);
            vdst1Conj[31] = V8;
            vdst1Conj[63] = V9;
            vdst1 += 2*dstStrideVec;
            vdst1Conj -= 2*dstStrideVec;

            V1_0 = Q6_Ww_vadd_WwWw_sat(V11_10, V1_0);        // x[3] = b + d  (Im : Re)
            vdst3[col] = V0;
            vdst3[col+32] = V1;
            // conjugate symmetry
            V1 = Q6_Vw_vsub_VwVw_sat(V30, V1);   // Im(X) = -Im(X)
            V0 = Q6_V_vdelta_VV(V0, V31);       // reverse(Re(X))
            V1 = Q6_V_vdelta_VV(V1, V31);       // reverse(-Im(X))
            V0 = Q6_V_vlalign_VVI(V0, V30, 4);
            V1 = Q6_V_vlalign_VVI(V1, V30, 4);
            vdst3Conj[31] = V0;
            vdst3Conj[63] = V1;
            vdst3 += 2*dstStrideVec;
            vdst3Conj -= 2*dstStrideVec;
        }
        else if (col < vecColumns) for (int j = workerCount; j < srcHeight / 4; j+=2)
        {
            V0 = vsrc0[0];             // Re(x[0])
            V1 = vsrc0[imagOffset];    // Im(x[0])
            V2 = vsrc1[0];             // Re(x[1])
            V3 = vsrc1[imagOffset];    // Im(x[1])
            V4 = vsrc2[0];             // Re(x[2])
            V5 = vsrc2[imagOffset];    // Im(x[2])
            V6 = vsrc3[0];             // Re(x[3])
            V7 = vsrc3[imagOffset];    // Im(x[3])
         
            vsrc0 += 2*srcStrideVec;
            vsrc1 += 2*srcStrideVec;
            vsrc2 += 2*srcStrideVec;
            vsrc3 += 2*srcStrideVec;

            V9_8 = Q6_Ww_vadd_WwWw_sat(V1_0, V3_2);        // a = x[0] + x[1]  (Im : Re)
            V11_10 = Q6_Ww_vsub_WwWw_sat(V1_0, V3_2);      // b = x[0] - x[1]  (Im : Re)

            V3_2 = Q6_Ww_vadd_WwWw_sat(V5_4, V7_6);        // c = x[2] + x[3]  (Im : Re)
            V0 = Q6_Vw_vsub_VwVw_sat(V7, V5);              // d = j * (x[2] - x[3])  (Im : Re)
            V1 = Q6_Vw_vsub_VwVw_sat(V4, V6);              // d = j * (x[2] - x[3])  (Im : Re)
            
            V7_6 = Q6_Ww_vadd_WwWw_sat(V9_8, V3_2);        // x[0] = a + c  (Im : Re)
            vdst0[col] = V6;
            vdst0[col+32] = V7;
            // conjugate symmetry
            V7 = Q6_Vw_vsub_VwVw_sat(V30, V7);   // Im(X) = -Im(X)
            V6 = Q6_V_vdelta_VV(V6, V31);       // reverse(Re(X))
            V7 = Q6_V_vdelta_VV(V7, V31);       // reverse(-Im(X))
            HVX_Vector *v0ptr = (j == 0 ? vdst0 : vdst0Conj);
            int32_t* conjR = ((int32_t*)(&v0ptr[31-col])) + 1;
            vmemu(conjR) = V6;
            int32_t* conjI = ((int32_t*)(&v0ptr[63-col])) + 1;
            vmemu(conjI) = V7;
            vdst0 += 2*dstStrideVec;
            vdst0Conj -= 2*dstStrideVec;


            V3_2 = Q6_Ww_vsub_WwWw_sat(V9_8, V3_2);        // x[2] = a - c  (Im : Re)
            vdst2[col] = V2;
            vdst2[col+32] = V3;
            // conjugate symmetry
            V3 = Q6_Vw_vsub_VwVw_sat(V30, V3);   // Im(X) = -Im(X)
            V2 = Q6_V_vdelta_VV(V2, V31);       // reverse(Re(X))
            V3 = Q6_V_vdelta_VV(V3, V31);       // reverse(-Im(X))
            conjR = ((int32_t*)(&vdst2Conj[31-col])) + 1;
            vmemu(conjR) = V2;
            conjI = ((int32_t*)(&vdst2Conj[63-col])) + 1;
            vmemu(conjI) = V3;
            vdst2 += 2*dstStrideVec;
            vdst2Conj -= 2*dstStrideVec;

            V9_8 = Q6_Ww_vsub_WwWw_sat(V11_10, V1_0);        // x[1] = b - d  (Im : Re)
            vdst1[col] = V8;
            vdst1[col+32] = V9;
            // conjugate symmetry
            V9 = Q6_Vw_vsub_VwVw_sat(V30, V9);   // Im(X) = -Im(X)
            V8 = Q6_V_vdelta_VV(V8, V31);       // reverse(Re(X))
            V9 = Q6_V_vdelta_VV(V9, V31);       // reverse(-Im(X))
            conjR = ((int32_t*)(&vdst1Conj[31-col])) + 1;
            vmemu(conjR) = V8;
            conjI = ((int32_t*)(&vdst1Conj[63-col])) + 1;
            vmemu(conjI) = V9;
            vdst1 += 2*dstStrideVec;
            vdst1Conj -= 2*dstStrideVec;

            V1_0 = Q6_Ww_vadd_WwWw_sat(V11_10, V1_0);        // x[3] = b + d  (Im : Re)
            vdst3[col] = V0;
            vdst3[col+32] = V1;
            // conjugate symmetry
            V1 = Q6_Vw_vsub_VwVw_sat(V30, V1);   // Im(X) = -Im(X)
            V0 = Q6_V_vdelta_VV(V0, V31);       // reverse(Re(X))
            V1 = Q6_V_vdelta_VV(V1, V31);       // reverse(-Im(X))
            conjR = ((int32_t*)(&vdst3Conj[31-col])) + 1;
            vmemu(conjR) = V0;
            conjI = ((int32_t*)(&vdst3Conj[63-col])) + 1;
            vmemu(conjI) = V1;
            vdst3Conj -= 2*dstStrideVec;
            vdst3 += 2*dstStrideVec;
        }
        else for (int j = workerCount; j < srcHeight / 4; j+=2)
        {
            V0 = vsrc0[0];             // Re(x[0])
            V1 = vsrc0[imagOffset];    // Im(x[0])
            V2 = vsrc1[0];             // Re(x[1])
            V3 = vsrc1[imagOffset];    // Im(x[1])
            V4 = vsrc2[0];             // Re(x[2])
            V5 = vsrc2[imagOffset];    // Im(x[2])
            V6 = vsrc3[0];             // Re(x[3])
            V7 = vsrc3[imagOffset];    // Im(x[3])
         
            vsrc0 += 2*srcStrideVec;
            vsrc1 += 2*srcStrideVec;
            vsrc2 += 2*srcStrideVec;
            vsrc3 += 2*srcStrideVec;

            V9_8 = Q6_Ww_vadd_WwWw_sat(V1_0, V3_2);        // a = x[0] + x[1]  (Im : Re)
            V11_10 = Q6_Ww_vsub_WwWw_sat(V1_0, V3_2);      // b = x[0] - x[1]  (Im : Re)

            V3_2 = Q6_Ww_vadd_WwWw_sat(V5_4, V7_6);        // c = x[2] + x[3]  (Im : Re)
            V0 = Q6_Vw_vsub_VwVw_sat(V7, V5);              // d = j * (x[2] - x[3])  (Im : Re)
            V1 = Q6_Vw_vsub_VwVw_sat(V4, V6);              // d = j * (x[2] - x[3])  (Im : Re)
            
            V7_6 = Q6_Ww_vadd_WwWw_sat(V9_8, V3_2);        // x[0] = a + c  (Im : Re)
            V4 = vdst0[col];
            V6 = Q6_V_vmux_QVV(Q0,V6,V4);
            vdst0[col] = V6;
            V4 = vdst0[col+32];
            V7 = Q6_V_vmux_QVV(Q0,V7,V4);
            vdst0[col+32] = V7;

            V3_2 = Q6_Ww_vsub_WwWw_sat(V9_8, V3_2);        // x[2] = a - c  (Im : Re)
            V4 = vdst2[col];
            V2 = Q6_V_vmux_QVV(Q0,V2,V4);
            vdst2[col] = V2;
            V4 = vdst2[col+32];
            V3 = Q6_V_vmux_QVV(Q0,V3,V4);
            vdst2[col+32] = V3;

            V9_8 = Q6_Ww_vsub_WwWw_sat(V11_10, V1_0);        // x[1] = b - d  (Im : Re)
            V4 = vdst1[col];
            V8 = Q6_V_vmux_QVV(Q0,V8,V4);
            vdst1[col] = V8;
            V4 = vdst1[col+32];
            V9 = Q6_V_vmux_QVV(Q0,V9,V4);
            vdst1[col+32] = V9;

            V1_0 = Q6_Ww_vadd_WwWw_sat(V11_10, V1_0);        // x[3] = b + d  (Im : Re)
            V4 = vdst3[col];
            V0 = Q6_V_vmux_QVV(Q0,V0,V4);
            vdst3[col] = V0;
            V4 = vdst3[col+32];
            V1 = Q6_V_vmux_QVV(Q0,V1,V4);
            vdst3[col+32] = V1;

            vdst0 += 2*dstStrideVec;
            vdst1 += 2*dstStrideVec;
            vdst2 += 2*dstStrideVec;
            vdst3 += 2*dstStrideVec;
        }
        qurt_barrier_wait(&dptr->barrier);
       
    }
#if (__HEXAGON_ARCH__ < 65)
    dspCV_hvx_unlock();
#endif
    dspCV_worker_pool_synctoken_jobdone(dptr->token); // release multi-threading job token
    // void unused registers to avoid compiler warnings.
    (void) V29_28;
    (void) V27_26;
    (void) V25_24;
    (void) V19_18;
    (void) V17_16;
    (void) V15_14;
    (void) V13_12;
}

/*===========================================================================
    GLOBAL FUNCTION
===========================================================================*/

int fft1024_hvx(const uint8* imgSrc, const int32_t useComputRes, int32_t* imgDst)
{
    const int32_t srcWidth = 1024;
    const int32_t srcHeight = 1024;
    const int32_t srcStride = 1024;
    const int32_t dstStride = 1024 * 2 * sizeof(imgDst[0]);

    //Scratch buffer
    uint32_t totalSize = (2 * srcWidth * srcHeight * sizeof(imgDst[0]));
    HVX_Vector* scratchBuf = NULL;
    HVX_Vector* vtcmLine = NULL;
    uint64_t t0, t1, t2, t3, t4;
    t0 = HAP_perf_get_pcycles();

    compute_res_attr_t compute_res;
    unsigned int context_id = 0;
    
    if(useComputRes&&compute_resource_attr_init)
    {
        compute_resource_attr_init(&compute_res);
        compute_resource_attr_set_serialize(&compute_res, 1);
        compute_resource_attr_set_vtcm_param(&compute_res, 256 * 1024, 0);
        
        context_id=compute_resource_acquire(&compute_res, 100000); // wait till 100ms
        
        if(context_id==0)
        {
            return AEE_ERESOURCENOTFOUND;
        }
        
        vtcmLine=compute_resource_attr_get_vtcm_ptr(&compute_res);
    }
    else
    {
        if(useComputRes&&(!compute_resource_attr_init))  
        {        
            FARF(HIGH, "Compute resource APIs not supported. Use legacy methods instead.");        
        }
        
        vtcmLine = (HVX_Vector*)HAP_request_VTCM(256 * 1024, 0);
    }
    
    if (!vtcmLine) 
    {
        FARF(HIGH,"WARNING-- Unable to reserve VTCM, performance will be degraded.");
        scratchBuf = (HVX_Vector*) memalign(128, totalSize);
        if (scratchBuf == NULL)
        {
            return AEE_ENOMEMORY;
        }
    }

    /* Transform the rows */
    dspCV_worker_job_t job;
    dspCV_synctoken_t token;
    fcvFFTu8_callback_t dptr;
    dptr.token = &token;
    dptr.workerCount = 0;
    dptr.jobCount = 0;
    dptr.src = (uint8_t *) imgSrc;
    dptr.srcWidth = srcWidth;
    dptr.srcHeight = srcHeight;
    dptr.srcStride = srcStride;
    dptr.dst = imgDst;
    dptr.dstStride = dstStride;
    dptr.pPrecision = 3;
    dptr.scratchBuf = vtcmLine ? vtcmLine : scratchBuf;
    job.dptr = (void *) &dptr;
    job.fptr = fcvFFTu8_horiz_callback;

    // init the synchronization token for this dispatch. Submit only dspCV_num_workers jobs, each of which may process multiple strips of the image.
    int numWorkers = dspCV_num_hvx128_contexts;
    dspCV_worker_pool_synctoken_init(&token, numWorkers);
    t1 = HAP_perf_get_pcycles();

    // multi-thread the horizontal transforms
    for (uint32_t i = 0; i < numWorkers; i++)
    {
        //job.fptr(job.dptr);
        (void) dspCV_worker_pool_submit(job);
    }
    dspCV_worker_pool_synctoken_wait(&token);
/* Transform the columns */
    // column processing must be 2 threads.
    t2 = HAP_perf_get_pcycles();
    numWorkers = 2;
    dspCV_worker_pool_synctoken_init(&token, numWorkers);
    dptr.workerCount = 0;
    dptr.jobCount = 0;
    qurt_barrier_init(&dptr.barrier, numWorkers);

    job.fptr = fcvFFTu8_vert_callback;
    // multi-thread the vertical transforms
    for (uint32_t i = 0; i < numWorkers; i++)
    {
        //job.fptr(job.dptr);
        (void) dspCV_worker_pool_submit(job);
    }
    dspCV_worker_pool_synctoken_wait(&token);
    t3 = HAP_perf_get_pcycles();

    if (scratchBuf) free(scratchBuf);
    
    if(useComputRes&&compute_resource_attr_init)
    {
        compute_resource_release(context_id);               
    }
    else
    {
        if (vtcmLine) HAP_release_VTCM((void*)vtcmLine);
    }
    qurt_barrier_destroy(&dptr.barrier);
    
    t4 = HAP_perf_get_pcycles();
#ifdef PROFILING_ON
    FARF(HIGH,"FFT profiling pcycles: scratch alloc & setup/teardown %llu, horizontal %llu, vertical %llu.",
        (t1 - t0) + (t4 - t3), (t2 - t1), (t3 - t2));
#endif
    return 0;
}


AEEResult benchmark_fft1024(remote_handle64 handle, const uint8* imgSrc, int srcLen,
    int32_t* imgDst, int dstLen, int32 LOOPS, int32 wakeupOnly, int32 useComputRes, int32* dspUsec, int32* dspCyc)
{
#if (__HEXAGON_ARCH__ < 65)
    dspCV_hvx_power_on();
#endif
    
    *dspUsec = 0, *dspCyc = 0;
    if (wakeupOnly)
    {
        return AEE_SUCCESS;
    }

    // Only supporting 128-byte aligned!!
    if (!(imgSrc && imgDst && ((((uint32)imgSrc | (uint32)imgDst) & 127) == 0)))
    {
        return AEE_EBADPARM;
    }
    
    // record start time (in both microseconds and pcycles) for profiling
#ifdef PROFILING_ON
    uint64_t startTime = HAP_perf_get_time_us();
    uint64 startCycles = HAP_perf_get_pcycles();
#endif

    int retVal = AEE_SUCCESS;
    
    for (int loops = 0; loops < LOOPS; loops++)
    {
        //Set scale factor appropriate for 1024x1024
        if(0 != fft1024_hvx(imgSrc, useComputRes, imgDst))
        {
            retVal = AEE_EFAILED;
        }
    }

#ifdef PROFILING_ON
    uint64 endCycles = HAP_perf_get_pcycles();
    uint64 endTime = HAP_perf_get_time_us();
    *dspUsec = (int)(endTime - startTime);
    *dspCyc = (int32)(endCycles - startCycles);
    FARF(HIGH,"fft profiling over %d iterations: %d PCycles, %d microseconds. Observed clock rate %d MHz",
        LOOPS, (int)(endCycles - startCycles), (int)(endTime - startTime),
        (int)((endCycles - startCycles) / (endTime - startTime)));
    FARF(HIGH,"Note - observed clock rate may appear low occasionally, when scratch alloc has to call CPU to grow DSP heap (during which cycle counter might not increment).");
#endif

    return retVal;
}
