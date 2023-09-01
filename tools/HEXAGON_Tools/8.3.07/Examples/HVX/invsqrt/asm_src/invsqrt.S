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
    .file    "invsqrt.S"

#include "hvx.cfg.h"
    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void invsqrt()                                                ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: This function computes 1 / squareroot(x) using interpolation. ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned short *input           -- pointer to input      ]*/
    /*[               R1 : unsigned short *sqrt_recip_shft -- pointer to shft       ]*/
    /*[               R2 : unsigned short *sqrt_recip_val  -- pointer to val        ]*/
    /*[               R3 : int width                       -- width                 ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
    /*[           -                                                                 ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  ASSUMPTIONS:                                                               ]*/
    /*[           - input, sqrt_recip_shft, sqrt_recip_val are aligned by VLEN      ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  REVISION HISTORY                                                           ]*/
    /*[  ----------------                                                           ]*/
    /*[  Version        Date                    Comments                            ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[   6.0.0         08/01/2014              created for HVX evaluation          ]*/
    /*[   6.0.1         08/10/2016              optimized for precision/cycles      ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define iptr                    R0
#define optrS                   R1
#define optrV                   R2
#define num                     R3
#define idxs                    R4
#define idxv                    R5
#define idx_v_s                 R5:4
#define c20                     R6
#define c7ff                    R7
#define c02c                    R8
#define cFFFE                   R9
#define const3                  R8
/* ============================================================ */
#define sIn0                    V0
#define sIn1                    V1
#define sClz0                   V2
#define sClz1                   V3
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
#define sMask7FF                V16
#define sConst44                V17
#define sC20                    V18
#define sMaskFFFE               V19
#define sInvSqrtLTab            V20
#define sInvSqrtHTab            V21
#define dInvSqrtTab             V21:20
#define sValueL                 sIn0
#define sValueH                 sIn1
#define sSlopeL                 sSlope0
#define sSlopeH                 sSlope1
#define sShft0                  sClz0
#define sShft1                  sClz1
#define sInvSqrtV0              sT0
#define sInvSqrtV1              sT1
#define sX0s                    sIdxS
#define sX1s                    sIdxV
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl invsqrt
    .type    invsqrt, @function
invsqrt:
    { R28 = ADD(num,#VLEN-1)                        //
      c20    = ##0x20202020                         //
      c02c.l = #0x002C                              //
    }
    { R28 = LSR(R28,#LOG2VLEN)                      //
      idx_v_s = COMBINE(#1,#0)                      //
      c02c.h = #0x002C                              //
      sC20 = VSPLAT(c20)                            //
    }
    { c7ff = ##0x07FF07FF                           //
      R10 = ADD(PC,##InvSqrtTABLE@PCREL)            //
    }
    { sMask7FF = VSPLAT(c7ff)                       //
      sConst44 = VSPLAT(c02c)                       //
      cFFFE = ##0xFFFEFFFE                          //
    }
    { sMaskFFFE = VSPLAT(cFFFE)                     // 
      const3 = #3                                   //
#if LOG2VLEN==6
      V30 = VMEM(R10++#1)                           //
#endif
    } 
    { P3 = SP2LOOP0(.invsqrt_LOOP,R28)              //
      V31.tmp = VMEM(R10+#0)                        //
#if LOG2VLEN==6
      dInvSqrtTab.b = VSHUFFOE(V31.b,V30.b)         //
#else
      dInvSqrtTab.uh = VZXT(V31.ub)                 //
#endif
    }

    .falign
.invsqrt_LOOP:
    { sIn0.cur = VMEM(iptr++#1)                     //[1]
      sClz0.uh = VCL0(sIn0.uh)                      //[1]
      sFrac0 = VAND(sX0,sMask7FF)                   //[2]
      sIdxV = VOR(sIdxS,sC20)                       //[2]
    }
    { sIn1.cur = VMEM(iptr++#1)                     //[1]
      sClz1.uh = VCL0(sIn1.uh)                      //[1]
      sClz0 = VAND(sClz0,sMaskFFFE)                 //[1]
      sSlopeL.b=VLUT32(sIdxS.b,sInvSqrtLTab.b,idxs) //[2]
    }
    { sClz1 = VAND(sClz1,sMaskFFFE)                 //[1]
      sSlopeH.b=VLUT32(sIdxS.b,sInvSqrtHTab.b,idxs) //[2]
      sFrac1 = VAND(sX1,sMask7FF)                   //[2]
    }
    { sX0.h = VASL(sIn0.h,sClz0.h)                  //[1]
      sValueL.b=VLUT32(sIdxV.b,sInvSqrtLTab.b,idxv) //[2]
      dSlope.b = VSHUFFOE(sSlopeH.b,sSlopeL.b)      //[2]
    }
    { sX1.h = VASL(sIn1.h,sClz1.h)                  //[1]
      sValueH.b=VLUT32(sIdxV.b,sInvSqrtHTab.b,idxv) //[2]
      sInvSqrtV0.h = VSUB(sY0.h,sT0.h)              //[3]
      IF P3 VMEM(optrV++#1) = sInvSqrtV0.new        //[3]
    }
    { sX0s.uh = VLSR(sX0.uh,const3)                 //[1]
      dY.b = VSHUFFOE(sValueH.b,sValueL.b)          //[2]
      sInvSqrtV1.h = VSUB(sY1.h,sT1.h)              //[3]
      IF P3 VMEM(optrV++#1) = sInvSqrtV1.new        //[3]
    }
    { sX1s.uh = VLSR(sX1.uh,const3)                 //[1]
      sShft0.h = VNAVG(sConst44.h,sClz0.h)          //[1]
      VMEM(optrS++#1) = sShft0.new                  //[1]
      sT0.h = VMPY(sSlope0.h,sFrac0.h):<<1:rnd:sat  //[2]
    }
    { sIdxS.b = VSHUFFO(sX1s.b,sX0s.b)              //[1]
      sShft1.h = VNAVG(sConst44.h,sClz1.h)          //[1]
      VMEM(optrS++#1) = sShft1.new                  //[1]
      sT1.h = VMPY(sSlope1.h,sFrac1.h):<<1:rnd:sat  //[2]
    }:endloop0

    //======
    { sFrac0 = VAND(sX0,sMask7FF)                   //[2]
      sIdxV = VOR(sIdxS,sC20)                       //[2]
    }
    { sSlopeL.b=VLUT32(sIdxS.b,sInvSqrtLTab.b,idxs) //[2]
    }
    { sSlopeH.b=VLUT32(sIdxS.b,sInvSqrtHTab.b,idxs) //[2]
      sFrac1 = VAND(sX1,sMask7FF)                   //[2]
    }
    { sValueL.b=VLUT32(sIdxV.b,sInvSqrtLTab.b,idxv) //[2]
      dSlope.b = VSHUFFOE(sSlopeH.b,sSlopeL.b)      //[2]
    }
    { sValueH.b=VLUT32(sIdxV.b,sInvSqrtHTab.b,idxv) //[2]
      sInvSqrtV0.h = VSUB(sY0.h,sT0.h)              //[3]
      IF P3 VMEM(optrV++#1) = sInvSqrtV0.new        //[3]
    }
    { dY.b = VSHUFFOE(sValueH.b,sValueL.b)          //[2]
      sInvSqrtV1.h = VSUB(sY1.h,sT1.h)              //[3]
      IF P3 VMEM(optrV++#1) = sInvSqrtV1.new        //[3]
    }
    { sT0.h = VMPY(sSlope0.h,sFrac0.h):<<1:rnd:sat  //[2]
    }
    { sT1.h = VMPY(sSlope1.h,sFrac1.h):<<1:rnd:sat  //[2]
      sInvSqrtV0.h = VSUB(sY0.h,sT0.h)              //[3]
      VMEM(optrV+#0) = sInvSqrtV0.new               //[3]
    }
    { sInvSqrtV1.h = VSUB(sY1.h,sT1.h)              //[3]
      VMEM(optrV+#1) = sInvSqrtV1.new               //[3]
    }
    { JUMPR R31                                     //
    }
    .size    invsqrt, .-invsqrt


    .section    .rodata
    .p2align    LOG2VLEN
    .type       InvSqrtTABLE, @object
    .size       InvSqrtTABLE, 128
InvSqrtTABLE:
    // slope
    .hword      0,    0,    0,    0,    0,    0,    0,    0
    .hword  29985,25366,21823,19035,16794,14961,13439,12158
    .hword  11069,10133, 9322, 8614, 7991, 7440, 6949, 6510
    .hword   6116, 5759, 5436, 5143, 4874, 4628, 4403, 4195
    // value
    .hword      0,    0,    0,    0,    0,    0,    0,    0
    .hword  32768,30893,29308,27944,26754,25705,24770,23930 
    .hword  23170,22478,21845,21262,20724,20224,19759,19325 
    .hword  18918,18536,18176,17836,17515,17210,16921,16646


