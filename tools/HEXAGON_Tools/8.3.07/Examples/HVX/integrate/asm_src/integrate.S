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
    .file	"integrate.S"

#include "hvx.cfg.h"
    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void integratePerRow                                          ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: calculate integrations of one image line                      ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned char *src  -- pointer to input image            ]*/
    /*[               R1 : int width           -- width of image block              ]*/
    /*[               R2 : unsigned int   *dst -- pointer to integration outputs    ]*/
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
    /*[   6.0.0         08/01/2014              created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define pIn                 R0
#define pOut                R2
#define cntrl               R1
#define const8              R4
#define const16             R5
#define const32             R6
#define const64             R7
#define const1              R8
#define vsize               R9
/* ============================================================ */
#define sPixels             V0
#define sEvenPixels_t       V1
#define sEvenPixels         V2
#define sD1Sum              V3
#define sSum_t              V4
#define sSum                V5
#define sPIntgEven          V6
#define sPIntgOdd           V7
#define sPIntg0             V8
#define sPIntg1             V9
#define dPIntg              V9:8
#define sPIntguw0           V10
#define sPIntguw1           V11
#define dPIntguw01          V11:10
#define sPIntguw2           V12
#define sPIntguw3           V13
#define dPIntguw23          V13:12
#define sPrvIntg            V14
#define zero                V15
#define mask                V16
#define pattern             V17
#define sD2Sum              sD1Sum
#define sD4Sum              sD1Sum
#define sD8Sum              sD1Sum
#define sD16Sum             sD1Sum
#define sD32Sum             sD1Sum
#define sD1PItgOdd          sPIntgEven
#define sIntg0              sPIntguw0
#define sIntg1              sPIntguw1
#define sIntg2              sPIntguw2
#define sIntg3              sPIntguw3
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl IntegratePerRow
    .type    IntegratePerRow, @function
IntegratePerRow:
    { R28 = ADD(R1,#VLEN-1)                         //
      R10 = ##SPLATpattern                          //
      sPixels= VMEM(pIn+#0)                         //[1]
    }
    { R28 = LSR(R28,#LOG2VLEN)                      // ceil(width/VLEN)
      const1 = ##0x01010101                         //
      pattern = VMEM(R10+#0)                        //
    }
    { R10 = ##0x00FF00FF                            //
      sSum_t.h = VDMPY(sPixels.ub,const1.b)         //[1]
      zero = #0                                     //
    }
    { mask = VSPLAT(R10)                            //
      cntrl = #-2                                   //
      R5:4 = COMBINE(#16,#8)                        //
      R7:6 = COMBINE(#64,#32)                       //
    }
    { sPIntgOdd   = #0                              //
      sEvenPixels = #0                              //
      sD1Sum = VLALIGN(sSum_t,zero,#2)              //[1]
    }
    { sPIntg1  = #0                                 //
      sPrvIntg = #0                                 //
      sSum_t.h = VADD(sSum_t.h,sD1Sum.h)            //[1]
    }
    { vsize = #VLEN                                 //
      P3 = SP2LOOP0(.integratePerRow_Loop,R28)      // setup loop
      P0 = CMP.GT(R28,#1)                           //[1]
    }
    { IF P0 pIn = ADD(pIn,vsize)                    //[1]
      sD2Sum = VLALIGN(sSum_t,zero,#4)              //[1]
    }
    { sSum_t.h = VADD(sSum_t.h,sD2Sum.h)            //[1]
    }
    { sEvenPixels_t = VAND(sPixels,mask)            //[1]
      R28 = ADD(R28,#-1)                            //[1]
    }

#if LOG2VLEN==6
    .falign
.integratePerRow_Loop:
    { sPixels= VMEM(pIn+#0)                         //[1]
      sD4Sum = VLALIGN(sSum_t,zero,const8)          //[2]
      sPIntgEven.h=VADD(sD1PItgOdd.h,sEvenPixels.h) //[3]
      sIntg1.w = VADD(sPrvIntg.w,sPIntguw1.w)       //[4]
    }
    { sSum.h = VADD(sSum_t.h,sD4Sum.h)              //[2]
      dPIntguw23.uw = VUNPACK(sPIntg1.uh)           //[4]
      IF P3 VMEM(pOut++#1) = sIntg0                 //[4]
    }
    { sSum_t.h = VDMPY(sPixels.ub,const1.b)         //[1]
      P0 = CMP.GT(R28,#1)                           //[1]
      dPIntg = VSHUFF(sPIntgOdd,sPIntgEven,cntrl)   //[3]
    }
    { sD8Sum = VLALIGN(sSum,zero,const16)           //[2]
      sIntg3.w = VADD(sPIntguw3.w,sPrvIntg.w)       //[4]
      IF P3 VMEM(pOut++#1) = sIntg1                 //[4]
    }
    { sD1Sum = VLALIGN(sSum_t,zero,#2)              //[1]
      sSum.h = VADD(sSum.h,sD8Sum.h)                //[2]
      sIntg2.w = VADD(sPrvIntg.w,sPIntguw2.w)       //[4]
      IF P3 VMEM(pOut++#1) = sIntg2.new             //[4]
    }
    { sSum_t.h = VADD(sSum_t.h,sD1Sum.h)            //[1]
      sEvenPixels = sEvenPixels_t                   //[2]
      sPrvIntg = VRDELTA(sIntg3,pattern)            //[3]
    }
    { IF P0 pIn = ADD(pIn,vsize)                    //[1]
      sD16Sum = VLALIGN(sSum,zero,const32)          //[2]
      IF P3 VMEM(pOut++#1) = sIntg3                 //[4]
    }
    { sD2Sum = VLALIGN(sSum_t,zero,#4)              //[1]
      R28 = ADD(R28,#-1)                            //[1]
      sPIntgOdd.h = VADD(sSum.h,sD16Sum.h)          //[2]
    }
    { sSum_t.h = VADD(sSum_t.h,sD2Sum.h)            //[1]
      dPIntguw01.uw = VUNPACK(sPIntg0.uh)           //[3]
    }
    { sEvenPixels_t = VAND(sPixels,mask)            //[1]
      sD1PItgOdd = VLALIGN(sPIntgOdd,zero,#2)       //[2]
      sIntg0.w = VADD(sPrvIntg.w,sPIntguw0.w)       //[3]
    }:endloop0

    .falign
.integratePerRow_LoopEND:
    { sPIntgEven.h=VADD(sD1PItgOdd.h,sEvenPixels.h) //[3]
      sIntg1.w = VADD(sPrvIntg.w,sPIntguw1.w)       //[4]
      dPIntguw23.uw = VUNPACK(sPIntg1.uh)           //[4]
    }
    { IF P3 VMEM(pOut++#1) = sIntg0                 //[4]
      sIntg3.w = VADD(sPIntguw3.w,sPrvIntg.w)       //[4]
    }
#else
    .falign
.integratePerRow_Loop:
    { sPixels= VMEM(pIn+#0)                         //[1]
      sD4Sum = VLALIGN(sSum_t,zero,const8)          //[2]
      sIntg1.w = VADD(sPrvIntg.w,sPIntguw1.w)       //[4]
    }
    { sSum.h = VADD(sSum_t.h,sD4Sum.h)              //[2]
      sIntg0.w = VADD(sPrvIntg.w,sPIntguw0.w)       //[4]
      IF P3 VMEM(pOut++#1) = sIntg0.new             //[4]
      dPIntguw23.uw = VUNPACK(sPIntg1.uh)           //[4]
    }
    { sSum_t.h = VDMPY(sPixels.ub,const1.b)         //[1]
      P0 = CMP.GT(R28,#1)                           //[1]
      sD1PItgOdd = VLALIGN(sPIntgOdd,zero,#2)       //[3]
    }
    { sD8Sum = VLALIGN(sSum,zero,const16)           //[2]
      sPIntgEven.h=VADD(sD1PItgOdd.h,sEvenPixels.h) //[3]
      sIntg3.w = VADD(sPIntguw3.w,sPrvIntg.w)       //[4]
      IF P3 VMEM(pOut++#1) = sIntg1                 //[4]
    }
    { sD1Sum = VLALIGN(sSum_t,zero,#2)              //[1]
      sSum.h = VADD(sSum.h,sD8Sum.h)                //[2]
      sIntg2.w = VADD(sPrvIntg.w,sPIntguw2.w)       //[4]
      IF P3 VMEM(pOut++#1) = sIntg2.new             //[4]
    }
    { sSum_t.h = VADD(sSum_t.h,sD1Sum.h)            //[1]
      sEvenPixels = sEvenPixels_t                   //[2]
      sPrvIntg = VRDELTA(sIntg3,pattern)            //[3]
    }
    { IF P0 pIn = ADD(pIn,vsize)                    //[1]
      sD16Sum = VLALIGN(sSum,zero,const32)          //[2]
      IF P3 VMEM(pOut++#1) = sIntg3                 //[4]
    }
    { sD2Sum = VLALIGN(sSum_t,zero,#4)              //[1]
      sSum.h = VADD(sSum.h,sD16Sum.h)               //[2]
    }
    { R28 = ADD(R28,#-1)                            //[1]
      dPIntg = VSHUFF(sPIntgOdd,sPIntgEven,cntrl)   //[3]
    }
    { sSum_t.h = VADD(sSum_t.h,sD2Sum.h)            //[1]
      sD32Sum = VLALIGN(sSum,zero,const64)          //[2]
    }
    { sEvenPixels_t = VAND(sPixels,mask)            //[1]
      sPIntgOdd.h = VADD(sSum.h,sD32Sum.h)          //[2]
      dPIntguw01.uw = VUNPACK(sPIntg0.uh)           //[3]
    }:endloop0

    .falign
.integratePerRow_LoopEND:
    { sIntg1.w = VADD(sPrvIntg.w,sPIntguw1.w)       //[4]
      dPIntguw23.uw = VUNPACK(sPIntg1.uh)           //[4]
    }
    { sIntg0.w = VADD(sPrvIntg.w,sPIntguw0.w)       //[4]
      IF P3 VMEM(pOut++#1) = sIntg0.new             //[4]
      sD1PItgOdd = VLALIGN(sPIntgOdd,zero,#2)       //[4]
    }
    { sPIntgEven.h=VADD(sD1PItgOdd.h,sEvenPixels.h) //[3]
    }
    { sIntg3.w = VADD(sPIntguw3.w,sPrvIntg.w)       //[4]
    }
#endif
    { dPIntg = VSHUFF(sPIntgOdd,sPIntgEven,cntrl)   //[3]
      IF P3 VMEM(pOut++#1) = sIntg1                 //[4]
    }
    { sPrvIntg = VRDELTA(sIntg3,pattern)            //[3]
      sIntg2.w = VADD(sPrvIntg.w,sPIntguw2.w)       //[4]
      IF P3 VMEM(pOut++#1) = sIntg2.new             //[4]
    }
    { dPIntguw01.uw = VUNPACK(sPIntg0.uh)           //[3]
      IF P3 VMEM(pOut++#1) = sIntg3                 //[4]
    }
    { sIntg0.w = VADD(sPrvIntg.w,sPIntguw0.w)       //[3]
    //======
      dPIntguw23.uw = VUNPACK(sPIntg1.uh)           //[4]
      VMEM(pOut++#1) = sIntg0.new                   //[4]
    }
    { sIntg1.w = VADD(sPrvIntg.w,sPIntguw1.w)       //[4]
      VMEM(pOut++#1) = sIntg1.new                   //[4]
    }
    { sIntg2.w = VADD(sPrvIntg.w,sPIntguw2.w)       //[4]
      VMEM(pOut++#1) = sIntg2.new                   //[4]
    }
    { sIntg3.w = VADD(sPIntguw3.w,sPrvIntg.w)       //[4]
      VMEM(pOut++#1) = sIntg3.new                   //[4]
    }
    { JUMPR R31                                     // return
    }
    .size    IntegratePerRow, .-IntegratePerRow



    .section .rodata
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



    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void integrateVertical                                        ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: Perform integrations on vertical                              ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned int *iimgPrev -- pointer to previous row        ]*/
    /*[               R1 : unsigned int *iimg     -- pointer to current row         ]*/
    /*[               R2 : int stride             -- stride of integration          ]*/
    /*[               R3 : int width              -- width of image block           ]*/
    /*[               R4 : int h                  -- height of image block          ]*/
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
    /*[   6.0.0         08/01/2014              created for Hoya evaluation         ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl IntegrateVertical
    .type    IntegrateVertical, @function
IntegrateVertical:
    { R28 = ADD(R3,#(VLEN/4-1))                     //
      R2 = ASL(R2,#2)                               // sizeof(uint32)*stride
      P0 = CMP.GT(R3,#0)                            //
      IF !P0.new JUMPR:nt R31                       //
    }
    { R28 = LSR(R28,#(LOG2VLEN-2))                  // ceil((width/(VLEN/4))
      M0 = R2                                       // set m0=sizeof(uint32)*stride
    }
    { LOOP1(.integrateVertical_outerLoop,R28)       // setup loop1
    }

    .falign
.integrateVertical_outerLoop:
    { LOOP0(.integrateVertical_Loop,R4)             // setup loop0 with lc0 = h
      V0 = VMEM(R0++#1)                             // sum = iimgPrev[i]
      R5 = R1                                       // &iimg[i]
      R1 = ADD(R1,#VLEN)                            //
    }

    .falign
.integrateVertical_Loop:
    { V1.tmp = VMEM(R5+#0)                          // load iimg[j*stride +i]
      V0.w = VADD(V0.w,V1.w)                        // sum += iimg[j*stride +i]
      VMEM(R5++M0) = V0.new                         // iimg[j*stride +i] = sum
    }:endloop0:endloop1

    { JUMPR R31                                     // return
    }
    .size    IntegrateVertical, .-IntegrateVertical

