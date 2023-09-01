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
    .file    "reciprocal.S"

#include "hvx.cfg.h"
    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void reciprocal()                                             ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: This function computes 1 / x using interpolation.             ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned short *input      -- pointer to input           ]*/
    /*[               R2 : unsigned short *recip_val  -- pointer to val             ]*/
    /*[               R1 : short          *recip_shft -- pointer to shft            ]*/
    /*[               R3 : int width                  -- width                      ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
    /*[           -                                                                 ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  ASSUMPTIONS:                                                               ]*/
    /*[           - input, recip_shft, recip_val are aligned by VLEN                ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  REVISION HISTORY                                                           ]*/
    /*[  ----------------                                                           ]*/
    /*[  Version        Date                    Comments                            ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[   6.0.0         08/04/2016              created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define iptr                    R0
#define optrV                   R1
#define optrS                   R2
#define num                     R3
#define idxs                    R4
#define idxv                    R5
#define idx_v_s                 R5:4
#define c20                     R6
#define c1ff                    R7
#define c01c                    R8
/* ============================================================ */
#define sIn0                    V0
#define sIn1                    V1
#define sNorm0                  V2
#define sNorm1                  V3
#define sX0                     V4
#define sX1                     V5
#define sIdxS                   V6
#define sIdxV                   V7
#define sSlope0                 V8
#define sSlope1                 V9
#define dSlope                  V9:8
#define sFrac0                  V10
#define sFrac1                  V11
#define sY0                     V12
#define sY1                     V13
#define dY                      V13:12
#define sT0                     V14
#define sT1                     V15
#define sMask1FF                V16
#define sConst28                V17
#define sC20                    V18
#define sC40                    V19
#define sRecipLTab              V20
#define sRecipHTab              V21
#define dRecipTab               V21:20
#define sValueL                 sIn0
#define sValueH                 sIn1
#define sSlopeL                 sSlope0
#define sSlopeH                 sSlope1
#define sShft0                  sNorm0
#define sShft1                  sNorm1
#define sRecipV0                sT0
#define sRecipV1                sT1
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl reciprocal
    .type    reciprocal, @function
reciprocal:
    { R28 = ADD(num,#VLEN-1)                        //
      c20    = ##0x20202020                         //
      c01c.l = #0x001C                              //
    }
    { R28 = LSR(R28,#LOG2VLEN)                      //
      idx_v_s = COMBINE(#1,#0)                      //
      c01c.h = #0x001C                              //
      sC20 = VSPLAT(c20)                            //
    }
    { c1ff = ##0x01FF01FF                           //
      R10 = ADD(PC,##RecipTABLE@PCREL)              //
    }
    { sMask1FF = VSPLAT(c1ff)                       //
      sConst28 = VSPLAT(c01c)                       //
      sC40.b = VADD(sC20.b,sC20.b)                  //
#if LOG2VLEN==6
      V30 = VMEM(R10++#1)                           //
#endif
    } 
    { P3 = SP2LOOP0(.reciprocal_LOOP,R28)           //
      V31.tmp = VMEM(R10+#0)                        //
#if LOG2VLEN==6
      dRecipTab.b = VSHUFFOE(V31.b,V30.b)           //
#else
      dRecipTab.uh = VZXT(V31.ub)                   //
#endif
    }

    .falign
.reciprocal_LOOP:
    { sIn0.cur = VMEM(iptr++#1)                     //[1]
      sNorm0.h = VNORMAMT(sIn0.h)                   //[1]
      sSlopeL.b = VLUT32(sIdxS.b,sRecipLTab.b,idxs) //[2]
      sFrac0 = VAND(sX0,sMask1FF)                   //[2]
    }
    { sIn1.cur = VMEM(iptr++#1)                     //[1]
      sNorm1.h = VNORMAMT(sIn1.h)                   //[1]
      sSlopeH.b = VLUT32(sIdxS.b,sRecipHTab.b,idxs) //[2]
      sFrac1 = VAND(sX1,sMask1FF)                   //[2]
    }
    { sX0.h = VASL(sIn0.h,sNorm0.h)                 //[1]
      sValueL.b = VLUT32(sIdxV.b,sRecipLTab.b,idxv) //[2]
      dSlope.b = VSHUFFOE(sSlopeH.b,sSlopeL.b)      //[2]
    }
    { sX1.h = VASL(sIn1.h,sNorm1.h)                 //[1]
      sValueH.b = VLUT32(sIdxV.b,sRecipHTab.b,idxv) //[2]
      sRecipV0.h = VSUB(sY0.h,sT0.h)                //[3]
      IF P3 VMEM(optrV++#1) = sRecipV0.new          //[3]
    }
    { sIdxS.b = VSHUFFO(sX1.b,sX0.b)                //[1]
      dY.b = VSHUFFOE(sValueH.b,sValueL.b)          //[2]
      sRecipV1.h = VSUB(sY1.h,sT1.h)                //[3]
      IF P3 VMEM(optrV++#1) = sRecipV1.new          //[3]
    }
    { sIdxS.b = VNAVG(sIdxS.ub,sC40.ub)             //[1]
      sShft0.h = VSUB(sConst28.h,sNorm0.h)          //[1]
      VMEM(optrS++#1) = sShft0.new                  //[1]
      sT0.h = VMPY(sSlope0.h,sFrac0.h):<<1:rnd:sat  //[2]
    }
    { sIdxV = VOR(sIdxS,sC20)                       //[1]
      sShft1.h = VSUB(sConst28.h,sNorm1.h)          //[1]
      VMEM(optrS++#1) = sShft1.new                  //[1]
      sT1.h = VMPY(sSlope1.h,sFrac1.h):<<1:rnd:sat  //[2]
    }:endloop0

    //======
    { sSlopeL.b = VLUT32(sIdxS.b,sRecipLTab.b,idxs) //[2]
      sFrac0 = VAND(sX0,sMask1FF)                   //[2]
    }
    { sSlopeH.b = VLUT32(sIdxS.b,sRecipHTab.b,idxs) //[2]
      sFrac1 = VAND(sX1,sMask1FF)                   //[2]
    }
    { sValueL.b = VLUT32(sIdxV.b,sRecipLTab.b,idxv) //[2]
      dSlope.b = VSHUFFOE(sSlopeH.b,sSlopeL.b)      //[2]
    }
    { sValueH.b = VLUT32(sIdxV.b,sRecipHTab.b,idxv) //[2]
      sRecipV0.h = VSUB(sY0.h,sT0.h)                //[3]
      IF P3 VMEM(optrV++#1) = sRecipV0.new          //[3]
    }
    { dY.b = VSHUFFOE(sValueH.b,sValueL.b)          //[2]
      sRecipV1.h = VSUB(sY1.h,sT1.h)                //[3]
      IF P3 VMEM(optrV++#1) = sRecipV1.new          //[3]
    }
    { sT0.h = VMPY(sSlope0.h,sFrac0.h):<<1:rnd:sat  //[2]
    }
    { sT1.h = VMPY(sSlope1.h,sFrac1.h):<<1:rnd:sat  //[2]
      sRecipV0.h = VSUB(sY0.h,sT0.h)                //[3]
      VMEM(optrV+#0) = sRecipV0.new                 //[3]
    }
    { sRecipV1.h = VSUB(sY1.h,sT1.h)                //[3]
      VMEM(optrV+#1) = sRecipV1.new                 //[3]
    }
    { JUMPR R31                                     //
    }
    .size    reciprocal, .-reciprocal


    .section    .rodata
    .p2align    LOG2VLEN
    .type       RecipTABLE, @object
    .size       RecipTABLE, 128
RecipTABLE:
    // slope
    .hword  31775,29905,28197,26630,25191,23865,22641,21509
    .hword  20460,19485,18579,17734,16946,16209,15520,14873
    .hword  14266,13695,13158,12652,12175,11724,11297,10894
    .hword  10512,10149, 9805, 9478, 9167, 8872, 8590, 8322
    // value
    .hword  16384,15887,15420,14979,14563,14169,13797,13443
    .hword  13107,12787,12483,12192,11915,11650,11397,11155
    .hword  10922,10699,10485,10280,10082, 9892, 9709, 9532
    .hword  9362, 9198, 9039, 8886, 8738, 8594, 8456, 8322
