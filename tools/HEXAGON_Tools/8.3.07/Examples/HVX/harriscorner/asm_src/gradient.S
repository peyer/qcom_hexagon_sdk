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
    .file	"gradient.S"

#include "hvx.cfg.h"
    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void gradients()                                              ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: Compute one row of gradients of gradxx, gradyy, gradxy        ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned char *img                                       ]*/
    /*[               R1 : unsigned int   stride                                    ]*/
    /*[               R2 : unsigned int   width                                     ]*/
    /*[               R3 : short         *gradxx                                    ]*/
    /*[               R4 : short         *gradyy                                    ]*/
    /*[               R5 : short         *gradxy                                    ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
    /*[           -                                                                 ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  ASSUMPTIONS:                                                               ]*/
    /*[           - img is aligned by vector size                                   ]*/
    /*[           - stride is a multiple of vector size                             ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  REVISION HISTORY                                                           ]*/
    /*[  ----------------                                                           ]*/
    /*[  Version        Date                    Comments                            ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[   6.0.0                                 created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define iptr                R0
#define stride              R1
#define width               R2
#define opxx                R3
#define opyy                R4
#define opxy                R5
#define ctrl                R6
#define vsize               R7
#define iptr0               R8
#define iptr1               R9
/* ============================================================ */

    .text
    .p2align 2
    .p2align 4,,15
    .globl gradients
    .type	gradients, @function
gradients:
    { R28 = ADD(width,#VLEN-1-VLEN)                 //
      iptr0 = SUB(iptr,stride)                      // srcImg - width
      iptr1 = ADD(iptr,stride)                      // srcImg + width
      V1 = VMEM(iptr+#0)                            // load srcImg[x]
    }
    { R28 = ASR(R28,#LOG2VLEN)                      //
      ctrl  = #-2                                   // permute control 
      vsize = #VLEN                                 //
    }
    { P1 = CMP.GT(R28,#0)                           //
      V4 = VMEM(iptr0++#1)                          //[1]load srcImg[x-width]
      V2 = VLALIGN(V1,V0,#1)                        //[1]srcImg[x-1]
      V0 = V1                                       //[1]get current srcImg[x]
    }
    { IF P1 iptr = ADD(iptr,vsize)                  //
      P3 = SP1LOOP0(.gradient_LOOP,R28)             // setup loop
      V5 = VMEM(iptr1++#1)                          //[1]load srcImg[x+width]
    }
    { V1.cur = VMEM(iptr+#0)                        //[1]load next srcImg[x]
      V3 = VALIGN(V1,V0,#1)                         //[1]srcImg[x+1]
      V5.b = VNAVG(V5.ub,V4.ub)                     //[1]iy = (srcImg[x+width]-srcImg[x-width])>>1
    }
    { V3.b = VNAVG(V3.ub,V2.ub)                     //[1]ix = (srcImg[x+1]-srcImg[x-1])>>1
    }
    { V7:6.h = VMPY(V5.b,V5.b)                      //[1]iy*iy
      R28 = ADD(R28,#-1)                            //[1]
    }
    { V9:8.h = VMPY(V3.b,V3.b)                      //[1]ix*ix
      P0 = CMP.GT(R28,#0)                           //[1]not end-of-line
      IF P0.new iptr = ADD(iptr,vsize)              //[1]if not EOL, update read pointer to next set
    }
    { V11:10.h = VMPY(V3.b,V5.b)                    //[1]ix*iy
      VSHUFF(V7,V6,ctrl)                            //[1]interleave gradyy to normal order
      IF !P1 JUMP .gradient_LPEND                   //
    }


    .falign
.gradient_LOOP:
    { V4 = VMEM(iptr0++#1)                          //[1]load srcImg[x-width]
      V2 = VLALIGN(V1,V0,#1)                        //[1]srcImg[x-1]
      V0 = V1                                       //[1]get current srcImg[x]
      VMEM(opyy++#1) = V6                           //[2]save gradyy
    }
    { V5 = VMEM(iptr1++#1)                          //[1]load srcImg[x+width]
      VSHUFF(V9,V8,ctrl)                            //[2]interleave gradxx to normal order
      VMEM(opyy++#1) = V7                           //[2]save gradyy
    }
    { V1.cur = VMEM(iptr+#0)                        //[1]load next srcImg[x]
      V3 = VALIGN(V1,V0,#1)                         //[1]srcImg[x+1]
      V5.b = VNAVG(V5.ub,V4.ub)                     //[1]iy = (srcImg[x+width]-srcImg[x-width])>>1
      VMEM(opxx++#1) = V8                           //[2]save gradxx
    }
    { V3.b = VNAVG(V3.ub,V2.ub)                     //[1]ix = (srcImg[x+1]-srcImg[x-1])>>1
      VSHUFF(V11,V10,ctrl)                          //[2]interleave to normal order
      VMEM(opxx++#1) = V9                           //[2]save gradxx
    }
    { V7:6.h = VMPY(V5.b,V5.b)                      //[1]iy*iy
      R28 = ADD(R28,#-1)                            //[1]
      VMEM(opxy++#1) = V10                          //[2]save gradxy
    }
    { V9:8.h = VMPY(V3.b,V3.b)                      //[1]ix*ix
      P0 = CMP.GT(R28,#0)                           //[1]not end-of-line
      VMEM(opxy++#1) = V11                          //[2]save gradxy
    }
    { V11:10.h = VMPY(V3.b,V5.b)                    //[1]ix*iy
      VSHUFF(V7,V6,ctrl)                            //[1]interleave gradyy to normal order
      IF P0 iptr = ADD(iptr,vsize)                  //[1]if not EOL, update read pointer to next set
    }:endloop0

    .falign
.gradient_LPEND:
    { VMEM(opyy+#0) = V6                            //[2]save gradyy
    }
    { VMEM(opyy+#1) = V7                            //[2]save gradyy
      VSHUFF(V9,V8,ctrl)                            //[2]interleave gradxx to normal order
    }
    { VMEM(opxx+#0) = V8                            //[2]save gradxx
    }
    { VMEM(opxx+#1) = V9                            //[2]save gradxx
      VSHUFF(V11,V10,ctrl)                          //[2]interleave to normal order
    }
    { VMEM(opxy+#0) = V10                           //[2]save gradxy
    }
    { VMEM(opxy+#1) = V11                           //[2]save gradxy
    }
    { JUMPR R31
    }
    .size	gradients, .-gradients

