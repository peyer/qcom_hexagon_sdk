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
    .file    "gaussian7x7.S"

#include "hvx.cfg.h"
    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void Gaussian7x7u8PerRow()                                    ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: Performs 7x7 Gaussian blur on an image block                  ]*/
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
#define iptr3               R0
#define stride              R1
#define width               R2
#define optr                R3
#define shift               R4
#define iptr0               R5
#define iptr1               R6
#define iptr2               R7
#define iptr4               R8
#define iptr5               R9
#define iptr6               R10
#define coef1               R11
#define coef2               R12
#define coef3               R13
#define coefs               R14
#define coef2coef3          R14
#define coef0coef1          R15
#define coef01_23           R15:14

/* ============================================================ */
#define sLine0              V0
#define sLine6              V1
#define dLine6Line0         V1:0
#define sLine1              V2
#define sLine5              V3
#define dLine5Line1         V3:2
#define sXV0e               V4
#define sXV0o               V5
#define dXV0                V5:4
#define sXV1e               V6
#define sXV1o               V7
#define dXV1                V7:6
#define sVsume              V8
#define sVsumo              V9
#define dVsum               V9:8
#define sSum02_L            V10
#define sSum02_H            V11
#define dSum02              V11:10
#define sSum13_L            V12
#define sSum13_H            V13
#define dSum13              V13:12
#define sX2X_2              V14
#define sX3X_3              V15
#define dX3X_3X2X_2         V15:14
#define sX3X_1              V16
#define sX4X_2              V17
#define dX4X_2X3X_1         V17:16
#define sX_1                V18
#define sLine2              sLine0
#define sLine4              sLine6
#define dLine4Line2         dLine6Line0
#define sLine3              sLine1
#define sXV1e_t             sVsume
#define sX_2                sX4X_2
#define sX_3                sX3X_3
#define sX2                 sX2X_2
#define sX3                 sLine1
#define sX4                 sLine5
#define sX0                 sSum02_L
#define sX1X_1              sSum02_H
#define dX1X_1X0            dSum02
#define sX1                 sSum13_L
#define sX2X0               sSum13_H
#define dX2X0X1             dSum13
#define sOut02              sSum02_L
#define sOut13              sSum13_L
#define sOut                sLine5 



/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl Gaussian7x7u8PerRow
    .type    Gaussian7x7u8PerRow, @function
Gaussian7x7u8PerRow:
    { R28 = ADD(width,#(VLEN-1))                    //
      shift = #12                                   //
      coefs = ##0x01060F14                          // coefs = 1, 6, 15, 20
    }
    { R28 = LSR(R28,#LOG2VLEN)                      // ceil(width/VLEN)
      coef2 = ASR(coefs,#8)                         //
      iptr2 = SUB(iptr3,stride)
      iptr4 = ADD(iptr3,stride)                     //
    }
    { iptr1 = SUB(iptr2,stride)                     //
      iptr5 = ADD(iptr4,stride)                     //
      coef2 = VSPLATB(coef2)                        //
      coef01_23 = PACKHL(coefs,coefs)               //
    }
    { iptr0 = SUB(iptr1,stride)                     //
      iptr6 = ADD(iptr5,stride)                     //
      coef1 = VSPLATB(coef0coef1)                   //
      coef3 = VSPLATB(coef2coef3)                   //
    }
    { P3 = SP2LOOP0(.Gaussian7x7u8PerRowLOOP,R28)   // setup loop0
    }

    .falign
.Gaussian7x7u8PerRowLOOP:        
    { sLine0 = VMEM(iptr0++#1)                      //[1]
      sX2 = VALIGN(sXV1e_t,sXV0e,#2)                //[2]
      sX0 = sXV0e                                   //[2]
      sX1 = sXV0o                                   //[2]
    }
    { sLine6 = VMEM(iptr6++#1)                      //[1]
      sX3 = VALIGN(sXV1o,sXV0o,#2)                  //[2]
      sX2X0.h  = VADD(sX2.h,sX0.h)                  //[2]
      sX2X_2.h = VADD(sX2.h,sX_2.h)                 //[2]
    }
    { sX4 = VALIGN(sXV1e,sXV0e,#4)                  //[2]
      dSum02.w = VMPA(dX1X_1X0.h,coef2coef3.b)      //[2]
      sX3X_3.h = VADD(sX3.h,sX_3.h)                 //[2]
    }
    { sLine1 = VMEM(iptr1++#1)                      //[1]
      dSum13.w = VMPA(dX2X0X1.h,coef2coef3.b)       //[2]
      sX3X_1.h = VADD(sX3.h,sX_1.h)                 //[2]
    }
    { sLine5 = VMEM(iptr5++#1)                      //[1]
      dVsum.h = VADD(sLine0.ub,sLine6.ub)           //[1]
      sX4X_2.h = VADD(sX4.h,sX_2.h)                 //[2]
    }
    { sLine2 = VMEM(iptr2++#1)                      //[1]
      sX_1 = VLALIGN(sXV1o,sXV0o,#2)                //[1]
      dSum02.w += VMPA(dX3X_3X2X_2.h,coef0coef1.b)  //[2]
    }
    { sLine4 = VMEM(iptr4++#1)                      //[1]
      sX_2 = VLALIGN(sXV1e,sXV0e,#2)                //[1]
      dSum13.w += VMPA(dX4X_2X3X_1.h,coef0coef1.b)  //[2]
    }
    { sLine3 = VMEM(iptr3++#1)                      //[1]
      dVsum.h += VMPA(dLine5Line1.ub,coef1.b)       //[1]
      sX_3 = VLALIGN(sXV1o,sXV0o,#4)                //[1]
    }
    { dVsum.h += VMPA(dLine4Line2.ub,coef2.b)       //[1]
      sXV0e = sXV1e                                 //[1]
      sOut02.h = VASR(sSum02_H.w,sSum02_L.w,shift)  //[2]
    }
    { dVsum.uh += VMPY(sLine3.ub,coef3.ub)          //[1]
      sXV0o = sXV1o                                 //[1]
      sOut13.h = VASR(sSum13_H.w,sSum13_L.w,shift)  //[2]
    }
    { dXV1 = dVsum                                  //[1]
      sX1X_1.h = VADD(sXV0o.h,sX_1.h)               //[1]
      sOut.b = VSHUFFE(sOut13.b,sOut02.b)           //[2]
      IF P3 VMEM(optr++#1) = sOut.new               //[2]
    }:endloop0


.Gaussian7x7u8PerRow_LPEND:
    { sX2 = VALIGN(sXV1e_t,sXV0e,#2)                //[2]
      sX0 = sXV0e                                   //[2]
      sX1 = sXV0o                                   //[2]
    }
    { sX3 = VALIGN(sXV1o,sXV0o,#2)                  //[2]
      sX2X0.h  = VADD(sX2.h,sX0.h)                  //[2]
      sX2X_2.h = VADD(sX2.h,sX_2.h)                 //[2]
    }
    { sX4 = VALIGN(sXV1e,sXV0e,#4)                  //[2]
      dSum02.w = VMPA(dX1X_1X0.h,coef2coef3.b)      //[2]
      sX3X_3.h = VADD(sX3.h,sX_3.h)                 //[2]
    }
    { dSum13.w = VMPA(dX2X0X1.h,coef2coef3.b)       //[2]
      sX3X_1.h = VADD(sX3.h,sX_1.h)                 //[2]
      sX4X_2.h = VADD(sX4.h,sX_2.h)                 //[2]
    }
    { sX_1 = VLALIGN(sXV1o,sXV0o,#2)                //[1]
      dSum02.w += VMPA(dX3X_3X2X_2.h,coef0coef1.b)  //[2]
    }
    { sX_2 = VLALIGN(sXV1e,sXV0e,#2)                //[1]
      dSum13.w += VMPA(dX4X_2X3X_1.h,coef0coef1.b)  //[2]
    }
    { sX_3 = VLALIGN(sXV1o,sXV0o,#4)                //[1]
      dXV0 = dXV1                                   //[1]
      sOut02.h = VASR(sSum02_H.w,sSum02_L.w,shift)  //[2]
    }
    { sOut13.h = VASR(sSum13_H.w,sSum13_L.w,shift)  //[2]
      sXV1e = #0                                    //
      sXV1o = #0                                    //
    }
    { //dXV1 = dVsum                                //[1]
      sX1X_1.h = VADD(sXV0o.h,sX_1.h)               //[1]
      sOut.b = VSHUFFE(sOut13.b,sOut02.b)           //[2]
      IF P3 VMEM(optr++#1) = sOut.new               //[2]
    }
    //========
    { sX2 = VALIGN(sXV1e_t,sXV0e,#2)                //[2]
      sX0 = sXV0e                                   //[2]
      sX1 = sXV0o                                   //[2]
    }
    { sX3 = VALIGN(sXV1o,sXV0o,#2)                  //[2]
      sX2X0.h  = VADD(sX2.h,sX0.h)                  //[2]
      sX2X_2.h = VADD(sX2.h,sX_2.h)                 //[2]
    }
    { sX4 = VALIGN(sXV1e,sXV0e,#4)                  //[2]
      dSum02.w = VMPA(dX1X_1X0.h,coef2coef3.b)      //[2]
      sX3X_3.h = VADD(sX3.h,sX_3.h)                 //[2]
    }
    { dSum13.w = VMPA(dX2X0X1.h,coef2coef3.b)       //[2]
      sX3X_1.h = VADD(sX3.h,sX_1.h)                 //[2]
      sX4X_2.h = VADD(sX4.h,sX_2.h)                 //[2]
    }
    { dSum02.w += VMPA(dX3X_3X2X_2.h,coef0coef1.b)  //[2]
    }
    { dSum13.w += VMPA(dX4X_2X3X_1.h,coef0coef1.b)  //[2]
    }
    { sOut02.h = VASR(sSum02_H.w,sSum02_L.w,shift)  //[2]
    }
    { sOut13.h = VASR(sSum13_H.w,sSum13_L.w,shift)  //[2]
    }
    { sOut.b = VSHUFFE(sOut13.b,sOut02.b)           //[2]
      VMEM(optr+#0) = sOut.new                      //[2]
    }
    { JUMPR R31                                     // return
    }
    .size    Gaussian7x7u8PerRow, .-Gaussian7x7u8PerRow


