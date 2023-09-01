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
    .file    "ncc.S"

#include "hvx.cfg.h"
    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void integrate64u8()                                          ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: compute sum(x) and sum(x^2) in a 8x8 patch                    ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned char* input -- pointer to input 8x8 patch       ]*/
    /*[               R1 : unsigned short* sum  -- pointer to output sum(x)         ]*/
    /*[               R2 : unsigned *sum2       -- pointer to output sum(x^2)       ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
    /*[           -                                                                 ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  ASSUMPTIONS:                                                               ]*/
    /*[           - sum/sum2 pointers need 64B alignment                            ]*/
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
    .text
    .p2align 2
    .p2align 4,,15
    .globl integrate64u8
    .type    integrate64u8, @function
integrate64u8:
    { R5:4 = MEMD(R0++#8)                           //
      LOOP0(.integrate64u8_LP,#7)                   //
      R7:6 = #0                                     //
      R9:8 = #0                                     //
    }
    .falign
.integrate64u8_LP:
    { R5:4 = MEMD(R0++#8)                           //
      R7:6 += VRADDUB(R5:4,R5:4)                    //
      R9:8 += VRMPYBU(R5:4,R5:4)                    //
    }:endloop0
    { R7:6 += VRADDUB(R5:4,R5:4)                    //
      R9:8 += VRMPYBU(R5:4,R5:4)                    //
    }
    { R6 = VAVGH(R7,R6)                             //
      R8 = ADD(R9,R8)                               //
    }
    { JUMPR R31                                     //
      MEMH(R1+#0) = R6                              //
      MEMW(R2+#0) = R8                              //
    }
    .size    integrate64u8, .-integrate64u8

    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void computecrossvar()                                        ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: compute cross correlation and variance                        ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : uint8_t * ptch8x8    -- pointer to input template        ]*/
    /*[               R1 : uint8_t * img        -- pointer to an image              ]*/
    /*[               R2 : int16_t stride       -- image stride                     ]*/
    /*[               R3 : uint16_t patchSum    -- template sum                     ]*/
    /*[               R4 : int32_t * noms       -- pointer to output noms           ]*/
    /*[               R5 : int32_t * denoms     -- pointer to output denoms         ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
    /*[           -                                                                 ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  ASSUMPTIONS:                                                               ]*/
    /*[           - img and ptch8x8 pointers are not VLEN B alignment               ]*/
    /*[           - nom and denom buffer requires 512+VLEN bytes                    ]*/
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
#define patchsum               R3
#define noms                   R4
#define minusone               R6
#define denoms                 R28
#define one                    R7
#define t1_03                  R8
#define t1_47                  R9
#define t1                     R9:8
#define t2_03                  R10
#define t2_47                  R11
#define t2                     R11:10
#define t3_03                  R12
#define t3_47                  R13
#define t3                     R13:12
#define t4_03                  R14
#define t4_47                  R15
#define t4                     R15:14
#define t5_03                  R16
#define t5_47                  R17
#define t5                     R17:16
#define t6_03                  R18
#define t6_47                  R19
#define t6                     R19:18
#define t7_03                  R20
#define t7_47                  R21
#define t7                     R21:20
#define SSncc                  (8*4)
/* ============================================================ */
#if LOG2VLEN == 6
#define t0_03                  R0
#define t0_47                  R1
#define t0                     R1:0
#define sLine0                 V0
#define sLine1                 V1
#define sLine2                 V2
#define sLine3                 V3
#define sLine4                 V4
#define sLine5                 V5
#define sLine6                 V6
#define sLine7                 V7
#define sOld03                 V8
#define sOld47                 V9
#define sSum                   V10
#define sSum2                  V11
#define sCross0                V12
#define sCross1                V13
#define sPatchsumxsum          V14
#define sSum0                  V14
#define sSum1                  V15
#define dPatchsumxsum          V15:14
#define sVar                   V16
#define sSumxsum               V16
#define dSumxsum               V17:16
#define sOldsum2               V18
#define sOldsum2_03            V18
#define sOldsum2_47            V19
#define sCross                 V20
#define sLine0_47              sLine0
#define sLine1_47              sLine1
#define sLine2_47              sLine2
#define sLine3_47              sLine3
#define sLine4_47              sLine4
#define sLine5_47              sLine5
#define sLine6_47              sLine6
#define sLine7_47              sLine7
#define sLine0_03              V21
#define sLine1_03              V22
#define sLine2_03              V23
#define sLine3_03              V24
#define sLine4_03              V25
#define sLine5_03              V26
#define sLine6_03              V27
#define sLine7_03              V28
#define sCtrl03                V29
#define sCtrl47                V30
#define sZero                  V31
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl computecrossvar
    .type    computecrossvar, @function
computecrossvar:
    { sLine0 = VMEMU(R1)                            //
      R8 = ##ncc_ctrl07                             //
      R1 = ADD(R1,R2)                               //
    }
    { sLine1 = VMEMU(R1)                            //
      R1 = ADD(R1,R2)                               //
      one = ##0x01010101                            //
    }
    { sCtrl03.cur = VMEM(R8+#0)                        //
      sLine0_03 = VDELTA(sLine0,sCtrl03)            //
      M0 = R2                                       //
      sZero = #0                                    //
    }
    { sCtrl47.cur = VMEM(R8+#1)                     //
      sLine0_47 = VDELTA(sLine0,sCtrl47)            //
      R29 = ADD(R29,#-SSncc)                        //
      R8 = #11*4                                    //
    }
    { sLine1_03 = VDELTA(sLine1,sCtrl03)            //
      M1 = R8                                       //
      R2 = R1                                       //
    }
    { sLine2 = VMEMU(R2++M0)                        //
      sSum2.uw = VRMPY(sLine0_03.ub,sLine0_03.ub)   //
      denoms = R5                                   //
    }
    { sLine1_47 = VDELTA(sLine1,sCtrl47)            //
      sSum0.uw = VRMPY(sLine0_03.ub,one.ub)         //
      sSum1.uw = VRMPY(sLine0_47.ub,one.ub)         //
    }
    { sLine2_03 = VDELTA(sLine2,sCtrl03)            //
      sSum2.uw += VRMPY(sLine0_47.ub,sLine0_47.ub)  //
      sSum.w = VADD(sSum0.w,sSum1.w)                //
    }
    { sLine3 = VMEMU(R2++M0)                        //
      sSum0.uw = VRMPY(sLine1_03.ub,one.ub)         //
      sSum1.uw = VRMPY(sLine1_47.ub,one.ub)         //
    }
    { sLine2_47 = VDELTA(sLine2,sCtrl47)            //
      sSum2.uw += VRMPY(sLine1_03.ub,sLine1_03.ub)  //
      sSum0.w = VADD(sSum0.w,sSum1.w)               //
      MEMD(R29+#0) = R17:16                         //
    }
    { sLine3_03 = VDELTA(sLine3,sCtrl03)            //
      sSum2.uw += VRMPY(sLine1_47.ub,sLine1_47.ub)  //
      sSum.w = VADD(sSum.w,sSum0.w)                 //
      MEMD(R29+#8) = R19:18                         //
    }
    { sLine4 = VMEMU(R2++M0)                        //
      sSum0.uw = VRMPY(sLine2_03.ub,one.ub)         //
      sSum1.uw = VRMPY(sLine2_47.ub,one.ub)         //
    }
    { sLine3_47 = VDELTA(sLine3,sCtrl47)            //
      sSum2.uw += VRMPY(sLine2_03.ub,sLine2_03.ub)  //
      sSum0.w = VADD(sSum0.w,sSum1.w)               //
      MEMD(R29+#16) = R21:20                        //
    }
    { sLine4_03 = VDELTA(sLine4,sCtrl03)            //
      sSum2.uw += VRMPY(sLine2_47.ub,sLine2_47.ub)  //
      sSum.w = VADD(sSum.w,sSum0.w)                 //
      minusone = #-1                                //
    }
    { sLine5 = VMEMU(R2++M0)                        //
      sSum0.uw = VRMPY(sLine3_03.ub,one.ub)         //
      sSum1.uw = VRMPY(sLine3_47.ub,one.ub)         //
    }
    { sLine4_47 = VDELTA(sLine4,sCtrl47)            //
      sSum2.uw += VRMPY(sLine3_03.ub,sLine3_03.ub)  //
      sSum0.w = VADD(sSum0.w,sSum1.w)               //
    }
    { sLine5_03 = VDELTA(sLine5,sCtrl03)            //
      sSum2.uw += VRMPY(sLine3_47.ub,sLine3_47.ub)  //
      sSum.w = VADD(sSum.w,sSum0.w)                 //
      t2 = MEMD(R0+#16)                             //
    }
    { sLine6 = VMEMU(R2++M0)                        //
      sSum0.uw = VRMPY(sLine4_03.ub,one.ub)         //
      sSum1.uw = VRMPY(sLine4_47.ub,one.ub)         //
    }
    { sLine5_47 = VDELTA(sLine5,sCtrl47)            //
      sSum2.uw += VRMPY(sLine4_03.ub,sLine4_03.ub)  //
      sSum0.w = VADD(sSum0.w,sSum1.w)               //
      t3 = MEMD(R0+#24)                             //
    }
    { sLine6_03 = VDELTA(sLine6,sCtrl03)            //
      sSum2.uw += VRMPY(sLine4_47.ub,sLine4_47.ub)  //
      sSum.w = VADD(sSum.w,sSum0.w)                 //
      t4 = MEMD(R0+#32)                             //
    }
    { sLine6_47 = VDELTA(sLine6,sCtrl47)            //
      sSum0.uw = VRMPY(sLine5_03.ub,one.ub)         //
      sSum1.uw = VRMPY(sLine5_47.ub,one.ub)         //
      VMEM(noms+#512/VLEN-1) = sZero                // set the last few entries to zero
    }
    { sSum2.uw += VRMPY(sLine5_03.ub,sLine5_03.ub)  //
      sSum0.w = VADD(sSum0.w,sSum1.w)               //
      t5 = MEMD(R0+#40)                             //
      Q0 = VSETQ(R8)                                //
    }
    { sSum2.uw += VRMPY(sLine5_47.ub,sLine5_47.ub)  //
      sSum.w = VADD(sSum.w,sSum0.w)                 //
      t6 = MEMD(R0+#48)                             //
      R5 = #6                                       //
    }
    { sSum0.uw = VRMPY(sLine6_03.ub,one.ub)         //
      sSum1.uw = VRMPY(sLine6_47.ub,one.ub)         //
      sOldsum2_03 = #0                              //
      sOldsum2_47 = #0                              //
    }
    { sSum2.uw += VRMPY(sLine6_03.ub,sLine6_03.ub)  //
      sSum0.w = VADD(sSum0.w,sSum1.w)               //
      t7 = MEMD(R0+#56)                             //
      P3 = SP1LOOP0(.computecrossvar_LP,#11)        //
    }
    { sSum2.uw += VRMPY(sLine6_47.ub,sLine6_47.ub)  //
      sSum.w = VADD(sSum.w,sSum0.w)                 //
      t1 = MEMD(R0+#8)                              //
      t0 = MEMD(R0+#0)                              //
    }
    .falign
.computecrossvar_LP:
    { sLine7 = VMEMU(R2++M0)                        //[1]
      sCross0.uw = VRMPY(sLine0_03.ub,t0_03.ub)     //[1]
      sCross1.uw = VRMPY(sLine0_47.ub,t0_47.ub)     //[1]
    }
    { sCross0.uw += VRMPY(sLine1_03.ub,t1_03.ub)    //[1]
      sCross1.uw += VRMPY(sLine1_47.ub,t1_47.ub)    //[1]
      sCross.w = VMAX(sCross.w,sZero.w)             //[2]
      sVar.w = VSUB(sSum2.w,sSumxsum.w)             //[2]
    }
    { sLine7_03 = VDELTA(sLine7,sCtrl03)            //[1]
      sCross0.uw += VRMPY(sLine2_03.ub,t2_03.ub)    //[1]
      sCross1.uw += VRMPY(sLine2_47.ub,t2_47.ub)    //[1]
      sOldsum2.w = VADD(sOldsum2_03.w,sOldsum2_47.w)//[2]
    }
    { IF P3 VMEMU(noms++M1) = sCross                //[2]
      sSum2.w = VSUB(sSum2.w,sOldsum2.w)            //[2]
    }
    { sLine7_47 = VDELTA(sLine7,sCtrl47)            //[1]
      sCross0.uw += VRMPY(sLine3_03.ub,t3_03.ub)    //[1]
      sCross1.uw += VRMPY(sLine3_47.ub,t3_47.ub)    //[1]
    }
    { sSum.uw += VRMPY(sLine7_03.ub,one.ub)         //[1]
      sCross0.uw += VRMPY(sLine4_03.ub,t4_03.ub)    //[1]
      IF P3 VMEMU(denoms++M1) = sVar                //[2]
    }
    { sSum.uw += VRMPY(sLine7_47.ub,one.ub)         //[1]
      sCross1.uw += VRMPY(sLine4_47.ub,t4_47.ub)    //[1]
      sOld03 = sLine0_03                            //[1]
      sOld47 = sLine0_47                            //[1]
    }
    { sSum2.uw += VRMPY(sLine7_03.ub,sLine7_03.ub)  //[1]
      sLine0_03 = sLine1_03                         //[1]
      sLine0_47 = sLine1_47                         //[1]
    }
    { sSum2.uw += VRMPY(sLine7_47.ub,sLine7_47.ub)  //[1]
      sLine1_03 = sLine2_03                         //[1]
      sLine1_47 = sLine2_47                         //[1]
    }
    { sCross0.uw += VRMPY(sLine5_03.ub,t5_03.ub)    //[1]
      sCross1.uw += VRMPY(sLine5_47.ub,t5_47.ub)    //[1]
      sLine2_03 = sLine3_03                         //[1]
      sLine2_47 = sLine3_47                         //[1]
    }
    { sCross0.uw += VRMPY(sLine6_03.ub,t6_03.ub)    //[1]
      sCross1.uw += VRMPY(sLine6_47.ub,t6_47.ub)    //[1]
      sLine3_03 = sLine4_03                         //[1]
      sLine3_47 = sLine4_47                         //[1]
    }
    { sCross0.uw += VRMPY(sLine7_03.ub,t7_03.ub)    //[1]
      sCross1.uw += VRMPY(sLine7_47.ub,t7_47.ub)    //[1]
      sLine4_03 = sLine5_03                         //[1]
      sLine4_47 = sLine5_47                         //[1]
    }
    { sCross0.w = VADD(sCross0.w,sCross1.w)         //[1]
      dPatchsumxsum.uw = VMPY(sSum.uh,patchsum.uh)  //[1]upper register will be all zeros
      sLine5_03 = sLine6_03                         //[1]
    }
    { dSumxsum.uw = VMPY(sSum.uh,sSum.uh)           //[1]
      sLine5_47 = sLine6_47                         //[1]
      sLine6_03 = sLine7_03                         //[1]
    }
    { sPatchsumxsum.w = VASR(sPatchsumxsum.w,R5)    //[1]
      sOldsum2_03.uw = VRMPY(sOld03.ub,sOld03.ub)   //[1]
      sSum.w += VRMPY(sOld03.ub,minusone.b)         //[1]
      sLine6_47 = sLine7_47                         //[1]
    }
    { sCross.w = VSUB(sCross0.w,sPatchsumxsum.w)    //[1]
      sSumxsum.w = VASR(sSumxsum.w,R5)              //[1]
      sOldsum2_47.uw = VRMPY(sOld47.ub,sOld47.ub)   //[1]
      sSum.w += VRMPY(sOld47.ub,minusone.b)         //[1]
    }:endloop0
    { sCross.w = VMAX(sCross.w,sZero.w)             //[2]
      //sOldsum2.w = VADD(sOldsum2_03.w,sOldsum2_47.w)//[2]
      sVar.w = VSUB(sSum2.w,sSumxsum.w)             //[2]
      R17:16 = MEMD(R29+#0)                         //
      R19:18 = MEMD(R29+#8)                         //
    }
    { sCross = VMUX(Q0,sCross,sZero)                //[2]
      sVar = VMUX(Q0,sVar,sZero)                    //[2]
      R21:20 = MEMD(R29+#16)                        //
      R29 = ADD(R29,#SSncc)                         //
    }
    { VMEMU(noms++M1) = sCross                      //[2]
    }
    { //sSum2.w = VSUB(sSum2.w,sOldsum2.w)          //[2]
      VMEMU(denoms++M1) = sVar                      //[2]
    }
    { JUMPR R31                                     //
    }
#else
#define t0_03                  R22
#define t0_47                  R23
#define t0                     R23:22
#define sLine0                 V0
#define sLine1                 V1
#define sLine2                 V2
#define sLine3                 V3
#define sLine4                 V4
#define sLine5                 V5
#define sLine6                 V6
#define sLine7                 V7
#define sLine8                 V8
#define sOld03                 V9
#define sOldsum2_03            sOld03
#define sOldsum2               sOld03
#define sOld47                 V10
#define sOldsum2_47            sOld47
#define sSum                   V11
#define sSum2                  V12
#define sCross0                V13
#define sPatchsumxsum          V14
#define sSum0                  V14
#define sCross                 V14
#define sSum1                  V15
#define dPatchsumxsum          V15:14
#define sSumxsum               V16
#define sVar                   V16
#define sCross1                V17
#define sVar1                  sCross1
#define sPatchsumxsum2         sCross1
#define dSumxsum               V17:16
#define sLine0_47              sLine0
#define sLine1_47              sLine1
#define sLine2_47              sLine2
#define sLine3_47              sLine3
#define sLine4_47              sLine4
#define sLine5_47              sLine5
#define sLine6_47              sLine6
#define sLine7_47              sLine7
#define sLine8_47              sLine8
#define sOldOld03              V18
#define sOldOld47              V19
#define sLine0_03              V20
#define sLine1_03              V21
#define sLine2_03              V22
#define sLine3_03              V23
#define sLine4_03              V24
#define sLine5_03              V25
#define sLine6_03              V26
#define sLine7_03              V27
#define sLine8_03              V28
#define sCtrl03                V29
#define sCtrl47                V30
#define sZero                  V31
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl computecrossvar
    .type    computecrossvar, @function
computecrossvar:
    { sLine0 = VMEMU(R1)                            //
      R8 = ##ncc_ctrl07                             //
      R1 = ADD(R1,R2)                               //
    }
    { sLine1 = VMEMU(R1)                            //
      R1 = ADD(R1,R2)                               //
      one = ##0x01010101                            //s
    }
    { sLine0_03 = VDELTA(sLine0,sCtrl03)            //
      sCtrl03.cur = VMEM(R8+#0)                     //
      M0 = R2                                       //
      R2 = R1                                       //
    }
    { sLine0_47 = VDELTA(sLine0,sCtrl47)            //
      sCtrl47.cur = VMEM(R8+#1)                     //
      R29 = ADD(R29,#-SSncc)                        //s
      MEMD(R29+#(0-SSncc)) = R17:16                 //s
    }
    { sLine1_03 = VDELTA(sLine1,sCtrl03)            //
      R9:8 = COMBINE(#11*4,#VLEN/2)                 //s
      MEMD(R29+#8) = R19:18                         //s
      MEMD(R29+#16) = R21:20                        //s
    }
    { Q1 = VSETQ(R8)                                //
      M1 = R9                                       //s
      MEMD(R29+#24) = R23:22                        //s
      t2 = MEMD(R0+#16)                             //s
    }
    { sLine2 = VMEMU(R2++M0)                        //
      sLine0_03 = VMUX(Q1,sLine0_03,sLine1_03)      //
      sZero = #0                                    //s
    }
    { sLine1_47 = VDELTA(sLine1,sCtrl47)            //
      t3 = MEMD(R0+#24)                             //s
      t4 = MEMD(R0+#32)                             //s
      minusone = #-1                                //s
    }
    { sLine2_03 = VDELTA(sLine2,sCtrl03)            //
      sLine0_47 = VMUX(Q1,sLine0_47,sLine1_47)      //
      sSum.uw = VRMPY(sLine0_03.ub,one.ub)          //
      sSum2.uw = VRMPY(sLine0_03.ub,sLine0_03.ub)   //
    }
    { sLine3 = VMEMU(R2++M0)                        //
      sLine1_03 = VMUX(Q1,sLine1_03,sLine2_03)      //
      P3 = SP1LOOP0(.computecrossvar_LP,#5)         //s
    }
    { sLine2_47 = VDELTA(sLine2,sCtrl47)            //
      sSum.uw += VRMPY(sLine0_47.ub,one.ub)         //
      sSum1.uw = VRMPY(sLine0_47.ub,sLine0_47.ub)   //
      VMEM(noms+#512/VLEN-1) = sZero                //s
    }
    { sLine3_03 = VDELTA(sLine3,sCtrl03)            //
      sLine1_47 = VMUX(Q1,sLine1_47,sLine2_47)      //
      sSum.uw += VRMPY(sLine1_03.ub,one.ub)         //
      sSum0.uw = VRMPY(sLine1_03.ub,sLine1_03.ub)   //
    }
    { sLine4 = VMEMU(R2++M0)                        //
      sLine2_03 = VMUX(Q1,sLine2_03,sLine3_03)      //
      sSum2.w = VADD(sSum2.w,sSum1.w)               //
    }
    { sLine3_47 = VDELTA(sLine3,sCtrl47)            //
      sSum.uw += VRMPY(sLine1_47.ub,one.ub)         //
      sSum1.uw = VRMPY(sLine1_47.ub,sLine1_47.ub)   //
      sSum2.w = VADD(sSum2.w,sSum0.w)               //
    }
    { denoms = R5                                   //s
      t5 = MEMD(R0+#40)                             //s
      t6 = MEMD(R0+#48)                             //s
      Q0 = VSETQ(R9)                                //s
    }
    { sLine4_03 = VDELTA(sLine4,sCtrl03)            //
      sLine2_47 = VMUX(Q1,sLine2_47,sLine3_47)      //
      sSum.uw += VRMPY(sLine2_03.ub,one.ub)         //
      sSum0.uw = VRMPY(sLine2_03.ub,sLine2_03.ub)   //
    }
    { sLine5 = VMEMU(R2++M0)                        //
      sLine3_03 = VMUX(Q1,sLine3_03,sLine4_03)      //
      sSum2.w = VADD(sSum2.w,sSum1.w)               //
    }
    { sLine4_47 = VDELTA(sLine4,sCtrl47)            //
      sSum.uw += VRMPY(sLine2_47.ub,one.ub)         //
      sSum1.uw = VRMPY(sLine2_47.ub,sLine2_47.ub)   //
      sSum2.w = VADD(sSum2.w,sSum0.w)               //
    }
    { sLine5_03 = VDELTA(sLine5,sCtrl03)            //
      sLine3_47 = VMUX(Q1,sLine3_47,sLine4_47)      //
      sSum.uw += VRMPY(sLine3_03.ub,one.ub)         //
      sSum0.uw = VRMPY(sLine3_03.ub,sLine3_03.ub)   //
    }
    { sLine6 = VMEMU(R2++M0)                        //
      sLine4_03 = VMUX(Q1,sLine4_03,sLine5_03)      //
      sSum2.w = VADD(sSum2.w,sSum1.w)               //
    }
    { sLine5_47 = VDELTA(sLine5,sCtrl47)            //
      sSum.uw += VRMPY(sLine3_47.ub,one.ub)         //
      sSum1.uw = VRMPY(sLine3_47.ub,sLine3_47.ub)   //
      sSum2.w = VADD(sSum2.w,sSum0.w)               //
    }
    { sLine6_03 = VDELTA(sLine6,sCtrl03)            //
      sLine4_47 = VMUX(Q1,sLine4_47,sLine5_47)      //
      sSum.uw += VRMPY(sLine4_03.ub,one.ub)         //
      sSum0.uw = VRMPY(sLine4_03.ub,sLine4_03.ub)   //
    }
    { sLine6_47 = VDELTA(sLine6,sCtrl47)            //
      sLine5_03 = VMUX(Q1,sLine5_03,sLine6_03)      //
      sSum2.w = VADD(sSum2.w,sSum1.w)               //
      R5 = #6                                       //s
    }
    { sLine5_47 = VMUX(Q1,sLine5_47,sLine6_47)      //
      sSum.uw += VRMPY(sLine4_47.ub,one.ub)         //
      sSum1.uw = VRMPY(sLine4_47.ub,sLine4_47.ub)   //
      sSum2.w = VADD(sSum2.w,sSum0.w)               //
    }
    { sSum.uw += VRMPY(sLine5_03.ub,one.ub)         //
      sSum0.uw = VRMPY(sLine5_03.ub,sLine5_03.ub)   //
      sSum2.w = VADD(sSum2.w,sSum1.w)               //
    }
    { sSum.uw += VRMPY(sLine5_47.ub,one.ub)         //
      sSum1.uw = VRMPY(sLine5_47.ub,sLine5_47.ub)   //
      sSum2.w = VADD(sSum2.w,sSum0.w)               //
      t7 = MEMD(R0+#56)                             //s
    }
    { sSum2.w = VADD(sSum2.w,sSum1.w)               //
      t0 = MEMD(R0+#0)                              //s
      t1 = MEMD(R0+#8)                              //s
      R0 = #VLEN/2                                  //s
    }
    .falign
.computecrossvar_LP:
    { sLine7 = VMEMU(R2++M0)                        //
      sCross0.uw = VRMPY(sLine0_03.ub,t0_03.ub)     //
      sCross1.uw = VRMPY(sLine0_47.ub,t0_47.ub)     //
    }
    { sLine8 = VMEMU(R2++M0)                        //
      sCross0.uw += VRMPY(sLine1_03.ub,t1_03.ub)    //
      sCross1.uw += VRMPY(sLine1_47.ub,t1_47.ub)    //
    }
    { sLine7_03 = VDELTA(sLine7,sCtrl03)            //
      sCross0.uw += VRMPY(sLine2_03.ub,t2_03.ub)    //
      sCross1.uw += VRMPY(sLine2_47.ub,t2_47.ub)    //
      sOldOld03 = sLine0_03                         //
    }
    { sLine7_47 = VDELTA(sLine7,sCtrl47)            //
      sLine6_03 = VMUX(Q1,sLine6_03,sLine7_03)      //
      sCross0.uw += VRMPY(sLine3_03.ub,t3_03.ub)    //
      sCross1.uw += VRMPY(sLine3_47.ub,t3_47.ub)    //
    }
    { sLine8_03 = VDELTA(sLine8,sCtrl03)            //
      sLine6_47 = VMUX(Q1,sLine6_47,sLine7_47)      //
      sCross0.uw += VRMPY(sLine4_03.ub,t4_03.ub)    //
      sCross1.uw += VRMPY(sLine4_47.ub,t4_47.ub)    //
    }
    { sLine8_47 = VDELTA(sLine8,sCtrl47)            //
      sLine7_03 = VMUX(Q1,sLine7_03,sLine8_03)      //
      sSum.uw += VRMPY(sLine6_03.ub,one.ub)         //
      sSum0.uw = VRMPY(sLine6_03.ub,sLine6_03.ub)   //
    }
    { sLine7_47 = VMUX(Q1,sLine7_47,sLine8_47)      //
      sSum.uw += VRMPY(sLine6_47.ub,one.ub)         //
      sSum0.uw = VRMPY(sLine6_47.ub,sLine6_47.ub)   //
      sSum2.w = VADD(sSum2.w,sSum0.w)               //
    }
    { sSum.uw += VRMPY(sLine7_03.ub,one.ub)         //
      sSum0.uw = VRMPY(sLine7_03.ub,sLine7_03.ub)   //
      sSum2.w = VADD(sSum2.w,sSum0.w)               //
      sOldOld47 = sLine0_47                         //
    }
    { sSum.uw += VRMPY(sLine7_47.ub,one.ub)         //
      sSum0.uw = VRMPY(sLine7_47.ub,sLine7_47.ub)   //
      sSum2.w = VADD(sSum2.w,sSum0.w)               //
      sOld03 = sLine1_03                            //
    }
    { sCross0.uw += VRMPY(sLine5_03.ub,t5_03.ub)    //
      sCross1.uw += VRMPY(sLine5_47.ub,t5_47.ub)    //
      sSum2.w = VADD(sSum2.w,sSum0.w)               //
      sOld47 = sLine1_47                            //
    }
    { sCross0.uw += VRMPY(sLine6_03.ub,t6_03.ub)    //
      sCross1.uw += VRMPY(sLine6_47.ub,t6_47.ub)    //
      sLine0_03 = sLine2_03                         //
      sLine0_47 = sLine2_47                         //
    }
    { sCross0.uw += VRMPY(sLine7_03.ub,t7_03.ub)    //
      sCross1.uw += VRMPY(sLine7_47.ub,t7_47.ub)    //
      sLine1_03 = sLine3_03                         //
      sLine1_47 = sLine3_47                         //
    }
    { dSumxsum.uw = VMPY(sSum.uh,sSum.uh)           //
      sCross0.w = VADD(sCross0.w,sCross1.w)         //
      sLine2_03 = sLine4_03                         //
    }
    { dPatchsumxsum.uw = VMPY(sSum.uh,patchsum.uh)  //
      sLine2_47 = sLine4_47                         //
      sLine3_03 = sLine5_03                         //
    }
    { sSum.w +=  VRMPY(sOldOld03.ub,minusone.b)     //
      sOldOld03.uw = VRMPY(sOldOld03.ub,sOldOld03.ub)//
      sSumxsum.w = VASR(sSumxsum.w,R5)              //
      sLine3_47 = sLine5_47                         //
    }
    { sVar.w = VSUB(sSum2.w,sSumxsum.w)             //
      sSum.w +=  VRMPY(sOldOld47.ub,minusone.b)     //
      sOldOld47.uw = VRMPY(sOldOld47.ub,sOldOld47.ub)//
      sSum2.w = VSUB(sSum2.w,sOldOld03.w)           //
    }
    { sPatchsumxsum2.w = VASR(sPatchsumxsum.w,R5)   //
      sSum.w +=  VRMPY(sOld03.ub,minusone.b)        //
      sOld03.uw = VRMPY(sOld03.ub,sOld03.ub)        //
      sSum2.w = VSUB(sSum2.w,sOldOld47.w)           //
    }
    { sCross.w = VSUB(sCross0.w,sPatchsumxsum2.w)   //
      sVar1 = VALIGN(sZero,sVar,R0)                 //
      sSum.w +=  VRMPY(sOld47.ub,minusone.b)        //
      sSum2.w = VSUB(sSum2.w,sOld03.w)              //
    }
    { sCross.w = VMAX(sCross.w,sZero.w)             //
      VMEMU(denoms++M1) = sVar                      //
    }
    { VMEMU(denoms++M1) = sVar1                     //
      sOld47.uw = VRMPY(sOld47.ub,sOld47.ub)        //
      sLine4_03 = sLine6_03                         //
    }
    { sCross1 = VALIGN(sZero,sCross,R0)             //
      sSum2.w = VSUB(sSum2.w,sOld47.w)              //
      sLine4_47 = sLine6_47                         //
    }
    { VMEMU(noms++M1) = sCross                      //
      sLine5_03 = sLine7_03                         //
      sLine5_47 = sLine7_47                         //
    }
    { VMEMU(noms++M1) = sCross1                     //
      sLine6_03 = sLine8_03                         //
      sLine6_47 = sLine8_47                         //
    }:endloop0
    { sLine7 = VMEMU(R2++M0)                        //
      sCross0.uw = VRMPY(sLine0_03.ub,t0_03.ub)     //
      sCross1.uw = VRMPY(sLine0_47.ub,t0_47.ub)     //
    }
    { sCross0.uw += VRMPY(sLine1_03.ub,t1_03.ub)    //
      sCross1.uw += VRMPY(sLine1_47.ub,t1_47.ub)    //
    }
    { sLine7_03 = VDELTA(sLine7,sCtrl03)            //
      sCross0.uw += VRMPY(sLine2_03.ub,t2_03.ub)    //
      sCross1.uw += VRMPY(sLine2_47.ub,t2_47.ub)    //
    }
    { sLine7_47 = VDELTA(sLine7,sCtrl47)            //
      sLine6_03 = VMUX(Q1,sLine6_03,sLine7_03)      //
      sCross0.uw += VRMPY(sLine3_03.ub,t3_03.ub)    //
      sCross1.uw += VRMPY(sLine3_47.ub,t3_47.ub)    //
    }
    { sLine6_47 = VMUX(Q1,sLine6_47,sLine7_47)      //
      sCross0.uw += VRMPY(sLine4_03.ub,t4_03.ub)    //
      sCross1.uw += VRMPY(sLine4_47.ub,t4_47.ub)    //
    }
    { sSum.uw += VRMPY(sLine6_03.ub,one.ub)         //
      sSum0.uw = VRMPY(sLine6_03.ub,sLine6_03.ub)   //
    }
    { sSum.uw += VRMPY(sLine6_47.ub,one.ub)         //
      sSum0.uw = VRMPY(sLine6_47.ub,sLine6_47.ub)   //
      sSum2.w = VADD(sSum2.w,sSum0.w)               //
    }
    { sSum.uw += VRMPY(sLine7_03.ub,one.ub)         //
      sSum0.uw = VRMPY(sLine7_03.ub,sLine7_03.ub)   //
      sSum2.w = VADD(sSum2.w,sSum0.w)               //
    }
    { sSum.uw += VRMPY(sLine7_47.ub,one.ub)         //
      sSum0.uw = VRMPY(sLine7_47.ub,sLine7_47.ub)   //
      sSum2.w = VADD(sSum2.w,sSum0.w)               //
    }
    { sCross0.uw += VRMPY(sLine5_03.ub,t5_03.ub)    //
      sCross1.uw += VRMPY(sLine5_47.ub,t5_47.ub)    //
      sSum2.w = VADD(sSum2.w,sSum0.w)               //
    }
    { sCross0.uw += VRMPY(sLine6_03.ub,t6_03.ub)    //
      sCross1.uw += VRMPY(sLine6_47.ub,t6_47.ub)    //
    }
    { sCross0.uw += VRMPY(sLine7_03.ub,t7_03.ub)    //
      sCross1.uw += VRMPY(sLine7_47.ub,t7_47.ub)    //
    }
    { dSumxsum.uw = VMPY(sSum.uh,sSum.uh)           //
      sCross0.w = VADD(sCross0.w,sCross1.w)         //
      R17:16 = MEMD(R29+#0)                         //
      R19:18 = MEMD(R29+#8)                         //
    }
    { dPatchsumxsum.uw = VMPY(sSum.uh,patchsum.uh)  //
      R21:20 = MEMD(R29+#16)                        //
      R23:22 = MEMD(R29+#24)                        //
    }
    { sSumxsum.w = VASR(sSumxsum.w,R5)              //
    }
    { sVar.w = VSUB(sSum2.w,sSumxsum.w)             //
      sPatchsumxsum2.w = VASR(sPatchsumxsum.w,R5)   //
    }
    { sVar = VMUX(Q0,sVar,sZero)                    //
      sCross.w = VSUB(sCross0.w,sPatchsumxsum2.w)   //
    }
    { sCross.w = VMAX(sCross.w,sZero.w)             //
      VMEMU(denoms++M1) = sVar                      //
    }
    { sCross = VMUX(Q0,sCross,sZero)                //
    }
    { VMEMU(noms++M1) = sCross                      //
      R29 = ADD(R29,#SSncc)                         //
    }
    { JUMPR R31                                     //
    }
#endif
    .size    computecrossvar, .-computecrossvar

    /*[*****************************************************************************]*/
    /*[  FUNCTION   : int searchbestncc()                                           ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: compute cross correlation and variance                        ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : int32_t * nom           -- pointer to input nom          ]*/
    /*[               R1 : int32_t * denom         -- pointer to input denom        ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
    /*[           -                                                                 ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  ASSUMPTIONS:                                                               ]*/
    /*[           - nom & denom are aligned by VLEN B length.                       ]*/
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
#define sN                     V0
#define sD                     V1
#define sL32nn                 V2
#define sH32nn                 V3
#define sBestL32nn             V4
#define sBestH32nn             V5
#define sBestd                 V6
#define sBestloc               V7
#define sCurloc                V8
#define sLocInc                V9
#define sHnnpLeft              V10
#define sMnnpLeft              V11
#define sLnnpLeft              V12
#define sHnnpRight             V13
#define sMnnpRight             V14
#define sLnnpRight             V15
#define sMask                  V16
#define sL32nnM                V17
#define sBestL32nnM            V18
#define sH32nn_1               V19
#define sD_1                   V20
#define SEARCH_SIZE            11
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl searchbestncc
    .type    searchbestncc, @function
searchbestncc:
    { R7 = #16                                      //
      R8 = #1                                       //
      R9 = ##ncc_initloc                            //
    }
    { sBestd = VSPLAT(R8)                           //
      sBestH32nn = #0                               //
      sCurloc = VMEM(R9)                            //
      R8 = #VLEN/4                                  //
    }
    { sLocInc = VSPLAT(R8)                          //
      R8 = ##0x7fffffff                             //
      sZero = #0                                    //
    }
    { sMask = VSPLAT(R8)                            //
      sBestL32nn = #0                               //
      sBestloc = #0                                 //
      R8 = #VLEN*((SEARCH_SIZE*SEARCH_SIZE + VLEN/4-1)/(VLEN/4)-1)//
    }
    { sCurloc.w = VSUB(sCurloc.w, sLocInc.w)        //
      LOOP0(.searchbestncc_LP0,#(SEARCH_SIZE*SEARCH_SIZE + VLEN/4-1)/(VLEN/4))//
      R8 = ADD(R0,R8)                               //
      sN = VMEM(R0++#1)                             //[0]
    }
    { Q0 = VCMP.GT(V0.w,V0.w)                       //
      Q3 = VCMP.GT(V0.w,V0.w)                       //
      sLnnpLeft = #0                                //
      sLnnpRight = #0                               //
    }

    .falign
.searchbestncc_LP0:
    { sD = VMEM(R1++#1)                             //[1]
      sH32nn.w = VMPYE(sN.w,sN.uh)                  //[1]
      Q1 &= VCMP.GT(sLnnpLeft.uw,sLnnpRight.uw)     //[2]
    }
    { sH32nn.w += VMPYO(sN.w,sN.h):<<1:sat:shift    //[1]
      Q0 = OR(Q0,Q1)                                //[2]
    }
    { sH32nn.w = VAVG(sH32nn.w,sZero.w)             //[1]
      sL32nn.w = VMPYIEO(sN.h,sN.h)                 //[1]
      sBestH32nn = VMUX(Q0,sH32nn_1,sBestH32nn)     //[2]
      sBestL32nn = VMUX(Q0,sL32nn,sBestL32nn)       //[2]
    }
    { sL32nn.w += VMPYIE(sN.w,sN.uh)                //[1]
      sBestd = VMUX(Q0,sD_1,sBestd)                 //[2]
    }
    { sHnnpRight.w = VMPYE(sD.w,sBestH32nn.uh)      //[1]
      sL32nnM = VAND(sL32nn,sMask)                  //[1]
      sBestloc = VMUX(Q0,sCurloc,sBestloc)          //[2]
    }
    { sHnnpLeft.w = VMPYE(sBestd.w,sH32nn.uh)       //[1]
      Q2 = VCMP.EQ(sL32nn.w,sL32nnM.w)              //[1]
      sH32nn_1 = sH32nn                             //[1]
    }
    { sHnnpRight.uw = VLSR(sHnnpRight.uw,R7)        //[1]
      sMnnpLeft.w = VMPYE(sBestd.w,sL32nn.uh)       //[1]
      sBestL32nnM = VAND(sBestL32nn,sMask)          //[1]
    }
    { sHnnpLeft.uw = VLSR(sHnnpLeft.uw,R7)          //[1]
      sMnnpLeft.w += VMPYO(sBestd.w,sL32nnM.h):<<1:sat:shift//[1]
      Q3 = VCMP.EQ(sBestL32nn.w,sBestL32nnM.w)      //[1]
    }
    { Q0 = VCMP.GT(sHnnpLeft.uw,sHnnpRight.uw)      //[1]
      IF (!Q2) sMnnpLeft.w += sBestd.w              //[1]
      sMnnpRight.w = VMPYE(sD.w,sBestL32nn.uh)      //[1]
    }
    { Q1 = VCMP.EQ(sHnnpLeft.uw,sHnnpRight.uw)      //[1]
      sMnnpLeft.w = VAVG(sMnnpLeft.w,sZero.w)       //[1]
      sMnnpRight.w += VMPYO(sD.w,sBestL32nnM.h):<<1:sat:shift//[1]
    }
    { Q3 = VCMP.EQ(sHnnpLeft.uw,sHnnpRight.uw)      //[1]
      sMnnpLeft.w += VMPYIE(sBestd.w,sH32nn.uh)     //[1]
      IF (!Q3) sMnnpRight.w += sD.w                 //[1]
    }
    { sMnnpRight.w = VAVG(sMnnpRight.w,sZero.w)     //[1]
      sLnnpLeft.w = VMPYIEO(sBestd.h,sL32nn.h)      //[1]
      sLnnpRight.w = VMPYIEO(sD.h,sBestL32nn.h)     //[1]
      sD_1 = sD                                     //[1]
    }
    { sCurloc.w = VADD(sCurloc.w, sLocInc.w)        //[1]
      sMnnpRight.w += VMPYIE(sD.w,sBestH32nn.uh)    //[1]
      sN = VMEM(R0++#1)                             //[0]
    }
    { Q3 &= VCMP.GT(sMnnpLeft.uw,sMnnpRight.uw)     //[1]
      Q1 &= VCMP.EQ(sMnnpLeft.w,sMnnpRight.w)       //[1]
      sLnnpLeft.w += VMPYIE(sBestd.w,sL32nn.uh)     //[1]
      R0 = MIN(R8,R0)                               //[1]
    }
    { Q0 = OR(Q3,Q0)                                //[2]
      sLnnpRight.w += VMPYIE(sD.w,sBestL32nn.uh)    //[2]
    }:endloop0
    { Q1 &= VCMP.GT(sLnnpLeft.uw,sLnnpRight.uw)     //[2]
    }
    { Q0 = OR(Q0,Q1)                                //[2]
      LOOP0(.searchbestncc_LP1,#(LOG2VLEN-2))       //
      R6 = #VLEN/2                                  //
    }
    { sBestH32nn = VMUX(Q0,sH32nn,sBestH32nn)       //[2]
      sBestd = VMUX(Q0,sD,sBestd)                   //[2]
      sBestL32nn = VMUX(Q0,sL32nn,sBestL32nn)       //[2]
      sBestloc = VMUX(Q0,sCurloc,sBestloc)          //[2]
    }
    .falign
.searchbestncc_LP1:
    { sH32nn = VALIGN(sZero,sBestH32nn,R6)          //[1]
    }
    { sD = VALIGN(sZero,sBestd,R6)                  //[1]
    }
    { sL32nn = VALIGN(sZero,sBestL32nn,R6)          //[1]
    }
    { sHnnpRight.w = VMPYE(sD.w,sBestH32nn.uh)      //[1]
      sL32nnM = VAND(sL32nn,sMask)                  //[1]
    }
    { sHnnpLeft.w = VMPYE(sBestd.w,sH32nn.uh)       //[1]
      Q2 = VCMP.EQ(sL32nn.w,sL32nnM.w)              //[1]
    }
    { sHnnpRight.uw = VLSR(sHnnpRight.uw,R7)        //[1]
      sMnnpLeft.w = VMPYE(sBestd.w,sL32nn.uh)       //[1]
      sBestL32nnM = VAND(sBestL32nn,sMask)          //[1]
    }
    { sHnnpLeft.uw = VLSR(sHnnpLeft.uw,R7)          //[1]
      sMnnpLeft.w += VMPYO(sBestd.w,sL32nnM.h):<<1:sat:shift//[1]
      Q3 = VCMP.EQ(sBestL32nn.w,sBestL32nnM.w)      //[1]
    }
    { Q0 = VCMP.GT(sHnnpLeft.uw,sHnnpRight.uw)      //[1]
      IF (!Q2) sMnnpLeft.w += sBestd.w              //[1]
      sMnnpRight.w = VMPYE(sD.w,sBestL32nn.uh)      //[1]
    }
    { Q1 = VCMP.EQ(sHnnpLeft.uw,sHnnpRight.uw)      //[1]
      sMnnpLeft.w = VAVG(sMnnpLeft.w,sZero.w)       //[1]
      sMnnpRight.w += VMPYO(sD.w,sBestL32nnM.h):<<1:sat:shift//[1]
    }
    { Q3 = VCMP.EQ(sHnnpLeft.uw,sHnnpRight.uw)      //[1]
      sMnnpLeft.w += VMPYIE(sBestd.w,sH32nn.uh)     //[1]
      IF (!Q3) sMnnpRight.w += sD.w                 //[1]
    }
    { sMnnpRight.w = VAVG(sMnnpRight.w,sZero.w)     //[1]
      sLnnpLeft.w = VMPYIEO(sBestd.h,sL32nn.h)      //[1]
      sLnnpRight.w = VMPYIEO(sD.h,sBestL32nn.h)     //[1]
    }
    { sMnnpRight.w += VMPYIE(sD.w,sBestH32nn.uh)    //[1]
    }
    { Q3 &= VCMP.GT(sMnnpLeft.uw,sMnnpRight.uw)     //[1]
      Q1 &= VCMP.EQ(sMnnpLeft.w,sMnnpRight.w)       //[1]
      sLnnpLeft.w += VMPYIE(sBestd.w,sL32nn.uh)     //[1]
    }
    { sLnnpRight.w += VMPYIE(sD.w,sBestL32nn.uh)    //[2]
    }
    { Q1 &= VCMP.GT(sLnnpLeft.uw,sLnnpRight.uw)     //[2]
      Q0 = OR(Q3,Q0)                                //[2]
    }
    { Q0 = OR(Q0,Q1)                                //[2]
      sCurloc = VALIGN(sZero,sBestloc,R6)           //[1]
      R6 = LSR(R6,#1)                               //[1]
    }
    { sBestL32nn = VMUX(Q0,sL32nn,sBestL32nn)       //[2]
      sBestH32nn = VMUX(Q0,sH32nn,sBestH32nn)       //[2]
      sBestd = VMUX(Q0,sD,sBestd)                   //[2]
      sBestloc = VMUX(Q0,sCurloc,sBestloc)          //[2]
    }:endloop0
    { sBestL32nn = VOR(sBestH32nn,sBestL32nn)       //
      R0 = #0                                       //
      R4 = ##780903145                              //
    }
    { R1 = VEXTRACT(sBestloc,R0)                    //
    }
    { R0 = VEXTRACT(sBestL32nn,R0)                  //
    }
    { R4 = MPY(R1,R4)                               //
      P0 = CMP.EQ(R0,#0)                            //
    }
    { R2 = LSR(R4,#31)                              //
      IF P0 JUMPR R31                               //
      R0 = #-3                                      //
    }
    { R2 += ASR(R4,#1)                              //
    }
    { R1 -= MPYI(R2,#11)                            //
    }
    { R0 = COMBINE(R2.L,R1.L)                       //
      JUMPR R31                                     //
    }
    .size    searchbestncc, .-searchbestncc
