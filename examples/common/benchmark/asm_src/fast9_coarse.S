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

#define VLEN 128
#define LOG2VLEN 7
    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void fast9_detect_coarse()                                    ]*/
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
    /*[   6.0.0         12/12/2014              created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define img                 R0
#define xsize               R1
#define stride              R2
#define barrier             R3
#define bitmask             R4
#define boundary            R5
#define numpixel            R6
#define lc                  R9
#define niter               R10
#define iptrC               R0
#define iptrUp              R1
#define iptrDn              R2
#define stridex3            stride
#define N_numpixel          numpixel
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl fast9_detect_coarse
    .type	fast9_detect_coarse, @function
fast9_detect_coarse:
    { iptrC = ADD(img,boundary)                     // p = img + boundary
      xsize -= ASL(boundary,#1)                     // xsize - 2*boundary
      R6 = AND(boundary,#VLEN-1)                    // boundary%VLEN
      Q0 = VSETQ(boundary)                          //
    }
    { numpixel = ADD(xsize,R6)                      // numpixel = xsize - 2*boundary +(boundary%/VLEN)
      barrier = VSPLATB(barrier)                    // duplicate barrier
      stridex3 = MPYI(stride,#3)                    // 3*stride 
      V13 = #0                                      // constant 0
    }
    { niter = ADD(numpixel,#VLEN-1-VLEN)            //
      iptrUp = SUB(iptrC,stridex3)                  // &p[pixel[0]]
      iptrDn = ADD(iptrC,stridex3)                  // &p[pixel[8]]
      V2 = VMEM(iptrC+#-1)                          //
    }
    { niter = LSR(niter,#LOG2VLEN)                  // niter = ceil(numpixel/VLEN)-1
      V0 = VMEM(iptrC++#1)                          // load P-
      R12 = ##0x01010101                            // (left  boundary mask)
    }
    { R5 = ASRRND(niter,#3)                         // R5 = ceil(niter/8)
      R9:8 = COMBINE(#8,#-1)                        // lc = 8 (R9), R8 = 0xFF
      V14 = VAND(Q0,R12)                            // (left boundary mask)
      V1 = VMEM(iptrC++#1)                          //[1]load P+
    }
    { V14 = VNOT(V14)                               // V14 = left bounary mask
      V16 = VSPLAT(R8)                              // 0xFF 
      V12 = #0                                      // reset flag-collector
      V6 = VLALIGN(V0,V2,#3)                        //[1]pixels #12
    }
    { P1 = CMP.GT(numpixel,#VLEN)                   // numpixel > VLEN ?
      lc = MIN(lc,niter)                            // min(niter,8)
      V3 = VSPLAT(barrier)                          // barrier
      V4 = VMEM(iptrUp++#1)                         //[1]load pixels #0
    }
    { R12 = ASL(R12,#7)                             // set R12 = 0x80808080  
      LOOP1(.detect_candidate_mainLOOP,R5)          // setup outer loop
      V5 = VMEM(iptrDn++#1)                         //[1]load pixels #8
      V10.ub = VSUB(V0.ub,V3.ub):sat                //[1]c_b = *p - barrier
    }
    { V7 = VALIGN(V1,V0,#3)                         //[1]pixels #4
      V4.ub = VMIN(V4.ub,V5.ub)                     //[1]sort pixel #0, #8
      V5.ub = VMAX(V4.ub,V5.ub)                     //[1]sort pixel #0, #8
      IF !P1 JUMP .detect_candidate_LPSend          // if !(numpixel>VLEN) skip loops
    }

    .falign
.detect_candidate_mainLOOP:
    { LOOP0(.detect_candidate_LOOP,lc)              // setup inner loop
      V12 = #0                                      // reset flag-collector
      niter = SUB(niter,lc)                         // niter_new = niter-8
      P0 = CMP.GT(niter,lc)                         // P0 = niter_new > 0
    }

    .falign
.detect_candidate_LOOP:
    { V6 = VLALIGN(V1,V0,#3)                        //[1]pixels #12
      V8.ub  = VMIN(V6.ub,V7.ub)                    //[2]sort pixel #4, #12
      V9.ub  = VMAX(V6.ub,V7.ub)                    //[2]sort pixel #4, #12
      V11.ub = VADD(V0.ub,V3.ub):sat                //[2]cb  = *p + barrier
    }
    { V1 = VMEM(iptrC++#1)                          //[1]load P+
      V0 = V1                                       //[1]P- = P+
      V9.ub = VMIN(V9.ub,V5.ub)                     //[2]min(max(pixel4,pixel12),max(pixel0,pixel8))
      V8.ub = VMAX(V8.ub,V4.ub)                     //[2]max(min(pixel4,pixel12),min(pixel0,pixel8))
    }
    { V4 = VMEM(iptrUp++#1)                         //[1]load pixels #0
      Q0 = VCMP.GT(V9.ub ,V11.ub)                   //[2]Q0 = any 2 contiguous pixels are brighter?
      Q1 = VCMP.GT(V10.ub,V8.ub )                   //[2]Q1 = any 2 contiguous pixels are darker?
    }
    { V5 = VMEM(iptrDn++#1)                         //[1]load pixels #8
      V10.ub = VSUB(V0.ub,V3.ub):sat                //[1]c_b = *p - barrier
      Q0 = OR(Q0,Q1)                                //[2]Q0 = Is a feature?
      R12 = ROL(R12,#1)                             //[2]update flag index
    }
    { V7 = VALIGN(V1,V0,#3)                         //[1]pixels #4
      V4.ub = VMIN(V4.ub,V5.ub)                     //[1]sort pixel #0, #8
      V5.ub = VMAX(V4.ub,V5.ub)                     //[1]sort pixel #0, #8
      V12 |= VAND(Q0,R12)                           //[2]collect flags 
    }:endloop0

    { V2 = VAND(V12,V14)                            // mask collected flags
      IF P0 VMEM(bitmask++#1) = V2.new              // save flags
      IF P0 V14 = V16                               // set mask = 0xFF
      lc = MIN(lc,niter)                            // min(LC,8)
    }:endloop1

    { P1 = !CMP.GT(R12,#0)                          // P1 = (flag index == 0x80808080)
    }

.detect_candidate_LPSend:
    { V8.ub  = VMIN(V6.ub,V7.ub)                    //[e]sort pixel #4, #12
      V9.ub  = VMAX(V6.ub,V7.ub)                    //[e]sort pixel #4, #12
      V11.ub = VADD(V0.ub,V3.ub):sat                //[e]cb  = *p + barrier
      IF P1 VMEM(bitmask++#1) = V2                  //[e]if P1, save flags
    }
    { V9.ub = VMIN(V9.ub,V5.ub)                     //[e]min(max(pixel4,pixel12),max(pixel0,pixel8))
      V8.ub = VMAX(V8.ub,V4.ub)                     //[e]max(min(pixel4,pixel12),min(pixel0,pixel8))
      N_numpixel = NEG(numpixel)                    // (right boundary mask)
    }
    { Q0 = VCMP.GT(V9.ub ,V11.ub)                   //[e]Q0 = any 2 contiguous pixels are brighter?
      Q1 = VCMP.GT(V10.ub,V8.ub )                   //[e]Q1 = any 2 contiguous pixels are darker?
      V15 = VALIGN(V13,V16,N_numpixel)              // (right boundary mask) 
    }
    { Q0 = OR(Q0,Q1)                                //[e]Q0 = Is a feature?
      R12 = ROL(R12,#1)                             //[e]update flag index
      Q2 = VCMP.EQ(V15.ub,V16.ub)                   // (right boundary mask)
    }
    { Q0 = AND(Q0,Q2)                               // mask out right boundary
      IF P1 V12 = V13                               // if P1, clean flag-collector
      IF P1 V14 = V16                               // if P1, set mask = 0xFF
    }
    { V12 |= VAND(Q0,R12)                           //[e]save flags in Vregister
    }
    { V2 = VAND(V12,V14)                            //[e]mask collected flags 
      VMEM(bitmask+#0) = V2.new                     // save flags
    }
    { JUMPR R31                                     // return
    }
    .size	fast9_detect_coarse, .-fast9_detect_coarse
