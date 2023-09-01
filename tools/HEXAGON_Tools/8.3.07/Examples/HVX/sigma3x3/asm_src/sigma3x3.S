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
    .file    "sigma3x3.S"

#include "hvx.cfg.h"
    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void sigma3x3PerRow()                                         ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: 3x3 sigma filtering on a row                                  ]*/
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
    /*[   6.0.0         08/01/2014              created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define c0                  R5
#define minus1              R6
#define vsize               R7
/* ============================================================ */
#define sRow0prv            V0
#define sRow0cur            V1
#define sRow0nxt            V2
#define sRow1prv            V3
#define sRow1cur            V4
#define sRow1nxt            V5
#define sRow2prv            V6
#define sRow2cur            V7
#define sRow2nxt            V8
#define sCnt                V9
#define sSumE               V10
#define sSumO               V11
#define dSum                V11:10
#define sX01t               V12
#define sX21t               V13
#define dX21tX01t           V13:12
#define sX00t               V14
#define sX20t               V15
#define dX20tX00t           V15:14
#define sX02t               V16
#define sX22t               V17
#define dX22tX02t           V17:16
#define sX10t               V18
#define sX12t               V19
#define dX12tX10t           V19:18
#define sAbdif01            V20
#define sAbdif21            V21
#define sInvE               V22
#define sInvO               V23
#define dInv                V23:22
#define sThres              V24
#define Zero                V25
#define One                 V26
#define Two                 V27
#define C80                 V28
#define sInvTab             V28
#define sCentr              sRow1cur
#define sX01                sRow0cur
#define sX21                sRow2cur
#define sX00                sX00t
#define sX20                sX20t
#define sX02                sX02t
#define sX22                sX22t
#define sX10                sX10t
#define sX12                sX12t
#define sAbdif00            sAbdif01
#define sAbdif20            sAbdif21
#define sAbdif02            sAbdif01
#define sAbdif22            sAbdif21
#define sAbdif10            sAbdif01
#define sAbdif12            sAbdif21
#define sIdx                sCnt
#define sOutE               sInvE
#define sOutO               sInvO
#define sOut                sOutE
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl sigma3x3PerRow
    .type    sigma3x3PerRow, @function
sigma3x3PerRow:
    { R28 = ADD(R2,#VLEN-1)                         //
      R3 = VSPLATB(R3)                              // splat threshold
      R12 = ##0x01010101                            //
    }
    { R28 = LSR(R28,#LOG2VLEN)                      // ceil(width/VLEN)
      R1 = SUB(R0,R1)                               // set R1 = src - stride
      R2 = ADD(R0,R1)                               // set R2 = src + stride
      One = VSPLAT(R12)                             //
    }
    { P3 = SP1LOOP0(.sigma3x3PerRowLOOP,R28)        // setup loop0
      R14 = ##invLUT                                //
      sThres = VSPLAT(R3)                           //
    }
    { V0.tmp = VMEM(R14+#0)                         //
      sInvTab.h = VSHUFF(V0.h)                      //
      vsize = #VLEN                                 //
      P0 = CMP.GT(R28,#1)                           //
    }
    { minus1 = #-1                                  //
      c0 = #0                                       //
      sRow0cur = VMEM(R1+#0)                        //
      IF P0 R1 = ADD(R1,vsize)                      //
    }
    { sRow1cur = VMEM(R0+#0)                        //
      IF P0 R0 = ADD(R0,vsize)                      //
      Zero = #0                                     //
      R28 = ADD(R28,#-1)                            //
    }
    { sRow2cur = VMEM(R2+#0)                        //
      IF P0 R2 = ADD(R2,vsize)                      //
      Two.b = VADD(One.b,One.b)                     //
    }

    .falign
.sigma3x3PerRowLOOP:
    { sRow0nxt = VMEM(R1+#0)                        //[1]
      sX00 = VLALIGN(sRow0cur,sRow0prv,#1)          //[1]
      sX12t = VMUX(Q1,Zero,sX12)                    //[2]
      IF (!Q1) sCnt.b += One.b                      //[2]
    }
    { sRow2nxt = VMEM(R2+#0)                        //[1]
      sX20 = VLALIGN(sRow2cur,sRow2prv,#1)          //[1]
      sAbdif01.ub = VABSDIFF(sX01.ub,sCentr.ub)     //[1]
      sAbdif21.ub = VABSDIFF(sX21.ub,sCentr.ub)     //[1]
    }
    { P0 = CMP.GT(R28,#1)                           //[1]
      sX02 = VALIGN(sRow0nxt,sRow0cur,#1)           //[1]
      sRow0prv = sRow0cur                           //[1]
      dSum.h += VMPA(dX22tX02t.ub,minus1.b)         //[2]
    }
    { sRow1nxt = VMEM(R0+#0)                        //[1]
      IF P0 R1 = ADD(R1,vsize)                      //[1]
      sX22 = VALIGN(sRow2nxt,sRow2cur,#1)           //[1]
      dSum.h += VMPA(dX12tX10t.ub,minus1.b)         //[2]
    }
    { Q0 = VCMP.GT(sAbdif01.ub,sThres.ub)           //[1]
      Q1 = VCMP.GT(sAbdif21.ub,sThres.ub)           //[1]
      IF P2 R2 = ADD(R2,vsize)                      //[1]
      dInv.h = VLUT16(sIdx.b,sInvTab.h,c0)          //[2]
    }
    { sX01t = VMUX(Q0,Zero,sX01)                    //[1]
      sX21t = VMUX(Q1,Zero,sX21)                    //[1]
      sCnt  = VMUX(Q0,One,Two)                      //[1]
      sRow2prv = sRow2cur                           //[1]
    }
    { IF (!Q1) sCnt.b += One.b                      //[1]
      sAbdif00.ub = VABSDIFF(sX00.ub,sCentr.ub)     //[1]
      sAbdif20.ub = VABSDIFF(sX20.ub,sCentr.ub)     //[1]
      sRow0cur = sRow0nxt                           //[1]
    }
    { Q0 = VCMP.GT(sAbdif00.ub,sThres.ub)           //[1]
      Q1 = VCMP.GT(sAbdif20.ub,sThres.ub)           //[1]
      IF P2 R0 = ADD(R0,vsize)                      //[1]
      sOutE.h = VMPY(sSumE.h,sInvE.h):<<1:rnd:sat   //[2]
    }
    { sX10 = VLALIGN(sRow1cur,sRow1prv,#1)          //[1]
      sX00t = VMUX(Q0,Zero,sX00)                    //[1]
      sX20t = VMUX(Q1,Zero,sX20)                    //[1]
      IF (!Q0) sCnt.b += One.b                      //[1]
    }
    { sX12 = VALIGN(sRow1nxt,sRow1cur,#1)           //[1]
      sAbdif02.ub = VABSDIFF(sX02.ub,sCentr.ub)     //[1]
      sAbdif22.ub = VABSDIFF(sX22.ub,sCentr.ub)     //[1]
      IF (!Q1) sCnt.b += One.b                      //[1]
    }
    { Q0 = VCMP.GT(sAbdif02.ub,sThres.ub)           //[1]
      Q1 = VCMP.GT(sAbdif22.ub,sThres.ub)           //[1]
      sOutO.h = VMPY(sSumO.h,sInvO.h):<<1:rnd:sat   //[2]
    }
    { sX02t = VMUX(Q0,Zero,sX02)                    //[1]
      IF (!Q0) sCnt.b += One.b                      //[1]
      dSum.h = VSUB(Zero.ub,sCentr.ub)              //[1]
    }
    { sX22t = VMUX(Q1,Zero,sX22)                    //[1]
      IF (!Q1) sCnt.b += One.b                      //[1]
      sAbdif10.ub = VABSDIFF(sX10.ub,sCentr.ub)     //[1]
      sAbdif12.ub = VABSDIFF(sX12.ub,sCentr.ub)     //[1]
    }
    { Q0 = VCMP.GT(sAbdif10.ub,sThres.ub)           //[1]
      Q1 = VCMP.GT(sAbdif12.ub,sThres.ub)           //[1]
      dSum.h += VMPA(dX21tX01t.ub,minus1.b)         //[1]
      R28 = ADD(R28,#-1)                            //[1]
    }
    { dSum.h += VMPA(dX20tX00t.ub,minus1.b)         //[1]
      sRow1prv = sRow1cur                           //[1]
      sOut.ub = VSAT(sOutO.h,sOutE.h)               //[2]
      IF P3 VMEM(R4++#1) = sOut.new                 //[2]
    }
    { sX10t = VMUX(Q0,Zero,sX10)                    //[1]
      IF (!Q0) sCnt.b += One.b                      //[1]
      sRow2cur = sRow2nxt                           //[1]
      sRow1cur = sRow1nxt                           //[1]
    }:endloop0

    //====== epilogue ======
    { dSum.h += VMPA(dX22tX02t.ub,minus1.b)         //[2]
      sX12t = VMUX(Q1,Zero,sX12)                    //[2]
      IF (!Q1) sCnt.b += One.b                      //[2]
    }
    { dSum.h += VMPA(dX12tX10t.ub,minus1.b)         //[2]
    }
    { dInv.h = VLUT16(sIdx.b,sInvTab.h,c0)          //[2]
    }
    { sOutE.h = VMPY(sSumE.h,sInvE.h):<<1:rnd:sat   //[2]
    }
    { sOutO.h = VMPY(sSumO.h,sInvO.h):<<1:rnd:sat   //[2]
    }
    { sOut.ub = VSAT(sOutO.h,sOutE.h)               //[2]
      VMEM(R4+#0) = sOut.new                        //[2]
    }
    { JUMPR R31                                     // return
    }
    .size    sigma3x3PerRow, .-sigma3x3PerRow





