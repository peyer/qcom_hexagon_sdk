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
    .file	"search.S"

#include "hvx.cfg.h"
    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void search()                                                 ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: search harris corners with non-max supression                 ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : int          *corner                                     ]*/
    /*[               R1 : int           flushcorner                                ]*/
    /*[               R2 : unsigned int  stride                                     ]*/
    /*[               R3 : unsigned int  width                                      ]*/
    /*[               R4 : int           threshold                                  ]*/
    /*[               R5 : unsigned int *masks                                      ]*/
    /*[         (R29+#0) : unsigned int  boundary_l                                 ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
    /*[           -                                                                 ]*/
    /*[=============================================================================]*/
    /*[  ASSUMPTIONS:                                                               ]*/
    /*[           - boundary_l is # of pixels to be excluded on left boundary and   ]*/
    /*[             must be less than VLEN/4                                        ]*/
    /*[           - width is the number of pixels to be actually searched           ]*/
    /*[=============================================================================]*/
    /*[  REVISION HISTORY                                                           ]*/
    /*[  ----------------                                                           ]*/
    /*[  Version        Date                    Comments                            ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[   6.0.0         01/09/2015              created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define iptr0               R0
#define idx0                R1
#define stride              R2
#define width               R3
#define threshold           R4
#define optr                R5
#define boundary            R6
#define idx1                R6
#define idx2                R7
#define idx3                R8
#define idx4                R9
#define c5                  R10
#define lc                  R11
#define iflag               R12
#define stridex4            R13
#define niter               R28
#define iptr1               R1
#define iptr2               R2
#define iptr3               R3
#define iptr3iptr2          R3:2
#define iptr4               R4
#define boundaryx4          R6
#define c8                  R6
#define niter31             R10
#define lcout               R10
/* ============================================================ */
#define sS0                 V0
#define sS1                 V1
#define sS2                 V2
#define sVal                V3
#define sVmax5cur           V4
#define sVmax5nxt           V5
#define sVmax4              V6
#define sMax                V7
#define sTemp               V8
#define sVmax5_0            V9
#define sFlags              V10
#define sOut                V11
#define sThres              V12
#define sMask               V13
#define sMaskFFFF           V14
#define sZero               V15
#define sS3                 sS1
#define sS4                 sS1
#define sVmax5_1            sVmax5_0
#define sVmax5_3            sVmax5_0
#define sVmax5_4            sVmax5_0
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl search
    .type	search, @function
search:
    { boundary = MEMW(R29+#0)                       //
      R28 = ADD(width,#VLEN/4-1)                    //
      sThres = VSPLAT(threshold)                    // threshold
      R15:14 = COMBINE(#-1,#3)                      //
    }
    { sZero = #0                                    //
      sMaskFFFF = VSPLAT(R15)                       //
      sMask     = VSPLAT(R14)                       // (left  boundary mask)
      R14 = NEG(width)                              // (right boundary mask)
    }
    { boundaryx4 = ASL(boundary,#2)                 // (left  boundary mask)
      stridex4 = ASL(stride,#2)                     // sizeof(int)*stride
      idx3 = ADD(idx0,#3)                           //  flushcorner + 3
      idx4 = ADD(idx0,#4)                           //  flushcorner + 4
    }
    { V1 = VLALIGN(sZero,sMaskFFFF,boundaryx4)      // (left  boundary mask)
      idx1 = ADD(idx0,#1)                           // flushcorner + 1
      idx2 = ADD(idx0,#2)                           // flushcorner + 2
      c5 = #5                                       //
    }
    { niter = LSR(R28,#LOG2VLEN-2)                  // 
      idx3 = MODWRAP(idx3,c5)                       // (flushcorner + 3)%5
      Q0 = VCMP.EQ(V1.w,sMaskFFFF.w)                // (left  boundary mask)
      iflag = #4                                    // (left  boundary mask)
    }
    { idx4 = MODWRAP(idx4,c5)                       // (flushcorner + 4)%5
      iptr4 = iptr0                                 //
      iptr3iptr2 = COMBINE(iptr0,iptr0)             //
      sMask |= VAND(Q0,iflag)                       // (left  boundary mask)
    }
    { sMask = VNOT(sMask)                           // (left  boundary mask)
      idx1 = MODWRAP(idx1,c5)                       // (flushcorner + 1)%5
      idx2 = MODWRAP(idx2,c5)                       // (flushcorner + 2)%5
    }
    { iptr3 += MPYI(idx3,stridex4)                  // &corner[( (flushcorner + 3) % 5) * stride]
      iptr4 += MPYI(idx4,stridex4)                  // &corner[( (flushcorner + 4) % 5) * stride]
      niter31 = ADD(niter,#31)                      //
    }
    { iptr0 += MPYI(idx0,stridex4)                  // &corner[( (flushcorner + 0) % 5) * stride]
      iptr1 = iptr0                                 //
      lcout = ASR(niter31,#5)                       // ceil(niter/32)
      lc = #32                                      //
    }
    { iptr1 += MPYI(idx1,stridex4)                  // &corner[( (flushcorner + 1) % 5) * stride]
      iptr2 += MPYI(idx2,stridex4)                  // &corner[( (flushcorner + 2) % 5) * stride]
      P0 = CMP.GT(lc,niter)                         //
      IF P0.new lc = niter                          //
    }
    { iflag = ##0x80000000                          //
      c8 = #8                                       //
      LOOP1(.search_outerLP,lcout)                  // set loop1 with lc1= ceil(LC/32)
    }

    .falign
.search_outerLP:
    { LOOP0(.search_LOOP,lc)                        // 
      niter = SUB(niter,lc)                         //
      sFlags = #0                                   // 
      NOP                                           //
    }

    .falign
.search_LOOP:
    { sS0 = VMEM(iptr0++#1)                         //[1]
      sTemp.w = VMAX(sVmax4.w,sThres.w)             //[1]
      sMax.w  = VMAX(sMax.w,sVmax5_1.w)             //[2]
      iflag = ROL(iflag,#1)                         //[2]
    }
    { sS1.tmp = VMEM(iptr1++#1)                     //[1]
      sVmax4.w = VMAX(sS0.w,sS1.w)                  //[1]
      sVmax5_3 = VALIGN(sVmax5nxt,sVmax5cur,#4)     //[2]
      P0 = !CMP.GT(iflag,#0)                        //[2]
    }
    { sS3.tmp = VMEM(iptr3++#1)                     //[1]
      sVmax4.w = VMAX(sVmax4.w,sS3.w)               //[1]
      sMax.w   = VMAX(sMax.w,sVmax5_3.w)            //[2]
      sVmax5_4 = VALIGN(sVmax5nxt,sVmax5cur,c8)     //[2]
    }
    { sS4.tmp = VMEM(iptr4++#1)                     //[1]
      sVmax4.w = VMAX(sVmax4.w,sS4.w)               //[1]
      sVmax5_0 = VLALIGN(sVmax5nxt,sVmax5cur,c8)    //[1]
      sMax.w   = VMAX(sMax.w,sVmax5_4.w)            //[2]
    }
    { sS2 = VMEM(iptr2++#1)                         //[1]
      sVal = sS2                                    //[1]
      sMax.w = VMAX(sTemp.w,sVmax5_0.w)             //[1]
      Q0 = VCMP.GT(sVal.w,sMax.w)                   //[2]
    }
    { sVmax5_1 = VLALIGN(sVmax5nxt,sVmax5cur,#4)    //[1]
      sVmax5cur = sVmax5nxt                         //[1]
      sVmax5nxt.w = VMAX(sVmax4.w,sS2.w)            //[1]
      sFlags|= VAND(Q0,iflag)                       //[2]
    }:endloop0

    { sOut = VAND(sFlags,sMask)                     //
      IF P0 VMEM(optr++#1) = sOut.new               //
      IF P0 sMask = sMaskFFFF                       //
      lc = MIN(lc,niter)                            // 
    }:endloop1

    //======
    { sTemp.w = VMAX(sVmax4.w,sThres.w)             //[1]
      sMax.w  = VMAX(sMax.w,sVmax5_1.w)             //[2]
      iflag = ROL(iflag,#1)                         //[2]
      sVmax5_3 = VALIGN(sVmax5nxt,sVmax5cur,#4)     //[2]
    }
    { sMax.w   = VMAX(sMax.w,sVmax5_3.w)            //[2]
      sVmax5_4 = VALIGN(sVmax5nxt,sVmax5cur,c8)     //[2]
      IF P0 sFlags = sZero                          //
    }
    { sVmax5_0 = VLALIGN(sVmax5nxt,sVmax5cur,c8)    //[1]
      sMax.w   = VMAX(sMax.w,sVmax5_4.w)            //[2]
      P0 = !CMP.GT(iflag,#0)                        //[2]
    }
    { sVal = sS2                                    //[1]
      sMax.w = VMAX(sTemp.w,sVmax5_0.w)             //[1]
      Q0 = VCMP.GT(sVal.w,sMax.w)                   //[2]
    }
    { sVmax5_1 = VLALIGN(sVmax5nxt,sVmax5cur,#4)    //[1]
      sVmax5cur = sVmax5nxt                         //[1]
      sFlags|= VAND(Q0,iflag)                       //[2]
    }
    { sOut = VAND(sFlags,sMask)                     //
      IF P0 VMEM(optr++#1) = sOut.new               //
      IF P0 sMask = sMaskFFFF                       //
      IF P0 sFlags = sZero                          //
    }
    //====== epilogue ======
    { sMax.w  = VMAX(sMax.w,sVmax5_1.w)             //[e]
      iflag = ROL(iflag,#1)                         //[e]
      sVmax5_3 = VALIGN(sVmax5nxt,sVmax5cur,#4)     //[e]
      R7 = AND(R14,#VLEN/4-1)                       // (right boundary mask)
    }
    { sMax.w   = VMAX(sMax.w,sVmax5_3.w)            //[e]
      sVmax5_4 = VALIGN(sVmax5nxt,sVmax5cur,c8)     //[e]
      R7 = ASL(R7,#2)                               // (right boundary mask)
    }
    { sMax.w = VMAX(sMax.w,sVmax5_4.w)              //[e]
      V0 = VALIGN(sZero,sMaskFFFF,R7)               // (right boundary mask)
    }
    { Q0 = VCMP.GT(sVal.w,sMax.w)                   //[e]
      Q1 = VCMP.EQ(V0.w,sMaskFFFF.w)                // (right boundary mask)
    }
    { Q0 = AND(Q0,Q1)                               // right boundary masking
    }
    { sFlags|= VAND(Q0,iflag)                       //[e]
    }
    { sOut = VAND(sFlags,sMask)                     //[e]
      VMEM(optr+#0) = sOut.new                      //[e]save flags to memory
    }
    { JUMPR R31                                     // return
    }
    .size	search, .-search








