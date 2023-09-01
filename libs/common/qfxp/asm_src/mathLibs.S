/**=============================================================================

@file
   mathLibs.S

Copyright (c) 2018 Qualcomm Technologies, Inc.  All Rights Reserved.
=============================================================================**/

#define DSP_INV_LUT_BITS   4                             // number of bits to index the inverse 
                                                         // lookup table (actually 2^(val-1) segments) 

        .data
        .p2align 7
        .type   hvx_sqrt_lut, @object
        .size   hvx_sqrt_lut, 384
hvx_sqrt_lut:
/* 128-byte mode sqrt LUT */
        .hword      0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0
        .hword  16389, 0, 17382, 0, 18322, 0, 19215, 0, 20069, 0, 20888, 0, 21676, 0, 22437, 0
        .hword  23172, 0, 23885, 0, 24578, 0, 25251, 0, 25907, 0, 26546, 0, 27171, 0, 27782, 0
        .hword  28379, 0, 28964, 0, 29538, 0, 30100, 0, 30652, 0, 31195, 0, 31728, 0, 32253, 0
/* 128-byte mode sqrt interpolation scales LUT */
        .hword      0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0
        .hword    993, 0,   940, 0,   893, 0,   854, 0,   819, 0,   788, 0,   761, 0,   735, 0 
        .hword    713, 0,   693, 0,   673, 0,   656, 0,   639, 0,   625, 0,   611, 0,   597, 0 
        .hword    585, 0,   574, 0,   562, 0,   552, 0,   543, 0,   533, 0,   525, 0,   516, 0
/* 64-byte mode sqrt LUT */
        .hword      0, 23172,     0, 23885,     0, 24578,     0, 25251,     0, 25907,     0, 26546,     0, 27171,     0, 27782
        .hword  16389, 28379, 17382, 28964, 18322, 29538, 19215, 30100, 20069, 30652, 20888, 31195, 21676, 31728, 22437, 32253
/* 64-byte mode sqrt interpolation scales LUT */
        .hword      0, 713,     0, 693,     0, 673,     0, 656,     0, 639,     0, 625,     0, 611,     0, 597
        .hword    993, 585,   940, 574,   893, 562,   854, 552,   819, 543,   788, 533,   761, 525,   735, 516 
/* normalization shift value lookup */
        .byte     -11, 0, -9, 0, -9, 0, -7, 0, -7, 0, -5, 0, -5, 0, -3, 0, -3, 0, -1, 0, -1, 0,  1, 0,  1, 0,  3, 0,  3, 0,  5, 0
        .byte       5, 0,  7, 0,  7, 0,  9, 0,  9, 0, 11, 0, 11, 0, 13, 0, 13, 0, 15, 0, 15, 0, 17, 0, 17, 0, 19, 0, 19, 0, 21, 0
        .byte       0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0
        .byte       0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0
/* de-normalization shift value lookup */
        .byte       1, 0,  0, 0,  0, 0, -1, 0, -1, 0, -2, 0, -2, 0, -3, 0, -3, 0, -4, 0, -4, 0, -5, 0, -5, 0, -6, 0, -6, 0, -7, 0
        .byte      -7, 0, -8, 0, -8, 0, -9, 0, -9, 0,-10, 0,-10, 0,-11, 0,-11, 0,-12, 0,-12, 0,-13, 0,-13, 0,-14, 0,-14, 0,-15, 0
        .byte       0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0
        .byte       0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0,  0, 0

    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void hvx_sqrt                                                  ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: approximate sqrt on a list of 32-bit unsigned integers        ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : uint32 *inp             -- pointer to input values       ]*/
    /*[               R1 : int N                   -- length of inp                 ]*/
    /*[               R2 : uint32 *outp            -- pointer to output             ]*/
    /*[               R3 : uint32 VLEN             -- HVX mode                      ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
    /*[           -                                                                 ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  ASSUMPTIONS:                                                               ]*/
    /*[           -                                                                 ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  REVISION HISTORY                                                           ]*/
    /*[  ----------------                                                           ]*/
    /*[  Version        Date                    Comments                            ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    
    .text
    .p2align 4
    .globl hvx_sqrt
    .type    hvx_sqrt, @function
hvx_sqrt:
    { 
      R6 = ADD(PC,##hvx_sqrt_lut@PCREL)   // table lookup base (i.e. 128-byte mode sqrt LUT)
      P0 = CMP.EQ(R3,#64)              // HVX in 64B mode?
      R8 = #11                         // const 11
    }{
      R9 = #1                          // const 1
      R15 = CT0(R3)                    // log2(VLEN)
      R7 = ADD(R6,#128)                // 128-byte mode sqrt interpolation scales LUT
      V11 = VSPLAT(R8)                 // vector of const 11
    }{
      R14 = ADD(R6,#384)               // &normShiftLut
    }{
      IF (P0) R6 = ADD(R6,#256)        // 64-byte mode sqrt LUT
      IF (P0) R7 = ADD(R6,#320)        // 64-byte mode sqrt interpolation scales LUT
    }{
      R1 = ASR(R1,R15)                 // loop count = N / (VLEN/2*sizeof(inp[0])))
      V31 = VMEM(R6)                    // hvx_sqrt_lut
    }{
      P3 = SP1LOOP0(.hvx_sqrt_outer,R1) // loop
      V2 = VMEM(R7)                    // sqRootInterpLut
      R8 = #2                          // const 2
    }{
      V14 = VMEM(R14)                  // normShiftLut
      R14 = ADD(R14,#128)              // &deNormShiftLut
    }{
      V15 = VMEM(R14)                  // deNormShiftLut
    }
    .falign
.hvx_sqrt_outer:
    {
      V3 = VMEM(R0++#1)                // inVec = *inp++
      V25:24.h |= VLUT16(V5.b,V2.h,R5) // [2] sqRootInterpLUT table lookup
    }{
      V4 = VMEM(R0++#1)                // inVec2 = *inp++
      V27:26.w |= VUNPACKO(V0.h)       // [2] Unpack sqRoot from LUT with << 16
    }{
      V5 = VMEM(R0++#1)                // inVec3 = *inp++
      V7.w = VNORMAMT(V3.w)            // rVec[.] = normamt(inVec[.])
      V19:18.h = VSXT(V10.b)           // [2] deal and expand de-normalization shift values to 16 bits
    }{
      V6 = VMEM(R0++#1)                // inVec4 = *inp++
      V8.w = VNORMAMT(V4.w)            // rVec2[.] = normamt(inVec2[.])
    }{
      V7.w = VNORMAMT(V5.w)            // rVec3[.] = normamt(inVec3[.])
      V8.h = VSHUFFE(V8.h,V7.h)       // rVec2 and rVec, shuffled down to 16-bits
      R5 = #0                           // VLUT index = 0
    }{
      V9.w = VNORMAMT(V6.w)           // rVec4[.] = normamt(inVec4[.])
      V29.w = VSUB(V29.w,V29.w)       // [2] const 0
      V28.w = VSUB(V28.w,V28.w)       // [2] const 0
    }{
      V7.h = VSHUFFE(V9.h,V7.h)       // rVec4 and rVec3, shuffled down to 16-bits
      V24.h = VSHUFF(V24.h)            // [2] shuffle sqRootInterpLUT
      V17:16.w = VSXT(V18.h)           // [2] deal and expand de-normalization shift values to 32 bits
    }{
      V7.b = VSHUFFE(V7.b,V8.b)     // rVec..rVec4, shuffled down to bytes
      V25.h = VSHUFF(V25.h)            // [2] shuffle sqRootInterpLUT
      V19:18.w = VSXT(V19.h)           // [2] deal and expand de-normalization shift values to 32 bits
    }{
      V29:28.w |= VUNPACKO(V1.h)       // [2] Unpack sqRoot from LUT with << 16
      V27:26.uw += VMPY(V24.uh,V11.uh) // [2] sqRoot[.] += fraction[.] * sqRootInterpLut[.]
    }{
      V8.b = VLUT32(V7.b,V14.b,R5)      // look up normalization shift values
      V29:28.uw += VMPY(V25.uh,V30.uh) // [2] sqRoot2[.] += fraction2[.] * sqRootInterpLut[.]
    }{
      V10.b = VLUT32(V7.b,V15.b,R5)     // look up de-normalization shift values
      V9:8.h = VSXT(V8.b)               // deal and expand normalization shift values to 16 bits
    }{
      V13:12.w = VSXT(V8.h)             // deal and expand normalization shift values to 32 bits
      V21:20.w = VSXT(V9.h)             // deal and expand normalization shift values to 32 bits
    }{
      V30.w = VSUB(V30.w,V30.w)        // vector of const 0
    }{
      V8.w = VASL(V3.w,V12.w)          // inVec[.] << shiftVec[.], valid if shiftVec[.] >= 0
      Q0 = VCMP.GT(V12.w,V30.w)         // (shiftVec[.] > 0) ?
      V12.w = VSUB(V30.w,V12.w)         // -shiftVec[.]
    }{
      V9.w = VASL(V4.w,V13.w)          // inVec2[.] << shiftVec2[.], valid if shiftVec2[.] >= 0
      V13.w = VSUB(V30.w,V13.w)         // -shiftVec2[.]
      Q1 = VCMP.GT(V13.w,V30.w)         // (shiftVec2[.] > 0) ?
    }{
      V3.w = VLSR(V3.w,V12.w)        //   inVec[.] >> -shiftVec[.], valid if shiftVec[.] <= 0
      Q2 = VCMP.GT(V20.w,V30.w)         // (shiftVec3[.] > 0) ?
      Q3 = VCMP.GT(V21.w,V30.w)         // (shiftVec4[.] > 0) ?
    }{
      V3 = VMUX(Q0,V8,V3)               // normalized inVec
      V8.w = VASL(V5.w,V20.w)          // inVec3[.] << shiftVec3[.], valid if shiftVec3[.] >= 0
      V20.w = VSUB(V30.w,V20.w)         // -shiftVec3[.]
    }{
      V4.w = VLSR(V4.w,V13.w)        //   inVec2[.] >> -shiftVec2[.], valid if shiftVec2[.] <= 0
    }{
      V4 = VMUX(Q1,V9,V4)               // normalized inVec2
      V9.w = VASL(V6.w,V21.w)          // inVec4[.] << shiftVec4[.], valid if shiftVec4[.] >= 0
      V21.w = VSUB(V30.w,V21.w)         // -shiftVec4[.]
    }{
      V5.w = VLSR(V5.w,V20.w)        //   inVec3[.] >> -shiftVec3[.], valid if shiftVec3[.] <= 0
      V7.h = VPACKO(V4.w,V3.w)         // lutIndex[0..VLEN-1] (not shuffled)
    }{
      V6.w = VLSR(V6.w,V21.w)        //   inVec4[.] >> -shiftVec4[.], valid if shiftVec4[.] <= 0
      V5 = VMUX(Q2,V8,V5)               // normalized inVec3
    }{
      V11.w = VASL(V26.w,V16.w)          // [2] sqRoot[.] << shiftVec[.], valid if shiftVec[.] >= 0
      V16.w = VSUB(V30.w,V16.w)         // [2] -shiftVec[.]
      Q0 = VCMP.GT(V16.w,V30.w)         // [2] (shiftVec[.] > 0) ?
    }{
      V6 = VMUX(Q3,V9,V6)               // normalized inVec4
      V22.w = VASL(V27.w,V17.w)          // [2] sqRoot2[.] << shiftVec2[.], valid if shiftVec2[.] >= 0
    }{
      V26.w = VLSR(V26.w,V16.w)        // [2] sqRoot[.] >> -shiftVec[.], valid if shiftVec[.] <= 0
      V17.w = VSUB(V30.w,V17.w)         // [2] -shiftVec2[.]
      Q1 = VCMP.GT(V17.w,V30.w)         // [2] (shiftVec2[.] > 0) ?
    }{
      V11 = VMUX(Q0,V11,V26)             // [2] de-normalized sqRoot
      V23.w = VASL(V28.w,V18.w)          // [2] sqRoot3[.] << shiftVec3[.], valid if shiftVec3[.] >= 0
      V19.w = VSUB(V30.w,V19.w)         // [2] -shiftVec4[.]
      Q3 = VCMP.GT(V19.w,V30.w)         // [2] (shiftVec4[.] > 0) ?
    }{
      V18.w = VSUB(V30.w,V18.w)         // [2] -shiftVec3[.]
      Q2 = VCMP.GT(V18.w,V30.w)         // [2] (shiftVec3[.] > 0) ?
      V27.w = VLSR(V27.w,V17.w)        // [2] sqRoot2[.] >> -shiftVec2[.], valid if shiftVec2[.] <= 0
      IF (P3) VMEM(R2++#1) = V11               // [2] store sqRoot
    }{
      V22 = VMUX(Q1,V22,V27)               // [2] de-normalized sqRoot2
      V0.w = VASL(V29.w,V19.w)          // [2] sqRoot4[.] << shiftVec4[.], valid if shiftVec4[.] >= 0
      V8.h = VPACKO(V6.w,V5.w)        // lutIndex2[0..VLEN-1] (not shuffled)
    }{
      V28.w = VLSR(V28.w,V18.w)        // [2] sqRoot3[.] >> -shiftVec3[.], valid if shiftVec3[.] <= 0
      IF (P3) VMEM(R2++#1) = V22               // [2] store sqRoot2
      V30.h = VSHUFFE(V6.h,V5.h)      // fraction2[0..VLEN-1] (shuffled)
      V5.b = VSHUFFE(V8.b,V7.b)       // lutIndex from all 4 inp vectors, packed and shuffled as bytes
    }{
      V29.w = VLSR(V29.w,V19.w)        // [2] sqRoot4[.] >> -shiftVec4[.], valid if shiftVec4[.] <= 0
      V23 = VMUX(Q2,V23,V28)               // [2] de-normalized sqRoot3
      V11.h = VSHUFFE(V4.h,V3.h)      // fraction[0..VLEN-1] (shuffled)
    }{
      V28 = VMUX(Q3,V0,V29)               // [2] de-normalized sqRoot4
      V1:0.h = VLUT16(V5.b,V31.h,R5)   // sqRootLUT table lookup
      V26.w=VSUB(V26.w,V26.w)           // 0
    }{
      V25:24.h = VLUT16(V5.b,V2.h,R5) // sqRootInterpLUT table lookup
      R5 = #1                          // table select = 1
      IF (P3) VMEM(R2++#1) = V23               // [2] store sqRoot3
    }{
      IF (P3) VMEM(R2++#1) = V28               // [2] store sqRoot4
      V1:0.h |= VLUT16(V5.b,V31.h,R5)  // sqRootLUT table lookup
      V27 = V26       // const 0
    }:endloop0

    {
      V29:28 = VCOMBINE(V26,V26)       // [2] const 0
    }{
      V25:24.h |= VLUT16(V5.b,V2.h,R5) // [2] sqRootInterpLUT table lookup
    }{
      V27:26.w |= VUNPACKO(V0.h)       // [2] Unpack sqRoot from LUT with << 16
      V19:18.h = VSXT(V10.b)           // [2] deal and expand de-normalization shift values to 16 bits
    }{
      V24.h = VSHUFF(V24.h)            // [2] shuffle sqRootInterpLUT
      V17:16.w = VSXT(V18.h)           // [2] deal and expand de-normalization shift values to 32 bits
    }{
      V25.h = VSHUFF(V25.h)            // [2] shuffle sqRootInterpLUT
      V19:18.w = VSXT(V19.h)           // [2] deal and expand de-normalization shift values to 32 bits
    }{
      V29:28.w |= VUNPACKO(V1.h)       // [2] Unpack sqRoot from LUT with << 16
      V27:26.uw += VMPY(V24.uh,V11.uh) // [2] sqRoot[.] += fraction[.] * sqRootInterpLut[.]
    }{
      V29:28.uw += VMPY(V25.uh,V30.uh) // [2] sqRoot2[.] += fraction2[.] * sqRootInterpLut[.]
      V30.w = VSUB(V30.w,V30.w)        // vector of const 0
    }{
      V11.w = VASL(V26.w,V16.w)          // [2] sqRoot[.] << shiftVec[.], valid if shiftVec[.] >= 0
      V16.w = VSUB(V30.w,V16.w)         // [2] -shiftVec[.]
      Q0 = VCMP.GT(V16.w,V30.w)         // [2] (shiftVec[.] > 0) ?
    }{
      V22.w = VASL(V27.w,V17.w)          // [2] sqRoot2[.] << shiftVec2[.], valid if shiftVec2[.] >= 0
    }{
      V26.w = VLSR(V26.w,V16.w)        // [2] sqRoot[.] >> -shiftVec[.], valid if shiftVec[.] <= 0
      V17.w = VSUB(V30.w,V17.w)         // [2] -shiftVec2[.]
      Q1 = VCMP.GT(V17.w,V30.w)         // [2] (shiftVec2[.] > 0) ?
    }{
      V11 = VMUX(Q0,V11,V26)             // [2] de-normalized sqRoot
      V23.w = VASL(V28.w,V18.w)          // [2] sqRoot3[.] << shiftVec3[.], valid if shiftVec3[.] >= 0
      V18.w = VSUB(V30.w,V18.w)         // [2] -shiftVec3[.]
      Q2 = VCMP.GT(V18.w,V30.w)         // [2] (shiftVec3[.] > 0) ?
    }{
      V27.w = VLSR(V27.w,V17.w)        // [2] sqRoot2[.] >> -shiftVec2[.], valid if shiftVec2[.] <= 0
      VMEM(R2++#1) = V11               // [2] store sqRoot
    }{
      V22 = VMUX(Q1,V22,V27)               // [2] de-normalized sqRoot2
      V0.w = VASL(V29.w,V19.w)          // [2] sqRoot4[.] << shiftVec4[.], valid if shiftVec4[.] >= 0
      V19.w = VSUB(V30.w,V19.w)         // [2] -shiftVec4[.]
      Q3 = VCMP.GT(V19.w,V30.w)         // [2] (shiftVec4[.] > 0) ?
    }{
      V28.w = VLSR(V28.w,V18.w)        // [2] sqRoot3[.] >> -shiftVec3[.], valid if shiftVec3[.] <= 0
      VMEM(R2++#1) = V22               // [2] store sqRoot2
    }{
      V29.w = VLSR(V29.w,V19.w)        // [2] sqRoot4[.] >> -shiftVec4[.], valid if shiftVec4[.] <= 0
      V23 = VMUX(Q2,V23,V28)               // [2] de-normalized sqRoot3
    }{
      V0 = VMUX(Q3,V0,V29)               // [2] de-normalized sqRoot4
      VMEM(R2++#1) = V23               // [2] store sqRoot3
    }{
      IF (P3) VMEM(R2++#1) = V0               // [2] store sqRoot4
    }{ JUMPR R31
    }
    .size    hvx_sqrt, .-hvx_sqrt
	

/*======================================================================*/
/* Copyright (c) 2018 Qualcomm Technologies, Inc.  All Rights Reserved.  */
/*======================================================================*/
/*  FUNCTIONS      : hvx_invert //list scheduled                       */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                 Compute 1/(x) x is a Qpoint number where adjusted    */
/*                 -1 to 1 x 2^exp, result is also a mantissa exponent  */
/*                 pdeudo float value.                                  */
/*                                                                      */
/*  ARCHITECTURE   : QDSP6V6  + HVX                                     */
/*======================================================================*/
/*  REVISION HISTORY:                                                   */
/*  =================                                                   */
/*                                                                      */
/*  Author              Date           Comments                         */
/*  -------------------------------------------------------------       */
/*  DJH                 09/26/16       created                          */
/*======================================================================*/
/*  IDEAL SCHEDULED CYCLE-COUNT:                                        */
/*     ->  n*10/128+10                                                  */
/*                                                                      */
/*     ASSUMPTIONS                                                      */
/*        y and x are 128 byte aligned                                  */
/*        n%128=0                                                       */
/*  C MODEL                                                             */
/*======================================================================*/

/*======================================================================*/
          .global hvx_invert
          .global hvx_invert_lut
          .balign 32
          .type  hvx_invert, @function
hvx_invert:
/*======================================================================*/
#define ptr_x     r0 //ptr to input data
#define qpoint    r1 //qpoint of input data
#define ptr_ym    r2 //mantissa of output data
#define ptr_ye    r3 //corresponding exponents of output data
/*======================================================================*/
#define n         r4
#define ptr_inverse r5
#define c4        r9
#define cf        r8
#define c0        r7
#define c1        r10
#define inv0      v0
#define inv1      v1
#define inv2      v2
#define vc7       v3
#define vc8       v4
#define vc1fff    v5
#define d0        v6
#define d1        v7
#define d0x2      v8
#define d1x2      v9 
#define d1x3      v15 
#define i1i0      v11
#define j1j0      v12
#define i3i2      v10
#define qexp0     v13
#define qexp1     v14
#define c21_c20   v17:16
#define c21       v17
#define     c20   v16
#define c11_c10   v19:18
#define c11       v19
#define     c10   v18
#define c01_c00   v21:20
#define c01       v21
#define     c00   v20
#define vzero     v22
#define vone      v23
#define y1        v24
#define y0        v25
#define exp0      v26
#define exp1      v27
#define x1        v28
#define x0        v29
#define vqpoint   v30
/*======================================================================*/
       {
         vzero = #0                              //[P,  ]
         c1 = ##0x00010001                       //[P,  ]
       } {  
         vone = vsplat(c1)                       //[P,  ]
         d0.cur = vmem(ptr_x++#2)                //[0, 0]
         q0 = vcmp.eq(vzero.h, d0.h)             //[0, 0]
       } { 
         d1.cur = vmem(ptr_x+#-1)                //[0, 1]
         q1 = vcmp.eq(vzero.h, d1.h)             //[0, 1]
         d0 = vmux(q0, vone, d0)                 //[0, 1]
       } {
         d1 = vmux(q1, vone, d1)                 //[0, 2]
         qpoint = sub(#13, qpoint)               //[P,  ]
       }
	   // optimize later
	{
      ptr_inverse = ADD(PC,##hvx_invert_lut@PCREL)            
    }
	   { 
         inv0 = vmem(ptr_inverse+#0)             //[P,  ]
	 exp0.h = vnormamt(d0.h)                 //[0, 3]
         qpoint = combine(qpoint.L, qpoint.L)    //[P,  ]
       } {   
         inv1 = vmem(ptr_inverse+#1)             //[P,  ]
         vqpoint = vsplat(qpoint)                //[P,  ]
         exp1.h = vnormamt(d1.h)                 //[0, 4]
       } {   
         inv2 = vmem(ptr_inverse+#2)             //[P,  ]
         cf = ##0x70707070                       //[P,  ]
       } { 
         qexp1.h = vsub(exp1.h, vqpoint.h)      //[0, 5] 
         vmem(ptr_ye+#1) = qexp1.new             //[0, 5]
         vc7 = vsplat(cf)                        //[P,  ]
         c4 = #4                                 //[P,  ]
       } { 
         d0x2.h = vasl(d0.h, exp0.h)             //[0, 7]
         n = lsr(n, #7)                          //[P, 7]
         cf = ##0x80808080                       //[P,  ]
       } {
         vc8 = vsplat(cf)                        //[P,  ]
         cf = ##0x0ffe0ffe                       //[P,  ]
         d1x2.h = vasl(d1.h, exp1.h)             //[0, 8]
       } {
         c0 = #0                                 //[P,  ]
         vc1fff = vsplat(cf)                     //[P,  ]
         i1i0.b = vshuffo(d1x2.b, d0x2.b)        //[0, 9]
         p3 = sp1loop0(.L_loopN, n)              //[P, 9]
       }
/*======================================================================*/

/*======================================================================*/
         .balign 32
.L_loopN:
       { 
         j1j0.b  = vadd(i1i0.b,i1i0.b)           //[1,10]
         i1i0    = vand(i1i0,vc8)                //[1,10]
         d0.cur = vmem(ptr_x++#2)                //[2, 0]
         q0 = vcmp.eq(vzero.h, d0.h)             //[2, 0]
       } { 
         j1j0    = vand(j1j0,vc7)                //[1,11]
         d1.cur = vmem(ptr_x+#-1)                //[2, 1]
         q1 = vcmp.eq(vzero.h, d1.h)             //[2, 1]
         d0 = vmux(q0, vone, d0)                 //[2, 1]
       } { 
         c11_c10.h = vlut16(i3i2.b, inv1.h, c0)  //[0,22]
         x1 = vand(d1x3, vc1fff)                 //[0,22]
         d1 = vmux(q1, vone, d1)                 //[2, 2]
       } { 
         y0.h = vmpy(x0.h, c20.h):<<1:rnd:sat    //[0,23]
         qexp0.h = vsub(exp0.h,vqpoint.h)       //[1,13]
         vmem(ptr_ye++#2) = qexp0.new            //[1,13]
	 exp0.h = vnormamt(d0.h)                 //[2, 3]
       } { 
         y1.h = vmpy(x1.h, c21.h):<<1:rnd:sat    //[0,24]
         i1i0 = vor(i1i0, j1j0)                  //[1,14]
         exp1.h = vnormamt(d1.h)                 //[2, 4]
       } { 
         c01_c00.h = vlut16(i3i2.b, inv0.h, c0)  //[0,25]
         y0.h = vadd(y0.h, c10.h)                //[0,25]
         qexp1.h = vsub(exp1.h, vqpoint.h )      //[2, 5] 
         vmem(ptr_ye+#1) = qexp1.new             //[2, 5]
       } { 
         y1.h = vadd(y1.h, c11.h)                //[0,26]
         i3i2.uh = vlsr(i1i0.uh, c4)             //[1,16] 
         d0x2.h = vadd(d0x2.h, d0x2.h)           //[1,16]
         d1x3.h = vadd(d1x2.h, d1x2.h)           //[1,16]
       } { 
         y0.h = vmpy(x0.h, y0.h):<<1:rnd:sat     //[0,27]
         x0 = vand(d0x2, vc1fff)                 //[1,17] 
         d0x2.h = vasl(d0.h, exp0.h)             //[2, 7]
       } { 
         y1.h = vmpy(x1.h, y1.h):<<1:rnd:sat     //[0,28]
         y0.h = vadd(y0.h, c00.h)                //[0,28]
         if(p3) vmem(ptr_ym++#1) = y0.new        //[0,28]
         d1x2.h = vasl(d1.h, exp1.h)             //[2, 8]
       } {
         y1.h = vadd(y1.h, c01.h)                //[0,29]
         if(p3) vmem(ptr_ym++#1) = y1.new        //[0,29]
         c21_c20.h = vlut16(i3i2.b, inv2.h, c0)  //[1,19]
         i1i0.b = vshuffo(d1x2.b, d0x2.b)        //[2, 9]
       }:endloop0 
/*======================================================================*/

/*======================================================================*/
       { 
         c11_c10.h = vlut16(i3i2.b, inv1.h, c0)  //[2,22]
         x1 = vand(d1x3, vc1fff)                 //[2,22]
       } { 
         y0.h = vmpy(x0.h, c20.h):<<1:rnd:sat    //[2,23]
       } { 
         y1.h = vmpy(x1.h, c21.h):<<1:rnd:sat    //[2,24]
       } { 
         c01_c00.h = vlut16(i3i2.b, inv0.h, c0)  //[2,25]
         y0.h = vadd(y0.h, c10.h)                //[2,25]
       } { 
         y1.h = vadd(y1.h, c11.h)                //[2,26]
       } { 
         y0.h = vmpy(x0.h, y0.h):<<1:rnd:sat     //[2,27]
       } { 
         y1.h = vmpy(x1.h, y1.h):<<1:rnd:sat     //[2,28]
         y0.h = vadd(y0.h, c00.h)                //[2,28]
         vmem(ptr_ym++#1) = y0.new               //[2,28]
       } {
         y1.h = vadd(y1.h, c01.h)                //[2,29]
        vmem(ptr_ym++#1) = y1.new               //[2,29]
       } {  jumpr r31                               //[E, 7]
       }
/*----------------------------------------------------------------*/
.L_end:
      .size hvx_invert, .L_end-hvx_invert

   .section .data
    .p2align 7
hvx_invert_lut:
    .hword  16384, 0, 14563, 0, 13107, 0, 11915, 0, 10923, 0, 10082, 0, 9362,  0, 8738,  0
    .hword -8192,  0, -8738, 0, -9362, 0,-10082, 0,-10923, 0,-11915, 0,-13107, 0,-14563, 0
    .hword  0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0
    .hword  0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0
    .hword -16265, 0,-12869, 0,-10434, 0,-8630,  0,-7256,  0,-6185,  0,-5335,  0,-4649,  0
    .hword -4084,  0,-4649,  0,-5335,  0,-6185,  0,-7248,  0,-8630,  0,-10434, 0,-12865, 0
    .hword  0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0
    .hword  0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0
    .hword  13718, 0, 9816 , 0, 7267 , 0, 5528 , 0, 4304 , 0, 3415 , 0, 2756 , 0, 2255 , 0
    .hword -2255 , 0,-2756 , 0,-3415 , 0,-4304 , 0,-5528 , 0,-7267 , 0,-9816 , 0,-13718, 0
    .hword  0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0
    .hword  0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0,     0, 0
														 
														 
    /*[*****************************************************************************]*/
    /*[  Function   : result_scale_t dsplib_invert()                                ]*/
    /*[*****************************************************************************]*/
    /*[  Description: calculate inversion of Wor16 with long devision               ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - Input : Q0                                                      ]*/
    /*[           - Return:                                                         ]*/
    /*[             R1:0 = (shift_factor : result)                                  ]*/
    /*[             where                                                           ]*/
    /*[             - result has 16-bit precision in LSB                            ]*/
    /*[                                                                             ]*/
    /*[             - Decimal-point:                                                ]*/
    /*[               If input is in Qi and the output is Qo, then                  ]*/
    /*[               o = (*shift_factor) -i                                        ]*/
    /*[                                                                             ]*/
    /*[               For example, input is 0x0F00 in Q10, i.e., x= 3.75            ]*/
    /*[               the function returns                                          ]*/
    /*[               result = 17476, shift_factor is 26, and thus the              ]*/
    /*[               actual result is in Q(26-10), i.e., inversion is              ]*/
    /*[               17476/(2^16) = 0.26666260                                     ]*/
    /*[                                                                             ]*/
    /*[             - if input <=0:  return                                         ]*/
    /*[               result       = 0xFFFFFFFF                                     ]*/
    /*[               shift_factor = 0xFFFFFFFF                                     ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word16 x                                                 ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R4                                                      ]*/
    /*[  Hardware Loops affected: None                                              ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 18                                                              ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl dsplib_invert
    .type    dsplib_invert, @function
dsplib_invert:
    { R2 = #0x4000                          //[ constant                            ]
      R3 = ASLH(R0)                         //[ L_denom = input<<16                 ]
      R4.H = #0x8000                        //[ L_num = 0x80000000                  ]
      R0 = #0                               //[ initialize result=0                 ]
    }
    { P0 = CMP.GT(R3,#0)                    //[ P0 = (input >0)                     ]
      IF !P0.new JUMP:nt .dsplib_invert_LE0 //[ if (input<=0) jump invert_LE0       ] 
      R4.L = #0                             //[ L_num = 0x80000000                  ]
      R5 = NORMAMT(R3)                      //[ nshft = norm_l(L_denom)             ]
    }
    { R3 = ASL(R3,R5)                       //[ L_denom <<= nshft                   ]
      R1 = SUB(#29,R5)                      //[ nshft = 29 - nshft                  ]
      LOOP0(.dsplib_invert_LOOP,#14)        //[ setup loop (unroll once)            ]
    }
    .falign
.dsplib_invert_LOOP:
    { P0 = CMP.GTU(R3,R4)                   //[ if (L_num>= L_denom)                ]
      IF !P0.new R4 = SUB(R4,R3)            //[  then L_num -= L_denom              ]
      IF !P0.new R0 = ADD(R0,R2)            //[  then  result++                     ]
      R3:2 = VLSRW(R3:2,#1)                 //[ equivalent to L_denom <<1, result<<1]
    }:endloop0

    { P0 = CMP.GTU(R3,R4)                   //[ if (L_num>= L_denom)                ]
      IF !P0.new R0 = ADD(R0,R2)            //[  then  result++                     ]
      JUMPR R31                             //[ return                              ]
    }
.dsplib_invert_LE0:
    { R1:0 = COMBINE(#-1,#-1)               //[ R1:0 = -1 : -1                      ]
      JUMPR R31                             //[ return                              ]
    }
    .size    dsplib_invert, .-dsplib_invert

    /*[*****************************************************************************]*/
    /*[  Function   : result_scale_t dsplib_approx_invert()                         ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Approximates inversion: out ~= 2^(31-floor(lg2(in)))/in       ]*/
    /*[               Approximation done with a linearly interpolated lookup table. ]*/
    /*[               With 9 point entries (8 line segments) the maximum error is   ]*/
    /*[               0.238%. The number to be inverted must be positive for valid  ]*/
    /*[               results. If not positive, then the lookup table is invalidly  ]*/
    /*[               indexed.                                                      ]*/
    /*[               If input is Qi and the output is Qo, then                     ]*/
    /*[               Qo = 32 + (*shift_factor) - Qi.                               ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - Return:                                                         ]*/
    /*[             R1:0 = (shift_factor : result)                                  ]*/
    /*[             where                                                           ]*/
    /*[               result       -  Q-shifted inverse                             ]*/
    /*[                               result ~= 2^(31-floor(lg2(input))) / input    ]*/
    /*[                                       = 2^(32+(*shift_factor)) / input      ]*/
    /*[                                                                             ]*/
    /*[               shift_factor - (output_Q_factor - 32) of integer inverse      ]*/
    /*[                             shift_factor = -1-floor(lg2(input))             ]*/
    /*[                                                                             ]*/
    /*[             if input <=0:  return                                           ]*/
    /*[               result       = 0xFFFFFFFF                                     ]*/
    /*[               shift_factor = 0xFFFFFFFF                                     ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word32 x                                                 ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R5                                                      ]*/
    /*[  Hardware Loops affected: None                                              ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 6                                                               ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/


    .text
    .p2align 2
    .p2align 4,,15
    .globl dsplib_approx_invert
    .type    dsplib_approx_invert, @function
dsplib_approx_invert:
    { R4 = NORMAMT(R0)                      //[ r = norm_l(input)                   ]
      R3 = R0                               //[ save input into R3                  ]
      P0 = CMP.GT(R0,#0)                    //[ P0 = (input>0)?                     ]
	}
	// Optimize later
	{
      R2 = ADD(PC,##dsplib_approx_invert_lut@PCREL)            
    }
    { R1:0 = COMBINE(#-1,#-1)               //[ result=-1, shift_factor=-1          ]
      IF !P0 JUMPR R31                      //[ if(input<=0) return                 ]
      R3 = ASL(R3,R4)                       //[ norm_divisor                        ]
    }
    { R2 = TABLEIDXH(R3,#(DSP_INV_LUT_BITS-1),#(31-DSP_INV_LUT_BITS))
                                            //[ R2 = &dsplib_approx_invert_lut[index]               ]
      R3 = ASR(R3,#(15-DSP_INV_LUT_BITS))   //[ norm_divisior>>(15-DSP_INV_LUT_BITS)]
      R1 = ADD(R4,#-31)                     //[ shift_fact = r-31                   ]
    }
    { R4 = MEMH(R2)                         //[ R4 = dsplib_approx_invert_lut[index]                ]
      R5 = MEMH(R2+#2)                      //[ R5 = dsplib_approx_invert_lut[index+1]              ]
    }
    { R0 = ASLH(R4)                         //[ dsplib_approx_invert_lut[index]<<16                 ]
      R4 = SUB(R4,R5)                       //[ dsplib_approx_invert_lut[index+1]-dsplib_approx_invert_lut[index]   ]
    }
    { R0 -= MPYU(R3.L,R4.L)                 //[ result=dsplib_approx_invert_lut[index]<<16 + interp*]
                                            //[ (dsplib_approx_invert_lut[index+1]-dsplib_approx_invert_lut[index]) ]
      JUMPR R31                             //[ return                              ]
    }
    .size    dsplib_approx_invert, .-dsplib_approx_invert

    /*-----------------------------------------------------------------------------*/
    /*            Lookup table for dsplib_approx_invert                            */
    /*-----------------------------------------------------------------------------*/

    .section .data
    .p2align DSP_INV_LUT_BITS
dsplib_approx_invert_lut:
    .hword    32690,29066,26171,23798,21820,20145,18709,17463
    .hword    16373
		
	
    /*[*****************************************************************************]*/
    /*[  Function   : Word32 dsplib_sqrt()                                      ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Approximates sqrt(in)                                         ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - if input in Q0, output is in Q16                                ]*/
    /*[           - input > 0                                                       ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word32 input                                             ]*/
    /*[               R1 : Word32 round_factor                                      ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R5                                                      ]*/
    /*[  Hardware Loops affected: None                                              ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 8                                                               ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl dsplib_sqrt
    .type    dsplib_sqrt, @function
dsplib_sqrt:
    { P0 = CMP.GT(R0,#0)                    //[ P0 = (input>0)?                     ]
      IF !P0.new JUMP:nt .dsplib_sqrtlut_excpt                 
                                            //[ if (input<=0) return (-1 or 0)      ]
      R2 = NORMAMT(R0)                      //[ r2 = norm_l(input)                  ]
    }
	
	// Optimize later
	{
      R5 = ADD(PC,##dsplib_sqrt_lut@PCREL)            
    }
	{
      R5 = ADD(R5,#-16)            
    }

    { P1 = TSTBIT(R2,#0)                    //[ P1 = !(r2&1)                        ]
      IF  P1.new R4 = ADD(R2,#-10)          //[ if(r2&1) R4 = r2-10                 ]
      IF !P1.new R4 = ADD(R2,#-11)          //[ else R4 = r2-11                     ]
    }
    { R0 = ASL(R0,R4)                       //[ R0 = n1                             ]
      R2 = NEG(R2)                          //[ -r2                                 ]
      R3 = #2                               //[ used for address                    ]
    }
    { R5 += MPYU(R0.H,R3.L)                 //[ &dsplib_sqrt_lut[index]                   ]
    }
    { R3 = MEMUH(R5)                        //[ dsplib_sqrt_lut[index]                    ]
      R4 = MEMUH(R5+#2)                     //[ dsplib_sqrt_lut[index+1]                  ]
      R2 = ASR(R2,#1)                       //[ r = (-r2)>>1                        ]
    }
    { R3 = ASLH(R3)                         //[ dsplib_sqrt_lut[index]<<16                ]
      R4 = SUB(R4,R3)                       //[ dsplib_sqrt_lut[index+1]-dsplib_sqrt_lut[index] ]
      R2 = ADD(R2,#1)                       //[ R2 = r+1                            ]
    }
    { R3 += MPYU(R0.L,R4.L)                 //[ interpolation and R3= val           ]
      R0 = R1                               //[ R0 = round_factor                   ]
    }
    { R0 += ASL(R3,R2)                      //[ result = val << (r+1) + round_factor] 
      JUMPR R31                             //[ return                              ]
    }

.dsplib_sqrtlut_excpt:
    { P0 = CMP.EQ(R0,#0)                    //[ if (input<0) return -1              ]
      IF !P0.new R0 = #-1                   //[ if (input==0) return 0              ]
      JUMPR R31                             //[ return                              ]
    }
    .size    dsplib_sqrt, .-dsplib_sqrt


    /*-----------------------------------------------------------------------------*/
    /*            Lookup table for dsplib_sqrt                                 */
    /*-----------------------------------------------------------------------------*/
    .section .data
    .p2align 3
dsplib_sqrt_lut:
    .hword    16389,17382,18322,19215,20069,20888,21676,22437
    .hword    23172,23885,24578,25251,25907,26546,27171,27782
    .hword    28379,28964,29538,30100,30652,31195,31728,32253
    .hword    -32767
	
    /*================================================================================================*/
    /*  FUNCTION     : hvx_float2frac                                                                 */
    /*================================================================================================*/
    /*  PARAMETERS   :                                                                                */
    /*  ============                                                                                  */
    /* R0 : valueVec  (assumed 128B aligned)                                                          */
    /* R1 : fracBitsVec (assumed 128B aligned)                                                        */
    /* R2 : dst (assumed 128B aligned)                                                                */
    /* R3 : widthVecs (number of iterations, i.e. number of vectors wide. Assumed >= 1)               */
    /*================================================================================================*/
    /***************************************************************************************************
    * IMPLEMENTATION SUMMARY                                                                           *
    * ======================                                                                           *
    ****************************************************************************************************/
    /*================================================================================================*/
    /*  STACK USAGE                                                                                   */
    /*================================================================================================*/

   .text
   .p2align 4                          // ensures 16-byte alignment of first packet
   .globl hvx_float2frac               // makes function have global scope
   .type       hvx_float2frac, @function
hvx_float2frac:
#if (__HEXAGON_ARCH__ < 60)
   JUMPR R31                           // just return (??)
#else
   {
     P3 = SP1LOOP0(.float2frac_loop,R3)// loop(N)
     R3 = ##0x007FFFFF                 // 23 LSB's mask
     R4 = #0x0100                      // sign bit position in float exponent = const 0x100
   }{
     V29.b = VSUB(V29.b,V29.b)         // 0
     R5 = #0x1f                        // 0x1f
     R15 = #23                         // 23
     R14 = #0xFF                       // exp bitmask
   }{
     V30 = VSPLAT(R4)                  // splat sign bit position in float exponent = const 0x100
     R4 = #150                         // const 150
     V31 = VSPLAT(R3)                  // 23 LSB's mask
     R3 = ADD(R3,#1)                   // 0x00800000, i.e. implied integer bit in float format
   }{
     V25 = VSPLAT(R5)                  // splat 0x1F
     V27 = VSPLAT(R14)                 // exp bitmask
     R6 = #1                           // 1
   }{
     V28 = VSPLAT(R3)                  // 0x00800000, i.e. implied integer bit in float format
     V26 = VSPLAT(R4)                  // const 150
   }{
     V24 = VSPLAT(R6)
   }
   .balign 32
.float2frac_loop:
   {
     V0 = VMEM(R0++#1)                 // valueVec[i]
     V10.w = VASL(V24.w,V10.w)           // [2] rounding const = 1 << (-shift[i] - 1), used for negative shift values only
   }{
     V1 = VMEM(R1++#1)                 // formatVec[i]
     V2 = VAND(V0,V31)                 // mant[i]
     V22.w = VADD(V2.w,V10.w)           // [2] mant[i] += rounding const
   }{
     V2 = VOR(V2,V28)                  // mant[i] with implied integer bit
     V3.w = VASR(V0.w,R15)             // sign[i] : exp[i]
     V4.w = VSUB(V26.w,V1.w)           // temp[i] = 150 - formatVec[i]
     V22 = VMUX(Q0,V29,V22)              // [2] if (exp[i] == 0 || shift[i] < -0x20) dst[i] = 0
   }{
     V3 = VAND(V3,V27)                 // exp[i]
     V9.w = VASR(V22.w,V7.w)            // [2] mant[i] >>= -shift[i]
   }{
     Q0 = VCMP.EQ(V3.w,V29.w)          // exp[i] == 0
     V3.w = VSUB(V3.w,V4.w)            // shift[i] = exp[i] - (150 - formatVec[i])
     V9 = VMUX(Q1,V9,V8)               // [2] if shift[i] < 0, use shift right by -shift[i], else shift left by shift[i]
   }{
     Q1 = VCMP.GT(V29.w,V3.w)          // (exp[i] < 0)
     V7.w = VSUB(V29.w,V3.w)           // -shift[i]
     V19.w = VSUB(V29.w,V9.w)           // [2] -dst[i]
     }{
     V8.w = VASL(V2.w,V3.w)            // mant[i] <<= shift[i]
     Q0 |= VCMP.GT(V7.w,V25.w)         // -shift[i] < 0x20?
     V10.w = VSUB(V7.w,V24.w)          // -shift[i] - 1
     V9 = VMUX(Q3,V19,V9)              // [2] use -dst[i] if original sign was negative
     }{
     Q3 = VCMP.GT(V29.w,V0.w)          // valueVec[i] < 0  (though numbers are float format, sign bit is in same place)
     V8 = VMUX(Q0,V29,V8)              // if (exp[i] == 0 || shift[i] < -0x20) dst[i] = 0
     IF (P3) VMEM(R2++#1) = V9         // [2] store integers
   }:endloop0
   {
     V10.w = VASL(V24.w,V10.w)           // [2] rounding const = 1 << (-shift[i] - 1), used for negative shift values only
     }{
     V22.w = VADD(V2.w,V10.w)           // [2] mant[i] += rounding const
     }{
     V22 = VMUX(Q0,V29,V22)              // [2] if (exp[i] == 0 || shift[i] < -0x20) dst[i] = 0
     }{
     V9.w = VASR(V22.w,V7.w)            // [2] mant[i] >>= -shift[i]
     }{
     V9 = VMUX(Q1,V9,V8)               // [2] if shift[i] < 0, use shift right by -shift[i], else shift left by shift[i]
     }{
     V19.w = VSUB(V29.w,V9.w)           // [2] -dst[i]
     }{
     V9 = VMUX(Q3,V19,V9)              // [2] use -dst[i] if original sign was negative
     }{
     VMEM(R2++#1) = V9         // [2] store integers
     }{
     JUMPR R31                         // return
   }
#endif                                 // __HEXAGON_ARCH__ < 60
   .size       hvx_float2frac, .-hvx_float2frac
	