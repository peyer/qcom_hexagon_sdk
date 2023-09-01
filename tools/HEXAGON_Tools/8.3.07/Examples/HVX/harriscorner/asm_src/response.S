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
    .file	"response.S"

#include "hvx.cfg.h"
    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void corner_response()                                        ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: compute one row of corner response                            ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : short *blurgradxx                                        ]*/
    /*[               R1 : short *blurgradyy                                        ]*/
    /*[               R2 : short *blurgradxy                                        ]*/
    /*[               R3 : unsigned int width                                       ]*/
    /*[               R4 : int   *corner                                            ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
    /*[           -                                                                 ]*/
    /*[=============================================================================]*/
    /*[  ASSUMPTIONS:                                                               ]*/
    /*[           - arrays are aligned by vector size                               ]*/
    /*[=============================================================================]*/
    /*[  REVISION HISTORY                                                           ]*/
    /*[  ----------------                                                           ]*/
    /*[  Version        Date                    Comments                            ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[   6.0.0                                 created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define iptrxx              R0
#define iptryy              R1
#define iptrxy              R2
#define width               R3
#define optr                R4
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl corner_response
    .type	corner_response, @function
corner_response:
    { R28 = LSR(width,#(LOG2VLEN-1))                // width/(VLEN/2)
      R7:6 = COMBINE(#-4,#4)                        // shuff params : shift amount
    }
    { P3 = SP2LOOP0(.corner_responseLOOP,R28)       // setup loop
    }

    .falign
.corner_responseLOOP:
    { V0 = VMEM(iptrxx++#1)                         //[1]load Ixx
      V14.w = VASR(V8.w,R6)                         //[2](trace * trace) >> 4
      V7:6.w = VMPY(V3.h,V3.h)                      //[2]Ixy*Ixy
    }
    { V1.cur = VMEM(iptryy++#1)                     //[1]load Iyy
      V2.h = VADD(V0.h,V1.h)                        //[1]trace = Ixx+Iyy
      V7:6.w = VSUB(V5:4.w,V7:6.w)                  //[2]det = Ixx*Iyy - Ixy*Ixy
    }
    { V3 = VMEM(iptrxy++#1)                         //[1]load Ixy
      V10.w = VSUB(V6.w,V14.w)                      //[2]corner = det - ((trace*trace)>>4)
      V13:12 = VSHUFF(V11,V10,R7)                   //[3]
    }
    { V9:8.w = VMPY(V2.h,V2.h)                      //[1]trace*trace
      V15.w = VASR(V9.w,R6)                         //[2](trace * trace) >> 4
      IF P3 VMEM(optr++#1) = V12                    //[3]save corner
    }
    { V5:4.w = VMPY(V0.h,V1.h)                      //[1]Ixx*Iyy
      V11.w = VSUB(V7.w,V15.w)                      //[2]corner = det - ((trace*trace)>>4)
      IF P3 VMEM(optr++#1) = V13                    //[3]save corner
    }:endloop0

    { V14.w = VASR(V8.w,R6)                         //[2](trace * trace) >> 4
      V7:6.w = VMPY(V3.h,V3.h)                      //[2]Ixy*Ixy
    }
    { V7:6.w = VSUB(V5:4.w,V7:6.w)                  //[2]det = Ixx*Iyy - Ixy*Ixy
    }
    { V10.w = VSUB(V6.w,V14.w)                      //[2]corner = det - ((trace*trace)>>4)
      V13:12 = VSHUFF(V11,V10,R7)                   //[3]
    }
    { V15.w = VASR(V9.w,R6)                         //[2](trace * trace) >> 4
      IF P3 VMEM(optr++#1) = V12                    //[3]save corner
    }
    { V11.w = VSUB(V7.w,V15.w)                      //[2]corner = det - ((trace*trace)>>4)
      IF P3 VMEM(optr++#1) = V13                    //[3]save corner
    }
    { V13:12 = VSHUFF(V11,V10,R7)                   //[3]
    }
    { VMEM(optr+#0) = V12                           //[3]save corner
    }
    { VMEM(optr+#1) = V13                           //[3]save corner
    }
    { JUMPR R31                                     // return
    }
    .size	corner_response, .-corner_response

