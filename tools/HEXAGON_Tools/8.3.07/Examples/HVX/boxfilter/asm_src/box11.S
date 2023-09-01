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
    .file    "box11.S"

#include "hvx.cfg.h"
    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void Box11perRow()                                            ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION:                                                               ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned char  *src0  -- pointer to line to be subtracted]*/
    /*[               R1 : unsigned char  *src1  -- pointer to line to be added     ]*/
    /*[               R2 : int             width -- width                           ]*/
    /*[               R3 : unsigned short *sum   -- pointer to sum                  ]*/
    /*[               R4 : unsigned char  *out   -- pointer to output               ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
    /*[           -                                                                 ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  ASSUMPTIONS:                                                               ]*/
    /*[           - input/output arrays are aligned by VLEN                         ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  REVISION HISTORY                                                           ]*/
    /*[  ----------------                                                           ]*/
    /*[  Version        Date                    Comments                            ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[   6.0.0         08/24/2016              created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define iptr0                   R0
#define iptr1                   R1
#define width                   R2
#define iptrS                   R3
#define optr                    R4
#define optrS                   R5
#define widthminus1             R6
#define c1d121                  R8
/* ============================================================ */
#define sDif0e                  V0
#define sDif0o                  V1
#define dDif0                   V1:0
#define sDif1e                  V2
#define sDif1o                  V3
#define dDif1                   V3:2
#define sS2v0                   V4
#define sS2v1                   V5
#define sS4v0                   V6
#define sS4v1                   V7
#define sSume                   V8
#define sSumo                   V9
#define sXn5                    V10
#define sX6                     V11
#define sS2d4                   V12
#define sSplatbCtrl             V13
#define sIn0                    V14
#define sIn1                    V15
#define sOute                   V16
#define sOuto                   V17
#define sIn0_last               V18
#define sIn1_last               V19
#define sS10                    sS2d4
#define sS4d                    sX6
#define sOut                    sOute
#define sIn0_0                  sOute
#define sIn1_0                  sOuto
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl Box11perRow
    .type    Box11perRow, @function
Box11perRow:
    { R28 = ADD(width,#VLEN-1)                      //
      R8 = ADD(PC,##splatBidx0control@PCREL)        //
    }
    { R28 = LSR(R28,#LOG2VLEN)                      //
      sSplatbCtrl = VMEM(R8+#0)                     //
      widthminus1 = ADD(width,#-1)                  //
    }
    { P0 = CMP.EQ(R28,#1)                           //
      optrS = iptrS                                 //
      c1d121 = ##0x010F010F                         //
    }
    { sIn0.cur = VMEM(iptr0++#1)                    //
      sIn0_0 = VRDELTA(sIn0,sSplatbCtrl)            // left-boundary handling
    }
    { sIn1.cur = VMEM(iptr1++#1)                    //
      sIn1_0 = VRDELTA(sIn1,sSplatbCtrl)            // left-boundary handling
    }
    { Q0 = VSETQ(widthminus1)                       // right-boundary handling
    }
    { dDif0.h = VSUB(sIn1_0.ub,sIn0_0.ub)           //
      P3 = SP1LOOP0(.Box11perRow_restLOOP,R28)      //
    }
    { sS2v0.h = VADD(sDif0e.h,sDif0o.h)             //
      dDif1.h = VSUB(sIn1.ub,sIn0.ub)               //
      IF P0 JUMP .Box11perRow_WIDTH_LE_VLEN         //
    }
    { sS2v1.h = VADD(sDif1e.h,sDif1o.h)             //
    }
    { P1 = CMP.GT(R28,#2)                           //
      R28 = ADD(R28,#-2)                            //
    }
    { sS4v1 = VLALIGN(sS2v1,sS2v0,#4)               //
    }
    { sS4v1.h = VADD(sS4v1.h,sS2v1.h)               //
      IF !P1 JUMP .Box11perRow_LPEND                //
    }
    { P3 = SP1LOOP0(.Box11perRow_LOOP,R28)          //
    }

    .falign
.Box11perRow_LOOP:
    { sIn0 = VMEM(iptr0++#1)                        //[1]
      sS4v0 = sS4v1                                 //[1]
      sSumo.h = VADD(sSumo.h,sX6.h)                 //[2]
      sS4d = VALIGN(sS4v1,sS4v0,#2)                 //[2]
    }
    { sXn5 = VLALIGN(sDif1o,sDif0o,#6)              //[1]
      dDif0 = dDif1                                 //[1]
      sS10.h = VADD(sS10.h,sS4d.h)                  //[2]
    }
    { sIn1.cur = VMEM(iptr1++#1)                    //[1]
      dDif1.h = VSUB(sIn1.ub,sIn0.ub)               //[1]
      sSume.h = VADD(sSume.h,sS10.h)                //[2]
    }
    { sS2v0 = sS2v1                                 //[1]
      sS2v1.h = VADD(sDif1e.h,sDif1o.h)             //[1]
      sSumo.h = VADD(sSumo.h,sS10.h)                //[2]
      IF P3 VMEM(optrS+#1) = sSumo.new              //[2]
    }
    { sX6 = VALIGN(sDif1e,sDif0e,#6)                //[1]
      sOute.h = VMPY(sSume.h,c1d121.h):<<1:rnd:sat  //[2]
      IF P3 VMEM(optrS++#2) = sSume                 //[2]
    }
    { sSume = VMEM(iptrS++#1)                       //[1]
      sS4v1 = VLALIGN(sS2v1,sS2v0,#4)               //[1]
      sOuto.h = VMPY(sSumo.h,c1d121.h):<<1:rnd:sat  //[2]
    }
    { sSumo = VMEM(iptrS++#1)                       //[1]
      sS4v1.h = VADD(sS4v1.h,sS2v1.h)               //[1]
      sS2d4 = VALIGN(sS2v1,sS2v0,#4)                //[1]
    }
    { sSume.h = VADD(sSume.h,sXn5.h)                //[1]
      sS10.h = VADD(sS4v0.h,sS2d4.h)                //[1]
      sOut.ub = VSAT(sOuto.h,sOute.h)               //[2]
      IF P3 VMEM(optr++#1) = sOut.new               //[2]
    }:endloop0

    { LOOP0(.Box11perRow_restLOOP,#2)               //
    }
.Box11perRow_LPEND:
    { sIn0.cur = VMEM(iptr0++#1)                    //[1]
      sIn0_last = VROR(sIn0,widthminus1)            // right-boundary handling
    }
    { sIn1.cur = VMEM(iptr1++#1)                    //[1]
      sIn1_last = VROR(sIn1,widthminus1)            // right-boundary handling
    }
    { sIn0_last = VRDELTA(sIn0_last,sSplatbCtrl)    // right-boundary handling
    }
    { sIn1_last = VRDELTA(sIn1_last,sSplatbCtrl)    // right-boundary handling
      sIn0 = VMUX(Q0,sIn0,sIn0_last)                // right-boundary handling
    }
    { sIn1 = VMUX(Q0,sIn1,sIn1_last)                // right-boundary handling
    }

    .falign
.Box11perRow_restLOOP:
    { sS4v0 = sS4v1                                 //[1]
      sSumo.h = VADD(sSumo.h,sX6.h)                 //[2]
      sS4d = VALIGN(sS4v1,sS4v0,#2)                 //[2]
    }
    { sXn5 = VLALIGN(sDif1o,sDif0o,#6)              //[1]
      dDif0 = dDif1                                 //[1]
      sS10.h = VADD(sS10.h,sS4d.h)                  //[2]
    }
    { dDif1.h = VSUB(sIn1.ub,sIn0.ub)               //[1]
      sIn0 = sIn0_last                              //[1]
      sSume.h = VADD(sSume.h,sS10.h)                //[2]
    }
    { sS2v0 = sS2v1                                 //[1]
      sS2v1.h = VADD(sDif1e.h,sDif1o.h)             //[1]
      sSumo.h = VADD(sSumo.h,sS10.h)                //[2]
      IF P3 VMEM(optrS+#1) = sSumo.new              //[2]
    }
    { sX6 = VALIGN(sDif1e,sDif0e,#6)                //[1]
      sOute.h = VMPY(sSume.h,c1d121.h):<<1:rnd:sat  //[2]
      IF P3 VMEM(optrS++#2) = sSume                 //[2]
    }
    { sSume = VMEM(iptrS++#1)                       //[1]
      sS4v1 = VLALIGN(sS2v1,sS2v0,#4)               //[1]
      sOuto.h = VMPY(sSumo.h,c1d121.h):<<1:rnd:sat  //[2]
    }
    { sSumo = VMEM(iptrS++#1)                       //[1]
      sS4v1.h = VADD(sS4v1.h,sS2v1.h)               //[1]
      sS2d4 = VALIGN(sS2v1,sS2v0,#4)                //[1]
      sIn1 = sIn1_last                              //[1]
    }
    { sSume.h = VADD(sSume.h,sXn5.h)                //[1]
      sS10.h = VADD(sS4v0.h,sS2d4.h)                //[1]
      sOut.ub = VSAT(sOuto.h,sOute.h)               //[2]
      IF P3 VMEM(optr++#1) = sOut.new               //[2]
    }:endloop0

    //======
    { sSumo.h = VADD(sSumo.h,sX6.h)                 //[2]
      sS4d = VALIGN(sS4v1,sS4v0,#2)                 //[2]
    }
    { sS10.h = VADD(sS10.h,sS4d.h)                  //[2]
    }
    { sSume.h = VADD(sSume.h,sS10.h)                //[2]
    }
    { sSumo.h = VADD(sSumo.h,sS10.h)                //[2]
      VMEM(optrS+#1) = sSumo.new                    //[2]
    }
    { sOute.h = VMPY(sSume.h,c1d121.h):<<1:rnd:sat  //[2]
      VMEM(optrS+#0) = sSume                        //[2]
    }
    { sOuto.h = VMPY(sSumo.h,c1d121.h):<<1:rnd:sat  //[2]
    }
    { sOut.ub = VSAT(sOuto.h,sOute.h)               //[2]
      VMEM(optr+#0) = sOut.new                      //[2]
    }
    { JUMPR R31                                     //
    }

.Box11perRow_WIDTH_LE_VLEN:
    /*----------------------------------------------*/
    /*  handle right boundary when width <=128      */
    /*----------------------------------------------*/
    { sIn0_last = VROR(sIn0,widthminus1)            // right-boundary handling
    }
    { sIn1_last = VROR(sIn1,widthminus1)            // right-boundary handling
    }
    { sIn0_last = VRDELTA(sIn0_last,sSplatbCtrl)    // right-boundary handling
    }
    { sIn1_last = VRDELTA(sIn1_last,sSplatbCtrl)    // right-boundary handling
      sIn0 = VMUX(Q0,sIn0,sIn0_last)                // right-boundary handling
    }
    { sIn1 = VMUX(Q0,sIn1,sIn1_last)                // right-boundary handling
    }
    { dDif1.h = VSUB(sIn1.ub,sIn0.ub)               //
    }
    { sS2v1.h = VADD(sDif1e.h,sDif1o.h)             //
    }
    { sIn0 = sIn0_last                              //
      sIn1 = sIn1_last                              //
    }
    { sS4v1 = VLALIGN(sS2v1,sS2v0,#4)               //
    }
    { sS4v1.h = VADD(sS4v1.h,sS2v1.h)               //
      JUMP .Box11perRow_restLOOP                    //
    }
    .size    Box11perRow, .-Box11perRow

