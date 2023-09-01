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
    .file    "gaussian3x3.S"

#include "hvx.cfg.h"
    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void Gaussian3x3u8PerRow()                                    ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: Performs 3x3 Gaussian blur on an image block                  ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned char *src  -- pointer to input image            ]*/
    /*[               R1 : int stride          -- stride of image input             ]*/
    /*[               R2 : int width           -- image width                       ]*/
    /*[               R3 : unsigned char *dst  -- pointer to output buffer          ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
    /*[           - process two rows                                                ]*/
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
    /*[   6.0.0         08/01/2014              created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define iptr1               R0
#define stride              R1
#define width               R2
#define optr                R3
#define curline             R4
#define height              R5
#define delta               R6
#define const4              R7
#define iptr0               R8
#define iptr2               R9
#define const1              R10
#define const2              R11
/* ============================================================ */
#define sLine0              V0
#define sLine2              V1
#define dLine2Line0         V1:0
#define sLine1              V2
#define sOut                V3
#define sVsumv0E            V4
#define sVsumv0O            V5
#define sVsumv1E            V6
#define sVsumv1O            V7
#define dVsumv1             V7:6
#define sVsumv2E            V8
#define sVsumv2O            V9
#define dVsumv2             V9:8
#define sVsum0              V10
#define sVsum3              V11
#define sSumE               V12
#define sSumO               V13
#define rightboundo         sLine0
#define rightbounde         sLine1
#define sVsum1              sVsumv1E
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl Gaussian3x3u8PerRow
    .type    Gaussian3x3u8PerRow, @function
Gaussian3x3u8PerRow:
    { R28 = ADD(width,#(VLEN-1))                    //
      iptr0 = SUB(iptr1,stride)                     //
      iptr2 = ADD(iptr1,stride)                     //
    }
    { const1 = ##0x01010101                         //
      const4 = #4                                   //
      R28 = LSR(R28,#LOG2VLEN)                      // ceil(width/VLEN)-1
    }
    { P3 = SP2LOOP0(.Gaussian3x3u8PerRowLOOP,R28)   // setup loop0
      const2 = ADD(const1,const1)                   //
    }

    .falign
.Gaussian3x3u8PerRowLOOP:
    { sLine0 = VMEM(iptr0++#1)                      //[1]
      sSumE.h = VADD(sVsum0.h,sVsumv0O.h)           //[2]
      sVsum1.h += VMPYI(sVsumv0O.h,const2.b)        //[2]
      sVsum3 = VALIGN(sVsumv2E,sVsumv1E,#2)         //[2]
    }
    { sVsumv1E = sVsumv2E                           //[1]
      sLine1.tmp = VMEM(iptr1++#1)                  //[1]
      dVsumv2.h  = VMPY(sLine1.ub,const2.b)         //[1]
      sSumO.h = VADD(sVsum1.h,sVsum3.h)             //[2]
    }
    { sLine2.tmp = VMEM(iptr2++#1)                  //[1]
      dVsumv2.h += VMPA(dLine2Line0.ub,const1.b)    //[1]
      sVsum0 = VLALIGN(sVsumv1O,sVsumv0O,#2)        //[1]
      sVsumv0O = sVsumv1O                           //[1]
    }
    { sVsumv1O = sVsumv2O                           //[1]
      sVsum0.h += VMPYI(sVsum1.h,const2.b)          //[1]
      sOut.ub = VASR(sSumO.h,sSumE.h,const4):rnd:sat//[2]
      IF P3 VMEM(optr++#1) = sOut.new               //[2]
    }:endloop0

    //==
    { sSumE.h = VADD(sVsum0.h,sVsumv0O.h)           //[2]
      sVsum1.h += VMPYI(sVsumv0O.h,const2.b)        //[2]
      sVsum3 = VALIGN(sVsumv2E,sVsumv1E,#2)         //[2]
    }
    { sVsumv1E = sVsumv2E                           //[1]
      sSumO.h = VADD(sVsum1.h,sVsum3.h)             //[2]
    }
    { sVsum0 = VLALIGN(sVsumv1O,sVsumv0O,#2)        //[1]
      sVsumv0O = sVsumv1O                           //[1]
    }
    { sVsum0.h += VMPYI(sVsum1.h,const2.b)          //[2]
      sOut.ub = VASR(sSumO.h,sSumE.h,const4):rnd:sat//[3]
      IF P3 VMEM(optr++#1) = sOut.new               //[3]
    }
    //==
    { sSumE.h = VADD(sVsum0.h,sVsumv0O.h)           //[2]
      sVsum1.h += VMPYI(sVsumv0O.h,const2.b)        //[2]
      sVsum3 = VALIGN(sVsumv2E,sVsumv1E,#2)         //[2]
    }
    { sSumO.h = VADD(sVsum1.h,sVsum3.h)             //[2]
    }
    { sOut.ub = VASR(sSumO.h,sSumE.h,const4):rnd:sat//[3]
      VMEM(optr+#0) = sOut.new                      //[3]
    }
    { JUMPR R31                                     // return
    }
    .size    Gaussian3x3u8PerRow, .-Gaussian3x3u8PerRow
