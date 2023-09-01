#define VLEN        128
#define LOG2VLEN    7

    .data
    .p2align 7
    .type   invLUT, @object
    .size   invLUT, 256
invLUT:
    .hword 0,16384,8192,5461,4096,3276,2730,2340,2048,1820,1638,1489,1365,1260,1170,1092
    .hword 1024,963,910,862,819,780,744,712,682,655,630,606,585,564,546,528
    .hword 512,496,481,468,455,442,431,420,409,399,390,381,372,364,356,348
    .hword 341,334,327,321,315,309,303,297,292,287,282,277,273,268,264,260
    .hword 256,252,248,244,240,237,234,230,227,224,221,218,215,212,210,207
    .hword 204,202,199,197,195,192,190,188,186,184,182,180,178,176,174,172
    .hword 170,168,167,165,163,162,160,159,157,156,154,153,151,150,148,147
    .hword 146,144,143,142,141,140,138,137,136,135,134,133,132,131,130,129

    /*[*****************************************************************************]*/
    /*[  Function   : void epsilonFiltPerRow                                        ]*/
    /*[*****************************************************************************]*/
    /*[  Description:                                                               ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           -                                                                 ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : uint8_t  *src                                            ]*/
    /*[               R1 : int       stride                                         ]*/
    /*[               R2 : int       width                                          ]*/
    /*[               R3 : int       threshold                                      ]*/
    /*[               R4 : uint8_t  *dst                                            ]*/
    /*[=============================================================================]*/
    /*[  Register Usage:                                                            ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes):                                   ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           -                                                                 ]*/
    /*[*****************************************************************************]*/
#if LOG2VLEN ==7
#define centr           V0
#define prv             V1
#define cur             V2
#define nxt             V3
#define thres           V4
#define cnt             V5
#define SUM_L           V6
#define SUM_H           V7
#define SUM             V7:6
#define x_1             V8
#define x1              V9
#define x_2             V10
#define x2              V11
#define ad0             V12
#define ad1             V13
#define xt0             V14
#define xt1             V15
#define XT01            V15:14
#define zero            V16
#define one             V17
#define inc             V18
#define sumOUT_L        V20
#define sumOUT_H        V21
#define sumOUT          V21:20
#define cntOUT_L        V22
#define cntOUT_H        V23
#define cntOUT          V23:22
#define sinvTab0        V24
#define sinvTab1        V25
#define sInv0           V26
#define sInv1           V27
#define dInv            V27:26
#define Sum0_t          V28
#define Sum1_t          V29
#define sProd0L         V0
#define sProd0H         V1
#define dProd0          V1:0
#define sProd1L         V2
#define sProd1H         V3
#define dProd1          V3:2
#define x_3             x_1
#define x3              x1
#define x_4             x_2
#define x4              x2
#define ad2             ad0
#define ad3             ad1
#define xt2             xt0
#define xt3             xt1
#define XT23            XT01
#define outh0           sProd0L
#define outh1           sProd1L
#define out             outh0
//==================================================
#define stride          R1
#define width           R2
#define threshold       R3
#define optr            R4
#define const5          R3
#define gidx            R5
#define const14         R6
#define rConstc         R7
#define nextline        R8
#define iptrCentr       R10
#define rOneu8          R15
#define iptrCur         R0
#define iptrPrv         R1
#define iptrNxt         R2
//==================================================
    .text
    .p2align 2
    .p2align 4,,15
    .globl epsilonFiltPerRow
    .type	epsilonFiltPerRow, @function
epsilonFiltPerRow:
    { M0 = stride                                   //
      iptrCentr = R0                                //
      R0 -= ASL(stride,#2)                          //
      R28 = ADD(width,#(VLEN-1))                    //
    }
    { R28 = LSR(R28,#LOG2VLEN)                      // ceil(width/VLEN)
      nextline = ADD(R0,#VLEN)                      //
      threshold = VSPLATB(threshold)                //
      zero = #0                                     //
    }
    { rOneu8 = ##0x01010101                         //
      thres = VSPLAT(threshold)                     //
      const14 = #14                                 //
    }
    { one = VSPLAT(rOneu8)                          //
      LOOP1(.epsilonFiltLOOP,R28)                   // set loop with lc0=ceil(width/VLEN)
    }{
      R14 = ADD(PC,##invLUT@PCREL)                  //
    }
    { V0.tmp = VMEM(R14+#1)                         //
      sinvTab1.h = VSHUFF(v0.h)                     //
      const5 = #5                                   //
      P3 = CMP.EQ(R0,R0)                            // set P3
    }
    { V0.tmp = VMEM(R14+#0)                         //
      sinvTab0.h = VSHUFF(v0.h)                     //
      inc     = #0                                  //
      rConstc = #0                                  //
    }

    .falign
.epsilonFiltLOOP:
    { dInv.h|= VLUT16(cnt.b,sinvTab1.h,gidx)        //[O2]
      iptrPrv = ADD(iptrCur,#-VLEN)                 //
      iptrNxt = ADD(iptrCur,# VLEN)                 //
      P0 = CMP.GT(R28,#1)                           //
    }
    { dInv.h|= VLUT16(cnt.b,sinvTab1.h,const5)      //[O2]
      IF  P3 iptrPrv = iptrCur                      //
      IF !P0 iptrNxt = iptrCur                      //
      centr = VMEM(iptrCentr++#1)                   //
    }
    { SUM_L = #0                                    //
      SUM_H = #0                                    //
      cnt   = #0                                    //
      LOOP0(.epsilonFilt_innerLOOP,#9)              //
    }

    .falign
.epsilonFilt_innerLOOP:
    { prv = VMEM(iptrPrv++M0)                       //[1]
      IF (!Q1) cnt.b += inc.b                       //[2]
      Q0 = VCMP.GT(ad2.ub,thres.ub)                 //[2]
      Q1 = VCMP.GT(ad3.ub,thres.ub)                 //[2]
    }
    { cur = VMEM(iptrCur++M0)                       //[1]
      SUM.h += VMPA(XT01.ub,rConstc.b)              //[2]
      IF (!Q0) cnt.b += inc.b                       //[2]
    }
    { nxt = VMEM(iptrNxt++M0)                       //[1]
      xt2 = VMUX(Q0,zero,x_4)                       //[2]
      xt3 = VMUX(Q1,zero,x4)                        //[2]
    } 
    { x_1 = VLALIGN(cur,prv,#1)                     //[1]
      ad3.ub = VABSDIFF(cur.ub,centr.ub)            //[1]
      inc = one                                     //[1]
      if (!Q1) cnt.b += inc.b                       //[2]
    }
    { x1 = VALIGN(nxt,cur,#1)                       //[1]
      Q1 = VCMP.GT(ad3.ub,thres.ub)                 //[1]
      rConstc = rOneu8                              //[1]
      SUM.h += VMPA(XT23.ub,rConstc.b)              //[2]
    }
    { x_2 = VLALIGN(cur,prv,#2)                     //[1]
      ad0.ub = VABSDIFF(x_1.ub,centr.ub)            //[1]
      IF (!Q1) cnt.b += inc.b                       //[1]
      xt3 = VMUX(Q1,zero,cur)                       //[1]
    }
    { x2 = VALIGN(nxt,cur,#2)                       //[1]
      ad1.ub = VABSDIFF(x1.ub,centr.ub)             //[1]
      Q0 = VCMP.GT(ad0.ub,thres.ub)                 //[1]
    }
    { Q1 = VCMP.GT(ad1.ub,thres.ub)                 //[1]
      xt0 = VMUX(Q0,zero,x_1)                       //[1]
      SUM.h += VMPY(xt3.ub,rConstc.b)               //[1]
    }
    { ad2.ub = VABSDIFF(x_2.ub,centr.ub)            //[1]
      ad3.ub = VABSDIFF(x2.ub,centr.ub)             //[1]
      IF (!Q0) cnt.b += inc.b                       //[1]
      xt1 = VMUX(Q1,zero,x1)                        //[1]
    }
    { x_3 = VLALIGN(cur,prv,#3)                     //[1]
      IF (!Q1) cnt.b += inc.b                       //[1]
      Q0 = VCMP.GT(ad2.ub,thres.ub)                 //[1]
      Q1 = VCMP.GT(ad3.ub,thres.ub)                 //[1]
    }
    { x3 = VALIGN(nxt,cur,#3)                       //[1]
      SUM.h += VMPA(XT01.ub,rConstc.b)              //[1]
      xt2 = VMUX(Q0,zero,x_2)                       //[1]
    }
    { x_4 = VLALIGN(cur,prv,#4)                     //[1]
      ad0.ub = VABSDIFF(x_3.ub,centr.ub)            //[1]
      xt3 = VMUX(Q1,zero,x2)                        //[1]
      IF (!Q0) cnt.b += inc.b                       //[1]
    }
    { x4 = VALIGN(nxt,cur,#4)                       //[1]
      ad1.ub = VABSDIFF(x3.ub,centr.ub)             //[1]
      Q0 = VCMP.GT(ad0.ub,thres.ub)                 //[1]
      if (!Q1) cnt.b += inc.b                       //[1]
    }
    { Q1 = VCMP.GT(ad1.ub,thres.ub)                 //[1]
      xt0 = VMUX(Q0,zero,x_3)                       //[1]
      SUM.h += VMPA(XT23.ub,rConstc.b)              //[1]
    }
    { ad2.ub = VABSDIFF(x_4.ub,centr.ub)            //[1]
      ad3.ub = VABSDIFF(x4.ub,centr.ub)             //[1]
      IF (!Q0) cnt.b += inc.b                       //[1]
      xt1 = VMUX(Q1,zero,x3)                        //[1]
    }:endloop0

    { IF (!Q1) cnt.b += inc.b                       //[e]
      Q0 = VCMP.GT(ad2.ub,thres.ub)                 //[e]
      dProd0.uw = VMPY(Sum0_t.uh,sInv0.uh)          //[O2]
      iptrCur = nextline                            //
    }
    { IF (!Q0) cnt.b += inc.b                       //[e]
      Q1 = VCMP.GT(ad3.ub,thres.ub)                 //[e]
      dProd1.uw = VMPY(Sum1_t.uh,sInv1.uh)          //[O2]
      nextline = ADD(nextline,#VLEN)                //
    }
    { IF (!Q1) cnt.b += inc.b                       //[e]
      SUM.h += VMPA(XT01.ub,rConstc.b)              //[e]
      outh0.h = VASR(sProd0H.w,sProd0L.w,const14)   //[O2]
      R28 = ADD(R28,#-1)                            //
    }
    { xt2 = VMUX(Q0,zero,x_4)                       //[e]
      xt3 = VMUX(Q1,zero,x4)                        //[e]
      outh1.h = VASR(sProd1H.w,sProd1L.w,const14)   //[O2]
      gidx = #0                                     //
    }
    { dInv.h = VLUT16(cnt.b,sinvTab0.h,gidx)        //
      gidx = #1                                     //
      out.b = VSHUFFE(outh1.b,outh0.b)              //[O2]
      IF !P3 VMEM(optr++#1) = out.new               //[O2]
    }
    { SUM.h += VMPA(XT23.ub,rConstc.b)              //[e]
      dInv.h|= VLUT16(cnt.b,sinvTab0.h,gidx)        //
      gidx = #2                                     //
      P3 = XOR(P3,P3)                               // clean P3
    }
    { dInv.h|= VLUT16(cnt.b,sinvTab0.h,gidx)        //
      inc = #0                                      //
      rConstc = #0                                  //
      gidx = #3                                     //
    }
    { dInv.h|= VLUT16(cnt.b,sinvTab0.h,gidx)        //
      gidx = #4                                     //
      Sum0_t = SUM_L                                //
      Sum1_t = SUM_H                                //
    }:endloop1

    { dInv.h|= VLUT16(cnt.b,sinvTab1.h,gidx)        //[O2]
    }
    { dInv.h|= VLUT16(cnt.b,sinvTab1.h,const5)      //[O2]
    }
    { dProd0.uw = VMPY(Sum0_t.uh,sInv0.uh)          //[O2]
    }
    { dProd1.uw = VMPY(Sum1_t.uh,sInv1.uh)          //[O2]
    }
    { outh0.h = VASR(sProd0H.w,sProd0L.w,const14)   //[O2]
    }
    { outh1.h = VASR(sProd1H.w,sProd1L.w,const14)   //[O2]
    }
    { out.b = VSHUFFE(outh1.b,outh0.b)              //[O2]
      VMEM(optr+#0) = out.new                       //[O2]
    }
    { JUMPR R31                                     // return
    }
    .size	epsilonFiltPerRow, .-epsilonFiltPerRow
#endif
