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

#define VLEN 128
#define LOG2VLEN 7
    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void histogramPernRow()                                       ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: calcluate histogram of a image block                          ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned char *inp  -- pointer to input image            ]*/
    /*[               R1 : int stride_i        -- stride of image input             ]*/
    /*[               R2 : int width           -- width of image block              ]*/
    /*[               R3 : int height          -- height of image block             ]*/
    /*[               R4 : int *hist           -- pointer to histogram array        ]*/
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
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl  histogramPernRow
    .type   histogramPernRow, @function
histogramPernRow:
    { R2 = LSR(R2,#LOG2VLEN)                        // size/VLEN
      R5 = AND(R2,#VLEN-1)                          // size%VLEN
      V1 = #0                                       //
      V0 = #0                                       //
    }
    //[----------------------------------------------]
    //[ Step 1: Clean the whole vector Register file ]
    //[ to get ready for vhist                       ]
    //[----------------------------------------------]
    { V3:2 = V1:0                                   //
      V5:4 = V1:0                                   //
      P0 = CMP.GT(R2, #0)                           // P0 = (width/VLEN > 0)
      P1 = CMP.EQ(R5, #0)                           // P1 = (width%VLEN==0)
    }
    { Q0 = VSETQ(R5)                                //
      V7:6 = V1:0                                   //
    }
    { V9:8   = V1:0                                 //
      V11:10 = V1:0                                 //
    }
    { V13:12 = V1:0                                 //
      V15:14 = V1:0                                 //
    }
    { V17:16 = V1:0                                 //
      V19:18 = V1:0                                 //
    }
    { V21:20 = V1:0                                 //
      V23:22 = V1:0                                 //
    }
    { V25:24 = V1:0                                 //
      V27:26 = V1:0                                 //
    }
    { V29:28 = V1:0                                 //
      V31:30 = V1:0                                 //
      R10 = ADD(R0,R1)                              // R10 = &src[2*stride]
      LOOP1(.histogram_verticalLoop,R3)             // set loop1 with lc1=height
    }
    //[----------------------------------------------]
    //[ Step 2: vhist                                ]
    //[----------------------------------------------]

    .falign
.histogram_verticalLoop:
    { IF !P0 JUMP .histogramLPEND                   //
      LOOP0(.histogramLoop, R2)                     //
      NOP; NOP                                      //
    }
    .falign
.histogramLoop:
    { V0.tmp = VMEM(R0++#1)                         //
      VHIST                                         //
      NOP; NOP                                      //
    }:endloop0

.histogramLPEND:
    { IF (P1) JUMP .skipRemainder                   // if (width%VLEN==0) done with current row
    }
    { V0.tmp = VMEM(R0+#0)                          // otherwise process the rest pixels
      VHIST(Q0)                                     //
    }
    .falign
.skipRemainder:
    { R0 = R10                                      // set R0  = &src[(i+1)*stride]
      R10 = ADD(R10,R1)                             // update R10 = &src[(i+2)*stride]
    }:endloop1

    //[----------------------------------------------]
    //[ Step 3:                                      ]
    //[  - Sum up the data in the register file      ]
    //[  - accumulate to hist[]                      ]
    //[----------------------------------------------]
    { V0.h = VSHUFF(V0.h)                           //
      R10 = ##0x00010001                            //
    }
    { V1.h = VSHUFF(V1.h)                           //
    }
    { V2.h = VSHUFF(V2.h)                           //
      V0.w = VDMPY(V0.h, R10.h):sat                 //
    }
    { V3.h = VSHUFF(V3.h)                           //
      V1.w = VDMPY(V1.h, R10.h):sat                 //
    }
    { V4.h = VSHUFF(V4.h)                           //
      V2.w = VDMPY(V2.h, R10.h):sat                 //
    }
    { V5.h = VSHUFF(V5.h)                           //
      V3.w = VDMPY(V3.h, R10.h):sat                 //
    }
    { V6.h = VSHUFF(V6.h)                           //
      V4.w = VDMPY(V4.h, R10.h):sat                 //
    }
    { V7.h = VSHUFF(V7.h)                           //
      V5.w = VDMPY(V5.h, R10.h):sat                 //
    }
    { V8.h = VSHUFF(V8.h)                           //
      V6.w = VDMPY(V6.h, R10.h):sat                 //
    }
    { V9.h = VSHUFF(V9.h)                           //
      V7.w = VDMPY(V7.h, R10.h):sat                 //
    }
    { V10.h = VSHUFF(V10.h)                         //
      V8.w = VDMPY(V8.h, R10.h):sat                 //
    }
    { V11.h = VSHUFF(V11.h)                         //
      V9.w = VDMPY(V9.h, R10.h):sat                 //
    }
    { V12.h = VSHUFF(V12.h)                         //
      V10.w = VDMPY(V10.h, R10.h):sat               //
    }
    { V13.h = VSHUFF(V13.h)                         //
      V11.w = VDMPY(V11.h, R10.h):sat               //
    }
    { V14.h = VSHUFF(V14.h)                         //
      V12.w = VDMPY(V12.h, R10.h):sat               //
    }
    { V15.h = VSHUFF(V15.h)                         //
      V13.w = VDMPY(V13.h, R10.h):sat               //
    }
    { V16.h = VSHUFF(V16.h)                         //
      V14.w = VDMPY(V14.h, R10.h):sat               //
    }
    { V17.h = VSHUFF(V17.h)                         //
      V15.w = VDMPY(V15.h, R10.h):sat               //
    }
    { V18.h = VSHUFF(V18.h)                         //
      V16.w = VDMPY(V16.h, R10.h):sat               //
    }
    { V19.h = VSHUFF(V19.h)                         //
      V17.w = VDMPY(V17.h, R10.h):sat               //
    }
    { V20.h = VSHUFF(V20.h)                         //
      V18.w = VDMPY(V18.h, R10.h):sat               //
    }
    { V21.h = VSHUFF(V21.h)                         //
      V19.w = VDMPY(V19.h, R10.h):sat               //
    }
    { V22.h = VSHUFF(V22.h)                         //
      V20.w = VDMPY(V20.h, R10.h):sat               //
    }
    { V23.h = VSHUFF(V23.h)                         //
      V21.w = VDMPY(V21.h, R10.h):sat               //
    }
    { V24.h = VSHUFF(V24.h)                         //
      V22.w = VDMPY(V22.h, R10.h):sat               //
    }
    { V25.h = VSHUFF(V25.h)                         //
      V23.w = VDMPY(V23.h, R10.h):sat               //
    }
    { V26.h = VSHUFF(V26.h)                         //
      V24.w = VDMPY(V24.h, R10.h):sat               //
    }
    { V27.h = VSHUFF(V27.h)                         //
      V25.w = VDMPY(V25.h, R10.h):sat               //
    }
    { V28.h = VSHUFF(V28.h)                         //
      V26.w = VDMPY(V26.h, R10.h):sat               //
    }
    { V29.h = VSHUFF(V29.h)                         //
      V27.w = VDMPY(V27.h, R10.h):sat               //
    }
    { V30.h = VSHUFF(V30.h)                         //
      V28.w = VDMPY(V28.h, R10.h):sat               //
    }
    { V31.h = VSHUFF(V31.h)                         //
      V29.w = VDMPY(V29.h, R10.h):sat               //
      R28 = #32                                     //
    }
    { VSHUFF(V1,V0,R28)                             //
      V30.w = VDMPY(V30.h, R10.h):sat               //
    }
    { VSHUFF(V3,V2,R28)                             //
      V31.w = VDMPY(V31.h, R10.h):sat               //
    }
    { VSHUFF(V5,V4,R28)                             //
      V0.w = VADD(V1.w, V0.w)                       //
      V2.w = VADD(V3.w, V2.w)                       //
    }
#if LOG2VLEN==6
    { VSHUFF(V7,V6,R28)                             //
      V1.tmp = VMEM(R4+#0)                          // update hist[0-15]
      V0.w  = VADD(V0.w, V1.w)                      //
      VMEM(R4++#1) = V0.new                         //
    }
    { VSHUFF(V9,V8,R28)                             //
      V4.w = VADD(V5.w, V4.w)                       //
      V6.w = VADD(V7.w, V6.w)                       //
    }
    { VSHUFF(V11,V10,R28)                           //
      V1.tmp = VMEM(R4+#0)                          // update hist[16-31]
      V2.w  = VADD(V2.w, V1.w)                      //
      VMEM(R4++#1) = V2.new                         //
    }
    { VSHUFF(V13,V12,R28)                           //
      V8.w = VADD(V9.w, V8.w)                       //
      V10.w = VADD(V11.w, V10.w)                    //
    }
    { VSHUFF(V15,V14,R28)                           //
      V1.tmp = VMEM(R4+#0)                          // update hist[32-47]
      V4.w  = VADD(V4.w, V1.w)                      //
      VMEM(R4++#1) = V4.new                         //
    }
    { VSHUFF(V17,V16,R28)                           //
      V12.w = VADD(V13.w, V12.w)                    //
      V14.w = VADD(V15.w, V14.w)                    //
    }
    { VSHUFF(V19,V18,R28)                           //
      V1.tmp = VMEM(R4+#0)                          // update hist[48-63]
      V6.w  = VADD(V6.w, V1.w)                      //
      VMEM(R4++#1) = V6.new                         //
    }
    { VSHUFF(V21,V20,R28)                           //
      V16.w = VADD(V17.w, V16.w)                    //
      V18.w = VADD(V19.w, V18.w)                    //
    }
    { VSHUFF(V23,V22,R28)                           //
      V1.tmp = VMEM(R4+#0)                          // update hist[64-79]
      V8.w  = VADD(V8.w, V1.w)                      //
      VMEM(R4++#1) = V8.new                         //
    }
    { VSHUFF(V25,V24,R28)                           //
      V20.w = VADD(V21.w, V20.w)                    //
      V22.w = VADD(V23.w, V22.w)                    //
    }
    { VSHUFF(V27,V26,R28)                           //
      V1.tmp = VMEM(R4+#0)                          // update hist[80-95]
      V10.w  = VADD(V10.w, V1.w)                    //
      VMEM(R4++#1) = V10.new                        //
    }
    { VSHUFF(V29,V28,R28)                           //
      V24.w = VADD(V25.w, V24.w)                    //
      V26.w = VADD(V27.w, V26.w)                    //
    }
    { VSHUFF(V31,V30,R28)                           //
      V1.tmp = VMEM(R4+#0)                          // update hist[96-111]
      V12.w  = VADD(V12.w, V1.w)                    //
      VMEM(R4++#1) = V12.new                        //
    }
    { V28.w = VADD(V29.w, V28.w)                    //
      V1.tmp = VMEM(R4+#0)                          // update hist[112-127]
      V14.w  = VADD(V14.w, V1.w)                    //
      VMEM(R4++#1) = V14.new                        //
    }
    { V30.w = VADD(V31.w, V30.w)                    //
      V1.tmp = VMEM(R4+#0)                          // update hist[128-143]
      V16.w  = VADD(V16.w, V1.w)                    //
      VMEM(R4++#1) = V16.new                        //
    }
    { V1.tmp = VMEM(R4+#0)                          // update hist[144-159]
      V18.w  = VADD(V18.w, V1.w)                    //
      VMEM(R4++#1) = V18.new                        //
    }
    { V1.tmp = VMEM(R4+#0)                          // update hist[160-175]
      V20.w  = VADD(V20.w, V1.w)                    //
      VMEM(R4++#1) = V20.new                        //
    }
    { V1.tmp = VMEM(R4+#0)                          // update hist[176-191]
      V22.w  = VADD(V22.w, V1.w)                    //
      VMEM(R4++#1) = V22.new                        //
    }
    { V1.tmp = VMEM(R4+#0)                          // update hist[192-207]
      V24.w  = VADD(V24.w, V1.w)                    //
      VMEM(R4++#1) = V24.new                        //
    }
    { V1.tmp = VMEM(R4+#0)                          // update hist[208-223]
      V26.w  = VADD(V26.w, V1.w)                    //
      VMEM(R4++#1) = V26.new                        //
    }
    { V1.tmp = VMEM(R4+#0)                          // update hist[224-239]
      V28.w  = VADD(V28.w, V1.w)                    //
      VMEM(R4++#1) = V28.new                        //
    }
    { V1.tmp = VMEM(R4+#0)                          // update hist[240-255]
      V30.w  = VADD(V30.w, V1.w)                    //
      VMEM(R4++#1) = V30.new                        //
    }
    { JUMPR R31                                     // return
    }
#else
    { VSHUFF(V7,V6,R28)                             //
      R7 = #64                                      //
    }
    { VSHUFF(V9,V8,R28)                             //
      V4.w = VADD(V5.w, V4.w)                       //
      V6.w = VADD(V7.w, V6.w)                       //
    }
    { VSHUFF(V11,V10,R28)                           //
    }
    { VSHUFF(V13,V12,R28)                           //
      V8.w = VADD(V9.w, V8.w)                       //
      V10.w = VADD(V11.w, V10.w)                    //
    }
    { VSHUFF(V15,V14,R28)                           //
    }
    { VSHUFF(V17,V16,R28)                           //
      V12.w = VADD(V13.w, V12.w)                    //
      V14.w = VADD(V15.w, V14.w)                    //
    }
    { VSHUFF(V19,V18,R28)                           //
    }
    { VSHUFF(V21,V20,R28)                           //
      V16.w = VADD(V17.w, V16.w)                    //
      V18.w = VADD(V19.w, V18.w)                    //
    }
    { VSHUFF(V23,V22,R28)                           //
    }
    { VSHUFF(V25,V24,R28)                           //
      V20.w = VADD(V21.w, V20.w)                    //
      V22.w = VADD(V23.w, V22.w)                    //
    }
    { VSHUFF(V27,V26,R28)                           //
    }
    { VSHUFF(V29,V28,R28)                           //
      V24.w = VADD(V25.w, V24.w)                    //
      V26.w = VADD(V27.w, V26.w)                    //
    }
    { VSHUFF(V31,V30,R28)                           //
    }
    { V28.w = VADD(V29.w, V28.w)                    //
      VSHUFF(V2, V0,R7)                             //
    }
    { V30.w = VADD(V31.w, V30.w)                    //
      VSHUFF(V6, V4,R7)                             //
      V0.w  = VADD(V0.w, V2.w)                      //
    }
    { VSHUFF(V10,V8,R7)                             //
      V1.tmp = VMEM(R4+#0)                          // update hist[0-15]
      V0.w  = VADD(V0.w, V1.w)                      //
      VMEM(R4++#1) = V0.new                         //
    }
    { VSHUFF(V14,V12,R7)                            //
      V4.w  = VADD(V4.w, V6.w)                      //
      V8.w  = VADD(V8.w, V10.w)                     //
    }
    { VSHUFF(V18,V16,R7)                            //
      V1.tmp = VMEM(R4+#0)                          // update hist[16-31]
      V4.w  = VADD(V4.w, V1.w)                      //
      VMEM(R4++#1) = V4.new                         //
    }
    { VSHUFF(V22,V20,R7)                            //
      V12.w = VADD(V12.w,V14.w)                     //
      V16.w = VADD(V16.w,V18.w)                     //
    }
    { VSHUFF(V26,V24,R7)                            //
      V1.tmp = VMEM(R4+#0)                          // update hist[32-47]
      V8.w  = VADD(V8.w, V1.w)                      //
      VMEM(R4++#1) = V8.new                         //
    }
    { VSHUFF(V30,V28,R7)                            //
      V1.tmp = VMEM(R4+#0)                          // update hist[48-63]
      V12.w  = VADD(V12.w, V1.w)                    //
      VMEM(R4++#1) = V12.new                        //
    }

    { V20.w = VADD(V20.w,V22.w)                     //
      V1.tmp = VMEM(R4+#0)                          // update hist[64-79]
      V16.w  = VADD(V16.w, V1.w)                    //
      VMEM(R4++#1) = V16.new                        //
    }
    { V24.w = VADD(V24.w,V26.w)                     //
      V1.tmp = VMEM(R4+#0)                          // update hist[80-95]
      V20.w  = VADD(V20.w, V1.w)                    //
      VMEM(R4++#1) = V20.new                        //
    }
    { V28.w = VADD(V28.w,V30.w)                     //
      V1.tmp = VMEM(R4+#0)                          // update hist[96-111]
      V24.w  = VADD(V24.w, V1.w)                    //
      VMEM(R4++#1) = V24.new                        //
    }
    { V1.tmp = VMEM(R4+#0)                          // update hist[112-127]
      V28.w  = VADD(V28.w, V1.w)                    //
      VMEM(R4++#1) = V28.new                        //
    }
    { JUMPR R31                                     // return
    }
#endif
    .size   histogramPernRow, .-histogramPernRow



/* ============================================================ */
    .p2align 2
    .p2align 4,,15
    .globl  clearHistogram
    .type   clearHistogram, @function
clearHistogram:
    { LOOP0(.clearHistogram_Loop,#256*4/VLEN)       //
      V0 = #0                                       //
    }
    .falign
.clearHistogram_Loop:
    { VMEM(R0++#1) = V0                             //hist[i] = 0
    }:endloop0

    { JUMPR R31                                     // return
    }
    .size   clearHistogram, .-clearHistogram


