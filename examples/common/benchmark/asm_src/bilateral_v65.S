
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
    .file	"bilateral_v65.S"

    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void bilateral9x9PerRow_v65                                   ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: apply 9x9 bilateral filter to a image block                   ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned char *inp      -- pointer to input image        ]*/
    /*[               R1 : int            stride_i -- stride of image input         ]*/
    /*[               R2 : int            width    -- width of image block          ]*/
    /*[               R3 : unsigned char *outp     -- pointer to output buffer      ]*/
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
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/

// only V65 and beyond support scatter/gather.
#if (__HEXAGON_ARCH__ >= 65)
    
// only 128B mode is supported.
#define VLEN                128
#define LOG2VLEN            7
    
#define iptr0               R0
#define stride              R1
#define width               R2
#define loopcount           R2
#define GAUSSLUT            R3
#define RANGELUT            R4
#define optr                R5
#define range               R6
#define ks                  R7
#define iptr_k              R8
#define iptrk               R9
#define range0              R10
#define gauss0              R11
#define const80             R12
#define const1              R13
#define const1_const80      R13:12
#define c64x2               R14
#define w00h                R15
#define Gaussk0             R16
#define Gaussk1             R17
#define Gaussks             R17:16
#define c01b80h             const80
#define w00                 range0
#define gptr                gauss0
#define lutsize             iptr_k
#define const0to63          iptrk
/* ============================================================ */
#define sPrv                V0
#define sCur                V1
#define sNxt                V2
#define sCentr              V3
#define sX_k                V4
#define sXk                 V5
#define dXkX_k              V5:4
#define sX_k_t0             V6
#define sXk_t0              V7
#define dXkX_k_t0           V7:6
#define sX_k_t1             V8
#define sXk_t1              V9
#define dXkX_k_t1           V9:8
#define sAbsDif0            V10
#define sAbsDif1            V11
#define dAbsDif             V11:10
#define sAdr0L              V12
#define sAdr0H              V13
#define dAdr0               V13:12
#define sAdr1L              V14
#define sAdr1H              V15
#define dAdr1               V15:14
#define sSumW0              V16
#define sSumW1              V17
#define dSumW               V17:16
#define sSumFe0             V18
#define sSumFe2             V19
#define dSumFe              V19:18
#define sSumFo1             V20
#define sSumFo3             V21
#define dSumFo              V21:20
#define sWeight0L           V22
#define sWeight0H           V23
#define dWeight0            V23:22
#define sWeight1L           V24
#define sWeight1H           V25
#define dWeight1            V25:24
#define sWeight0            V26
#define sWeight1            V27
#define sCi                 V28
#define sMask               V29
#define sBit                V30
#define sPixelxW0L          sWeight0L
#define sPixelxW0H          sWeight0H
#define dPixelxW0           dWeight0
#define sPixelxW1L          sWeight1L
#define sPixelxW1H          sWeight1H
#define dPixelxW1           dWeight1
#define sRange0             sBit
#define sRange1             sBit
#define sDenomE0            sAdr0L
#define sDenomE2            sAdr0H
#define dDenomE             dAdr0
#define sDenomO1            sAdr1L
#define sDenomO3            sAdr1H
#define dDenomO             dAdr1
#define sResultE0           sX_k_t0
#define sResultE2           sXk_t0
#define dResultE02          dXkX_k_t0
#define sResultO1           sX_k_t1
#define sResultO3           sXk_t1
#define dResultO13          dXkX_k_t1
#define sOut                sResultE0
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl bilateral9x9PerRow_v65
    .type    bilateral9x9PerRow_v65, @function
bilateral9x9PerRow_v65:
    {
      GAUSSLUT = ADD(PC,##GAUSS_LUT_V65@PCREL)      //
      P0 = CMP.GT(R0,R0)                            //
    }
    { range = MEMW(R29+#0)                          //
      R28 = ADD(width,#VLEN-1)                      //
      c01b80h = ##0x01010080                        //
    }
    { R28 = LSR(R28,#LOG2VLEN)                      //
      range0 = MEMUB(RANGELUT+#0)                   //
      gauss0 = MEMUB(GAUSSLUT+#4*9+4)               //
      const1_const80 = PACKHL(c01b80h,c01b80h)      // 
    }
    { const0to63 = ADD(PC,##CONST0to63H@PCREL)      //
      w00h = MPYI(range0,gauss0)                    //
      R29 = ADD(R29,#-8)                            // reserve stack
    }
    { MEMD(R29+#0) = R17:16                         // callee-saved regs
      w00h = LSR(w00h,#8)                           //
      sCi = VMEM(const0to63)                        //
      sMask = VSPLAT(const80)                       //
    }
    { lutsize = ##256*64*2                          //
      Q0 = VAND(sMask,const80)                      //
      w00 = VSPLATB(w00h)                           //
    }
    { c64x2 = ASL(const1,#7)                        //
      w00h = COMBINE(w00h.L,w00h.L)                 //
      M0 = lutsize                                  //
    }

    .falign
.bilateral9x9PerRow_v65_LOOP:
    { iptrk = iptr0                                 //
      sCentr.cur = VMEM(iptr0++#1)                  //
      dPixelxW0.uh = VMPY(sCentr.ub,w00.ub)         //
      Q1 = NOT(Q0)                                  //
    }
    { R28 = ADD(R28,#-1)                            //
      gptr = ADD(GAUSSLUT,#-5)                      //
      sSumW0 = VSPLAT(w00h)                         //
      dSumFe = #0                                   //
    }
    { iptrk -= ASL(stride,#2)                       //
      Gaussks= #0                                   //
      dSumFo = #0                                   //
      sSumW1 = sSumW0                               // 
    }
    { iptr_k = iptrk                                //
      dPixelxW1 = #0                                //
      LOOP1(.bilateral_9x8LOOP,#9)                  //
      loopcount = #9                                //
    }

    .falign
.bilateral_9x8LOOP:
    { if (P0) sPrv = VMEM(iptrk+#-1)                //
      P0 = CMP.EQ(R0,R0)                            //
      loopcount = ADD(loopcount, #-1)
      ks   = #4                                     //
    }
    { sCur = VMEM(iptrk+#0)                         //
      gptr = ADD(gptr,#5)                           //
      P1 = CMP.EQ(loopcount, #0)
      P1 = CMP.EQ(R28,#0)
    }
    { if (!P1) sNxt = VMEM(iptrk+#1)                //
      iptrk = ADD(iptrk,stride)                     //
      LOOP0(.bilateral_1x8LOOP,#2)                  //
    }

    .falign
.bilateral_1x8LOOP:
    { sX_k = VLALIGN(sCur,sPrv,ks)                  //[1]
      Gaussk0 = VSPLATB(Gaussk0)                    //[2]
      dSumFe.w += VADD(sPixelxW0L.uh,sPixelxW1L.uh) //[3]
    }
    { sXk  = VALIGN( sNxt,sCur,ks)                  //[1]
      dSumFo.w += VADD(sPixelxW0H.uh,sPixelxW1H.uh) //[3]
    }
    { dAdr0 = VCOMBINE(sCi,sCi)                     //[1]
      sRange0.tmp = VMEM(range+#0)                  //[2]
      dWeight0.uh = VMPY(sRange0.ub,Gaussk0.ub)     //[2]
    }
    { sAbsDif0.ub = VABSDIFF(sX_k.ub,sCentr.ub)     //[1]
      sAbsDif1.ub = VABSDIFF(sXk.ub ,sCentr.ub)     //[1]
      ks = ADD(ks,#-1)                              //[1]
      sWeight0.b = VSHUFFO(sWeight0H.b,sWeight0L.b) //[2]
    }
    { dAdr1 = VCOMBINE(sCi,sCi)                     //[1]
      sRange1.tmp = VMEM(range+#1)                  //[2]
      dWeight1.uh = VMPY(sRange1.ub,Gaussk0.ub)     //[2]
    }
    { dAdr0.uh += VMPY(sAbsDif0.ub,c64x2.ub)        //[1]
      Gaussk0 = MEMUB(gptr++#1)                     //[1]
      sWeight1.b = VSHUFFO(sWeight1H.b,sWeight1L.b) //[2]
    }
//    V10 = VMEM(RANGELUT)
//      R17:16 = MEMD(R29)
//      R29 = ADD(R29,#8)
//    JUMPR R31
    { dAdr1.uh += VMPY(sAbsDif1.ub,c64x2.ub)        //[1]
      IF (Q0) VTMP.h=VGATHER(RANGELUT,M0,sAdr0L.h).h//[1]
      VMEM(range+#0) = VTMP.new                     //[1]
    }
    { IF (Q1) VTMP.h=VGATHER(RANGELUT,M0,sAdr0H.h).h//[1]
      VMEM(range+#0) = VTMP.new                     //[1]
      sX_k_t0 = sX_k                                //[1]
      dPixelxW0.uh = VMPY(sX_k_t0.ub,sWeight0.ub)   //[2]
    }
    { IF (Q0) VTMP.h=VGATHER(RANGELUT,M0,sAdr1L.h).h//[1]
      VMEM(range+#1) = VTMP.new                     //[1]
      sXk_t0 = sXk                                  //[1]
      dPixelxW1.uh = VMPY(sXk_t0.ub, sWeight1.ub)   //[2]
    }
    { IF (Q1) VTMP.h=VGATHER(RANGELUT,M0,sAdr1H.h).h//[1]
      VMEM(range+#1) = VTMP.new                     //[1]
      sX_k = VLALIGN(sCur,sPrv,ks)                  //[1]
      dSumW.h += VADD(sWeight0.ub,sWeight1.ub)      //[2]
    }
    { sXk  = VALIGN( sNxt,sCur,ks)                  //[1]
      dSumFe.w += VADD(sPixelxW0L.uh,sPixelxW1L.uh) //[2]
    }
    { ks = ADD(ks,#-1)                              //[1]
      dSumFo.w += VADD(sPixelxW0H.uh,sPixelxW1H.uh) //[2]
      Gaussk1 = VSPLATB(Gaussk1)                    //[2]
    }
    { sAbsDif0.ub = VABSDIFF(sX_k.ub,sCentr.ub)     //[1]
      sAbsDif1.ub = VABSDIFF(sXk.ub ,sCentr.ub)     //[1]
    }
    { dAdr0 = VCOMBINE(sCi,sCi)                     //[1]
      sRange0.tmp = VMEM(range+#2)                  //[2]
      dWeight0.uh = VMPY(sRange0.ub,Gaussk1.ub)     //[2]
    }
    { dAdr0.uh += VMPY(sAbsDif0.ub,c64x2.ub)        //[1]
      sWeight0.b = VSHUFFO(sWeight0H.b,sWeight0L.b) //[2]
    }
    { dAdr1 = VCOMBINE(sCi,sCi)                     //[1]
      Gaussk1 = MEMUB(gptr++#1)                     //[1]
      sRange1.tmp = VMEM(range+#3)                  //[2]
      dWeight1.uh = VMPY(sRange1.ub,Gaussk1.ub)     //[2]
    }
    { dAdr1.uh += VMPY(sAbsDif1.ub,c64x2.ub)        //[1]
      IF (Q0) VTMP.h=VGATHER(RANGELUT,M0,sAdr0L.h).h//[1]
      VMEM(range+#2) = VTMP.new                     //[1]
      sWeight1.b = VSHUFFO(sWeight1H.b,sWeight1L.b) //[2]
    }
    { IF (Q1) VTMP.h=VGATHER(RANGELUT,M0,sAdr0H.h).h//[1]
      VMEM(range+#2) = VTMP.new                     //[1]
      sX_k_t1 = sX_k                                //[1]
      dPixelxW0.uh = VMPY(sX_k_t1.ub,sWeight0.ub)   //[2]
    }
    { IF (Q0) VTMP.h=VGATHER(RANGELUT,M0,sAdr1L.h).h//[1]
      VMEM(range+#3) = VTMP.new                     //[1]
      sXk_t1 = sXk                                  //[1]
      dPixelxW1.uh = VMPY(sXk_t1.ub, sWeight1.ub)   //[2]
    }
    { IF (Q1) VTMP.h=VGATHER(RANGELUT,M0,sAdr1H.h).h//[1]
      VMEM(range+#3) = VTMP.new                     //[1]
      dSumW.h += VADD(sWeight0.ub,sWeight1.ub)      //[2]
    }:endloop0:endloop1


.bilateral_9x8LPEND:
    { gptr = ADD(GAUSSLUT,#4)                       //
      iptrk = SUB(iptrk,stride)                     //
      LOOP0(.bilateral_8x1LOOP,#2)                  //
    }

    .falign
.bilateral_8x1LOOP:
    { sX_k = VMEM(iptr_k+#0)                        //[1]
      iptr_k = ADD(iptr_k,stride)                   //[1]
      Gaussk0 = VSPLATB(Gaussk0)                    //[2]
      dSumFe.w += VADD(sPixelxW0L.uh,sPixelxW1L.uh) //[3]
    }
    { sXk = VMEM(iptrk+#0)                          //[1]
      iptrk  = SUB(iptrk,stride)                    //[1]
      dSumFo.w += VADD(sPixelxW0H.uh,sPixelxW1H.uh) //[3]
    }
    { dAdr0 = VCOMBINE(sCi,sCi)                     //[1]
      sRange0.tmp = VMEM(range+#0)                  //[2]
      dWeight0.uh = VMPY(sRange0.ub,Gaussk0.ub)     //[2]
    }
    { sAbsDif0.ub = VABSDIFF(sX_k.ub,sCentr.ub)     //[1]
      sAbsDif1.ub = VABSDIFF(sXk.ub ,sCentr.ub)     //[1]
      sWeight0.b = VSHUFFO(sWeight0H.b,sWeight0L.b) //[2]
    }
    { dAdr1 = VCOMBINE(sCi,sCi)                     //[1]
      sRange1.tmp = VMEM(range+#1)                  //[2]
      dWeight1.uh = VMPY(sRange1.ub,Gaussk0.ub)     //[2]
    }
    { dAdr0.uh += VMPY(sAbsDif0.ub,c64x2.ub)        //[1]
      Gaussk0 = MEMUB(gptr+#0)                      //[1]
      sWeight1.b = VSHUFFO(sWeight1H.b,sWeight1L.b) //[2]
    }
    { dAdr1.uh += VMPY(sAbsDif1.ub,c64x2.ub)        //[1]
      IF (Q0) VTMP.h=VGATHER(RANGELUT,M0,sAdr0L.h).h//[1]
      VMEM(range+#0) = VTMP.new                     //[1]
    }
    { IF (Q1) VTMP.h=VGATHER(RANGELUT,M0,sAdr0H.h).h//[1]
      VMEM(range+#0) = VTMP.new                     //[1]
      sX_k_t0 = sX_k                                //[1]
      dPixelxW0.uh = VMPY(sX_k_t0.ub,sWeight0.ub)   //[2]
    }
    { IF (Q0) VTMP.h=VGATHER(RANGELUT,M0,sAdr1L.h).h//[1]
      VMEM(range+#1) = VTMP.new                     //[1]
      sXk_t0 = sXk                                  //[1]
      dPixelxW1.uh = VMPY(sXk_t0.ub, sWeight1.ub)   //[2]
    }
    { IF (Q1) VTMP.h=VGATHER(RANGELUT,M0,sAdr1H.h).h//[1]
      VMEM(range+#1) = VTMP.new                     //[1]
      dSumW.h += VADD(sWeight0.ub,sWeight1.ub)      //[2]
    }
    { sX_k = VMEM(iptr_k+#0)                        //[1]
      iptr_k = ADD(iptr_k,stride)                   //[1]
      dSumFe.w += VADD(sPixelxW0L.uh,sPixelxW1L.uh) //[2]
    }
    { dSumFo.w += VADD(sPixelxW0H.uh,sPixelxW1H.uh) //[2]
      Gaussk1 = VSPLATB(Gaussk1)                    //[2]
    }
    { sXk.cur = VMEM(iptrk+#0)                      //[1]
      iptrk  = SUB(iptrk,stride)                    //[1]
      sAbsDif0.ub = VABSDIFF(sX_k.ub,sCentr.ub)     //[1]
      sAbsDif1.ub = VABSDIFF(sXk.ub ,sCentr.ub)     //[1]
    }
    { dAdr0 = VCOMBINE(sCi,sCi)                     //[1]
      sRange0.tmp = VMEM(range+#2)                  //[2]
      dWeight0.uh = VMPY(sRange0.ub,Gaussk1.ub)     //[2]
    }
    { dAdr0.uh += VMPY(sAbsDif0.ub,c64x2.ub)        //[1]
      sWeight0.b = VSHUFFO(sWeight0H.b,sWeight0L.b) //[2]
    }
    { dAdr1 = VCOMBINE(sCi,sCi)                     //[1]
      Gaussk1 = MEMUB(gptr+#9)                      //[1]
      sRange1.tmp = VMEM(range+#3)                  //[2]
      dWeight1.uh = VMPY(sRange1.ub,Gaussk1.ub)     //[2]
    }
    { dAdr1.uh += VMPY(sAbsDif1.ub,c64x2.ub)        //[1]
      IF (Q0) VTMP.h=VGATHER(RANGELUT,M0,sAdr0L.h).h//[1]
      VMEM(range+#2) = VTMP.new                     //[1]
      sWeight1.b = VSHUFFO(sWeight1H.b,sWeight1L.b) //[2]
    }
    { IF (Q1) VTMP.h=VGATHER(RANGELUT,M0,sAdr0H.h).h//[1]
      VMEM(range+#2) = VTMP.new                     //[1]
      sX_k_t1 = sX_k                                //[1]
      dPixelxW0.uh = VMPY(sX_k_t1.ub,sWeight0.ub)   //[2]
    }
    { IF (Q0) VTMP.h=VGATHER(RANGELUT,M0,sAdr1L.h).h//[1]
      VMEM(range+#3) = VTMP.new                     //[1]
      sXk_t1 = sXk                                  //[1]
      dPixelxW1.uh = VMPY(sXk_t1.ub, sWeight1.ub)   //[2]
    }
    { IF (Q1) VTMP.h=VGATHER(RANGELUT,M0,sAdr1H.h).h//[1]
      VMEM(range+#3) = VTMP.new                     //[1]
      gptr = ADD(gptr,#9*2)                         //[1]
      dSumW.h += VADD(sWeight0.ub,sWeight1.ub)      //[2]
    }:endloop0

.bilateral_8x1LPEND:
    { Gaussk0 = VSPLATB(Gaussk0)                    //[2]
      dSumFe.w += VADD(sPixelxW0L.uh,sPixelxW1L.uh) //[3]
    }
    { dSumFo.w += VADD(sPixelxW0H.uh,sPixelxW1H.uh) //[3]
    }
    { sRange0.tmp = VMEM(range+#0)                  //[2]
      dWeight0.uh = VMPY(sRange0.ub,Gaussk0.ub)     //[2]
    }
    { sRange1.tmp = VMEM(range+#1)                  //[2]
      dWeight1.uh = VMPY(sRange1.ub,Gaussk0.ub)     //[2]
      sWeight0.b = VSHUFFO(sWeight0H.b,sWeight0L.b) //[2]
    }
    { sWeight1.b = VSHUFFO(sWeight1H.b,sWeight1L.b) //[2]
    }
    { dPixelxW0.uh = VMPY(sX_k_t0.ub,sWeight0.ub)   //[2]
    }
    { dPixelxW1.uh = VMPY(sXk_t0.ub, sWeight1.ub)   //[2]
    }
    { dSumW.h += VADD(sWeight0.ub,sWeight1.ub)      //[2]
    }
    { dSumFe.w += VADD(sPixelxW0L.uh,sPixelxW1L.uh) //[2]
    }
    { dSumFo.w += VADD(sPixelxW0H.uh,sPixelxW1H.uh) //[2]
      Gaussk1 = VSPLATB(Gaussk1)                    //[2]
    }
    { sRange0.tmp = VMEM(range+#2)                  //[2]
      dWeight0.uh = VMPY(sRange0.ub,Gaussk1.ub)     //[2]
    }
    { sRange1.tmp = VMEM(range+#3)                  //[2]
      dWeight1.uh = VMPY(sRange1.ub,Gaussk1.ub)     //[2]
      sWeight0.b = VSHUFFO(sWeight0H.b,sWeight0L.b) //[2]
    }
    { sWeight1.b = VSHUFFO(sWeight1H.b,sWeight1L.b) //[2]
      sBit = VSPLAT(const80)                        //
    }
    { dPixelxW0.uh = VMPY(sX_k_t1.ub,sWeight0.ub)   //[2]
      sBit.w = VADD(sBit.w,sBit.w)                  //
    }
    { dPixelxW1.uh = VMPY(sXk_t1.ub, sWeight1.ub)   //[2]
    }
    { dSumW.h += VADD(sWeight0.ub,sWeight1.ub)      //[2]
    }
    { dSumFe.w += VADD(sPixelxW0L.uh,sPixelxW1L.uh) //[3]
    }
    { dSumFo.w += VADD(sPixelxW0H.uh,sPixelxW1H.uh) //[3]
      LOOP0(.division_LOOP,#8)                      //
    }
    { dDenomE.w = VMPY(sSumW0.h,const80.h)          //
      dResultO13 = #0                               //
    }
    { dDenomO.w = VMPY(sSumW1.h,const80.h)          //
      dResultE02 = #0                               //
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
      Q0 = VAND(sMask,const80)                      // for next iteration
    }
    { sOut.b = VSHUFFE(sResultO1.b,sResultE0.b)     //
      VMEM(R5++#1) = sOut.new                       //
      IF P1 JUMP .bilateral9x9PerRow_v65_LOOP           //
    }

.bilateral9x9PerRwo_END:
    { R17:16 = MEMD(R29+#0)                         // restore callee-saved regs
      R29 = ADD(R29,#8)                             // pop stack
      JUMPR R31                                     // return
    }
    .size    bilateral9x9PerRow_v65, .-bilateral9x9PerRow_v65




    .section        .data
    .p2align LOG2VLEN
    .type   CONST0to63H, @object
    .size   CONST0to63H, 128
CONST0to63H:
    .hword  0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30
    .hword  32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62
    .hword  64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94
    .hword  96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,126

    .data
    .globl GAUSS_LUT_V65
    .p2align 3
    .type   GAUSS_LUT_V65, @object
    .size   GAUSS_LUT_V65, 81
GAUSS_LUT_V65:
    .byte   146,165,180,190,193,190,180,165,146
    .byte   165,187,203,214,218,214,203,187,165
    .byte   180,203,222,233,238,233,222,203,180
    .byte   190,214,233,246,250,246,233,214,190
    .byte   193,218,238,250,255,250,238,218,193
    .byte   190,214,233,246,250,246,233,214,190
    .byte   180,203,222,233,238,233,222,203,180
    .byte   165,187,203,214,218,214,203,187,165
    .byte   146,165,180,190,193,190,180,165,146
    
#endif //(__HEXAGON_ARCH__ >= 65)
