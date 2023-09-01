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
    .file    "fast9_det.S"

#include "hvx.cfg.h"
    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void fast9_detect()                                           ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: coarse FAST detection of feature candidates                   ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned char *img     -- pointer to input image         ]*/
    /*[               R1 : unsigned int xsize     -- search size in x-direction     ]*/
    /*[               R2 : unsigned int stride    -- stride of image input          ]*/
    /*[               R3 : int barrier            -- barrier                        ]*/
    /*[               R4 : unsigned int *bitmask  -- pointer to output              ]*/
    /*[               R5 : unsigned int  boundary -- boundary                       ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
    /*[           -                                                                 ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  ASSUMPTIONS:                                                               ]*/
    /*[           - img is aligned by vector size                                   ]*/
    /*[           - stride is a multiple of vector size                             ]*/
    /*[           - bitmaks start from  boundary&(-VLEN)                            ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  REVISION HISTORY                                                           ]*/
    /*[  ----------------                                                           ]*/
    /*[  Version        Date                    Comments                            ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[   6.0.0         10/23/2015              created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define SS                  (2*8)
#define img                 R0
#define xsize               R1
#define stride              R2
#define barrier             R3
#define lc1                 barrier
#define bitmask             R4
#define boundary            R5
#define const4              R6
#define const6              R7
#define lc0                 R9
#define niter               R10
#define iptrCn              R0
#define iptrUp              R8
#define iptrDn              stride
#define stridex3            R11
#define idx0                R12
#define idx1                R13
#define idx2                R14
#define idx3                R15
#define idx4                R16
#define idx5                R17
#define idx6                R18
#define idx7                R19
#define idx                 R28
/* ============================================================ */
#define sCb                 V0
#define sCn                 sCb
#define sRightBoundry       sCb
#define sBr8_L              sCb
#define sBrS8               sCb
#define sC_b                V1
#define sBitMaskt1          sC_b
#define sBr8_H              sC_b
#define sBrS15              sC_b
#define dBr8                sC_b:0
#define sBrBitL             V2
#define sBrBitH             V3
#define sBr15_L             sBrBitL
#define sBr15_H             sBrBitH
#define dBr15               sBrBitH:2
#define sDkBitL             V4
#define sDkBitH             V5
#define sDk15_L             sDkBitL
#define sDk15_H             sDkBitH
#define dDk15               sDkBitH:4
#define sBarrier            V6
#define sBitMask            V7
#define sCn0                V8
#define sPixel12            sCn0
#define sUp30               V9
#define sUp20               V10
#define sUp10               V11
#define sPixel13            sUp10
#define sDn10               V12
#define sPixel11            sDn10
#define sDn20               V13
#define sDn30               V14
#define sCn1                V15
#define sUp31               V18
#define sUp21               sCn1
#define sUp11               sCn1
#define sDn11               sCn1
#define sDn21               sCn1
#define sBitMaskt0           sCn1
#define sDn31               sUp31
#define sPixel04            V28
#define sPixel15            sPixel04
#define sPixel14            sPixel04
#define sPixel03            sPixel04
#define sPixel05            sPixel04
#define sPixel10            sPixel04
#define sPixel09            sPixel04
#define sPixel00            V16
#define sPixel02            sPixel00
#define sPixel06            sPixel00
#define sPixel08            sPixel00
#define sDkBitLt            sPixel00
#define sDk8_L              sPixel00
#define sPixel01            V17
#define sDk8_H              sPixel01
#define sPixel07            sPixel01
#define sBrBit01            sCn1
#define sBrBit03            sCn1
#define sDkBit01            sUp31
#define sDkBit03            sUp31
#define sBrBit23            V19
#define sBrBit47            sBrBit23
#define sDkBit23            V20
#define sDkBit47            sDkBit23
#define sBrBit47_L          V20
#define sBrBit47_H          V21
#define dBrBit47            V21:20
#define sDkBit47_L          V22
#define sBrBit03t           sDkBit47_L
#define sDkBit07            sDkBit47_L
#define sDkIsCorner         sDkBit47_L
#define sIsCorner           sDkBit47_L
#define sDkBit47_H          V23
#define dDkBit47            V23:22
#define sBrIsCorner         V23
#define sMaskFF             V24
#define sMaskL              V25
#define sBrBit07            sBrIsCorner
#define sDkBit03t           V26
#define sBrBitLt            sPixel04
#define sDkS8               sDkBit03t
#define sDkS15              V27
#define sBr0815             sPixel04
#define sDk0815             sDkBit03t
#define sConst3f            V29
#define sConst0f            V30
#define sZero               V31

/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl fast9_detect
    .type    fast9_detect, @function
fast9_detect:
    { R29 = ADD(R29,#-SS)                           //
      MEMD(R29+#(0*8-SS)) = R17:16                  //
      MEMD(R29+#(1*8-SS)) = R19:18                  //
      stridex3 = MPYI(stride,#3)                    // 3*stride
    }
    { iptrUp = SUB(iptrCn,stridex3)                 //
      iptrDn = ADD(iptrCn,stride)                   //
      M0 = stride                                   //
      R7 = SUB(stride, stridex3)                    //
    }
    { idx0 = ##0x1010101                            //
      xsize -= ADD(boundary,#3)                     // xsize - boundary - 3
      boundary = ADD(boundary,#-3)                  //
    }
    { R7 = ADD(R7,#VLEN)                            // -2*stride+VLEN
      Q0 = VSETQ(boundary)                          //
      boundary = AND(boundary,#-VLEN)               //
    }
    { sMaskL = VAND(Q0,idx0)                        // (left boundary mask)
      xsize = SUB(xsize,boundary)                   //
      M1 = R7                                       // -2*stride+VLEN
    }
    { sMaskL = VNOT(sMaskL)                         //
      idx1 = ADD(idx0,idx0)                         //
      barrier = VSPLATB(barrier)                    // duplicate barrier
      sCn0 = VMEM(iptrCn++#1)                       //
    }
    { idx2 = ADD(idx1,idx1)                         //
      niter = ADD(xsize,#VLEN-1)                    //
      R7 = ##0x3f3f0f0f                             //
    }
    { idx3 = ADD(idx2,idx2)                         //
      niter = LSR(niter,#LOG2VLEN)                  // niter = ceil(numpixel/VLEN)-1
      R7:6 = PACKHL(R7,R7)                          //
      sUp30 = VMEM(iptrUp++M0)                      //
    }
    { idx4 = ADD(idx3,idx3)                         //
      sConst3f = VSPLAT(R7)                         //
      sConst0f = VSPLAT(R6)                         //
      sUp20 = VMEM(iptrUp++M0)                      //
    }
    { idx5 = ADD(idx4,idx4)                         //
      lc0 = #8                                      //
      lc1 = ADD(niter,#7)                           // lc1 = (niter/8)
      sBarrier = VSPLAT(barrier)                    // barrier
    }
    { lc0 = MIN(lc0,niter)                          // min(niter,8)
      lc1 = ASR(lc1,#3)                             // lc1 = ceil(niter/8)
      xsize = NEG(xsize)                            //
      sZero = #0                                    //
    }
    { idx6 = ADD(idx5,idx5)                         //
      sUp10 = VMEM(iptrUp++M1)                      //
      P1 = CMP.EQ(lc1,#1)                           //
      sMaskFF = VNOT(sZero)                         //
    }
    { sDn10 = VMEM(iptrDn++M0)                      //
      idx7 = ADD(idx6,idx6)                         //
      idx = ADD(idx6,idx6)                          //
      LOOP1(.fast9_detect_LP1,lc1)                  // setup outer loop
    }
    { sDn20 = VMEM(iptrDn++M0)                      //
      LOOP0(.fast9_detect_LP0,lc0)                  //[O]setup inner loop
      niter = SUB(niter,lc0)                        //[O]niter_new = niter-8
      R7:6 = COMBINE(#6,#4)                         //
    }
    { sDn30 = VMEM(iptrDn++M1)                      //
      sDkBit07 = #0                                 //[O]
      sBrBit07 = #0                                 //[O]
      sBitMask = #0                                 //[O]
    }

    .falign
.fast9_detect_LP1:
.fast9_detect_LP0:
    { sCn1.cur = VMEM(iptrCn++#1)                   //[1]
      sCn = VALIGN(sCn1,sCn0,#3)                    //[1]
      sDk0815 = VOR(sDkS8,sDkS15)                   //[2]
      sBrIsCorner = VAND(sBrBit07,sBr0815)          //[2]
    }
    { sC_b.ub = VSUB(sCn.ub,sBarrier.ub):sat        //[1]
      sUp31.cur = VMEM(iptrUp++M0)                  //[1]
      sPixel00 = VALIGN(sUp31,sUp30,#3)             //[1]
      sDkIsCorner = VAND(sDkBit07,sDk0815)          //[2]
    }
    { sCb.ub = VADD(sCn.ub,sBarrier.ub):sat         //[1]
      sPixel04 = VALIGN(sCn1,sCn0,#6)               //[1]
      Q1 = VCMP.GT(sC_b.ub,sPixel12.ub)             //[1]
      sIsCorner = VOR(sBrIsCorner,sDkIsCorner)      //[2]
    }
    { Q0 = VCMP.GT(sPixel12.ub,sCb.ub)              //[1]
      sCn0 = sCn1                                   //[1]
      sDkBitL = VAND(Q1,idx1)                       //[1]
      Q2 = VCMP.GT(sIsCorner.ub,sZero.ub)           //[2]
    }
    { sBrBitL = VAND(Q0,idx1)                       //[1]
      Q2 = VCMP.GT(sPixel04.ub,sCb.ub)              //[1]
      sPixel01 = VALIGN(sUp31,sUp30,#4)             //[1]
      sBitMask |= VAND(Q2,idx)                      //[2]
    }
    { sBrBitL |= VAND(Q2,idx5)                      //[1]
      Q3 = VCMP.GT(sC_b.ub,sPixel04.ub)             //[1]
      sPixel15 = VALIGN(sUp31,sUp30,#2)             //[1]
      sUp30 = sUp31                                 //[1]
    }
    { sDkBitL |= VAND(Q3,idx5)                      //[1]
      Q0 = VCMP.GT(sPixel00.ub,sCb.ub)              //[1]
      Q1 = VCMP.GT(sC_b.ub,sPixel00.ub)             //[1]
      sUp21 = VMEM(iptrUp++M0)                      //[1]
    }
    { sBrBitL |= VAND(Q0,idx7)                      //[1]
      sDkBitL |= VAND(Q1,idx7)                      //[1]
      Q2 = VCMP.GT(sPixel01.ub,sCb.ub)              //[1]
      Q3 = VCMP.GT(sC_b.ub,sPixel01.ub)             //[1]
    }
    { sBrBitH = VAND(Q2,idx7)                       //[1]
      Q0 = VCMP.GT(sPixel15.ub,sCb.ub)              //[1]
      Q1 = VCMP.GT(sC_b.ub,sPixel15.ub)             //[1]
      sPixel14 = VALIGN(sUp21,sUp20,#1)             //[1]
    }
    { sDkBitH = VAND(Q3,idx7)                       //[1]
      sBrBitH |= VAND(Q0,idx0)                      //[1]
      sPixel02 = VALIGN(sUp21,sUp20,#5)             //[1]
      sUp20 = sUp21                                 //[1]
    }
    { sDkBitH |= VAND(Q1,idx0)                      //[1]
      Q2 = VCMP.GT(sPixel14.ub,sCb.ub)              //[1]
      Q3 = VCMP.GT(sC_b.ub,sPixel14.ub)             //[1]
      sUp11 = VMEM(iptrUp++M1)                      //[1]
    }
    { sBrBitL |= VAND(Q2,idx0)                      //[1]
      sDkBitL |= VAND(Q3,idx0)                      //[1]
      Q0 = VCMP.GT(sPixel02.ub,sCb.ub)              //[1]
      Q1 = VCMP.GT(sC_b.ub,sPixel02.ub)             //[1]
    }
    { sBrBitL |= VAND(Q0,idx6)                      //[1]
      sDkBitL |= VAND(Q1,idx6)                      //[1]
      sPixel03 = VALIGN(sUp11,sUp10,#6)             //[1]
      Q0 = VCMP.GT(sPixel13.ub,sCb.ub)              //[1]
    }
    { sUp10 = sUp11                                 //[1]
      Q1 = VCMP.GT(sC_b.ub,sPixel13.ub)             //[1]
      sBrBitH |= VAND(Q0,idx1)                      //[1]
      Q2 = VCMP.GT(sPixel03.ub,sCb.ub)              //[1]
    }
    { sDkBitH |= VAND(Q1,idx1)                      //[1]
      Q3 = VCMP.GT(sC_b.ub,sPixel03.ub)             //[1]
      sDn11.cur = VMEM(iptrDn++M0)                  //[1]
      sPixel05 = VALIGN(sDn11,sDn10,#6)             //[1]
    }
    { sBrBitH |= VAND(Q2,idx6)                      //[1]
      Q0 = VCMP.GT(sPixel11.ub,sCb.ub)              //[1]
      Q1 = VCMP.GT(sC_b.ub,sPixel11.ub)             //[1]
      sDn10 = sDn11                                 //[1]
    }
    { sDkBitH |= VAND(Q3,idx6)                      //[1]
      sBrBitH |= VAND(Q0,idx2)                      //[1]
      sDn21.cur = VMEM(iptrDn++M0)                  //[1]
      sPixel06 = VALIGN(sDn21,sDn20,#5)             //[1]
    }
    { sDkBitH |= VAND(Q1,idx2)                      //[1]
      Q2 = VCMP.GT(sPixel05.ub,sCb.ub)              //[1]
      sDn31.cur = VMEM(iptrDn++M1)                  //[1]
      sPixel07 = VALIGN(sDn31,sDn30,#4)             //[1]
    }
    { Q3 = VCMP.GT(sC_b.ub,sPixel05.ub)             //[1]
      sBrBitH |= VAND(Q2,idx5)                      //[1]
      sPixel10 = VALIGN(sDn21,sDn20,#1)             //[1]
      Q2 = VCMP.GT(sPixel06.ub,sCb.ub)              //[1]
    }
    { sDkBitH |= VAND(Q3,idx5)                      //[1]
      Q0 = VCMP.GT(sPixel10.ub,sCb.ub)              //[1]
      Q1 = VCMP.GT(sC_b.ub,sPixel10.ub)             //[1]
      sPixel09 = VALIGN(sDn31,sDn30,#2)             //[1]
    }
    { Q3 = VCMP.GT(sC_b.ub,sPixel06.ub)             //[1]
      sPixel08 = VALIGN(sDn31,sDn30,#3)             //[1]
      sBrBitL |= VAND(Q0,idx2)                      //[1]
      sDkBitL |= VAND(Q1,idx2)                      //[1]
    }
    { Q0 = VCMP.GT(sPixel09.ub,sCb.ub)              //[1]
      Q1 = VCMP.GT(sC_b.ub,sPixel09.ub)             //[1]
      sBrBitL |= VAND(Q2,idx4)                      //[1]
      sDkBitL |= VAND(Q3,idx4)                      //[1]
    }
    { Q2 = VCMP.GT(sPixel08.ub,sCb.ub)              //[1]
      Q3 = VCMP.GT(sC_b.ub,sPixel08.ub)             //[1]
      sBrBitH |= VAND(Q0,idx3)                      //[1]
      sDkBitH |= VAND(Q1,idx3)                      //[1]
    }
    { Q0 = VCMP.GT(sPixel07.ub,sCb.ub)              //[1]
      Q1 = VCMP.GT(sC_b.ub,sPixel07.ub)             //[1]
      sBrBitL |= VAND(Q2,idx3)                      //[1]
      sDkBitL |= VAND(Q3,idx3)                      //[1]
    }
    { sBrBitH |= VAND(Q0,idx4)                      //[1]
      sDkBitH |= VAND(Q1,idx4)                      //[1]
      sDn20 = sDn21                                 //[1]
      sDn30 = sDn31                                 //[1]
    }
    { sBrBit01 = VAND(sBrBitH,sBrBitL)              //[1]
      sDkBit01 = VAND(sDkBitH,sDkBitL)              //[1]
      sBrBitLt = VAND(sBrBitL,sConst0f)             //[1]
      sDkBitLt = VAND(sDkBitL,sConst0f)             //[1]
    }
    { sBrBit23.b = VADD(sBrBit01.b,sBrBit01.b)      //[1]
      sDkBit23.b = VADD(sDkBit01.b,sDkBit01.b)      //[1]
      Q2 = VCMP.GT(sZero.b,sBrBit01.b)              //[1]
      Q3 = VCMP.GT(sZero.b,sDkBit01.b)              //[1]
    }
    { sBrBit23 |= VAND(Q2,idx0)                     //[1]
      sDkBit23 |= VAND(Q3,idx0)                     //[1]
      dBr8.b = VSHUFFOE(sBrBitLt.b,sBrBitL.b)       //[1]
    }
    { sBrBit03 = VAND(sBrBit23,sBrBit01)            //[1]
      sDkBit03 = VAND(sDkBit23,sDkBit01)            //[1]
      dBr15.b = VSHUFFOE(sBrBitH.b,sBrBitH.b)       //[1]
    }
    { sBr15_L.h = VAVG(sBr15_L.h,sZero.h)           //[1]
      sBrBit03t = VAND(sBrBit03,sConst3f)           //[1]
      sBrS8.ub = VASR(sBr8_H.h,sBr8_L.h,const4):sat //[1]
      sDkBit03t = VAND(sDkBit03,sConst3f)           //[1]
    }
    { dBrBit47.b = VSHUFFOE(sBrBit03t.b,sBrBit03.b) //[1]
      sBr15_H.h = VAVG(sBr15_H.h,sZero.h)           //[1]
      sDk8_H.b = VSHUFFO(sDkBitLt.b,sDkBitL.b)      //[1]
    }
    { dDkBit47.b = VSHUFFOE(sDkBit03t.b,sDkBit03.b) //[1]
      sBrS15.b = VSHUFFE(sBr15_H.b,sBr15_L.b)       //[1]
      sDk8_L.b = VSHUFFE(sDkBitLt.b,sDkBitL.b)      //[1]
    }
    { sBrBit47.ub = VASR(sBrBit47_H.h,sBrBit47_L.h,const6):sat//[1]
      sBr0815 = VOR(sBrS8,sBrS15)                   //[1]
      dDk15.b = VSHUFFOE(sDkBitH.b,sDkBitH.b)       //[1]
    }
    { sBrBit07 = VAND(sBrBit47,sBrBit03)            //[1]
      sDkBit47.ub = VASR(sDkBit47_H.h,sDkBit47_L.h,const6):sat//[1]
      sDk15_L.h = VAVG(sDk15_L.h,sZero.h)           //[1]
      sDk15_H.h = VAVG(sDk15_H.h,sZero.h)           //[1]
    }
    { sDkBit07 = VAND(sDkBit47,sDkBit03)            //[1]
      sDkS8.ub = VASR(sDk8_H.h,sDk8_L.h,const4):sat //[1]
      idx = ROL(idx,#1)                             //[1]
      sDkS15.b = VSHUFFE(sDk15_H.b,sDk15_L.b)       //[1]
    }:endloop0
    { sDk0815 = VOR(sDkS8,sDkS15)                   //[e]
      sBrIsCorner = VAND(sBrBit07,sBr0815)          //[e]
      P0 = CMP.EQ(lc1,#1)                           // right boundary
      lc1 = ADD(lc1,#-1)                            // right boundary
    }
    { sDkIsCorner = VAND(sDkBit07,sDk0815)          //[e]
      sRightBoundry = VALIGN(sZero,sConst3f,xsize)  // (right boundary mask)
      sBitMaskt0 = sBitMask                         // right boundary
      sBitMaskt1 = sBitMask                         // right boundary
    }
    { sIsCorner = VOR(sBrIsCorner,sDkIsCorner)      //[e]
      lc0 = MIN(lc0,niter)                          // min(LC,8)
      Q0 = VCMP.EQ(sRightBoundry.ub,sConst3f.ub)    // (right boundary mask)
      sBitMask = #0                                 //[O]
    }
    { Q2 = VCMP.GT(sIsCorner.ub,sZero.ub)           //[e]
      niter = SUB(niter,lc0)                        //[O]niter_new = niter-8
      sBrBit07 = #0                                 //[O]
      sDkBit07 = #0                                 //[O]
    }
    { sBitMaskt1 |= VAND(Q2,idx)                    //[e]
      LOOP0(.fast9_detect_LP0,lc0)                  //[O]setup inner loop
      Q2 = AND(Q0,Q2)                               // mask out right boundary
    }
    { sBitMaskt1 = VAND(sBitMaskt1,sMaskL)          //
      IF !P1 sMaskL = sMaskFF                       //
      IF !P0 VMEM(bitmask++#1) = sBitMaskt1.new     //
      sBitMaskt0 |= VAND(Q2,idx)                    // right boundary
    }:endloop1
    { sBitMaskt0 = VAND(sBitMaskt0,sMaskL)          // left boundary if applicable
      VMEM(bitmask+#0) = sBitMaskt0.new             // left boundary if applicable
    }
    { JUMPR R31                                     // return
      R17:16 = MEMD(R29+#0*8)                       //
      R19:18 = MEMD(R29+#1*8)                       //
      R29 = ADD(R29,#SS)                            //
    }
    .size    fast9_detect, .-fast9_detect

    /*[*****************************************************************************]*/
    /*[  FUNCTION   : int fast9_coord()                                             ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: get x positions of corners                                    ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned int *bitmask  -- pointer to masks               ]*/
    /*[               R1 : unsigned int  size     -- # of masks in 32-bit           ]*/
    /*[               R2 : unsigned int  xstart   -- start value of x               ]*/
    /*[               R3 : short        *xpos     -- pointer to output              ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
    /*[           -                                                                 ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  ASSUMPTIONS:                                                               ]*/
    /*[           -                                                                 ]*/
    /*[=============================================================================]*/
    /*[  REVISION HISTORY                                                           ]*/
    /*[  ----------------                                                           ]*/
    /*[  Version        Date                    Comments                            ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[   6.0.0         10/23/2015              created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define NumCorner           R0
#define BaseIndex           R1
#define MaskInput           R12
#define BitMask             R9
#define XStart              R2
#define BitPos              R8
#define XOffset             R10
#define Temp                R11
#define XCoord              R14
    .text
    .p2align 2
    .p2align 4,,15
    .globl fast9_coord
    .type    fast9_coord, @function
fast9_coord:
    { LOOP1(.fast9_coordLOOP,R1)                    // setup loop
      R1:0 = COMBINE(#0,#0)                         // numcorners = 0, IO = 0
      BitMask = MEMW(R0+#0)                         // load mask[k]
      MaskInput = ADD(R0,#4)                        //
    }
    .falign
.fast9_coordLOOP:
    { IF(BitMask!=#0) JUMP:NT .fast9_coord_Check32Entry// load next candidate set if this one's empty
      BitPos = CT0(BitMask)                         // bitpos = ct0(mask[k])
      NOP                                           //
      NOP                                           //
    }
    .falign
.fast9_coord_LPCONT:
    { BitMask = MEMW(MaskInput++#4)                 // load mask[k]
      BaseIndex = ADD(BaseIndex,#32)                // i += 32
    }:endloop1
    { JUMPR R31                                     // return numcorners
    }

    .falign
.fast9_coord_Check32Entry:
    { XOffset = ADD(BaseIndex,BitPos)               //[0]convert i to offset
      BitMask = CLRBIT(BitMask,BitPos)              //[0]clear bit at BITPOS in BITMASK
      P3 = CMP.GT(R0,R0)                            //
    }
    .falign
.fast9_coord_Check32:
    { XOffset = INSERT(XOffset,#3,#LOG2VLEN)        //[1]convert i to offset
      Temp = EXTRACTU(XOffset,#LOG2VLEN,#3)         //[1]convert i to offset
      NumCorner = ADD(NumCorner,#1)                 //[1]numcorners++
      IF P3 MEMH(R3++#2) = XCoord                   //[2]*(xpos++)
    }
    { XOffset = INSERT(Temp,#LOG2VLEN,#0)           //[1]offset to xstart
      BitPos = CT0(BitMask)                         //[1]bitpos = ct0(mask[k])
      P3 = CMP.EQ(R0,R0)                            //
    }
    { XOffset = ADD(BaseIndex,BitPos)               //[0]convert i to offset
      BitMask = CLRBIT(BitMask,BitPos)              //[0]clear bit at BITPOS in BITMASK
      XCoord = ADD(XStart,XOffset)                  //[1]calculate X coordinate of the next candidate
      IF (BitMask!=#0) JUMP:t .fast9_coord_Check32  //[1]if any candidates, continue process
    }
    { MEMH(R3++#2) = XCoord                         //[e]*(xpos++)
      BitMask = MEMW(MaskInput++#4)                 // load mask[k]
      BaseIndex = ADD(BaseIndex,#32)                // i += 32
    }:endloop1
    { JUMPR R31                                     // return numcorners
    }
    .size    fast9_coord, .-fast9_coord

