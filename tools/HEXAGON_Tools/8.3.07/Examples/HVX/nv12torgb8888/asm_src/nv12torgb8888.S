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
    .file    "nv12torgb8888.S"

#include "hvx.cfg.h"
    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void color_NV12toRGB8888_line()                               ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: delta vertical computation initialization                     ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned char* yuv420sp -- pointer to input image        ]*/
    /*[               R1 : unsigned char* uv420sp  -- pointer to input uv image     ]*/
    /*[               R2 : unsigned char *rgb      -- pointer to output buffer      ]*/
    /*[               R3 : int width               -- width                         ]*/
    /*[               R4 : int stride              -- stride                        ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
    /*[           -                                                                 ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  ASSUMPTIONS:                                                               ]*/
    /*[           - yuv420sp, rgb, and uv420sp are aligned by VLEN                  ]*/
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
#define     sY0            V0
#define     sY1            V1
#define     sUv            V2
#define     sV_833u_400    V3
#define     sY0x2_L        V4
#define     sY0x2_H        V5
#define     dY0x2          V5:4
#define     sY1x2_L        V6
#define     sY1x2_H        V7
#define     dY1x2          V7:6
#define     sUvx2_L        V8
#define     sUvx2_H        V9
#define     dUvx2          V9:8
#define     sY1192a_L      V10
#define     sY1192a_H      V11
#define     dY1192a        V11:10
#define     sY1192b_L      sY0x2_L
#define     sY1192b_H      sY0x2_H
#define     dY1192b        dY0x2
#define     sV1634         V12
#define     sU2066         V13
#define     dU2066v1634    V13:12
#define     sRE            V14
#define     sRO            V15
#define     sGE            V16
#define     sGO            V17
#define     sBE            V18
#define     sBO            V19
#define     sR2E           sY0x2_L
#define     sR2O           sY0x2_H
#define     sG2E           sY1x2_L
#define     sG2O           sY1x2_H
#define     sB2E           sUvx2_L
#define     sB2O           sUvx2_H
#define     sIffG          V20
#define     sIBR           V21
#define     sIBR2          sY1x2_L
#define     dIffBGR        V21:20
#define     sIffBGR_L      sIffG
#define     sIffBGR_H      sIBR
#define     dIffBGR2       dY0x2
#define     sIffBGR2_L     sY0x2_L
#define     sIffBGR2_H     sY0x2_H
#define     sR             V22
#define     sG             V23
#define     sB             V24
#define     sConst0xff     V25
#define     sY1192c_L      V26
#define     sY1192c_H      V27
#define     dY1192c        V27:26
#define     sConst128      V30
#define     sConst16       V31
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl color_NV12toRGB8888_line
    .type       color_NV12toRGB8888_line, @function
color_NV12toRGB8888_line:
    { R6 = ##0x808000ff                             //
      R10 = ##((16<<24)+(16<<16)+1192)              //
    }
    { R7:6 = PACKHL(R6,R6)                          //
      R11:10 = PACKHL(R10,R10)                      //
      R5 = ADD(R0,R4)                               // sY1
      R8 = ADD(R3,#VLEN-1)                          // width+VLEN-1
    }
    { sConst0xff = VSPLAT(R6)                       //
      sConst128 = VSPLAT(R7)                        //
      R9 = ##((2066<<16)+1634)                      // 2066|1634
    }
    { sConst16 = VSPLAT(R11)                        //
      R7:6 = COMBINE(#-1,#10)                       //
      R8 = ASR(R8,#LOG2VLEN)                        //
    }
    { R8 = ##((-400<<16)+((-833)&0xffff))           // -400|-833
      P3 = SP1LOOP0(.color_NV12toRGB8888_LP,R8)     //
      R11 = ADDASL(R2,R4,#2)                        // rgb1
    }

    .falign
.color_NV12toRGB8888_LP:
    { sUv.tmp = VMEM(R1++#1)                        //[1]
      sUv.b = VSHUFF(sUv.b)                         //[1]
      sBO.w = VADD(sY1192a_H.w,sU2066.w)            //[2]
      sR.uh = VASR(sRO.w,sRE.w,R6):sat              //[2]
    }
    { sY0.tmp = VMEM(R0++#1)                        //[1]
      sY0.ub = VSUB(sY0.ub,sConst16.ub):sat         //[1]
      dIffBGR = VSHUFF(sIffG,sIBR,R7)               //[2]
    }
    { dUvx2.h = VSUB(sUv.ub,sConst128.ub)           //[1]
      IF P3 VMEM(R11++#1) = sIffBGR2_H              //[2]
      sBE.w = VADD(sY1192a_L.w,sU2066.w)            //[2]
    }
    { dY0x2.uh = VUNPACK(sY0.ub)                    //[1]
      sY1.tmp = VMEM(R5++#1)                        //[1]
      sY1.ub = VSUB(sY1.ub,sConst16.ub):sat         //[1]
    }
    { sV_833u_400.w = VDMPY(sUvx2_L.h,R8.h):sat     //[1]
      IF P3 VMEM(R2++#1) = sIffBGR_L                //[2]
      sB.uh = VASR(sBO.w,sBE.w,R6):sat              //[2]
    }
    { dY1x2.uh = VUNPACK(sY1.ub)                    //[1]
      dY1192a.uw = VMPY(sY0x2_L.uh,R10.uh)          //[1]
    }
    { IF P3 VMEM(R2++#1) = sIffBGR_H                //[2]
      sIffG.ub = VSAT(sConst0xff.h,sG.h)            //[2]
    }
    { dU2066v1634.w = VMPY(sUvx2_L.h,R9.h)          //[1]
      sGE.w = VADD(sY1192a_L.w,sV_833u_400.w)       //[1]
      sIBR.ub = VSAT(sB.h,sR.h)                     //[2]
    }
    { sGO.w = VADD(sY1192a_H.w,sV_833u_400.w)       //[1]
      sRE.w = VADD(sY1192a_L.w,sV1634.w)            //[1]
      sRO.w = VADD(sY1192a_H.w,sV1634.w)            //[1]
      sBE.w = VADD(sY1192a_L.w,sU2066.w)            //[1]
    }
    { dY1192c.uw = VMPY(sY1x2_L.uh,R10.uh)          //[1]
      dIffBGR = VSHUFF(sIffG,sIBR,R7)               //[2]
    }
    { sBO.w = VADD(sY1192a_H.w,sU2066.w)            //[1]
      sG.uh = VASR(sGO.w,sGE.w,R6):sat              //[1]
      sGE.w = VADD(sY1192c_L.w,sV_833u_400.w)       //[1]
      IF P3 VMEM(R11++#1) = sIffBGR_L               //[2]
    }
    { sR.uh = VASR(sRO.w,sRE.w,R6):sat              //[1]
      sGO.w = VADD(sY1192c_H.w,sV_833u_400.w)       //[1]
      sRE.w = VADD(sY1192c_L.w,sV1634.w)            //[1]
      IF P3 VMEM(R11++#1) = sIffBGR_H               //[2]
    }
    { sB.uh = VASR(sBO.w,sBE.w,R6):sat              //[1]
      sRO.w = VADD(sY1192c_H.w,sV1634.w)            //[1]
      sV_833u_400.w = VDMPY(sUvx2_H.h,R8.h):sat     //[1]
    }
    { sIffG.ub = VSAT(sConst0xff.h,sG.h)            //[1]
      sBE.w = VADD(sY1192c_L.w,sU2066.w)            //[1]
      dY1192b.uw = VMPY(sY0x2_H.uh,R10.uh)          //[1]
    }
    { sIBR.ub = VSAT(sB.h,sR.h)                     //[1]
      sBO.w = VADD(sY1192c_H.w,sU2066.w)            //[1]
      dU2066v1634.w = VMPY(sUvx2_H.h,R9.h)          //[1]
    }
    { sG.uh = VASR(sGO.w,sGE.w,R6):sat              //[1]
      sGE.w = VADD(sY1192b_L.w,sV_833u_400.w)       //[1]
      dY1192a.uw = VMPY(sY1x2_H.uh,R10.uh)          //[1]
    }
    { sR.uh = VASR(sRO.w,sRE.w,R6):sat              //[1]
      sGO.w = VADD(sY1192b_H.w,sV_833u_400.w)       //[1]
      sRE.w = VADD(sY1192b_L.w,sV1634.w)            //[1]
      sG.h = VMIN(sG.h,sConst0xff.h)                //[1]
    }
    { sB.uh = VASR(sBO.w,sBE.w,R6):sat              //[1]
      sRO.w = VADD(sY1192b_H.w,sV1634.w)            //[1]
      sBE.w = VADD(sY1192b_L.w,sU2066.w)            //[1]
      sR.h = VMIN(sR.h,sConst0xff.h)                //[1]
    }
    { dIffBGR = VSHUFF(sIffG,sIBR,R7)               //[1]
      sB.h = VMIN(sB.h,sConst0xff.h)                //[1]
      sBO.w = VADD(sY1192b_H.w,sU2066.w)            //[1]
    }
    { VMEM(R2++#1) = sIffBGR_L                      //[1]
      sIffG.b = VSHUFFE(sConst0xff.b,sG.b)          //[1]
      sIBR2.b = VSHUFFE(sB.b,sR.b)                  //[1]
      sG.uh = VASR(sGO.w,sGE.w,R6):sat              //[1]
    }
    { sR.uh = VASR(sRO.w,sRE.w,R6):sat              //[1]
      sGE.w = VADD(sY1192a_L.w,sV_833u_400.w)       //[1]
      sGO.w = VADD(sY1192a_H.w,sV_833u_400.w)       //[1]
      VMEM(R2++#1) = sIffBGR_H                      //[1]
    }
    { sG.h = VMIN(sG.h,sConst0xff.h)                //[1]
      sR.h = VMIN(sR.h,sConst0xff.h)                //[1]
      sB.uh = VASR(sBO.w,sBE.w,R6):sat              //[1]
      sRE.w = VADD(sY1192a_L.w,sV1634.w)            //[1]
    }
    { dIffBGR2 = VSHUFF(sIffG,sIBR2,R7)             //[1]
      sB.h = VMIN(sB.h,sConst0xff.h)                //[1]
      sRO.w = VADD(sY1192a_H.w,sV1634.w)            //[1]
    }
    { VMEM(R11++#1) = sIffBGR2_L                    //[1]
      sIffG.b = VSHUFFE(sConst0xff.b,sG.b)          //[1]
      sIBR.b = VSHUFFE(sB.b,sR.b)                   //[1]
      sG.uh = VASR(sGO.w,sGE.w,R6):sat              //[1]
    }:endloop0
    { VMEM(R11++#1) = sIffBGR2_H                    //[2]
      sBE.w = VADD(sY1192a_L.w,sU2066.w)            //[2]
      sBO.w = VADD(sY1192a_H.w,sU2066.w)            //[2]
      sR.uh = VASR(sRO.w,sRE.w,R6):sat              //[2]
    }
    { dIffBGR = VSHUFF(sIffG,sIBR,R7)               //[2]
    }
    { VMEM(R2++#1) = sIffBGR_L                      //[2]
      sB.uh = VASR(sBO.w,sBE.w,R6):sat              //[2]
    }
    { VMEM(R2++#1) = sIffBGR_H                      //[2]
      sIffG.ub = VSAT(sConst0xff.h,sG.h)            //[2]
    }
    { sIBR.ub = VSAT(sB.h,sR.h)                     //[2]
    }
    { dIffBGR = VSHUFF(sIffG,sIBR,R7)               //[2]
    }
    { VMEM(R11++#1) = sIffBGR_L                     //[2]
    }
    { VMEM(R11++#1) = sIffBGR_H                     //[2]
    }
    { JUMPR R31                                     //
    }
    .size       color_NV12toRGB8888_line, .-color_NV12toRGB8888_line
