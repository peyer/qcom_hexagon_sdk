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
    .file    "sigma9x9.S"

#include "hvx.cfg.h"
    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void sigma9x9PerRow()                                         ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: 9x9 sigma filtering on a row                                  ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned char *src       -- pointer to input image       ]*/
    /*[               R1 : int stride               -- stride of image input        ]*/
    /*[               R2 : int width                -- image width                  ]*/
    /*[               R3 : unsigned char  threshold -- threshold                    ]*/
    /*[               R4 : unsigned char *dst       -- pointer to output buffer     ]*/
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
    /*[   6.0.0         10/21/2014              created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define iptr                R0
#define stride              R1
#define width               R2
#define threshold           R3
#define optr                R4
#define gidx                R5
#define const4              R6
#define const5              R7
#define rConst              R8
#define iptrk               R9
#define pCentr              R10
#define pCur                R0
#define pNxt                R1
#define pPrv                R2
#define rOneu8              R15
/* ============================================================ */
#define sCentr              V0
#define sXprev              V1
#define sXcurr              V2
#define sXnext              V3
#define sThres              V4
#define sCount              V5
#define sSum_L              V6
#define sSum_H              V7
#define dSum                V7:6
#define sX_1                V8
#define sX1                 V9
#define sX_2                V10
#define sX2                 V11
#define sAd0                V12
#define sAd1                V13
#define sXt0                V14
#define sXt1                V15
#define dXt01               V15:14
#define sZero               V16
#define sOne                V17
#define sIncrement          V18
#define sInv0               V20
#define sInv1               V21
#define dInv                V21:20
#define sSum0_t             V22
#define sSum1_t             V23
#define dSum01_t            V23:22
#define sInvTab0            V24
#define sInvTab2            V26
#if LOG2VLEN==6
#define sInvTab1            V25
#else
#define sInvTab1            sInvTab0
#endif
#define sX_3                sX_1
#define sX3                 sX1
#define sX_4                sX_2
#define sX4                 sX2
#define sAd2                sAd0
#define sAd3                sAd1
#define sXt2                sXt0
#define sXt3                sXt1
#define dXt23               dXt01
#define sOut0               sCentr
#define sOut1               sXcurr
#define sOut                sOut0
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl sigma9x9PerRow
    .type    sigma9x9PerRow, @function
sigma9x9PerRow:
    { M0 = stride                                   //
      pCentr = iptr                                 //
      pCur -= ASL(stride,#2)                        //
      R28 = ADD(width,#(VLEN-1))                    //
    }
    { R28 = LSR(R28,#LOG2VLEN)                        // ceil(width/VLEN)
      pNxt = ADD(pCur,#VLEN)                        //
      threshold = VSPLATB(threshold)                //
      sZero = #0                                     //
    }
    { R15 = ##0x01010101                            //
      sThres = VSPLAT(threshold)                    //
      LOOP1(.sigma9x9PerRowLOOP,R28)                // set loop with lc0=ceil(width/VLEN)
    }
    { sOne = VSPLAT(R15)                            //
      R14 = ##invLUT                                //
      iptrk = pNxt                                  //
    }
    { V0.tmp = VMEM(R14+#1)                         //
#if LOG2VLEN==6
      sInvTab1.h = VSHUFF(v0.h)                     //
    }
    { V0.tmp = VMEM(R14+#2)                         //
#endif
      sInvTab2.h = VSHUFF(v0.h)                     //
      P3 = CMP.EQ(R0,R0)                            // set P3
      sIncrement = #0                               //
    }
    { V0.tmp = VMEM(R14+#0)                         //
      sInvTab0.h = VSHUFF(v0.h)                     //
      rConst = #0                                   //
      pPrv = ADD(pCur,#-VLEN)                       //
    }
    { const5 = #5                                   //
      const4 = #4                                   //
    }

    .falign
.sigma9x9PerRowLOOP:
    { dInv.h|= VLUT16(sCount.b,sInvTab1.h,gidx)     //[O2]
      sSum_L = #0                                   //
      sSum_H = #0                                   //
      R28 = ADD(R28,#-1)                            //
    }
    { dInv.h|= VLUT16(sCount.b,sInvTab2.h,const4)   //[O2]
      sCentr = VMEM(pCentr++#1)                     //
      IF P3 pPrv = pCur                             //
      P2 = CMP.EQ(R28,#0)                           //
    }
    { dInv.h|= VLUT16(sCount.b,sInvTab2.h,const5)   //[O2]
      sCount = #0                                   //
      LOOP0(.sigma9x9PerRow_innerLOOP,#9)           //
      IF P2 pNxt = pCur                             //
    }

    .falign
.sigma9x9PerRow_innerLOOP:
    { sXprev = VMEM(pPrv++M0)                       //[1]
      IF (!Q1) sCount.b += sIncrement.b             //[2]
      Q0 = VCMP.GT(sAd2.ub,sThres.ub)               //[2]
      Q1 = VCMP.GT(sAd3.ub,sThres.ub)               //[2]
    }
    { sXcurr = VMEM(pCur++M0)                       //[1]
      dSum.h += VMPA(dXt01.ub,rConst.b)             //[2]
      IF (!Q0) sCount.b += sIncrement.b             //[2]
    }
    { sXnext = VMEM(pNxt++M0)                       //[1]
      sXt2 = VMUX(Q0,sZero,sX_4)                    //[2]
      sXt3 = VMUX(Q1,sZero,sX4)                     //[2]
    }
    { sX_1 = VLALIGN(sXcurr,sXprev,#1)              //[1]
      sAd3.ub = VABSDIFF(sXcurr.ub,sCentr.ub)       //[1]
      sIncrement = sOne                             //[1]
      if (!Q1) sCount.b += sIncrement.b             //[2]
    }
    { sX1 = VALIGN(sXnext,sXcurr,#1)                //[1]
      Q1 = VCMP.GT(sAd3.ub,sThres.ub)               //[1]
      rConst = #-1                                  //[1]
      dSum.h += VMPA(dXt23.ub,rConst.b)             //[2]
    }
    { sX_2 = VLALIGN(sXcurr,sXprev,#2)              //[1]
      sAd0.ub = VABSDIFF(sX_1.ub,sCentr.ub)         //[1]
      IF (!Q1) sCount.b += sIncrement.b             //[1]
      sXt3 = VMUX(Q1,sZero,sXcurr)                  //[1]
    }
    { sX2 = VALIGN(sXnext,sXcurr,#2)                //[1]
      sAd1.ub = VABSDIFF(sX1.ub,sCentr.ub)          //[1]
      Q0 = VCMP.GT(sAd0.ub,sThres.ub)               //[1]
    }
    { Q1 = VCMP.GT(sAd1.ub,sThres.ub)               //[1]
      sXt0 = VMUX(Q0,sZero,sX_1)                    //[1]
      dSum.h += VMPY(sXt3.ub,rConst.b)              //[1]
    }
    { sAd2.ub = VABSDIFF(sX_2.ub,sCentr.ub)         //[1]
      sAd3.ub = VABSDIFF(sX2.ub,sCentr.ub)          //[1]
      IF (!Q0) sCount.b += sIncrement.b             //[1]
      sXt1 = VMUX(Q1,sZero,sX1)                     //[1]
    }
    { sX_3 = VLALIGN(sXcurr,sXprev,#3)              //[1]
      IF (!Q1) sCount.b += sIncrement.b             //[1]
      Q0 = VCMP.GT(sAd2.ub,sThres.ub)               //[1]
      Q1 = VCMP.GT(sAd3.ub,sThres.ub)               //[1]
    }
    { sX3 = VALIGN(sXnext,sXcurr,#3)                //[1]
      dSum.h += VMPA(dXt01.ub,rConst.b)             //[1]
      sXt2 = VMUX(Q0,sZero,sX_2)                    //[1]
    }
    { sX_4 = VLALIGN(sXcurr,sXprev,#4)              //[1]
      sAd0.ub = VABSDIFF(sX_3.ub,sCentr.ub)         //[1]
      sXt3 = VMUX(Q1,sZero,sX2)                     //[1]
      IF (!Q0) sCount.b += sIncrement.b             //[1]
    }
    { sX4 = VALIGN(sXnext,sXcurr,#4)                //[1]
      sAd1.ub = VABSDIFF(sX3.ub,sCentr.ub)          //[1]
      Q0 = VCMP.GT(sAd0.ub,sThres.ub)               //[1]
      if (!Q1) sCount.b += sIncrement.b             //[1]
    }
    { Q1 = VCMP.GT(sAd1.ub,sThres.ub)               //[1]
      sXt0 = VMUX(Q0,sZero,sX_3)                    //[1]
      dSum.h += VMPA(dXt23.ub,rConst.b)             //[1]
    }
    { sAd2.ub = VABSDIFF(sX_4.ub,sCentr.ub)         //[1]
      sAd3.ub = VABSDIFF(sX4.ub,sCentr.ub)          //[1]
      IF (!Q0) sCount.b += sIncrement.b             //[1]
      sXt1 = VMUX(Q1,sZero,sX3)                     //[1]
    }:endloop0

    { IF (!Q1) sCount.b += sIncrement.b             //[e]
      Q0 = VCMP.GT(sAd2.ub,sThres.ub)               //[e]
      sOut0.h = VMPY(sSum0_t.h,sInv0.h):<<1:rnd:sat //[O2]
      pCur = iptrk                                  //
    }
    { IF (!Q0) sCount.b += sIncrement.b             //[e]
      Q1 = VCMP.GT(sAd3.ub,sThres.ub)               //[e]
      sOut1.h = VMPY(sSum1_t.h,sInv1.h):<<1:rnd:sat //[O2]
      pNxt = ADD(iptrk,#VLEN)                       //
    }
    { IF (!Q1) sCount.b += sIncrement.b             //[e]
      dSum.h += VMPA(dXt01.ub,rConst.b)             //[e]
      pPrv = ADD(iptrk,#-VLEN)                      //
      iptrk = pNxt                                  //
    }
    { sXt2 = VMUX(Q0,sZero,sX_4)                    //[e]
      sXt3 = VMUX(Q1,sZero,sX4)                     //[e]
      dSum01_t = dSum                               //
      gidx = #0                                     //
    }
    { dInv.h = VLUT16(sCount.b,sInvTab0.h,gidx)     //
      gidx = #1                                     //
      sOut.b = VSHUFFE(sOut1.b,sOut0.b)             //[O2]
      IF !P3 VMEM(R4++#1) = sOut.new                //[O2]
    }
    { dSum01_t.h += VMPA(dXt23.ub,rConst.b)         //[e]
      dInv.h|= VLUT16(sCount.b,sInvTab0.h,gidx)     //
      gidx = #2                                     //
      P3 = XOR(P3,P3)                               // clean P3
    }
    { dInv.h|= VLUT16(sCount.b,sInvTab1.h,gidx)     //
      sIncrement = #0                               //
      rConst = #0                                   //
      gidx = #3                                     //
    }:endloop1

    //====== epilogue ======
    { dInv.h|= VLUT16(sCount.b,sInvTab1.h,gidx)     //[O2]
    }
    { dInv.h|= VLUT16(sCount.b,sInvTab2.h,const4)   //[O2]
    }
    { dInv.h|= VLUT16(sCount.b,sInvTab2.h,const5)   //[O2]
    }
    { sOut0.h = VMPY(sSum0_t.h,sInv0.h):<<1:rnd:sat //[O2]
    }
    { sOut1.h = VMPY(sSum1_t.h,sInv1.h):<<1:rnd:sat //[O2]
    }
    { sOut.b = VSHUFFE(sOut1.b,sOut0.b)             //[O2]
      VMEM(R4+#0) = sOut.new                        //[O2]
    }
    { JUMPR R31                                     // return
    }
    .size    sigma9x9PerRow, .-sigma9x9PerRow

