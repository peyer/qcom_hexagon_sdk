    /* ============================================================================ */
    /*  QUALCOMM TECHNOLOGIES, INC.                                                 */
    /*                                                                              */
    /*  HEXAGON HVX Image/Video Processing Library                                  */
    /*                                                                              */
    /* ---------------------------------------------------------------------------- */
    /*            Copyright (c) 2014 QUALCOMM TECHNOLOGIES Incorporated.            */
    /*                             All Rights Reserved.                             */
    /*                    QUALCOMM Confidential and Proprietary                     */
    /* ============================================================================ */
    .file    "wiener9x9.S"

#include "hvx.cfg.h"
    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void blending()                                               ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: delta vertical computation initialization                     ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned char  *src     -- pointer to input              ]*/
    /*[               R1 : unsigned char  *rgmean  -- pointer to mean               ]*/
    /*[               R2 : unsigned short *rgvar   -- pointer to value for 1/var    ]*/
    /*[               R3 : short          *rgshft  -- pointer to shift for 1/var    ]*/
    /*[               R4 : unsigned char  *dst     -- pointer to dst                ]*/
    /*[               R5 : unsigned char   noise   -- noise                         ]*/
    /*[             SP+#0: int             width   -- width                         ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
    /*[           -                                                                 ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  ASSUMPTIONS:                                                               ]*/
    /*[           - src, rgmean, rgvar, rgshft, dst, are aligned by VLEN            ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  REVISION HISTORY                                                           ]*/
    /*[  ----------------                                                           ]*/
    /*[  Version        Date                    Comments                            ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[   6.0.0         08/01/2014              created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define     sVar0            V0
#define     sVar1            V1
#define     sShft0_L         V2
#define     sShft0_H         V3
#define     dShft0           V3:2
#define     sShft1_L         V4
#define     sShft1_H         V5
#define     dShft1           V5:4
#define     sNvar0_L         V6
#define     sNvar0_H         V7
#define     dNvar0           V7:6
#define     sNvar1_L         V8
#define     sNvar1_H         V9
#define     dNvar1           V9:8
#define     sMaxvalh         V10
#define     sMaxvalb         V11
#define     sAlpha           V12
#define     sOneMalpha       V13
#define     sDst_L           V14
#define     sDst_H           V15
#define     dDst             V15:14
#define     sMean            V16
#define     sSrc             V17
#define     sNv0             V18
#define     sNv1             V19
#define     sOut             V20
#define     sConst8          V21
#define     sZero            V22
#define     sTemp            V23
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl blending
    .type   blending, @function
blending:
    { R7 = MEMW(r29+#0)                             // width
      R5 = COMBINE(R5.L,R5.L)                       // noise
      R9 = ##0x80008                                // 15-BIT
    }
    { R8 = ADD(R7,#VLEN-1)                          // width + VLEN - 1
      sConst8 = VSPLAT(R9)                          // 15-BIT
      R9 = MPYI(R9,#16)                             // 1<<BIT in halfword
      R11:10 = COMBINE(R4,R0)                       // dDst:sSrc
    }
    { R8 = ASR(R8,#LOG2VLEN)                        // (width+VLEN-1)>>LOG2VLEN
      sMaxvalh = VSPLAT(R9)                         // 1<<BIT in halfword
      R9 = ##0x80808080                             // 1<<BIT in byte
    }
    { P3 = SP2LOOP0(.blending_LP,R8)                // loop
      sZero = #0                                    // 0
      sMaxvalb = VSPLAT(R9)                         // 1<<BIT in byte
      R6 = #7                                       // BIT
    }
    .falign
.blending_LP:
    { sVar0.cur = VMEM(R2++#1)                      //[1]
      dNvar0.uw = VMPY(sVar0.uh,R5.uh)              //[1]
      sNv1.h = VSHUFFE(sNvar1_H.h,sNvar1_L.h)       //[2]
    }
    { sVar1.tmp = VMEM(R2+#0)                       //[1]
      dNvar1.uw = VMPY(sVar1.uh,R5.uh)              //[1]
      sNv0 = VMUX(Q0,sMaxvalh,sNv0)                 //[2]
      sNv1 = VMUX(Q1,sMaxvalh,sNv1)                 //[2]
    }
    { sTemp.tmp = VMEM(R3++#1)                      //[1]
      dShft0.w = VADD(sConst8.uh,sTemp.uh)          //[1]
      Q0 = VCMP.EQ(sVar0.uh,sZero.uh)               //[1]
      sOut.ub = VASR(sDst_H.h,sDst_L.h,R6):rnd:sat  //[3]
    }
    { sTemp.tmp = VMEM(R3++#1)                      //[1]
      dShft1.w = VADD(sConst8.uh,sTemp.uh)          //[1]
      sNv0.ub = VPACK(sNv1.h,sNv0.h):sat            //[2]
    }
    { sNvar0_L.w = VLSR(sNvar0_L.w,sShft0_L.w)      //[1]
      sVar1.cur = VMEM(R2++#1)                      //[1]
      Q1 = VCMP.EQ(sVar1.uh,sZero.uh)               //[1]
      sAlpha.ub = VMIN(sNv0.ub,sMaxvalb.ub)         //[2]
    }
    { sNvar0_H.w = VLSR(sNvar0_H.w,sShft0_H.w)      //[1]
      sOneMalpha.ub = VSUB(sMaxvalb.ub,sAlpha.ub):sat//[2]
      IF P3 VMEM(R4++#1) = sOut                     //[3]
      IF P3 R0 = ADD(R0,R8)                         //[3]
    }
    { sNvar1_L.w = VLSR(sNvar1_L.w,sShft1_L.w)      //[1]
      dDst.uh = VMPY(sMean.ub,sAlpha.ub)            //[2]
      R8 = #VLEN                                    //[1]
    }
    { sNvar1_H.w = VLSR(sNvar1_H.w,sShft1_H.w)      //[1]
      sSrc.tmp = VMEM(R0+#0)                        //[2]
      dDst.uh += VMPY(sSrc.ub,sOneMalpha.ub)        //[2]
    }
    { sNv0.h = VSHUFFE(sNvar0_H.h,sNvar0_L.h)       //[1]
      sMean = VMEM(R1++#1)                          //[1]
    }:endloop0
    { sNv1.h = VSHUFFE(sNvar1_H.h,sNvar1_L.h)       //[2]
    }
    { sNv0 = VMUX(Q0,sMaxvalh,sNv0)                 //[2]
      sNv1 = VMUX(Q1,sMaxvalh,sNv1)                 //[2]
    }
    { sOut.ub = VASR(sDst_H.h,sDst_L.h,R6):rnd:sat  //[3]
    }
    { sNv0.ub = VPACK(sNv1.h,sNv0.h):sat            //[2]
      IF P3 VMEM(R4++#1) = sOut                     //[3]
      IF P3 R0 = ADD(R0,R8)                         //[3]
    }
    { sAlpha.ub = VMIN(sNv0.ub,sMaxvalb.ub)         //[2]
    }
    { sOneMalpha.ub = VSUB(sMaxvalb.ub,sAlpha.ub):sat//[2]
    }
    { dDst.uh = VMPY(sMean.ub,sAlpha.ub)            //[2]
    }
    { sSrc.tmp = VMEM(R0++#1)                       //[2]
      dDst.uh += VMPY(sSrc.ub,sOneMalpha.ub)        //[2]
    }
    { R0 = MEMW(R10++#-4)                           //
      MEMW(R11++#-4) = R0.new                       //
    }
    { sOut.ub = VASR(sDst_H.h,sDst_L.h,R6):rnd:sat  //
      VMEM(R4++#1) = sOut.new                       //[3]
    }
    { R0 = MEMW(R10+R7<<#0)                         //
      MEMW(R11+R7<<#0) = R0.new                     //
      JUMPR R31                                     //
    }
    .size   blending, .-blending

    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void deltainit()                                              ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: delta vertical computation initialization                     ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned char  *src    -- pointer to input               ]*/
    /*[               R1 : unsigned short *rgm    -- pointer to output m            ]*/
    /*[               R2 : unsigned       *rgm2   -- pointer to output m^2          ]*/
    /*[               R3 : int             width  -- width                          ]*/
    /*[               R4 : int             stride -- stride                         ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
    /*[           -                                                                 ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  ASSUMPTIONS:                                                               ]*/
    /*[           - src, rgm, rgm2 are aligned by VLEN                              ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  REVISION HISTORY                                                           ]*/
    /*[  ----------------                                                           ]*/
    /*[  Version        Date                    Comments                            ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[   6.0.0         08/01/2014              created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define     sIn_5            V0
#define     sIn_4            V1
#define     dIn_45           V1:0
#define     sIn_3            V2
#define     sIn_2            V3
#define     dIn_23           V3:2
#define     sIn_1            V4
#define     sIn0             V5
#define     dIn_01           V5:4
#define     sIn1             V6
#define     sIn2             V7
#define     dIn21            V7:6
#define     sIn3             V8
#define     sSumx0_L         V10
#define     sSumx0_H         V11
#define     dSumx0           V11:10
#define     sTmp0_L          V12
#define     sTmp0_H          V13
#define     dTmp0            V13:12
#define     sTmp1_L          V14
#define     sTmp1_H          V15
#define     dTmp1            V15:14
#define     sTmp2_L          V16
#define     sTmp2_H          V17
#define     dTmp2            V17:16
#define     sTmp3_L          V18
#define     sTmp3_H          V19
#define     dTmp3            V19:18
#define     sSqsum0          V20
#define     sSqsum2          V21
#define     dSqsum02         V21:20
#define     sSqsum1          V22
#define     sSqsum3          V23
#define     dSqsum13         V23:22
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl deltainit
    .type   deltainit, @function
deltainit:
    { R8 = ADD(R3,#VLEN-1)                          // width+VLEN-1
      R5 = MPYI(R4,#-8)                             // -8*stride
      R0 -= MPYI(R4,#5)                             // in[-5][x]
      R6.L = #1                                     // 1
    }
    { R8 = ASR(R8,#LOG2VLEN)                        // (width+VLEN-1)>>LOG2VLEN
      R5 = ADD(R5,#VLEN)                            // -8*stride+VLEN
      R9 = #VLEN                                    //
    }
    { P3 = SP1LOOP0(.deltainit_LP,R8)               // loop
      R7 = #-2                                      //
      R3 = #-4                                      //
    }
    { C7:6 = R5:4                                   // -8*stride+VLEN:stride
      R5 = ##0x1010101                              // 1
      R6.H = #1                                     // 1
    }
    .falign
.deltainit_LP:
    // Add workaound for P-Vmem HW bug: predicate to output pointer
    { sIn_5 = VMEM(R0++M0)                          //[1]
      dSqsum02 = VSHUFF(sSqsum2,sSqsum0,R3)         //[2]
      VMEM(R1+#0) = sSumx0_H                        //[2]
      IF P3 R1 = ADD(R1,R9)                         //[2]
    }
    { sIn_4 = VMEM(R0++M0)                          //[1]
      dSqsum13 = VSHUFF(sSqsum3,sSqsum1,R3)         //[2]
      VMEM(R2+#0) = sSqsum0                         //[2]
      IF P3 R2 = ADD(R2,R9)                         //[2]
    }
    { sIn_3 = VMEM(R0++M0)                          //[1]
      dTmp0.b = VSHUFFOE(sIn_4.b,sIn_5.b)           //[1]
      VMEM(R2+#0) = sSqsum1                         //[2]
      IF P3 R2 = ADD(R2,R9)                         //[2]
    }
    { sIn_2 = VMEM(R0++M0)                          //[1]
      dSumx0.h = VMPA(dIn_45.ub,R5.b)               //[1]
      VMEM(R2+#0) = sSqsum2                         //[2]
      IF P3 R2 = ADD(R2,R9)                         //[2]
    }
    { sIn_1 = VMEM(R0++M0)                          //[1]
      dTmp1.b = VSHUFFOE(sIn_2.b,sIn_3.b)           //[1]
      VMEM(R2+#0) = sSqsum3                         //[2]
      IF P3 R2 = ADD(R2,R9)                         //[2]
    }
    { sIn0 = VMEM(R0++M0)                           //[1]
      dSumx0.h += VMPA(dIn_23.ub,R5.b)              //[1]
      sIn_5.h = VSHUFFE(sTmp1_L.h,sTmp0_L.h)        //[1]
    }
    { sIn1 = VMEM(R0++M0)                           //[1]
      sIn_4.h = VSHUFFO(sTmp1_L.h,sTmp0_L.h)        //[1]
      dIn_23.h = VSHUFFOE(sTmp1_H.h,sTmp0_H.h)      //[1]
    }
    { sIn2 = VMEM(R0++M0)                           //[1]
      dSumx0.h += VMPA(dIn_01.ub,R5.b)              //[1]
      sTmp0_L.b = VSHUFFE(sIn0.b,sIn_1.b)           //[1]
    }
    { sIn3 = VMEM(R0++M1)                           //[1]
      sSqsum0.uw = VRMPY(sIn_5.ub,sIn_5.ub)         //[1]
      sSqsum2.uw = VRMPY(sIn_4.ub,sIn_4.ub)         //[1]
      sTmp0_H.b = VSHUFFO(sIn0.b,sIn_1.b)           //[1]
    }
    { sSqsum1.uw = VRMPY(sIn_3.ub,sIn_3.ub)         //[1]
      sSqsum3.uw = VRMPY(sIn_2.ub,sIn_2.ub)         //[1]
      dTmp1.b = VSHUFFOE(sIn1.b,sIn2.b)             //[1]
    }
    { dSumx0.h += VMPA(dIn21.ub,R5.b)               //[1]
      dIn_01.h = VSHUFFOE(sTmp1_L.h,sTmp0_L.h)      //[1]
    }
    { dIn21.h = VSHUFFOE(sTmp1_H.h,sTmp0_H.h)       //[1]
      dTmp2.uh = VMPY(sIn3.ub,sIn3.ub)              //[1]
    }
    { sSqsum0.uw += VRMPY(sIn_1.ub,sIn_1.ub)        //[1]
      dTmp3.uw = VZXT(sTmp2_L.uh)                   //[1]
    }
    { sSqsum2.uw += VRMPY(sIn0.ub,sIn0.ub)          //[1]
      dTmp2.uw = VZXT(sTmp2_H.uh)                   //[1]
    }
    { sSqsum1.uw += VRMPY(sIn1.ub,sIn1.ub)          //[1]
      sSqsum3.w = VADD(sSqsum3.w,sTmp2_H.w)         //[1]
      sSqsum0.w = VADD(sSqsum0.w,sTmp3_L.w)         //[1]
    }
    { dSumx0.h += VMPY(sIn3.ub,R5.b)                //[1]
      sSqsum1.w = VADD(sSqsum1.w,sTmp2_L.w)         //[1]
      sSqsum2.w = VADD(sSqsum2.w,sTmp3_H.w)         //[1]
    }
    { sSqsum3.uw += VRMPY(sIn2.ub,sIn2.ub)          //[1]
      VMEM(R1++#1) = sSumx0_L                       //[1]
    }:endloop0
    { dSqsum02 = VSHUFF(sSqsum2,sSqsum0,R3)         //[2]
      VMEM(R1++#1) = sSumx0_H                       //[2]
    }
    { dSqsum13 = VSHUFF(sSqsum3,sSqsum1,R3)         //[2]
      VMEM(R2++#1) = sSqsum0                        //[2]
    }
    { VMEM(R2++#1) = sSqsum1                        //[2]
    }
    { VMEM(R2++#1) = sSqsum2                        //[2]
    }
    { VMEM(R2++#1) = sSqsum3                        //[2]
    }
    { JUMPR R31                                     //
    }
    .size   deltainit, .-deltainit

    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void horboxfilter()                                           ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: horizontal box filter                                         ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned short *rgmean   -- pointer to output            ]*/
    /*[               R1 : unsigned short *rgm      -- input                        ]*/
    /*[               R1 : int width                -- width                        ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
    /*[           -                                                                 ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  ASSUMPTIONS:                                                               ]*/
    /*[           - *rgmean, rgm are aligned by VLEN                                ]*/
    /*[           - loop count is multiple of 2*VLEN                                ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  REVISION HISTORY                                                           ]*/
    /*[  ----------------                                                           ]*/
    /*[  Version        Date                    Comments                            ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[   6.0.0         08/01/2014              created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define     sL0              V0
#define     sL1              V1
#define     sEven0           V2
#define     sEven1           V3
#define     sOdd0            V4
#define     sOdd1            V5
#define     sEvenodd0        V6
#define     sEvenodd1        V7
#define     sEven0s          V8
#define     sDelay1          V9
#define     sDelay2          V10
#define     sDelay3          V11
#define     sDelay4          V12
#define     sSum             V13
#define     dHout            V15:14
#define     sHout_H          V15
#define     sHout_L          V14
#define     sSumo            V16
#define     sSume            V17
#define     sS10             V18
#define     sOutdelay        V19
#define     sOutalign        V20
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl horboxfilter
    .type    horboxfilter, @function
horboxfilter:
    { R3 = #8                                       // 8
      R5:4 = COMBINE(#8,#VLEN-4)                    // 8, VLEN-4
      R7:6 = COMBINE(#1,#-2)                        // 1, -2
      sEven0 = VMEM(R1++#1)                         //[0] load input
    }
    { R8 = ADD(R2,#VLEN-1-2*VLEN)                   // width+VLEN-1
      sHout_L = #0                                  // init
      sOdd1 = VMEM(R1++#1)                          //[0] load input
    }
    { R8 = ASR(R8,#LOG2VLEN)                        // (width+VLEN-1)>>LOG2VLEN
      sL0.cur = VMEM(R1++#1)                        //[0] load input
      sEven0s = VALIGN(sL0,sEven0,#2)               //[0] even[-1]
      sEven0 = sL0                                  //[0] delay
    }
    { P3 = SP2LOOP0(.horboxfilter_LP,R8)            // loop
      sEvenodd1.h = VADD(sEven0s.h,sOdd1.h)         //[0] combine odd even
      R8 = ##((809<<16)+809)                        // scaling factor
    }
    .falign
.horboxfilter_LP:
    { sL0.cur = VMEM(R1+#1)                         //[1]
      sEven0s = VALIGN(sL0,sEven0,#2)               //[1]
      sSumo.h = VADD(sSum.h,sS10.h)                 //[2]
      sSume.h = VADD(sSum.h,sEven1.h)               //[2]
    }
    { sEvenodd0.h = VADD(sEven0s.h,sOdd0.h)         //[1]
      sOdd0.cur = VMEM(R1++#2)                      //[1]
    }
    { sOutalign = VALIGN(sHout_L,sOutdelay,R4)      //[3]
      sOutdelay = sHout_L                           //[3]
      sSume.h = VMPY(sSume.h,R8.h):<<1:sat          //[2]
    }
    { sDelay1 = VALIGN(sEvenodd0,sEvenodd1,#2)      //[1]
      sSumo.h = VMPY(sSumo.h,R8.h):<<1:sat          //[2]
      IF P3 VMEM(R0++#1) = sOutalign                //[3]
    }
    { sDelay2 = VALIGN(sEvenodd0,sEvenodd1,#4)      //[1]
      sSum.h = VADD(sEvenodd1.h,sDelay1.h)          //[1]
      sEven0 = sL0                                  //[1]
    }
    { sDelay3 = VALIGN(sEvenodd0,sEvenodd1,#6)      //[1]
      sSum.h = VADD(sSum.h,sDelay2.h)               //[1]
      sEvenodd1 = sEvenodd0                         //[1]
      sHout_L.ub = VASR(sSumo.h,sSume.h,R7):rnd:sat //[2]
    }
    { sSum.h = VADD(sSum.h,sDelay3.h)               //[1]
      sS10 = VALIGN(sOdd0,sOdd1,R5)                 //[1]
      sOdd1 = sOdd0                                 //[1]
      sEven1 = VMEM(R1+#-5)                         //[1]
    }:endloop0
    { sEven0s = VALIGN(sL0,sEven0,#2)               //[1]
      sSumo.h = VADD(sSum.h,sS10.h)                 //[2]
      sSume.h = VADD(sSum.h,sEven1.h)               //[2]
    }
    { sEvenodd0.h = VADD(sEven0s.h,sOdd0.h)         //[1]
      sOdd0.cur = VMEM(R1++#2)                      //[1]
    }
    { sOutalign = VALIGN(sHout_L,sOutdelay,R4)      //[3]
      sOutdelay = sHout_L                           //[3]
      sSume.h = VMPY(sSume.h,R8.h):<<1:sat          //[2]
    }
    { sDelay1 = VALIGN(sEvenodd0,sEvenodd1,#2)      //[1]
      sSumo.h = VMPY(sSumo.h,R8.h):<<1:sat          //[2]
      IF P3 VMEM(R0++#1) = sOutalign                //[3]
    }
    { sDelay2 = VALIGN(sEvenodd0,sEvenodd1,#4)      //[1]
      sSum.h = VADD(sEvenodd1.h,sDelay1.h)          //[1]
      sEven0 = sL0                                  //[1]
    }
    { sDelay3 = VALIGN(sEvenodd0,sEvenodd1,#6)      //[1]
      sSum.h = VADD(sSum.h,sDelay2.h)               //[1]
      sEvenodd1 = sEvenodd0                         //[1]
      sHout_L.ub = VASR(sSumo.h,sSume.h,R7):rnd:sat //[2]
    }
    { sSum.h = VADD(sSum.h,sDelay3.h)               //[1]
      sS10 = VALIGN(sOdd0,sOdd1,R5)                 //[1]
      sOdd1 = sOdd0                                 //[1]
      sEven1 = VMEM(R1+#-5)                         //[1]
    }
    { sEven0s = VALIGN(sL0,sEven0,#2)               //[1]
      sSumo.h = VADD(sSum.h,sS10.h)                 //[2]
      sSume.h = VADD(sSum.h,sEven1.h)               //[2]
    }
    { sEvenodd0.h = VADD(sEven0s.h,sOdd0.h)         //[1]
      R1 = ADD(R1,##VLEN*2)                         //
    }
    { sOutalign = VALIGN(sHout_L,sOutdelay,R4)      //[3]
      sOutdelay = sHout_L                           //[3]
      sSume.h = VMPY(sSume.h,R8.h):<<1:sat          //[2]
    }
    { sDelay1 = VALIGN(sEvenodd0,sEvenodd1,#2)      //[1]
      sSumo.h = VMPY(sSumo.h,R8.h):<<1:sat          //[2]
      IF P3 VMEM(R0++#1) = sOutalign                //[3]
    }
    { sDelay2 = VALIGN(sEvenodd0,sEvenodd1,#4)      //[1]
      sSum.h = VADD(sEvenodd1.h,sDelay1.h)          //[1]
      sEven0 = sL0                                  //[1]
    }
    { sDelay3 = VALIGN(sEvenodd0,sEvenodd1,#6)      //[1]
      sSum.h = VADD(sSum.h,sDelay2.h)               //[1]
      sEvenodd1 = sEvenodd0                         //[1]
      sHout_L.ub = VASR(sSumo.h,sSume.h,R7):rnd:sat //[2]
    }
    { sSum.h = VADD(sSum.h,sDelay3.h)               //[1]
      sS10 = VALIGN(sOdd0,sOdd1,R5)                 //[1]
      sOdd1 = sOdd0                                 //[1]
      sEven1 = VMEM(R1+#-5)                         //[1]
    }
    { sOutalign = VALIGN(sHout_L,sOutdelay,R4)      //[3]
      sOutdelay = sHout_L                           //[3]
      sSumo.h = VADD(sSum.h,sS10.h)                 //[2]
      sSume.h = VADD(sSum.h,sEven1.h)               //[2]
    }
    { IF P3 VMEM(R0++#1) = sOutalign                //
      sSume.h = VMPY(sSume.h,R8.h):<<1:sat          //
    }
    { sSumo.h = VMPY(sSumo.h,R8.h):<<1:sat          //
    }
    { sHout_L.ub = VASR(sSumo.h,sSume.h,R7):rnd:sat //
    }
    { sOutalign = VALIGN(sHout_L,sOutdelay,R4)      //
      VMEM(R0++#1) = sOutalign.new                  //
    }
    { JUMPR R31                                     //
    }
    .size    horboxfilter, .-horboxfilter

    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void horvarcomp()                                             ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: horizontal variance computation                               ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned short *rgvar  -- pointer to output              ]*/
    /*[               R1 : unsigned short *rgm2   -- pointer to input               ]*/
    /*[               R2 : int width              -- width                          ]*/
    /*[               R3 : unsigned char *rgmean  -- pointer to mean                ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
    /*[           -                                                                 ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  ASSUMPTIONS:                                                               ]*/
    /*[           - *rgvar, rgm2, rgmean are aligned by VLEN                        ]*/
    /*[           - loop count is multiple of 2*VLEN                                ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  REVISION HISTORY                                                           ]*/
    /*[  ----------------                                                           ]*/
    /*[  Version        Date                    Comments                            ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[   6.0.0         08/01/2014              created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define     sVarl0           V0
#define     sVarl1           V1
#define     sVareven0        V2
#define     sVareven1        V3
#define     sVarodd0         V4
#define     sVarodd1         V5
#define     sVarevenodd0     V6
#define     sVarevenodd1     V7
#define     sVareven0s       V8
#define     sVardelay1       V9
#define     sVardelay2       V10
#define     sVardelay3       V11
#define     sVardelay4       V12
#define     sVarsum          V13
#define     dVarout          V15:14
#define     sVarout_H        V15
#define     sVarout_L        V14
#define     sVarsumo         V16
#define     sVarsume         V17
#define     sVars10          V18
#define     sVaroutdelay     V19
#define     sVaroutalign     V20
#define     sVarmean         V21
#define     sVarmean2_L      V22
#define     sVarmean2_H      V23
#define     dVarmean2        V23:22
#define     sVaravge         V24
#define     sVaravgo         V25
#define     sVarhalf         V26
#define     sVarmaxval       V27
#define     sVarodd0next     V28
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl horvarcomp
    .type    horvarcomp, @function
horvarcomp:
    { R5:4 = COMBINE(#16,#VLEN-8)                   //
      R9 = R3                                       //
      sVareven0 = VMEM(R1++#1)                      //
      R7:6 = COMBINE(#12,#8)                        //
    }
    { R8 = ADD(R2,#VLEN-1-VLEN)                     //
      sVarsume = #0                                 //
      sVarsumo = #0                                 //
      sVarodd1 = VMEM(R1++#1)                       //
    }
    { R12 = ASR(R8,#LOG2VLEN)                       //
      sVaroutdelay = #0                             //
      sVarl0.cur = VMEM(R1++#1)                     //
      sVareven0s = VALIGN(sVarl0,sVareven0,#4)      //
    }
    { P3 = SP1LOOP0(.horvarcomp_LP,R12)             //
      sVarevenodd1.w = VADD(sVareven0s.w,sVarodd1.w)//
      R8 = #809                                     //
      R3 = #1                                       //
    }
    { R8 = COMBINE(R8.L,R8.L)                       // 809
      sVareven1 = sVareven0                         //[2]
      R10 = ##0x8000                                // 0x8000
    }
    { sVarhalf = VSPLAT(R10)                        // 0x8000
      R10 = ##0x7fff7fff                            // 2^15-1
      sVareven0 = sVarl0                            //[1]
    }
    { sVarmaxval = VSPLAT(R10)                      // sVarmaxval
      R12 = AND(R12,#1)                             //
    }
    .falign
.horvarcomp_LP:
    { sVarl0.cur = VMEM(R1+#1)                      //[1a]
      sVareven0s = VALIGN(sVarl0,sVareven0,#4)      //[1a]
      sVaroutalign.uh = VMIN(sVaroutalign.uh,sVarmaxval.uh)//[2a]
    }
    { sVarodd0.cur = VMEM(R1++#2)                   //[1a]
      sVarevenodd0.w = VADD(sVareven0s.w,sVarodd0.w)//[1a]
      sVaravge.w += VMPYI(sVarsume.w,R8.h)          //[2b]
    }
    { sVarmean.tmp = VMEM(R9++#1)                   //[1a]
      sVarmean.b = VSHUFF(sVarmean.b)               //[1a]
      sVaravgo.w += VMPYI(sVarsumo.w,R8.h)          //[2b]
    }
    { sVardelay1 = VALIGN(sVarevenodd0,sVarevenodd1,#4)//[1a]
      sVareven0 = sVarl0                            //[1a]
      sVarl0 = VMEM(R1+#1)                          //[1b]
      sVarout_L.h = VSHUFFO(sVaravgo.h,sVaravge.h)  //[2b]
    }
    { sVardelay2 = VALIGN(sVarevenodd0,sVarevenodd1,R6)//[1a]
      sVarsum.w = VADD(sVarevenodd1.w,sVardelay1.w) //[1a]
      sVarodd0next = VMEM(R1++#2)                   //[1b]
    }
    { sVaroutalign = VALIGN(sVarout_L,sVaroutdelay,R4)//[2b]
      sVaroutdelay = sVarout_L                      //[2b]
      IF P3 VMEM(R0++#1) = sVaroutalign             //[2a]
    }
    { sVardelay3 = VALIGN(sVarevenodd0,sVarevenodd1,R7)//[1a]
      sVarevenodd1 = sVarevenodd0                   //[1a]
      sVarsum.w = VADD(sVarsum.w,sVardelay2.w)      //[1a]
      sVaroutalign.uh = VSUB(sVaroutalign.uh,sVarmean2_H.uh):sat//[2b]
    }
    { sVarsum.w = VADD(sVarsum.w,sVardelay3.w)      //[1a]
      sVareven0s = VALIGN(sVarl0,sVareven0,#4)      //[1b]
      sVaroutalign.uh = VMIN(sVaroutalign.uh,sVarmaxval.uh)//[2b]
      IF P3 VMEM(R0++#1) = sVaroutalign.new         //[2b]
    }
    { sVareven1.tmp = VMEM(R1+#-5-2)                //[1a]
      sVarsume.w = VADD(sVarsum.w,sVareven1.w)      //[1a]
      sVars10 = VALIGN(sVarodd0,sVarodd1,R5)        //[1a]
      sVarodd1 = sVarodd0                           //[1a]
    }
    { sVarsumo.w = VADD(sVarsum.w,sVars10.w)        //[1a]
      //
      sVarevenodd0.w = VADD(sVareven0s.w,sVarodd0next.w)//[1b]
      sVaravge = sVarhalf                           //[1a]
    }
    { sVaravge.w += VMPYI(sVarsume.w,R8.h)          //[1a]
      sVars10 = VALIGN(sVarodd0next,sVarodd1,R5)    //[1b]
      sVaravgo = sVarhalf                           //[1a]
    }
    { sVaravgo.w += VMPYI(sVarsumo.w,R8.h)          //[1a]
      sVardelay1 = VALIGN(sVarevenodd0,sVarevenodd1,#4)//[1b]
      sVareven0 = sVarl0                            //[1b]
    }
    { sVardelay2 = VALIGN(sVarevenodd0,sVarevenodd1,R6)//[1b]
      sVarsum.w = VADD(sVarevenodd1.w,sVardelay1.w) //[1b]
      dVarmean2.uh = vmpy(sVarmean.ub,sVarmean.ub)  //[1a]
    }
    { sVarout_L.h = VSHUFFO(sVaravgo.h,sVaravge.h)  //[1a]
      sVaravge = sVarhalf                           //[1b]
      sVaravgo = sVarhalf                           //[1b]
      sVarodd1 = sVarodd0next                       //[1b]
    }
    { sVardelay3 = VALIGN(sVarevenodd0,sVarevenodd1,R7)//[1b]
      sVarsum.w = VADD(sVarsum.w,sVardelay2.w)      //[1b]
      sVarevenodd1 = sVarevenodd0                   //[1b]
    }
    { sVaroutalign = VALIGN(sVarout_L,sVaroutdelay,R4)//[1a]
      sVaroutdelay = sVarout_L                      //[1a]
      sVarsum.w = VADD(sVarsum.w,sVardelay3.w)      //[1b]
    }
    { sVaroutalign.uh = VSUB(sVaroutalign.uh,sVarmean2_L.uh):sat//[1a]
      sVarsumo.w = VADD(sVarsum.w,sVars10.w)        //[1b]
      sVareven1.tmp = VMEM(R1+#-5)                  //[1b]
      sVarsume.w = VADD(sVarsum.w,sVareven1.w)      //[1b]
    }:endloop0
    { sVareven0s = VALIGN(sVarl0,sVareven0,#4)      //[1a]
      sVaroutalign.uh = VMIN(sVaroutalign.uh,sVarmaxval.uh)//[2a]
      VMEM(R0++#1) = sVaroutalign.new               //[2a]
    }
    { sVarodd0.cur = VMEM(R1++#2)                   //[1a]
      sVarevenodd0.w = VADD(sVareven0s.w,sVarodd0.w)//[1a]
      sVaravge.w += VMPYI(sVarsume.w,R8.h)          //[2b]
    }
    { sVarmean.tmp = VMEM(R9++#1)                   //[1a]
      sVarmean.b = VSHUFF(sVarmean.b)               //[1a]
      sVaravgo.w += VMPYI(sVarsumo.w,R8.h)          //[2b]
    }
    { sVardelay1 = VALIGN(sVarevenodd0,sVarevenodd1,#4)//[1a]
      sVareven0 = sVarl0                            //[1a]
      P2 = CMP.EQ(R12,#0)                           //
    }
    { sVarout_L.h = VSHUFFO(sVaravgo.h,sVaravge.h)  //[2b]
    }
    { sVardelay2 = VALIGN(sVarevenodd0,sVarevenodd1,R6)//[1a]
      sVarsum.w = VADD(sVarevenodd1.w,sVardelay1.w) //[1a]
    }
    { sVaroutalign = VALIGN(sVarout_L,sVaroutdelay,R4)//[2b]
      sVaroutdelay = sVarout_L                      //[2b]
    }
    { sVardelay3 = VALIGN(sVarevenodd0,sVarevenodd1,R7)//[1a]
      sVarevenodd1 = sVarevenodd0                   //[1a]
      sVarsum.w = VADD(sVarsum.w,sVardelay2.w)      //[1a]
      sVaroutalign.uh = VSUB(sVaroutalign.uh,sVarmean2_H.uh):sat//[2b]
    }
    { sVarsum.w = VADD(sVarsum.w,sVardelay3.w)      //[1a]
      sVareven0s = VALIGN(sVarl0,sVareven0,#4)      //[1b]
      sVaroutalign.uh = VMIN(sVaroutalign.uh,sVarmaxval.uh)//[2b]
      IF P3 VMEM(R0++#1) = sVaroutalign.new         //[2b]
    }
    { sVareven1.tmp = VMEM(R1+#-5)                  //[1a]
      sVarsume.w = VADD(sVarsum.w,sVareven1.w)      //[1a]
      sVars10 = VALIGN(sVarodd0,sVarodd1,R5)        //[1a]
      sVarodd1 = sVarodd0                           //[1a]
    }
    { sVarsumo.w = VADD(sVarsum.w,sVars10.w)        //[1a]
      R1 = ADD(R1,#2*VLEN)                          //
      sVarevenodd0.w = VADD(sVareven0s.w,sVarodd0.w)//[1b]
      sVaravge = sVarhalf                           //[1a]
    }
    { sVaravge.w += VMPYI(sVarsume.w,R8.h)          //[1a]
      sVars10 = VALIGN(sVarodd0,sVarodd1,R5)        //[1b]
      sVaravgo = sVarhalf                           //[1a]
    }
    { sVaravgo.w += VMPYI(sVarsumo.w,R8.h)          //[1a]
      sVardelay1 = VALIGN(sVarevenodd0,sVarevenodd1,#4)//[1b]
      sVareven0 = sVarl0                            //[1b]
    }
    { sVardelay2 = VALIGN(sVarevenodd0,sVarevenodd1,R6)//[1b]
      sVarsum.w = VADD(sVarevenodd1.w,sVardelay1.w) //[1b]
      dVarmean2.uh = vmpy(sVarmean.ub,sVarmean.ub)  //[1a]
    }
    { sVarout_L.h = VSHUFFO(sVaravgo.h,sVaravge.h)  //[1a]
      sVaravge = sVarhalf                           //[1b]
      sVaravgo = sVarhalf                           //[1b]
      sVarodd1 = sVarodd0                           //[1b]
    }
    { sVardelay3 = VALIGN(sVarevenodd0,sVarevenodd1,R7)//[1b]
      sVarsum.w = VADD(sVarsum.w,sVardelay2.w)      //[1b]
      sVarevenodd1 = sVarevenodd0                   //[1b]
    }
    { sVaroutalign = VALIGN(sVarout_L,sVaroutdelay,R4)//[1a]
      sVaroutdelay = sVarout_L                      //[1a]
      sVarsum.w = VADD(sVarsum.w,sVardelay3.w)      //[1b]
    }
    { sVaroutalign.uh = VSUB(sVaroutalign.uh,sVarmean2_L.uh):sat//[1a]
      sVarsumo.w = VADD(sVarsum.w,sVars10.w)        //[1b]
      sVareven1.tmp = VMEM(R1+#-5)                  //[1b]
      sVarsume.w = VADD(sVarsum.w,sVareven1.w)      //[1b]
    }
    { sVaroutalign.uh = VMIN(sVaroutalign.uh,sVarmaxval.uh)//
      VMEM(R0++#1) = sVaroutalign.new               //
    }
    { sVaravge.w += VMPYI(sVarsume.w,R8.h)          //
    }
    { sVaravgo.w += VMPYI(sVarsumo.w,R8.h)          //
    }
    { sVarout_L.h = VSHUFFO(sVaravgo.h,sVaravge.h)  //
    }
    { sVaroutalign = VALIGN(sVarout_L,sVaroutdelay,R4)//
      sVaroutdelay = #0                             //
    }
    { sVaroutalign.uh = VSUB(sVaroutalign.uh,sVarmean2_H.uh):sat//
    }
    { sVaroutalign.uh = VMIN(sVaroutalign.uh,sVarmaxval.uh)//
      VMEM(R0++#1) = sVaroutalign.new               //
    }
    { IF !P2 JUMPR R31                              //
    }
    { VMEM(R0++#1) = sVaroutdelay                   //
    }
    { VMEM(R0++#1) = sVaroutdelay                   //
    }
    { JUMPR R31                                     //
    }
    .size    horvarcomp, .-horvarcomp

    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void reciprocal()                                             ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: create reciprocal                                             ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned short *input         -- pointer to input        ]*/
    /*[               R1 : unsigned short *recipval      -- pointer to recip value  ]*/
    /*[               R2 : short          *recipshft     -- pointer to recip shft   ]*/
    /*[               R3 : int width                     -- width                   ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
    /*[           -                                                                 ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  ASSUMPTIONS:                                                               ]*/
    /*[           - *input, *recipval, *recipshft are aligned by VLEN               ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  REVISION HISTORY                                                           ]*/
    /*[  ----------------                                                           ]*/
    /*[  Version        Date                    Comments                            ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[   6.0.0         08/01/2014              created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define     sInput0             V0
#define     sInput1             V1
#define     sClb0               V2
#define     sClb1               V3
#define     sFrac0              V4
#define     sFrac1              V5
#define     sFrac01             V6
#define     sSubidx             V7
#define     sY0_L               V8
#define     sY0_H               V9
#define     dY0                 V9:8
#define     sRetmp0_L           V10
#define     sRetmp0_H           V11
#define     dRetmp0             V11:10
#define     sVallbase           V12
#define     sValhbase           V13
#define     sSlope              V14
#define     sMask0x1f           V15
#define     sConst12            V16
#define     sSlopeTab           V17
#define     sValLTab            V18
#define     sValHTab            V19
#define     sValTab             V19:18
#define     sIdx0               V20
#define     sIdx1               V21
//==================================================
    .text
    .p2align 2
    .p2align 4,,15
    .globl reciprocal
    .type    reciprocal, @function
reciprocal:
    { R10 = ##val_table.recip                       // ptr val_table.recip
      R3 = ADD(R3,#VLEN-1)                          // width + VLEN-1
    }
    { V0.tmp = VMEM(R10+#0)                         //
      sValTab.uh = VZXT(V0.ub)                      //
      R11 = ##slope_table.recip                     // ptr slope_table.recip
    }
    { V0.tmp = VMEM(R11+#0)                         //
      sSlopeTab.b = VSHUFF(V0.b)                    //
      R4 = ##0x000C001F                             //
    }
    { R3 = LSR(R3,#LOG2VLEN)                        // ceil(width/VLEN)
      R5:4 = PACKHL(R4,R4)                          //
      R7:6 = COMBINE(#5,#0)                         //
    }
    { sMask0x1f = VSPLAT(R4)                        // 0x1f
      sConst12  = VSPLAT(R5)                        // 12
      R5 = #5                                       // R5 = #5
    }
    { P3 = SP2LOOP0(.reciprocal_LP,R3)              // set loop0 with lc0 = ceil(width/2*VLEN)
    }

    .falign
.reciprocal_LP:
    { sInput0.cur = VMEM(R0++#1)                    //[1]
      sClb0.uh = VCL0(sInput0.uh)                   //[1]
      sSubidx.b = VSHUFFE(sIdx1.b,sIdx0.b)          //[2]
      sFrac01.b = VSHUFFE(sFrac1.b,sFrac0.b)        //[2]
    }
    { sInput1.cur = VMEM(R0++#1)                    //[1]
      sClb1.uh = VCL0(sInput1.uh)                   //[1]
      dRetmp0.uh = VZXT(sRetmp0_L.ub)               //[3]
    }
    { sInput0.h = VASL(sInput0.h,sClb0.h)           //[1]
      sSlope.b = VLUT32(sSubidx.b,sSlopeTab.b,R6)   //[2]
      dY0.h = VSUB(dY0.h,dRetmp0.h)                 //[3]
    }
    { sInput1.h = VASL(sInput1.h,sClb1.h)           //[1]
      sVallbase.b = VLUT32(sSubidx.b,sValLTab.b,R6) //[2]
      IF P3 VMEM(R1++#1) = sY0_L                    //[3]
    }
    { sFrac0.uh = VLSR(sInput0.uh,R7)               //[1]
      sClb0.h = VSUB(sConst12.h,sClb0.h)            //[1]
      VMEM(R2++#1) = sClb0.new                      //[1]
      sValhbase.b = VLUT32(sSubidx.b,sValHTab.b,R6) //[2]
    }
    { sFrac1.uh = VLSR(sInput1.uh,R7)               //[1]
      sClb1.h = VSUB(sConst12.h,sClb1.h)            //[1]
      VMEM(R2++#1) = sClb1.new                      //[1]
      dRetmp0.uh = VMPY(sSlope.ub,sFrac01.ub)       //[2]
    }
    { sIdx0.uh = VLSR(sFrac0.uh,R7)                 //[1]
      sFrac0 = VAND(sFrac0,sMask0x1f)               //[1]
      IF P3 VMEM(R1++#1) = sY0_H                    //[3]
    }
    { sIdx1.uh = VLSR(sFrac1.uh,R7)                 //[1]
      sFrac1 = VAND(sFrac1,sMask0x1f)               //[1]
      sIdx0 = VAND(sIdx0,sMask0x1f)                 //[1]
    }
    { sIdx1 = VAND(sIdx1,sMask0x1f)                 //[1]
      sRetmp0_L.ub = VASR(sRetmp0_H.h,sRetmp0_L.h,R7):rnd:sat//[2]
      dY0.b = VSHUFFOE(sValhbase.b,sVallbase.b)     //[2]
    }:endloop0

    //====
    { sSubidx.b = VSHUFFE(sIdx1.b,sIdx0.b)          //[2]
      sFrac01.b = VSHUFFE(sFrac1.b,sFrac0.b)        //[2]
    }
    { dRetmp0.uh = VZXT(sRetmp0_L.ub)               //[3]
    }
    { sSlope.b = VLUT32(sSubidx.b,sSlopeTab.b,R6)   //[2]
      dY0.h = VSUB(dY0.h,dRetmp0.h)                 //[3]
    }
    { sVallbase.b = VLUT32(sSubidx.b,sValLTab.b,R6) //[2]
      IF P3 VMEM(R1++#1) = sY0_L                    //[3]
    }
    { sValhbase.b = VLUT32(sSubidx.b,sValHTab.b,R6) //[2]
    }
    { dRetmp0.uh = VMPY(sSlope.ub,sFrac01.ub)       //[2]
    }
    { IF P3 VMEM(R1++#1) = sY0_H                    //[3]
    }
    { sRetmp0_L.ub = VASR(sRetmp0_H.h,sRetmp0_L.h,R7):rnd:sat//[2]
      dY0.b = VSHUFFOE(sValhbase.b,sVallbase.b)     //[2]
    }
    { dRetmp0.uh = VZXT(sRetmp0_L.ub)               //[3]
    }
    { dY0.h = VSUB(dY0.h,dRetmp0.h)                 //[3]
    }
    { VMEM(R1++#1) = sY0_L                          //[3]
    }
    { VMEM(R1++#1) = sY0_H                          //[3]
    }
    { JUMPR R31                                     //
    }
    .size    reciprocal, .-reciprocal

    .section        .rodata
    .p2align LOG2VLEN
    .type   slope_table.recip, @object
    .size   slope_table.recip, 32
slope_table.recip:
    .byte  125,116,111,104,98,93,89,84
    .byte  80,76,72,70,66,63,61,58
    .byte  56,53,51,50,47,46,44,43
    .byte  41,40,38,37,36,34,34,32

    .p2align LOG2VLEN
    .type   val_table.recip, @object
    .size   val_table.recip, 64
val_table.recip:
    .hword  4096,3971,3855,3744,3640,3542,3449,3360
    .hword  3276,3196,3120,3048,2978,2912,2849,2788
    .hword  2730,2674,2621,2570,2520,2473,2427,2383
    .hword  2340,2299,2259,2221,2184,2148,2114,2080

    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void vertboxfiltervarcomp()                                   ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: vertical box filter and variance computation                  ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned char *src    -- pointer to input                ]*/
    /*[               R1 : unsigned short *rgm   -- pointer to intermediate mean    ]*/
    /*[               R2 : unsigned *rgm2        -- pointer to intermediate x^2     ]*/
    /*[               R3 : int width             -- width                           ]*/
    /*[               R4 : int stride            -- stride                          ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
    /*[           -                                                                 ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  ASSUMPTIONS:                                                               ]*/
    /*[           - *src, *rgmean, *rgvar are aligned by VLEN                       ]*/
    /*[           - width must be equal or greater than 2 * VLEN                    ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  REVISION HISTORY                                                           ]*/
    /*[  ----------------                                                           ]*/
    /*[  Version        Date                    Comments                            ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[   6.0.0         08/01/2014              created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define     sVIn0               V0
#define     sVIn1               V1
#define     sVM_L               V2
#define     sVMH_H              V3
#define     DVM                 V3:2
#define     sVXsq0_L            V4
#define     sVXsq0_H            V5
#define     dVXsq0              V5:4
#define     sVXsq1_L            V6
#define     sVXsq1_H            V7
#define     dVXsq1              V7:6
#define     sVM1_L              V8
#define     sVM1_H              V9
#define     dVM_1               V9:8
#define     sVRgm2a             V10
#define     sVRgm2b             V11
#define     sVRgm2c             V12
#define     sVRgm2d             V13
#define     sVIn0p              V14
#define     sVIn1p              V15
#define     dVDelta0            V17:16
#define     sVDelta0_L          V16
#define     sVDelta0_H          V17
#define     dVDelta1            V19:18
#define     sVDelta1_L          V18
#define     sVDelta1_H          V19
#define     sVRgm               V20
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl vertboxfiltervarcomp
    .type    vertboxfiltervarcomp, @function
vertboxfiltervarcomp:
    { R7 = ADD(R0,MPYI(R4,#4))                      // src[4*stride]
      R3 = ADD(R3,#VLEN-1-VLEN)                     // width + VLEN - 1 - VLEN
      R6 = R2                                       // rgm2
    }
    // Add workaound for P-Vmem HW bug: pull out the first loop
    { R3 = LSR(R3,#LOG2VLEN)                        // (width - 1)>>LOG2VLEN
      R0 -= MPYI(R4,#5)                             // src[-5*stride]
      sVIn1.cur = VMEM(R7++#1)                      //[1]
      sVIn1p.b = VSHUFF(sVIn1.b)                    //[1]
    }
    { sVIn0.cur = VMEM(R0++#1)                      //[1]
      sVIn0p.b = VSHUFF(sVIn0.b)                    //[1]
      P3 = SP1LOOP0(.vertboxfiltervarcomp_LP,R3)    // loop
    }
    { sVRgm2a = VMEM(R6++#1)                        //[1]
      dVXsq0.uh = VMPY(sVIn0p.ub,sVIn0p.ub)         //[1]
    }
    { sVRgm = VMEM(R1+#0)                           //[1]
      DVM.h = VSUB(sVIn1.ub,sVIn0.ub)               //[1]
    }
    { sVRgm = VMEM(R1+#1)                           //[1]
      sVM_L.h = VADD(sVM_L.h,sVRgm.h)               //[1]
      VMEM(R1++#1) = sVM_L.new                      //[1]
      dVXsq1.uh = VMPY(sVIn1p.ub,sVIn1p.ub)         //[1]
    }
    { sVRgm2b = VMEM(R6++#1)                        //[1]
      sVMH_H.h = VADD(sVMH_H.h,sVRgm.h)             //[1]
      VMEM(R1++#1) = sVMH_H.new                     //[1]
    }
    { sVRgm2c = VMEM(R6++#1)                        //[1]
      dVDelta0.w = VSUB(sVXsq1_L.uh,sVXsq0_L.uh)    //[1]
    }
    { sVRgm2d = VMEM(R6++#1)                        //[1]
      dVDelta1.w = VSUB(sVXsq1_H.uh,sVXsq0_H.uh)    //[1]
      sVDelta0_L.w = VADD(sVRgm2a.w,sVDelta0_L.w)   //[1]
      VMEM(R2++#1) = sVDelta0_L.new                 //[1]
    }
    .falign
.vertboxfiltervarcomp_LP:
    { sVIn0.cur = VMEM(R0++#1)                      //[1]
      sVIn0p.b = VSHUFF(sVIn0.b)                    //[1]
      sVDelta0_H.w = VADD(sVRgm2b.w,sVDelta0_H.w)   //[2]
      VMEM(R2++#1) = sVDelta0_H.new                 //[2]
    }
    { sVIn1.cur = VMEM(R7++#1)                      //[1]
      sVIn1p.b = VSHUFF(sVIn1.b)                    //[1]
      sVDelta1_L.w = VADD(sVRgm2c.w,sVDelta1_L.w)   //[2]
      VMEM(R2++#1) = sVDelta1_L.new                 //[2]
    }
    { sVRgm2a = VMEM(R6++#1)                        //[1]
      dVXsq0.uh = VMPY(sVIn0p.ub,sVIn0p.ub)         //[1]
      sVDelta1_H.w = VADD(sVRgm2d.w,sVDelta1_H.w)   //[2]
      VMEM(R2++#1) = sVDelta1_H.new                 //[2]
    }
    { sVRgm = VMEM(R1+#0)                           //[1]
      DVM.h = VSUB(sVIn1.ub,sVIn0.ub)               //[1]
    }
    { sVRgm = VMEM(R1+#1)                           //[1]
      sVM_L.h = VADD(sVM_L.h,sVRgm.h)               //[1]
      VMEM(R1++#1) = sVM_L.new                      //[1]
      dVXsq1.uh = VMPY(sVIn1p.ub,sVIn1p.ub)         //[1]
    }
    { sVRgm2b = VMEM(R6++#1)                        //[1]
      sVMH_H.h = VADD(sVMH_H.h,sVRgm.h)             //[1]
      VMEM(R1++#1) = sVMH_H.new                     //[1]
    }
    { sVRgm2c = VMEM(R6++#1)                        //[1]
      dVDelta0.w = VSUB(sVXsq1_L.uh,sVXsq0_L.uh)    //[1]
    }
    { sVRgm2d = VMEM(R6++#1)                        //[1]
      dVDelta1.w = VSUB(sVXsq1_H.uh,sVXsq0_H.uh)    //[1]
      sVDelta0_L.w = VADD(sVRgm2a.w,sVDelta0_L.w)   //[1]
      VMEM(R2++#1) = sVDelta0_L.new                 //[1]
    }:endloop0
    { sVDelta0_H.w = VADD(sVRgm2b.w,sVDelta0_H.w)   //[2]
      VMEM(R2++#1) = sVDelta0_H.new                 //[2]
    }
    { sVDelta1_L.w = VADD(sVRgm2c.w,sVDelta1_L.w)   //[2]
      VMEM(R2++#1) = sVDelta1_L.new                 //[2]
    }
    { sVDelta1_H.w = VADD(sVRgm2d.w,sVDelta1_H.w)   //[2]
      VMEM(R2++#1) = sVDelta1_H.new                 //[2]
    }
    { JUMPR R31                                     //
    }
    .size    vertboxfiltervarcomp, .-vertboxfiltervarcomp
