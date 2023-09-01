
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
    .file	"bilateral.S"

#include "hvx.cfg.h"
    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void bilateral9x9PerRow                                       ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: apply 9x9 bilateral filter to a image block                   ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned char *inp      -- pointer to input image        ]*/
    /*[               R1 : int            stride_i -- stride of image input         ]*/
    /*[               R2 : int            width    -- width of image block          ]*/
    /*[               R3 : unsigned char *gaussLUT -- pointer to gaussian table     ]*/
    /*[               R4 : unsigned char *rangeLUT -- pointer to range table        ]*/
    /*[               R5 : unsigned char *outp     -- pointer to output buffer      ]*/
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
    /*[   6.0.0         09/25/2014              created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define iptr0               R0
#define stride              R1
#define width               R2
#define GAUSSLUT            R3
#define RANGELUT            R4
#define optr                R5
#define ks                  R6
#define const6              R7
#define iptr_k              R8
#define iptrk               R9
#define w00                 R10
#define gptr                R11
#define const80             R12
#define const1              R13
#define Gaussk              R14
/* ============================================================ */
#define sPrv                V0
#define sCur                V1
#define sNxt                V2
#define sCentr              V3
#define sX_k                V4
#define sXk                 V5
#define sAbsDif0            V6
#define sAbsDif1            V7
#define dAbsDif             V7:6
#define sRange0             V8
#define sRange1             V9
#define sWeight0L           V10
#define sWeight0H           V11
#define dWeight0            V11:10
#define sWeight1L           V12
#define sWeight1H           V13
#define dWeight1            V13:12
#define sSumW0              V14
#define sSumW1              V15
#define dSumW               V15:14
#define sSumFe0             V16
#define sSumFe2             V17
#define dSumFe              V17:16
#define sSumFo1             V18
#define sSumFo3             V19
#define dSumFo              V19:18
#define sPixelxW1L          V20
#define sPixelxW1H          V21
#define dPixelxW1           V21:20
#define sRangeTab0          V22
#define sRangeTab2          V23
#if LOG2VLEN == 7
#define sRangeTab1          sRangeTab0
#define sRangeTab3          sRangeTab2
#else
#define sRangeTab1          V24
#define sRangeTab3          V25
#endif
#define sBit                V26
#define sX_k_t              sPixelxW1L
#define sXk_t               sPixelxW1H
#define sCentru16e          sAbsDif0
#define sCentru16o          sAbsDif1
#define dCentru16           dAbsDif
#define sPixelxW0L          sWeight1L
#define sPixelxW0H          sWeight1H
#define dPixelxW0           dWeight1
#define sWeight0            sWeight0L
#define sWeight1            sWeight0H
#define dWeight01           dWeight0
#define dSumTmpe            dWeight01
#define dSumTmpo            dWeight01
#define sDenomE0            sPixelxW0L
#define sDenomE2            sPixelxW0H
#define dDenomE             dPixelxW0
#define sDenomO1            sPixelxW1L
#define sDenomO3            sPixelxW1H
#define dDenomO             dPixelxW1 
#define sResultE0           sAbsDif0
#define sResultE2           sAbsDif1
#define sResultO1           sRange0
#define sResultO3           sRange1
#define output              sResultE0
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl bilateral9x9PerRow
    .type    bilateral9x9PerRow, @function
bilateral9x9PerRow:
    { R28 = ADD(width,#VLEN-1)                      //
      const6  = #6                                  //
      R10 = MEMUB(RANGELUT+#0)                      //
      R11 = MEMUB(GAUSSLUT+#4*9+4)                  //
    }
    { R28 = LSR(R28,#LOG2VLEN)                      //
      w00 = MPYI(R10,R11)                           //
      R12 = ##0x01010080                            //
    }
#if LOG2VLEN == 7
    { V0.tmp = VMEM(RANGELUT+#0)                    //
      sRangeTab0.b = VSHUFF(V0.b)                   //
      w00 = ASR(w00,#8)                             //
    }
    { V0.tmp = VMEM(RANGELUT+#1)                    //
      sRangeTab2.b = VSHUFF(V0.b)                   //
      w00 = COMBINE(w00.L,w00.L)                    //
      R13:12 = PACKHL(R12,R12)                      // set const1, const80
    }
#else
    { V0.tmp = VMEM(RANGELUT+#0)                    //
      sRangeTab0.b = VSHUFF(V0.b)                   //
      w00 = ASR(w00,#8)                             //
    }
    { V0.tmp = VMEM(RANGELUT+#1)                    //
      sRangeTab1.b = VSHUFF(V0.b)                   //
      w00 = COMBINE(w00.L,w00.L)                    //
      R13:12 = PACKHL(R12,R12)                      // set const1, const80
    }
    { V0.tmp = VMEM(RANGELUT+#2)                    //
      sRangeTab2.b = VSHUFF(V0.b)                   //
    }
    { V0.tmp = VMEM(RANGELUT+#3)                    //
      sRangeTab3.b = VSHUFF(V0.b)                   //
    }
#endif

    .falign
.bilateral9x9PerRow_LOOP:
    { iptrk = iptr0                                 //
      sCentr.cur = VMEM(iptr0++#1)                  //
      dCentru16.uh = VZXT(sCentr.ub)                //
      LOOP1(.bilateral_9x8LOOP,#9)                  //
    }
    { R28 = ADD(R28,#-1)                            //
      gptr = ADD(GAUSSLUT,#-5)                      //
      sSumW0 = VSPLAT(w00)                          //
      sSumW1 = VSPLAT(w00)                          //
    }
    { iptrk -= ASL(stride,#2)                       //
      dSumFe.w = VMPY(sCentru16e.h,w00.h)           //
      Gaussk = #0
    }
    { iptr_k = iptrk                                //
      dSumFo.w = VMPY(sCentru16o.h,w00.h)           //
    }

    .falign
.bilateral_9x8LOOP:
    { sPrv = VMEM(iptrk+#-1)                        //
    }
    { sCur = VMEM(iptrk+#0)                         //
      ks   = #4                                     //
      gptr = ADD(gptr,#5)                           //
    }
    { sNxt = VMEM(iptrk+#1)                         //
      iptrk = ADD(iptrk,stride)                     //
      LOOP0(.bilateral_1x8LOOP,#4)                  //
    }

    .falign
.bilateral_1x8LOOP:
    { sRange0.b|=VLUT32(sAbsDif0.b,sRangeTab3.b,R4) //[2]
    }
    { sRange1.b|=VLUT32(sAbsDif1.b,sRangeTab3.b,R4) //[2]
      R4 = #7                                       //[2]
    }
    { sX_k = VLALIGN(sCur,sPrv,ks)                  //[1]
      sX_k_t = sX_k                                 //[2]
    }
    { sXk  = VALIGN( sNxt,sCur,ks)                  //[1]
      sXk_t = sXk                                   //[2]
    }
    { sAbsDif0.ub = VABSDIFF(sX_k.ub,sCentr.ub)     //[1]
      sRange0.b|=VLUT32(sAbsDif0.b,sRangeTab3.b,R4) //[2]
    }
    { sAbsDif1.ub = VABSDIFF(sXk.ub ,sCentr.ub)     //[1]
      R4 = #0                                       //[1]
      sRange1.b|=VLUT32(sAbsDif1.b,sRangeTab3.b,R4) //[2]
    }
    { sRange0.b =VLUT32(sAbsDif0.b,sRangeTab0.b,R4) //[1]
      dWeight0.uh = VMPY(sRange0.ub,Gaussk.ub)      //[2]
    }
    { sRange1.b =VLUT32(sAbsDif1.b,sRangeTab0.b,R4) //[1]
      R4 = #1                                       //[1]
      dWeight1.uh = VMPY(sRange1.ub,Gaussk.ub)      //[2]
    }
    { sRange0.b|=VLUT32(sAbsDif0.b,sRangeTab0.b,R4) //[1]
    }
    { sRange1.b|=VLUT32(sAbsDif1.b,sRangeTab0.b,R4) //[1]
      R4 = #2                                       //[1]
      sWeight0.b = VSHUFFO(sWeight0H.b,sWeight0L.b) //[2]
    }
    { sRange0.b|=VLUT32(sAbsDif0.b,sRangeTab1.b,R4) //[1]
      Gaussk = MEMUB(gptr++#1)                      //[1]
      sWeight1.b = VSHUFFO(sWeight1H.b,sWeight1L.b) //[2]
    }
    { sRange1.b|=VLUT32(sAbsDif1.b,sRangeTab1.b,R4) //[1]
      R4 = #3                                       //[1]
      dPixelxW0.uh = VMPY(sX_k_t.ub,sWeight0.ub)    //[2]
    }
    { sRange0.b|=VLUT32(sAbsDif0.b,sRangeTab1.b,R4) //[1]
      Gaussk = VSPLATB(Gaussk)                      //[1]
      dPixelxW1.uh = VMPY(sXk_t.ub, sWeight1.ub)    //[2]
    }
    { sRange1.b|=VLUT32(sAbsDif1.b,sRangeTab1.b,R4) //[1]
      R4 = #4                                       //[1]
      dSumW.h += VMPA(dWeight01.ub,const1.b)        //[2]
    }
    { sRange0.b|=VLUT32(sAbsDif0.b,sRangeTab2.b,R4) //[1]
      dSumTmpe.w=VADD(sPixelxW0L.uh,sPixelxW1L.uh)  //[2]
    }
    { sRange1.b|=VLUT32(sAbsDif1.b,sRangeTab2.b,R4) //[1]
      R4 = #5                                       //[1]
      dSumFe.w = VADD(dSumFe.w,dSumTmpe.w)          //[2]
    }
    { sRange0.b|=VLUT32(sAbsDif0.b,sRangeTab2.b,R4) //[1]
      ks = ADD(ks,#-1)                              //[1]
      dSumTmpo.w=VADD(sPixelxW0H.uh,sPixelxW1H.uh)  //[2]
    }
    { sRange1.b|=VLUT32(sAbsDif1.b,sRangeTab2.b,R4) //[1]
      R4 = #6                                       //[1]
      dSumFo.w = VADD(dSumFo.w,dSumTmpo.w)          //[2]
    }:endloop0:endloop1

.bilateral_9x8LPEND:
    { gptr = ADD(GAUSSLUT,#4)                       //
      iptrk = SUB(iptrk,stride)                     //
      LOOP0(.bilateral_8x1LOOP,#4)                  //
    }

    .falign
.bilateral_8x1LOOP:
    { sX_k = VMEM(iptr_k+#0)                        //[1]
      sRange0.b|=VLUT32(sAbsDif0.b,sRangeTab3.b,R4) //[2]
      sX_k_t = sX_k                                 //[2]
    }
    { sXk  = VMEM(iptrk+#0)                         //[1]
      sRange1.b|=VLUT32(sAbsDif1.b,sRangeTab3.b,R4) //[2]
      sXk_t = sXk                                   //[2]
      R4 = #7                                       //[2]
    }
    { sAbsDif0.ub = VABSDIFF(sX_k.ub,sCentr.ub)     //[1]
      sRange0.b|=VLUT32(sAbsDif0.b,sRangeTab3.b,R4) //[2] 
    }
    { sAbsDif1.ub = VABSDIFF(sXk.ub ,sCentr.ub)     //[1]
      R4 = #0                                       //[1]
      sRange1.b|=VLUT32(sAbsDif1.b,sRangeTab3.b,R4) //[2] 
    }
    { sRange0.b =VLUT32(sAbsDif0.b,sRangeTab0.b,R4) //[1]
      iptr_k = ADD(iptr_k,stride)                   //[1]
      iptrk  = SUB(iptrk,stride)                    //[1]
      dWeight0.uh = VMPY(sRange0.ub,Gaussk.ub)      //[2]
    }
    { sRange1.b =VLUT32(sAbsDif1.b,sRangeTab0.b,R4) //[1]
      R4 = #1                                       //[1]
      dWeight1.uh = VMPY(sRange1.ub,Gaussk.ub)      //[2]
    }
    { sRange0.b|=VLUT32(sAbsDif0.b,sRangeTab0.b,R4) //[1]
    }
    { sRange1.b|=VLUT32(sAbsDif1.b,sRangeTab0.b,R4) //[1]
      R4 = #2                                       //[1]
      sWeight0.b = VSHUFFO(sWeight0H.b,sWeight0L.b) //[2]
    }
    { sRange0.b|=VLUT32(sAbsDif0.b,sRangeTab1.b,R4) //[1]
      Gaussk = MEMUB(gptr+#0)                       //[1]
      sWeight1.b = VSHUFFO(sWeight1H.b,sWeight1L.b) //[2]
    }
    { sRange1.b|=VLUT32(sAbsDif1.b,sRangeTab1.b,R4) //[1]
      R4 = #3                                       //[1]
      dPixelxW0.uh = VMPY(sX_k_t.ub,sWeight0.ub)    //[2]
    }
    { sRange0.b|=VLUT32(sAbsDif0.b,sRangeTab1.b,R4) //[1]
      Gaussk = VSPLATB(Gaussk)                      //[1]
      dPixelxW1.uh = VMPY(sXk_t.ub, sWeight1.ub)    //[2]
    }
    { sRange1.b|=VLUT32(sAbsDif1.b,sRangeTab1.b,R4) //[1]
      R4 = #4                                       //[1]
      dSumW.h += VMPA(dWeight01.ub,const1.b)        //[2]
    }
    { sRange0.b|=VLUT32(sAbsDif0.b,sRangeTab2.b,R4) //[1]
      gptr = ADD(gptr,#9)                           //[1]
      dSumTmpe.w=VADD(sPixelxW0L.uh,sPixelxW1L.uh)  //[2]
    }
    { sRange1.b|=VLUT32(sAbsDif1.b,sRangeTab2.b,R4) //[1]
      R4 = #5                                       //[1]
      dSumFe.w = VADD(dSumFe.w,dSumTmpe.w)          //[2]
    }
    { sRange0.b|=VLUT32(sAbsDif0.b,sRangeTab2.b,R4) //[1]
      ks = ADD(ks,#-1)                              //[1]
      dSumTmpo.w=VADD(sPixelxW0H.uh,sPixelxW1H.uh)  //[2]
    }
    { sRange1.b|=VLUT32(sAbsDif1.b,sRangeTab2.b,R4) //[1]
      R4 = #6                                       //[1]
      dSumFo.w = VADD(dSumFo.w,dSumTmpo.w)          //[2]
    }:endloop0

.bilateral_8x1LPEND:
    { sRange0.b|=VLUT32(sAbsDif0.b,sRangeTab3.b,R4) //[2]
    }
    { sRange1.b|=VLUT32(sAbsDif1.b,sRangeTab3.b,R4) //[2]
      R4 = #7                                       //[2]
    }
    { sRange0.b|=VLUT32(sAbsDif0.b,sRangeTab3.b,R4) //[2]
    }
    { sRange1.b|=VLUT32(sAbsDif1.b,sRangeTab3.b,R4) //[2]
    }
    { dWeight0.uh = VMPY(sRange0.ub,Gaussk.ub)      //[2]
    }
    { dWeight1.uh = VMPY(sRange1.ub,Gaussk.ub)      //[2]
      sWeight0.b = VSHUFFO(sWeight0H.b,sWeight0L.b) //[2]
    }
    { sWeight1.b = VSHUFFO(sWeight1H.b,sWeight1L.b) //[2]
      sBit = VSPLAT(const80)                        //
    }
    { dPixelxW0.uh = VMPY(sX_k.ub,sWeight0.ub)      //[2]
      sBit.w = VADD(sBit.w,sBit.w)
    }
    { dPixelxW1.uh = VMPY(sXk.ub, sWeight1.ub)      //[2]
    }
    { dSumW.h += VMPA(dWeight01.ub,const1.b)        //[2]
      LOOP0(.division_LOOP,#8)                      //
    }
    { dSumTmpe.w=VADD(sPixelxW0L.uh,sPixelxW1L.uh)  //[2]
      sResultO1 = #0                                //
      sResultO3 = #0                                //
    }
    { dSumFe.w = VADD(dSumFe.w,dSumTmpe.w)          //[2]
      dSumTmpo.w=VADD(sPixelxW0H.uh,sPixelxW1H.uh)  //[2]
    }
    { dSumFo.w = VADD(dSumFo.w,dSumTmpo.w)          //[2]
      dDenomE.w = VMPY(sSumW0.h,const80.h)          //
    }
    { dDenomO.w = VMPY(sSumW1.h,const80.h)          //
      sResultE0 = #0                                //
      sResultE2 = #0                                //
    }

    .falign
.division_LOOP:
    { Q0 = VCMP.GT(sDenomE0.w,sSumFe0.w)            //
      sBit.w = VASR(sBit.w,const1)                  //
    }
    { Q1 = VCMP.GT(sDenomE2.w,sSumFe2.w)            //
      sDenomE0.w = VASR(sDenomE0.w,const1)          //
      IF (!Q0) sSumFe0.w -= sDenomE0.w              //
      IF (!Q0) sResultE0.w += sBit.w                //
    }
    { Q2 = VCMP.GT(sDenomO1.w,sSumFo1.w)            //
      IF (!Q1) sResultE2.w += sBit.w                //
      IF (!Q1) sSumFe2.w -= sDenomE2.w              //
      sDenomE2.w = VASR(sDenomE2.w,const1)          //
    }
    { Q3 = VCMP.GT(sDenomO3.w,sSumFo3.w)            //
      IF (!Q2) sResultO1.w += sBit.w                //
      IF (!Q2) sSumFo1.w -= sDenomO1.w              //
      sDenomO1.w = VASR(sDenomO1.w,const1)          //
    }
    { IF (!Q3) sResultO3.w += sBit.w                //
      IF (!Q3) sSumFo3.w -= sDenomO3.w              //
      sDenomO3.w = VASR(sDenomO3.w,const1)          //
    }:endloop0

    { sResultE0.h=VSHUFFE(sResultE2.h,sResultE0.h)  //
      sResultO1.h=VSHUFFE(sResultO3.h,sResultO1.h)  //
      P1 = CMP.GT(R28,#0)                           //
    }
    { output.b = VSHUFFE(sResultO1.b,sResultE0.b)   //
      VMEM(R5++#1) = output.new                     //
      IF P1 JUMP .bilateral9x9PerRow_LOOP           //
    }

.bilateral9x9PerRwo_END:
    { JUMPR R31
    }
    .size    bilateral9x9PerRow, .-bilateral9x9PerRow
