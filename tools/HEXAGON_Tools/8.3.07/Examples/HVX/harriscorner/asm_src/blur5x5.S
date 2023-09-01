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
    .file    "blur5x5.S"

#include "hvx.cfg.h"
    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void blur5x5s16Row()                                          ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: Performs 5x5 Gaussian bluring on a row of image buffer        ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : short *src          -- pointer to input buffer           ]*/
    /*[               R1 : unsigned int stride -- stride of image input             ]*/
    /*[               R2 : unsigned int width  -- image width                       ]*/
    /*[               R3 : short *dst          -- pointer to output buffer          ]*/
    /*[               R4 : unsigned int  flush -- row index in the input buffer     ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  ASSUMPTIONS:                                                               ]*/
    /*[           -                                                                 ]*/
    /*[=============================================================================]*/
    /*[  REVISION HISTORY                                                           ]*/
    /*[  ----------------                                                           ]*/
    /*[  Version        Date                    Comments                            ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[   6.0.0                                 created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define iptr                R0
#define stride              R1
#define width               R2
#define optr                R3
#define flush               R4
#define Cnst5               R5
#define iptr1               R6
#define iptr2               R7
#define iptr2iptr1          R7:6
#define iptr3               R8
#define iptr4               R9
#define iptr4iptr3          R9:8
#define flush1              R10
#define flush2              R11
#define flush3              R12
#define flush4              R13
#define iptr0               R0
#define stridex2            R1
#define Cnst8               R2
#define const6              R10
#define const6h             R11
#define const4              R12
/* ============================================================ */
#define sLine0              V0
#define sLine4              V1
#define sLine1              V2
#define sLine3              V3
#define dLine3Line1         V3:2
#define sLine2              V4
#define sOut                V5
#define sVsumv0E            V6
#define sVsumv0O            V7
#define dVsumv0             V7:6
#define sVsumv1E            V8
#define sVsumv1O            V9
#define dVsumv1             V9:8
#define sVsumv2E            V10
#define sVsumv2O            V11
#define dVsumv2             V11:10
#define sVsum0              V12
#define sVsum1              V13
#define sVsum2              V14
#define sVsum3              V15
#define sVsum4              V16
#define sVsum5              V17
#define sVsum1a3            V18
#define sVsum2a4            V19
#define sSumE               V20
#define sSumO               V21
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl blur5x5s16Row
    .type    blur5x5s16Row, @function
blur5x5s16Row:
    { R28 = ADD(width,#(VLEN/2-1))                  //
      flush1 = ADD(flush,#1)                        //
      flush2 = ADD(flush,#2)                        //
      Cnst5 = #5                                    //
    }
    { flush3 = ADD(flush,#3)                        // 
      flush4 = ADD(flush,#4)                        // 
      flush1 = MODWRAP(flush1,Cnst5)                // (flush + 1)%5
      flush2 = MODWRAP(flush2,Cnst5)                // (flush + 2)%5
    }
    { flush3 = MODWRAP(flush3,Cnst5)                // (flush + 3)%5
      flush4 = MODWRAP(flush4,Cnst5)                // (flush + 4)%5
      stridex2 = ADD(stride,stride)                 // sizeof(short)*stride
      iptr2iptr1 = COMBINE(iptr0,iptr0)             //
    }
    { R28 = LSR(R28,#LOG2VLEN-1)                    // ceil(width/(VLEN/2))
      iptr0 += MPYI(flush,stridex2)                 // &src[((flush + 0) % 5) * stride]
      iptr4iptr3 = COMBINE(iptr0,iptr0)             //
    }
    { iptr1 += MPYI(flush1,stridex2)                // &src[((flush + 1) % 5) * width]
      iptr2 += MPYI(flush2,stridex2)                // &src[((flush + 2) % 5) * width]
      const6 = ##0x06060606                         //
    }
    { iptr3 += MPYI(flush3,stridex2)                // &src[((flush + 3) % 5) * width]
      iptr4 += MPYI(flush4,stridex2)                // &src[((flush + 4) % 5) * width]
      const6h= ##0x00060006                         //
    }
    { const4 = ##0x04040404                         //
      Cnst8 = #8                                    //
      P3 = SP3LOOP0(.blur5x5s16RowLOOP,R28)         // setup loop0
    }

    .falign
.blur5x5s16RowLOOP:
    { sLine0 = VMEM(iptr0++#1)                      //[1]
      sVsum1 = sVsumv0O                             //[2]
      sSumE.w += VMPYI(sVsum2.w,const6.b)           //[3]
      sSumO.w += VMPYI(sVsum3.w,const6.b)           //[3]
    }
    { sLine1 = VMEM(iptr1++#1)                      //[1]
      sVsum3 = VALIGN(sVsumv1O,sVsumv0O,#4)         //[2]
      sSumE.w += VMPYI(sVsum1a3.w,const4.b)         //[3]
      sSumO.w += VMPYI(sVsum2a4.w,const4.b)         //[3]
    }
    { sLine4 = VMEM(iptr4++#1)                      //[1]
      sVsum5 = VALIGN(sVsumv1O,sVsumv0O,Cnst8)      //[2]
      sVsum1a3.w = VADD(sVsum1.w,sVsum3.w)          //[2]
      sVsum0 = sVsumv0E                             //[2]
    }
    { sLine2 = VMEM(iptr2++#1)                      //[1]
      dVsumv0 = dVsumv1                             //[1]
      sVsum2 = VALIGN(sVsumv1E,sVsumv0E,#4)         //[2]
    }
    { sLine3 = VMEM(iptr3++#1)                      //[1]
      dVsumv1.w  = VADD(sLine0.h,sLine4.h)          //[1]
      sVsum4 = VALIGN(sVsumv1E,sVsum0,Cnst8)        //[2]
    }
    { dVsumv1.w += VMPY(sLine2.h,const6h.h):sat     //[1]
      sSumE.w = VADD(sVsum0.w,sVsum4.w)             //[2]
      sOut.h = VASR(sSumO.w,sSumE.w,Cnst8)          //[3]
      IF P3 VMEM(optr++#1) = sOut.new               //[3]
    }
    { dVsumv1.w += VMPA(dLine3Line1.h,const4.b)     //[1]
      sSumO.w = VADD(sVsum1.w,sVsum5.w)             //[2]
      sVsum2a4.w = VADD(sVsum2.w,sVsum4.w)          //[2]
    }:endloop0

    //======
    { sVsum1 = sVsumv0O                             //[2]
      sSumE.w += VMPYI(sVsum2.w,const6.b)           //[3]
      sSumO.w += VMPYI(sVsum3.w,const6.b)           //[3]
    }
    { sVsum3 = VALIGN(sVsumv1O,sVsumv0O,#4)         //[2]
      sSumE.w += VMPYI(sVsum1a3.w,const4.b)         //[3]
      sSumO.w += VMPYI(sVsum2a4.w,const4.b)         //[3]
    }
    { sVsum5 = VALIGN(sVsumv1O,sVsumv0O,Cnst8)      //[2]
      sVsum1a3.w = VADD(sVsum1.w,sVsum3.w)          //[2]
      sVsum0 = sVsumv0E                             //[2]
    }
    { dVsumv0 = dVsumv1                             //[1]
      sVsum2 = VALIGN(sVsumv1E,sVsumv0E,#4)         //[2]
    }
    { sVsum4 = VALIGN(sVsumv1E,sVsum0,Cnst8)        //[2]
      sOut.h = VASR(sSumO.w,sSumE.w,Cnst8)          //[3]
      IF P3 VMEM(optr++#1) = sOut.new               //[3]
    }
    { sSumE.w = VADD(sVsum0.w,sVsum4.w)             //[2]
      sSumO.w = VADD(sVsum1.w,sVsum5.w)             //[2]
      sVsum2a4.w = VADD(sVsum2.w,sVsum4.w)          //[2]
    }
    //====== epilogue ======
    { sVsum1 = sVsumv0O                             //[2]
      sSumE.w += VMPYI(sVsum2.w,const6.b)           //[3]
      sSumO.w += VMPYI(sVsum3.w,const6.b)           //[3]
    }
    { sVsum3 = VALIGN(sVsumv1O,sVsumv0O,#4)         //[2]
      sSumE.w += VMPYI(sVsum1a3.w,const4.b)         //[3]
      sSumO.w += VMPYI(sVsum2a4.w,const4.b)         //[3]
    }
    { sVsum5 = VALIGN(sVsumv1O,sVsumv0O,Cnst8)      //[2]
      sVsum1a3.w = VADD(sVsum1.w,sVsum3.w)          //[2]
    }
    { sVsum2 = VALIGN(sVsumv1E,sVsumv0E,#4)         //[2]
      P3 = CMP.GT(R28,#1)                           //
    }
    { sVsum4 = VALIGN(sVsumv1E,sVsumv0E,Cnst8)      //[2]
      sOut.h = VASR(sSumO.w,sSumE.w,Cnst8)          //[3]
      IF P3 VMEM(optr++#1) = sOut.new               //[3]
    }
    { sSumE.w = VADD(sVsumv0E.w,sVsum4.w)           //[2]
      sSumO.w = VADD(sVsum1.w,sVsum5.w)             //[2]
      sVsum2a4.w = VADD(sVsum2.w,sVsum4.w)          //[2]
    }
    { sSumE.w += VMPYI(sVsum2.w,const6.b)           //[3]
      sSumO.w += VMPYI(sVsum3.w,const6.b)           //[3]
    }
    { sSumE.w += VMPYI(sVsum1a3.w,const4.b)         //[3]
      sSumO.w += VMPYI(sVsum2a4.w,const4.b)         //[3]
    }
    { sOut.h = VASR(sSumO.w,sSumE.w,Cnst8)          //[3]
      VMEM(optr+#0) = sOut.new                      //[3]
    }
    { JUMPR R31                                     // return
    }
    .size    blur5x5s16Row, .-blur5x5s16Row


