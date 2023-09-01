    /* ============================================================================ */
    /*  QUALCOMM TECHNOLOGIES, INC.                                                 */
    /*                                                                              */
    /*  HEXAGON HVX Image/Video Processing Library                                  */
    /*                                                                              */
    /* ---------------------------------------------------------------------------- */
    /*            Copyright (c) 2016 QUALCOMM TECHNOLOGIES Incorporated.            */
    /*                             All Rights Reserved.                             */
    /*                    QUALCOMM Confidential and Proprietary                     */
    /* ============================================================================ */

#define VLEN 128
#define LOG2VLEN 7

    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void integratePerRow                                          ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: calculate integrations of one image line                      ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned char *src  -- pointer to input image            ]*/
    /*[               R1 : int width           -- width of image block              ]*/
    /*[               R2 : int stride_i        -- stride of image block             ]*/
    /*[               R3 : unsigned int   *dst -- pointer to integration outputs    ]*/
    /*[               R4 : int stride_o        -- stride of output                  ]*/
    /*[               R5 : int *preint         -- pointer to previous integration   ]*/
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
    /*[   6.0.0         05/22/2016              created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define pIn0                R0
#define pIn1                R8
#define pOut_1              R9
#define pOut0               R3
#define pOut1               R10
#define c_4                 R2
#define c8                  R4
#define c16                 R5
#define c32                 R6
#define c64                 R7
#define ch0x1               R13
#define ch0x100             R14
#define const1              R15
/* ============================================================ */
#define sPixels0            V0
#define sPixels1            V1
#define sL0                 V2
#define sL1                 V3
#define sL01_L              V24
#define sL01_H              V25
#define dL01                sL01_H:24
#define sA3                 V4
#define sA3t                V5
#define sSum3               V6
#define sA1                 V7
#define sA30_4              sPixels0
#define sA70_4              sPixels0
#define sA150_4             sPixels0
#define sA310_4             sPixels0
#define sA630_4             sA3
#define sSum3t              V8
#define sSum1               V9
#define sSum2               V10
#define sSum0               sL0
#define sPreIntg            sL1
#define dSum31              sSum1:8
#define sSum31_L            sSum3t
#define sSum31_H            sSum1
#define sIntgA_L            V12
#define sIntgA_H            V13
#define dIntgA              sIntgA_H:12
#define sSum20_L            V14
#define sSum20_H            V15
#define dSum20              sSum20_H:14
#define sPreA               sPixels0
#define sIntgB_L            V16
#define sIntgB_H            V17
#define dIntgB              sIntgB_H:16
#define sIntgC_L            V18
#define sIntgC_H            V19
#define dIntgC              sIntgC_H:18
#define sPreB               sL0
#define sIntgD_L            V20
#define sIntgD_H            V21
#define dIntgD              sIntgD_H:20
#define sPreC               sSum3t
#define sPreD               sSum1
#define sSumBA_L            V22
#define sSumBA_H            V23
#define dSumBA              sSumBA_H:22
#define sSumDC_L            sSum20_L
#define sSumDC_H            sSum20_H
#define dSumDC              dSum20
#define sIntA               sPixels0
#define sIntB               sIntgA_L
#define sIntC               sIntgA_H
#define sIntD               sSum20_L
#define sCachePreInt        sSumBA_L
#define sCachePreInt_4      sSumBA_H
#define sPreIntg_L          V28
#define sPreIntg_H          V29
#define dPreIntg            V29:28
#define sPattern            V30
#define sZero               V31
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl IntegrateRowAcc
    .type    IntegrateRowAcc, @function
IntegrateRowAcc:
    { const1 = ##0x01010101                         //
      R10 = ADD(PC,##SPLATpattern@PCREL)            //
    }
    { sPixels0.cur = VMEM(pIn0++#1):NT              //[1]
      sL0.h = VDMPY(sPixels0.ub,const1.b)           //[1]
      pIn1 = ADD(pIn0,R2)                           //
      R13:12 = BITSPLIT(R1,#24)                     //
    }
    { sPixels1.cur = VMEM(pIn1++#1):NT              //[1]
      sL1.h = VDMPY(sPixels1.ub,const1.b)           //[1]
      sA3.h = VSHUFFO(sPixels1.h,sPixels0.h)        //[1]
      P0 = !CMP.EQ(R13,#0)                          //
    }
    { dL01.h = VSHUFFOE(sL1.h,sL0.h)                //[1]
      ch0x100 = ##0x1000100                         //
      IF P0 JUMP .skipAccPreInt                     //
    }
    { sCachePreInt = VMEM(R5)                       //
    }
    .falign
.skipAccPreInt:
    { sSum3.h = VADD(sL01_H.h,sL01_L.h)             //[1]
      sA3t.h = VDMPY(sA3.ub,ch0x100.b)              //[1]
      sZero = #0                                    //
    }
    { sA1.h = VSHUFFE(sPixels1.h,sPixels0.h)        //[1]
      pOut_1 = SUB(pOut0,R4)                        //
      pOut1 = ADDASL(pOut0,R4,#2)                   //
      sPattern = VMEM(R10+#0)                       //
    }
    { sA30_4 = VLALIGN(sSum3,sZero,#4)              //[1]
      pOut_1 -= MPYI(R4,#3)                         //
    }
    { sSum3.h = VADD(sSum3.h,sA30_4.h)              //[1]
      sCachePreInt_4 = VLALIGN(sCachePreInt,sCachePreInt,#4);//
      R12 = ADD(R12,#VLEN-1-VLEN)                   //
    }
    { sA1.h = VDMPY(sA1.ub,ch0x100.b)               //[1]
      c8 = #8                                       //
      sPreIntg_H = vrdelta(sCachePreInt,sPattern)   //
    }
    { sA70_4 = VLALIGN(sSum3,sZero,c8)              //[1]
      R12 = ASR(R12,#LOG2VLEN)                      //
    }
    { sSum3.h = VADD(sSum3.h,sA70_4.h)              //[1]
      c16 = #16                                     //
      R28 = R5                                      //
      sPreIntg_L = vrdelta(sCachePreInt_4,sPattern) //
    }
    { sA150_4 = VLALIGN(sSum3,sZero,c16)            //[1]
      P3 = SP1LOOP0(.integratePerRowAcc_Loop,R12)   //
      P1 = CMP.GT(R12,#0)                           //
    }
    { sSum3.h = VADD(sSum3.h,sA150_4.h)             //[1]
      c32 = #32                                     //
      IF P0 sPreIntg_H = sZero                      //
      IF P0 sPreIntg_L = sZero                      //
    }
    { sA310_4 = VLALIGN(sSum3,sZero,c32)            //[1]
      ch0x1 = ##0x00010001                          //
    }
    { sSum3.h = VADD(sSum3.h,sA310_4.h)             //[1]
      c64 = #64                                     //
      c_4 = #-4                                     //
      IF !P1 JUMP .integratePerRowAcc_LoopEND       //
    }
    .falign
.integratePerRowAcc_Loop:
    { sPixels0.cur = VMEM(pIn0++#1):NT              //[1]
      sL0.h = VDMPY(sPixels0.ub,const1.b)           //[1]
      sA630_4 = VLALIGN(sSum3,sZero,c64)            //[2]
      sIntD.w = VADD(sIntgD_H.w,sIntgD_L.w)         //[3]
    }
    { sPixels1.cur = VMEM(pIn1++#1):NT              //[1]
      sL1.h = VDMPY(sPixels1.ub,const1.b)           //[1]
      sA3.h = VSHUFFO(sPixels1.h,sPixels0.h)        //[1]
      sSum3t.h = VADD(sSum3.h,sA630_4.h)            //[2]
    }
    { dL01.h = VSHUFFOE(sL1.h,sL0.h)                //[1]
      sSum1.h = VSUB(sSum3t.h,sL01_H.h)             //[2]
      sSum2.h = VSUB(sSum3t.h,sA3t.h)               //[2]
    }
    { sSum3.h = VADD(sL01_H.h,sL01_L.h)             //[1]
      sA3t.h = VDMPY(sA3.ub,ch0x100.b)              //[1]
      sSum0.h = VSUB(sSum1.h,sA1.h)                 //[2]
      sPreIntg = VRDELTA(sSum3t,sPattern)           //[2]
    }
    { sA1.h = VSHUFFE(sPixels1.h,sPixels0.h)        //[1]
      dSum31 = VSHUFF(sSum3t,sSum1,c_4)             //[2]
      IF P3 VMEM(pOut1++#1) = sIntB                 //[3]
    }
    { sA30_4 = VLALIGN(sSum3,sZero,#4)              //[1]
      sIntgA_H = sPreIntg_H                         //[2]
      IF P3 VMEM(pOut1++#1) = sIntC                 //[3]
    }
    { sSum3.h = VADD(sSum3.h,sA30_4.h)              //[1]
      dSum20 = VSHUFF(sSum2,sSum0,c_4)              //[2]
      IF P3 VMEM(pOut1++#1) = sIntD                 //[3]
    }
    { sA1.h = VDMPY(sA1.ub,ch0x100.b)               //[1]
      sPreA = VMEM(pOut_1++#1):NT                   //[2]
      sIntgB_H = sPreIntg_H                         //[2]
      sIntgC_H = sPreIntg_H                         //[2]
    }
    { sA70_4 = VLALIGN(sSum3,sZero,c8)              //[1]
      sPreB = VMEM(pOut_1++#1):NT                   //[2]
      sIntgD_H = sPreIntg_H                         //[2]
      sIntgA_L.w = VADD(sPreIntg_L.w,sPreA.w)       //[2]
    }
    { sSum3.h = VADD(sSum3.h,sA70_4.h)              //[1]
      dSumBA = VSHUFF(sSum31_L,sSum20_L,c_4)        //[2]
      sPreC = VMEM(pOut_1++#1):NT                   //[2]
    }
    { dSumDC = VSHUFF(sSum31_H,sSum20_H,c_4)        //[2]
      sPreD = VMEM(pOut_1++#1):NT                   //[2]
      sIntgB_L.w = VADD(sPreIntg_L.w,sPreB.w)       //[2]
    }
    { sA150_4 = VLALIGN(sSum3,sZero,c16)            //[1]
      dIntgA.w += VMPA(dSumBA.h,ch0x1.b)            //[2]
      sIntgC_L.w = VADD(sPreIntg_L.w,sPreC.w)       //[2]
    }
    { sSum3.h = VADD(sSum3.h,sA150_4.h)             //[1]
      dIntgB.w += VMPA(dSumBA.h,ch0x100.b)          //[2]
      VMEM(pOut0++#1):NT = sIntgA_L                 //[2]
    }
    { dIntgC.w += VMPA(dSumDC.h,ch0x1.b)            //[2]
      sIntgD_L.w = VADD(sPreIntg_L.w,sPreD.w)       //[2]
      VMEM(pOut0++#1):NT = sIntgB_L                 //[2]
    }
    { sA310_4 = VLALIGN(sSum3,sZero,c32)            //[1]
      dIntgD.w += VMPA(dSumDC.h,ch0x100.b)          //[2]
      VMEM(pOut0++#1):NT = sIntgC_L                 //[2]
    }
    { sSum3.h = VADD(sSum3.h,sA310_4.h)             //[1]
      sIntA.w = VADD(sIntgA_H.w,sIntgA_L.w)         //[2]
      sIntB.w = VADD(sIntgB_H.w,sIntgB_L.w)         //[2]
      VMEM(pOut0++#1):NT = sIntgD_L                 //[2]
    }
    { dPreIntg.w += VMPA(sPreIntg:2.h,ch0x100.b)    //[2]
      sIntC.w = VADD(sIntgC_H.w,sIntgC_L.w)         //[2]
      VMEM(pOut1++#1) = sIntA                       //[2]
    }:endloop0
.integratePerRowAcc_LoopEND:
    { sA630_4 = VLALIGN(sSum3,sZero,c64)            //[2]
      sIntD.w = VADD(sIntgD_H.w,sIntgD_L.w)         //[3]
    }
    { sSum3t.h = VADD(sSum3.h,sA630_4.h)            //[2]
    }
    { sSum1.h = VSUB(sSum3t.h,sL01_H.h)             //[2]
      sSum2.h = VSUB(sSum3t.h,sA3t.h)               //[2]
    }
    { sSum0.h = VSUB(sSum1.h,sA1.h)                 //[2]
      sPreIntg = VRDELTA(sSum3t,sPattern)           //[2]
    }
    { dSum31 = VSHUFF(sSum3t,sSum1,c_4)             //[2]
      IF P3 VMEM(pOut1++#1) = sIntB                 //[3]
    }
    { sIntgA_H = sPreIntg_H                         //[2]
      IF P3 VMEM(pOut1++#1) = sIntC                 //[3]
    }
    { dSum20 = VSHUFF(sSum2,sSum0,c_4)              //[2]
      IF P3 VMEM(pOut1++#1) = sIntD                 //[3]
    }
    { sPreA = VMEM(pOut_1++#1)                      //[2]
      sIntgB_H = sPreIntg_H                         //[2]
      sIntgC_H = sPreIntg_H                         //[2]
    }
    { sPreB = VMEM(pOut_1++#1)                      //[2]
      sIntgD_H = sPreIntg_H                         //[2]
      sIntgA_L.w = VADD(sPreIntg_L.w,sPreA.w)       //[2]
    }
    { dSumBA = VSHUFF(sSum31_L,sSum20_L,c_4)        //[2]
      sPreC = VMEM(pOut_1++#1)                      //[2]
    }
    { dSumDC = VSHUFF(sSum31_H,sSum20_H,c_4)        //[2]
      sPreD = VMEM(pOut_1++#1)                      //[2]
      sIntgB_L.w = VADD(sPreIntg_L.w,sPreB.w)       //[2]
    }
    { dIntgA.w += VMPA(dSumBA.h,ch0x1.b)            //[2]
      sIntgC_L.w = VADD(sPreIntg_L.w,sPreC.w)       //[2]
    }
    { dIntgB.w += VMPA(dSumBA.h,ch0x100.b)          //[2]
      VMEM(pOut0++#1) = sIntgA_L                    //[2]
    }
    { dIntgC.w += VMPA(dSumDC.h,ch0x1.b)            //[2]
      sIntgD_L.w = VADD(sPreIntg_L.w,sPreD.w)       //[2]
      VMEM(pOut0++#1) = sIntgB_L                    //[2]
    }
    { dIntgD.w += VMPA(dSumDC.h,ch0x100.b)          //[2]
      VMEM(pOut0++#1) = sIntgC_L                    //[2]
    }
    { sIntA.w = VADD(sIntgA_H.w,sIntgA_L.w)         //[2]
      sIntB.w = VADD(sIntgB_H.w,sIntgB_L.w)         //[2]
      VMEM(pOut0++#1) = sIntgD_L                    //[2]
    }
    { dPreIntg.w += VMPA(sPreIntg:2.h,ch0x100.b)    //[2]
      sIntC.w = VADD(sIntgC_H.w,sIntgC_L.w)         //[2]
      VMEM(pOut1++#1) = sIntA                       //[2]
      R7 = #4                                       //
    }
    { VMEM(pOut1++#1) = sIntB                       //[3]
      sIntD.w = VADD(sIntgD_H.w,sIntgD_L.w)         //[3]
      dPreIntg = VSHUFF(sPreIntg_H, sPreIntg_L, R7);//
    }
    { VMEM(pOut1++#1) = sIntC                       //[3]
    }
    { VMEM(pOut1++#1) = sIntD                       //[3]
    }
    { IF P0 VMEM(R28+#0) = sPreIntg_H               //
    }
    { JUMPR R31                                     // return
    }
    .size    IntegrateRowAcc, .-IntegrateRowAcc

/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl IntegrateRow
    .type    IntegrateRow, @function
IntegrateRow:
    { const1 = ##0x01010101                         //
      R10 = ADD(PC,##SPLATpattern@PCREL)            //
    }
    { sPixels0.cur = VMEM(pIn0++#1):NT              //[1]
      sL0.h = VDMPY(sPixels0.ub,const1.b)           //[1]
      pIn1 = ADD(pIn0,R2)                           //
      R13:12 = BITSPLIT(R1,#24)                     //
    }
    { sPixels1.cur = VMEM(pIn1++#1):NT              //[1]
      sL1.h = VDMPY(sPixels1.ub,const1.b)           //[1]
      sA3.h = VSHUFFO(sPixels1.h,sPixels0.h)        //[1]
      P0 = !CMP.EQ(R13,#0)                          //
    }
    { dL01.h = VSHUFFOE(sL1.h,sL0.h)                //[1]
      ch0x100 = ##0x1000100                         //
      IF P0 JUMP .skipPreInt                        //
    }
    { sCachePreInt = VMEM(R5)                       //
    }
    .falign
.skipPreInt:
    { sSum3.h = VADD(sL01_H.h,sL01_L.h)             //[1]
      sA3t.h = VDMPY(sA3.ub,ch0x100.b)              //[1]
      sZero = #0                                    //
    }
    { sA1.h = VSHUFFE(sPixels1.h,sPixels0.h)        //[1]
      pOut_1 = SUB(pOut0,R4)                        //
      pOut1 = ADDASL(pOut0,R4,#2)                   //
      sPattern = VMEM(R10+#0)                       //
    }
    { sA30_4 = VLALIGN(sSum3,sZero,#4)              //[1]
      pOut_1 -= MPYI(R4,#3)                         //
    }
    { sSum3.h = VADD(sSum3.h,sA30_4.h)              //[1]
      sCachePreInt_4 = VLALIGN(sCachePreInt,sCachePreInt,#4);//
      R12 = ADD(R12,#VLEN-1-VLEN)                   //
    }
    { sA1.h = VDMPY(sA1.ub,ch0x100.b)               //[1]
      c8 = #8                                       //
      sPreIntg_H = vrdelta(sCachePreInt,sPattern)   //
    }
    { sA70_4 = VLALIGN(sSum3,sZero,c8)              //[1]
      R12 = ASR(R12,#LOG2VLEN)                      //
    }
    { sSum3.h = VADD(sSum3.h,sA70_4.h)              //[1]
      c16 = #16                                     //
      R28 = R5                                      //
      sPreIntg_L = vrdelta(sCachePreInt_4,sPattern) //
    }
    { sA150_4 = VLALIGN(sSum3,sZero,c16)            //[1]
      P3 = SP1LOOP0(.integratePerRow_Loop,R12)      //
      P1 = CMP.GT(R12,#0)                           //
    }
    { sSum3.h = VADD(sSum3.h,sA150_4.h)             //[1]
      c32 = #32                                     //
      IF P0 sPreIntg_H = sZero                      //
      IF P0 sPreIntg_L = sZero                      //
    }
    { sA310_4 = VLALIGN(sSum3,sZero,c32)            //[1]
      ch0x1 = ##0x00010001                          //
    }
    { sSum3.h = VADD(sSum3.h,sA310_4.h)             //[1]
      c64 = #64                                     //
      c_4 = #-4                                     //
      IF !P1 JUMP .integratePerRow_LoopEND          //
    }
    .falign
.integratePerRow_Loop:
    { sPixels0.cur = VMEM(pIn0++#1):NT              //[1]
      sL0.h = VDMPY(sPixels0.ub,const1.b)           //[1]
      sA630_4 = VLALIGN(sSum3,sZero,c64)            //[2]
      sIntD.w = VADD(sIntgD_H.w,sIntgD_L.w)         //[3]
    }
    { sPixels1.cur = VMEM(pIn1++#1):NT              //[1]
      sL1.h = VDMPY(sPixels1.ub,const1.b)           //[1]
      sA3.h = VSHUFFO(sPixels1.h,sPixels0.h)        //[1]
      sSum3t.h = VADD(sSum3.h,sA630_4.h)            //[2]
    }
    { dL01.h = VSHUFFOE(sL1.h,sL0.h)                //[1]
      sSum1.h = VSUB(sSum3t.h,sL01_H.h)             //[2]
      sSum2.h = VSUB(sSum3t.h,sA3t.h)               //[2]
    }
    { sSum3.h = VADD(sL01_H.h,sL01_L.h)             //[1]
      sA3t.h = VDMPY(sA3.ub,ch0x100.b)              //[1]
      sSum0.h = VSUB(sSum1.h,sA1.h)                 //[2]
      sPreIntg = VRDELTA(sSum3t,sPattern)           //[2]
    }
    { sA1.h = VSHUFFE(sPixels1.h,sPixels0.h)        //[1]
      dSum31 = VSHUFF(sSum3t,sSum1,c_4)             //[2]
      IF P3 VMEM(pOut1++#1) = sIntB                 //[3]
    }
    { sA30_4 = VLALIGN(sSum3,sZero,#4)              //[1]
      sIntgA_H = sPreIntg_H                         //[2]
      IF P3 VMEM(pOut1++#1) = sIntC                 //[3]
    }
    { sSum3.h = VADD(sSum3.h,sA30_4.h)              //[1]
      dSum20 = VSHUFF(sSum2,sSum0,c_4)              //[2]
      IF P3 VMEM(pOut1++#1) = sIntD                 //[3]
    }
    { sA1.h = VDMPY(sA1.ub,ch0x100.b)               //[1]
      sPreA = sZero                                 //[2]
      sIntgB_H = sPreIntg_H                         //[2]
      sIntgC_H = sPreIntg_H                         //[2]
    }
    { sA70_4 = VLALIGN(sSum3,sZero,c8)              //[1]
      sPreB = sZero                                 //[2]
      sIntgD_H = sPreIntg_H                         //[2]
      sIntgA_L.w = VADD(sPreIntg_L.w,sPreA.w)       //[2]
    }
    { sSum3.h = VADD(sSum3.h,sA70_4.h)              //[1]
      dSumBA = VSHUFF(sSum31_L,sSum20_L,c_4)        //[2]
      sPreC = sZero                                 //[2]
    }
    { dSumDC = VSHUFF(sSum31_H,sSum20_H,c_4)        //[2]
      sPreD = sZero                                 //[2]
      sIntgB_L.w = VADD(sPreIntg_L.w,sPreB.w)       //[2]
    }
    { sA150_4 = VLALIGN(sSum3,sZero,c16)            //[1]
      dIntgA.w += VMPA(dSumBA.h,ch0x1.b)            //[2]
      sIntgC_L.w = VADD(sPreIntg_L.w,sPreC.w)       //[2]
    }
    { sSum3.h = VADD(sSum3.h,sA150_4.h)             //[1]
      dIntgB.w += VMPA(dSumBA.h,ch0x100.b)          //[2]
      VMEM(pOut0++#1):NT = sIntgA_L                 //[2]
    }
    { dIntgC.w += VMPA(dSumDC.h,ch0x1.b)            //[2]
      sIntgD_L.w = VADD(sPreIntg_L.w,sPreD.w)       //[2]
      VMEM(pOut0++#1):NT = sIntgB_L                 //[2]
    }
    { sA310_4 = VLALIGN(sSum3,sZero,c32)            //[1]
      dIntgD.w += VMPA(dSumDC.h,ch0x100.b)          //[2]
      VMEM(pOut0++#1):NT = sIntgC_L                 //[2]
    }
    { sSum3.h = VADD(sSum3.h,sA310_4.h)             //[1]
      sIntA.w = VADD(sIntgA_H.w,sIntgA_L.w)         //[2]
      sIntB.w = VADD(sIntgB_H.w,sIntgB_L.w)         //[2]
      VMEM(pOut0++#1):NT = sIntgD_L                 //[2]
    }
    { dPreIntg.w += VMPA(sPreIntg:2.h,ch0x100.b)    //[2]
      sIntC.w = VADD(sIntgC_H.w,sIntgC_L.w)         //[2]
      VMEM(pOut1++#1) = sIntA                       //[2]
    }:endloop0
.integratePerRow_LoopEND:
    { sA630_4 = VLALIGN(sSum3,sZero,c64)            //[2]
      sIntD.w = VADD(sIntgD_H.w,sIntgD_L.w)         //[3]
    }
    { sSum3t.h = VADD(sSum3.h,sA630_4.h)            //[2]
    }
    { sSum1.h = VSUB(sSum3t.h,sL01_H.h)             //[2]
      sSum2.h = VSUB(sSum3t.h,sA3t.h)               //[2]
    }
    { sSum0.h = VSUB(sSum1.h,sA1.h)                 //[2]
      sPreIntg = VRDELTA(sSum3t,sPattern)           //[2]
    }
    { dSum31 = VSHUFF(sSum3t,sSum1,c_4)             //[2]
      IF P3 VMEM(pOut1++#1) = sIntB                 //[3]
    }
    { sIntgA_H = sPreIntg_H                         //[2]
      IF P3 VMEM(pOut1++#1) = sIntC                 //[3]
    }
    { dSum20 = VSHUFF(sSum2,sSum0,c_4)              //[2]
      IF P3 VMEM(pOut1++#1) = sIntD                 //[3]
    }
    { sPreA = sZero                                 //[2]
      sIntgB_H = sPreIntg_H                         //[2]
      sIntgC_H = sPreIntg_H                         //[2]
    }
    { sPreB = sZero                                 //[2]
      sIntgD_H = sPreIntg_H                         //[2]
      sIntgA_L.w = VADD(sPreIntg_L.w,sPreA.w)       //[2]
    }
    { dSumBA = VSHUFF(sSum31_L,sSum20_L,c_4)        //[2]
      sPreC = sZero                                 //[2]
    }
    { dSumDC = VSHUFF(sSum31_H,sSum20_H,c_4)        //[2]
      sPreD = sZero                                 //[2]
      sIntgB_L.w = VADD(sPreIntg_L.w,sPreB.w)       //[2]
    }
    { dIntgA.w += VMPA(dSumBA.h,ch0x1.b)            //[2]
      sIntgC_L.w = VADD(sPreIntg_L.w,sPreC.w)       //[2]
    }
    { dIntgB.w += VMPA(dSumBA.h,ch0x100.b)          //[2]
      VMEM(pOut0++#1) = sIntgA_L                    //[2]
    }
    { dIntgC.w += VMPA(dSumDC.h,ch0x1.b)            //[2]
      sIntgD_L.w = VADD(sPreIntg_L.w,sPreD.w)       //[2]
      VMEM(pOut0++#1) = sIntgB_L                    //[2]
    }
    { dIntgD.w += VMPA(dSumDC.h,ch0x100.b)          //[2]
      VMEM(pOut0++#1) = sIntgC_L                    //[2]
    }
    { sIntA.w = VADD(sIntgA_H.w,sIntgA_L.w)         //[2]
      sIntB.w = VADD(sIntgB_H.w,sIntgB_L.w)         //[2]
      VMEM(pOut0++#1) = sIntgD_L                    //[2]
    }
    { dPreIntg.w += VMPA(sPreIntg:2.h,ch0x100.b)    //[2]
      sIntC.w = VADD(sIntgC_H.w,sIntgC_L.w)         //[2]
      VMEM(pOut1++#1) = sIntA                       //[2]
      R7 = #4                                       //
    }
    { VMEM(pOut1++#1) = sIntB                       //[3]
      sIntD.w = VADD(sIntgD_H.w,sIntgD_L.w)         //[3]
      dPreIntg = VSHUFF(sPreIntg_H, sPreIntg_L, R7);//
    }
    { VMEM(pOut1++#1) = sIntC                       //[3]
    }
    { VMEM(pOut1++#1) = sIntD                       //[3]
    }
    { IF P0 VMEM(R28+#0) = sPreIntg_H               //
    }
    { JUMPR R31                                     // return
    }
    .size    IntegrateRow, .-IntegrateRow

    .section .data
    .p2align LOG2VLEN
SPLATpattern:
#if LOG2VLEN == 7
    .word 0x40404040
    .word 0x40404040
    .word 0x40404040
    .word 0x40404040
    .word 0x40404040
    .word 0x40404040
    .word 0x40404040
    .word 0x40404040
    .word 0x40404040
    .word 0x40404040
    .word 0x40404040
    .word 0x40404040
    .word 0x40404040
    .word 0x40404040
    .word 0x40404040
    .word 0x40404040
    .word 0x20202020
    .word 0x20202020
    .word 0x20202020
    .word 0x20202020
    .word 0x20202020
    .word 0x20202020
    .word 0x20202020
    .word 0x20202020
    .word 0x10101010
    .word 0x10101010
    .word 0x10101010
    .word 0x10101010
    .word 0x08080808
    .word 0x08080808
    .word 0x04040404
    .word 0x00000000
#elif LOG2VLEN == 6
    .word 0x20202020
    .word 0x20202020
    .word 0x20202020
    .word 0x20202020
    .word 0x20202020
    .word 0x20202020
    .word 0x20202020
    .word 0x20202020
    .word 0x10101010
    .word 0x10101010
    .word 0x10101010
    .word 0x10101010
    .word 0x08080808
    .word 0x08080808
    .word 0x04040404
    .word 0x00000000
#else
#error not support
#endif
