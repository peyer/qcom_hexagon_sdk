    /* ============================================================================ */
    /*  QUALCOMM TECHNOLOGIES, INC.                                                 */
    /*                                                                              */
    /*  HEXAGON HVX Image/Video Processing Library                                  */
    /*                                                                              */
    /* ---------------------------------------------------------------------------- */
    /*            Copyright (c) 2015 QUALCOMM TECHNOLOGIES Incorporated.            */
    /*                             All Rights Reserved.                             */
    /*                    QUALCOMM Confidential and Proprietary                     */
    /* ============================================================================ */
#include "hvx.cfg.h"
    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void convertMipiToRaw16_aligned_PerNRow()                     ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: convert Mipi to Raw16                                         ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned char  *input   -- pointer to input image        ]*/
    /*[               R1 : unsigned        istride -- stride of intput image        ]*/
    /*[               R2 : unsigned        width   -- image width                   ]*/
    /*[               R3 : unsigned        height  -- image height                  ]*/
    /*[               R4 : unsigned short *output  -- pointer to output image       ]*/
    /*[               R5 : unsigned        ostride -- stride of output image        ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
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
    /*[   6.0.0         09/18/2014              created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define SS                  (8*2)
#define iptr                R0
#define optr                R4
#define const0401           R6
#define const103            R8
#define const100            R9
#define const77             R10
#define const76             R11
#define const52             R12
#define const48             R13
#define const26             R14
#define const24             R15
#define pos0                R8
#define pos1                R9
#define pos2                R10
#define pos3                R11
#define pos4                R12
#define pos5                R13
#define pos6                R14
#define pos7                R15
/* ============================================================ */
#define Qoddeven            Q3
#define sPermLSB0           V0
#define sPermMSB0           V1
#define sA                  sPermMSB0
#define sOddEven            sPermMSB0
#define sPermLSB1           V2
#define sPermMSB1           V3
#define sB                  sPermMSB1
#define sPermLSB2           V4
#define sPermMSB2           V5
#define sC                  sPermMSB2
#define sPermLSB3           V6
#define sPermMSB3           V7
#define sD                  sPermMSB3
#define sPermLSB4           V8
#define sPermMSB4           V9
#define sE                  sPermMSB4
#define sMSB0               sPermMSB0
#define sLSB0               sPermLSB0
#define sMSB1               sPermMSB1
#define sLSB1               sPermLSB1
#define sMSB2               sPermMSB2
#define sLSB2               sPermLSB2
#define sMSB3               sPermMSB3
#define sLSB3               sPermLSB3
#define dMSB0               sPermMSB0:0
#define dMSB1               sPermMSB1:2
#define dMSB2               sPermMSB2:4
#define dMSB3               sPermMSB3:6
#define sLSB0shft           sLSB0
#define sLSB1shft           sLSB1
#define sLSB2shft           sLSB2
#define sLSB3shft           sLSB3
#define sLSB0shft2          V10
#define sLSB1shft2          sLSB0shft2
#define sLSB2shft2          sLSB0shft2
#define sLSB3shft2          sLSB0shft2
#define sOutPerm0_L         V12
#define sOutPerm0_H         V13
#define dOutPerm0           sOutPerm0_H:12
#define sOutPerm1_L         V14
#define sOutPerm1_H         V15
#define dOutPerm1           sOutPerm1_H:14
#define sOutPerm2_L         V16
#define sOutPerm2_H         V17
#define dOutPerm2           sOutPerm2_H:16
#define sOutPerm3_L         V18
#define sOutPerm3_H         V19
#define dOutPerm3           sOutPerm3_H:18
#define dOut0               dOutPerm0
#define dOut1               dOutPerm1
#define dOut2               dOutPerm2
#define dOut3               dOutPerm3
#define sOut0_L             sOutPerm0_L
#define sOut0_H             sOutPerm0_H
#define sOut1_L             sOutPerm1_L
#define sOut1_H             sOutPerm1_H
#define sOut2_L             sOutPerm2_L
#define sOut2_H             sOutPerm2_H
#define sOut3_L             sOutPerm3_L
#define sOut3_H             sOutPerm3_H
#define sQpred              V11
#define sShift              V20
#define sConst3             V21
#define sCtrlMSB_A          V22
#define sCtrlLSB_A          V23
#define sCtrlMSB_AB         V24
#define sCtrlLSB_AB         V25
#define sCtrlMSB_BC         V26
#define sCtrlLSB_BC         V27
#define sCtrlMSB_CD         V28
#define sCtrlLSB_CD         V29
#define sCtrlMSB_E          V30
#define sCtrlLSB_E          V31
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl convertMipiToRaw16_aligned_PerNRow
    .type    convertMipiToRaw16_aligned_PerNRow, @function
convertMipiToRaw16_aligned_PerNRow:
    { R29 = ADD(R29,#-SS)                           //
      MEMD(R29+#(0*8-SS)) = R17:16                  //
      R16 = ADD(PC,##_GLOBAL_OFFSET_TABLE_@PCREL)   // ##CtrlPerm
    }
    { R16 = MEMW(R16+##CtrlPerm@GOT)                // ##CtrlPerm
      MEMD(R29+#(1*8)) = R19:18                     //
      R18 = ADD(R0,R1)                              //
    }
    { sCtrlMSB_A = VMEM(R16++#1)                    //
      R6 = ##0x3030401                              //
      R19 = ADDASL(R4,R5,#1)                        //
    }
    { sCtrlLSB_A = VMEM(R16++#1)                    //
      R7:6 = PACKHL(R6,R6)                          //
      R17 = AND(R2,##4*VLEN-1)                      //
    }
    { sCtrlMSB_AB = VMEM(R16++#1)                   //
      R2 = ADD(R2,##4*VLEN-1)                       //
      sConst3 = VSPLAT(R7)                          //
    }
    { sCtrlLSB_AB = VMEM(R16++#1)                   //
      R2 = LSR(R2,#LOG2VLEN+2)                      //
      R9 = ##0x40000                                //
    }
    { sCtrlMSB_BC = VMEM(R16++#1)                   //
      P3 = SP1LOOP0(.convertMipiToRaw16_aligned_PerNRow_LP0,R2)//
      sShift = VSPLAT(R9)                           //
      R7 = #-2                                      //
    }
    { sCtrlLSB_BC = VMEM(R16++#1)                   //
      R8 = ##0x2020101                              //
    }
    { sCtrlMSB_CD = VMEM(R16++#1)                   //
      R15 = ##0x30003                               //
      R9:8 = PACKHL(R8,R8)                          //
    }
    { sCtrlLSB_CD = VMEM(R16++#1)                   //
      sOddEven = VSPLAT(R15)                        //
      R11:10 = VASLW(R9:8,#2)                       //
    }
    { sCtrlMSB_E = VMEM(R16++#1)                    //
      Qoddeven = VCMP.EQ(sOddEven.b,sConst3.b)      //
      R13:12 = VASLW(R9:8,#4)                       //
    }
    { sQpred = VMEM(R16+#1)                         //
      R15:14 = VASLW(R9:8,#6)                       //
    }
    { sCtrlLSB_E = VMEM(R16+#0)                     //
      R16 = #2                                      //
      Q2 = VAND(sQpred,pos4)                        //
    }
    .falign
.convertMipiToRaw16_aligned_PerNRow_LP1:
.convertMipiToRaw16_aligned_PerNRow_LP0:
    { sA.cur = VMEM(iptr++#1)                       //[1]
      sPermLSB0 = VRDELTA(sA,sCtrlLSB_A)            //[1]
      Q0 = VAND(sQpred,pos1)                        //[1]
    }
    { sB.cur = VMEM(iptr++#1)                       //[1]
      sPermLSB1 = VRDELTA(sB,sCtrlLSB_AB)           //[1]
      Q1 = VAND(sQpred,pos3)                        //[1]
      VMEM(R4+#0) = sOut0_L                         //[2]
    }
    { sLSB0 = VMUX(Q0,sPermLSB0,sPermLSB1)          //[1]
      dOut2 = VSHUFF(sOutPerm2_H,sOutPerm2_L,R7)    //[2]
      IF P3 VMEM(R4+#1) = sOut0_H                   //[2]
    }
    { sC.cur = VMEM(iptr++#1)                       //[1]
      sPermLSB2 = VRDELTA(sC,sCtrlLSB_BC)           //[1]
      Q0 = VAND(sQpred,pos5)                        //[1]
    }
    { sLSB1 = VMUX(Q1,sPermLSB1,sPermLSB2)          //[1]
      dOut3 = VSHUFF(sOutPerm3_H,sOutPerm3_L,R7)    //[2]
      IF P3 VMEM(R4+#2) = sOut1_L                   //[2]
    }
    { sD.cur = VMEM(iptr++#1)                       //[1]
      sPermLSB3 = VRDELTA(sD,sCtrlLSB_CD)           //[1]
      Q1 = VAND(sQpred,pos7)                        //[1]
      sLSB0shft.h = VASR(sLSB0.h,sShift.h)          //[1]
    }
    { sE.cur = VMEM(iptr++#1)                       //[1]
      sPermLSB4 = VRDELTA(sE,sCtrlLSB_E)            //[1]
      sLSB2 = VMUX(Q0,sPermLSB2,sPermLSB3)          //[1]
      sLSB1shft.h = VASR(sLSB1.h,sShift.h)          //[1]
    }
    { sPermMSB0 = VRDELTA(sA,sCtrlMSB_A)            //[1]
      Q0 = VAND(sQpred,pos0)                        //[1]
      sLSB3 = VMUX(Q1,sPermLSB3,sPermLSB4)          //[1]
      IF P3 VMEM(R4+#3) = sOut1_H                   //[2]
    }
    { sPermMSB1 = VRDELTA(sB,sCtrlMSB_AB)           //[1]
      Q1 = VAND(sQpred,pos2)                        //[1]
      sLSB2shft.h = VASR(sLSB2.h,sShift.h)          //[1]
      IF P3 VMEM(R4+#4) = sOut2_L                   //[2]
    }
    { sPermMSB2 = VRDELTA(sC,sCtrlMSB_BC)           //[1]
      sMSB0 = VMUX(Q0,sPermMSB0,sPermMSB1)          //[1]
      sLSB3shft.h = VASR(sLSB3.h,sShift.h)          //[1]
      IF P3 VMEM(R4+#5) = sOut2_H                   //[2]
    }
    { sPermMSB3 = VRDELTA(sD,sCtrlMSB_CD)           //[1]
      Q1 = VAND(sQpred,pos6)                        //[1]
      sMSB1 = VMUX(Q1,sPermMSB1,sPermMSB2)          //[1]
      sLSB0shft2.h = VASR(sLSB0shft.h,R16)          //[1]
    }
    { sPermMSB4 = VRDELTA(sE,sCtrlMSB_E)            //[1]
      sMSB2 = VMUX(Q2,sPermMSB2,sPermMSB3)          //[1]
      sLSB1shft2.h = VASR(sLSB1shft.h,R16)          //[1]
      sLSB0 = VMUX(Qoddeven,sLSB0shft,sLSB0shft2)   //[1]
    }
    { sMSB3 = VMUX(Q1,sPermMSB3,sPermMSB4)          //[1]
      sLSB2shft2.h = VASR(sLSB2shft.h,R16)          //[1]
      sLSB1 = VMUX(Qoddeven,sLSB1shft,sLSB1shft2)   //[1]
      sLSB0 = VAND(sLSB0,sConst3)                   //[1]
    }
    { sLSB3shft2.h = VASR(sLSB3shft.h,R16)          //[1]
      sLSB2 = VMUX(Qoddeven,sLSB2shft,sLSB2shft2)   //[1]
      sLSB1 = VAND(sLSB1,sConst3)                   //[1]
      IF P3 VMEM(R4+#6) = sOut3_L                   //[2]
    }
    { sLSB3 = VMUX(Qoddeven,sLSB3shft,sLSB3shft2)   //[1]
      sLSB2 = VAND(sLSB2,sConst3)                   //[1]
      dOutPerm0.h = VMPA(dMSB0.ub,const0401.b)      //[1]
    }
    { sLSB3 = VAND(sLSB3,sConst3)                   //[1]
      dOutPerm1.h = VMPA(dMSB1.ub,const0401.b)      //[1]
      IF P3 VMEM(R4+#7) = sOut3_H                   //[2]
    }
    { dOutPerm2.h = VMPA(dMSB2.ub,const0401.b)      //[1]
      dOut0 = VSHUFF(sOutPerm0_H,sOutPerm0_L,R7)    //[1]
      IF P3 R4 = ADD(R4,##8*VLEN)                   //[1]
    }
    { dOutPerm3.h = VMPA(dMSB3.ub,const0401.b)      //[1]
      dOut1 = VSHUFF(sOutPerm1_H,sOutPerm1_L,R7)    //[1]
    }:endloop0
    { dOut2 = VSHUFF(sOutPerm2_H,sOutPerm2_L,R7)    //[2]
      VMEM(R4++#1) = sOut0_L                        //[2]
      P2 = CMP.GTU(R17,#VLEN/2*1)                   //
    }
    { dOut3 = VSHUFF(sOutPerm3_H,sOutPerm3_L,R7)    //[2]
      IF P2 VMEM(R4++#1) = sOut0_H                  //[2]
      P2 = CMP.GTU(R17,#VLEN/2*2)                   //
    }
    { IF P2 VMEM(R4++#1) = sOut1_L                  //[2]
      P2 = CMP.GTU(R17,#VLEN/2*3)                   //
      P3 = SP1LOOP0(.convertMipiToRaw16_aligned_PerNRow_LP0,R2)//
    }
    { IF P2 VMEM(R4++#1) = sOut1_H                  //[2]
      P2 = CMP.GTU(R17,#VLEN/2*4)                   //
      R18 = ADD(R18,R1)                             //
      R0 = R18                                      //
    }
    { IF P2 VMEM(R4++#1) = sOut2_L                  //[2]
      P2 = CMP.GTU(R17,#VLEN/2*5)                   //
      R3 = ADD(R3,#-1)                              //
    }
    { IF P2 VMEM(R4++#1) = sOut2_H                  //[2]
      P2 = CMP.GTU(R17,#VLEN/2*6)                   //
    }
    { IF P2 VMEM(R4++#1) = sOut3_L                  //[2]
      P2 = CMP.GTU(R17,#VLEN/2*7)                   //
    }
    { IF P2 VMEM(R4+#0) = sOut3_H                   //[2]
      R19 = ADDASL(R19,R5,#1)                       //
      R4 = R19                                      //
      IF (R3!=#0) JUMP:t .convertMipiToRaw16_aligned_PerNRow_LP1//
    }
    { JUMPR R31                                     // return
      R17:16 = MEMD(R29+#0*8)                       //
      R19:18 = MEMD(R29+#1*8)                       //
      R29 = ADD(R29,#SS)                            //
    }
    .size    convertMipiToRaw16_aligned_PerNRow, .-convertMipiToRaw16_aligned_PerNRow
