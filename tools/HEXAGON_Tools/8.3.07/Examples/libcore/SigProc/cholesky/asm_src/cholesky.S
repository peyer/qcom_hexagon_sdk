/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file   "cholesky.S"

#define  ptrX      R0     //
#define  n         R1     //
#define  k         R3     //
#define  sptr      R2     //
#define  vptr      R4     //
#define  hptr      R5     //
#define  hptri     R4     //
#define  exp       R9     //
#define  isq       R8     //
#define  bi_br     R7:6   //
#define  bi        R7     //
#define  br        R6     //
#define  ai_ar     R9:8   //
#define  ai        R9     //
#define  ar        R8     //
#define  pr        R11:10 //
#define  prh       R11    //
#define  prl       R10    //
#define  pi        R13:12 //
#define  pih       R13    //
#define  pil       R12    //
#define  sumi_sumr R15:14 //
#define  sumi      R15    //
#define  sumr      R14    //
#define  i         R16    //
#define  optr      R17    //
#define  zero      R19:18 //
#define  zerol     R18    //
#define  j         R20    //
#define  n3        R21    //
#define  input     R6     // 
#define  ninput    R7     // 
#define  tablePtr  R14    // 
#define  I0p75     R9:8   // 
#define  texp      R4     //
#define  offset    R15    // 
#define  tseed_x   R15    // 
#define  x1        R13:12 //
#define  x1l       R12    //
#define  x2        R15:14 // 
#define  x2l       R14    // 
#define  ax3       R11:10 // 
#define  ax3h      R11    // 
#define  SQRT_HALF R14    // 
#define  x_707     R11:10 // 
#define  x_707l    R10    // 
#define  Odd       P0     // 

    /*[*****************************************************************************]*/
    /*[  Function   : void cholesky()                                               ]*/
    /*[*****************************************************************************]*/
    /*[  Description: cholesky Factorization                                        ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - Data is 8byte aligned as 32bit I+Q complex numbers, and         ]*/
    /*[             data is normalized either by max value or by variance           ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word32 *input -- pointer to NxN complex matrix           ]*/
    /*[               R1 : int N         -- matrix demision                         ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R21                                                     ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 24                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           -> 1/2.N^3 + 5/2.N^2 + 24.N + 36                                  ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  Revision History:                                                          ]*/
    /*[  ----------------                                                           ]*/
    /*[  Author           Date                Comments                              ]*/
    /*[  -------------------------------------------------------------------        ]*/
    /*[  DJH              09/19/09           Created                                ]*/
    /*[*****************************************************************************]*/
    .text
    .global invSqrtTable
    .global cholesky
    .type   cholesky, @function
cholesky:            
    { sp = ADD(sp, #-24)                    //[ allocate stack                      ]
      br = MEMW(ptrX)                       //[                                     ]
    }
    { MEMD(sp+#0) = R17:16                  //[ callee-saved registers              ]
      R17 = ASL(n, #3)                      //[ 8*n                                 ]
    }
    { MEMD(sp+#8) = R19:18                  //[ callee-saved registers              ]
      M0 = R17                              //[ M0 = 8*n                            ]
    }
    { MEMD(sp+#16)= R21:20                  //[ callee-saved registers              ]
      n3 = ASL(n, #3)                       //[ 8*n                                 ] 
      zero = #0                             //[ constant 0                          ]
      j = #1                                //[                                     ]
    }
    .falign
.FOR_J:
    { tablePtr = const32(#invSqrtTable)     //[ 1/sqrt seed table                   ]
      texp = NORMAMT(br);                   //[ Number of redundant sign bits  exp  ]
    }
    { ninput = ASL(br, texp)                //[ Normalize 1/sqrt value to 0.5-1.0   ]
      texp = ADD(texp, #2)                  //[ increase exp, setup for exponent    ]
      Odd = TSTBIT(texp, #0)                //[ extract bit0 even/odd test of       ]
                                            //[ exponent                            ]
    }
    { offset = EXTRACTU(ninput, #8, #22)    //[ Change from 0.5-1.0 to 0.0-0.5      ]
      texp = LSR(texp, #1)                  //[                                     ]
    }
    { texp = SUB(#31, texp)                 //[ precorrect for cholesky loop        ]
      offset = ADD(tablePtr, offset)        //[                                     ]
      x1l.H = #0x4000                       //[                                     ]
    }
    { tseed_x = MEMUB(offset)               //[ lookup seed                         ]
      x1l.L = #0x0000                       //[ x1l = 0x40000000                    ]
    }
    { x1l += ASL(tseed_x,#22)               //[ (seed_x + 0x100) << 22              ]
    }
    { x2    = MPY(x1l, x1l)                 //[ x[n]^2                              ]
      I0p75 = COMBINE(#6, #0)               //[                                     ]
    }
    { x2 = ASR(x2, #31)                     //[                                     ]
      I0p75 = ASL(I0p75, #28)               //[ 0.75 constant                       ]
    }
    { x2 = MPY(ninput, x2l)                 //[ (0.75 - a*x[n]^2)                   ]
      ax3 = I0p75                           //[                                     ]
    }
    { ax3 -= ASL(x2, #1)                    //[                                     ]
    }
    { x1 = MPY(x1l, ax3h)                   //[                                     ]
    }
    { x1= ASR(x1, #30)                      //[ x[n+1] = 2*x[n]*(0.75-a*x[n]^2)     ]
    }
    { x2  = MPY(x1l, x1l)                   //[ x[n]^2                              ]
    }
    { x2 = ASR(x2, #31)                     //[                                     ]
    }
    { x2 = MPY(ninput, x2l)                 //[ a*x[n]^2                            ]
      ax3 = I0p75                           //[                                     ]
    }
    { ax3 -= ASL(x2, #1)                    //[                                     ]
      exp = texp                            //[                                     ]
    }
    { x1= MPY(x1l, ax3h)                    //[                                     ]
      SQRT_HALF.H= #0x5A82                  //[ ~ sqrt(1/2)                         ]
    }
    { x1= ASR(x1, #30)                      //[ x[n+1] = 2*x[n]*(0.75-a*x[n]^2)     ]
      SQRT_HALF.L= #0x799A                  //[ ~ sqrt(1/2)                         ]
    }
    { x_707  = MPY(x1l, SQRT_HALF)          //[ if exponent is Odd mpy by sqrt(2)   ]
      isq = x1l                             //[ inverse srq rot even powers         ]
      hptr = ADD(ptrX, n3)                  //[                                     ]
      MEMW(ptrX+#4) = exp                   //[                                     ]
    }
    { x_707 = ASR(x_707, #30)               //[ frac<<1 and  x2 (2/sqrt(2))         ]
      sptr = ADD(ptrX, #8)                  //[                                     ]
      hptri = hptr                          //[                                     ]
      p1 = CMP.GT(j, n)                     //[                                     ]
    }
    { IF(Odd) isq = x_707l                  //[                                     ]
      bi_br = MEMD(hptri++M0)               //[ [0,0]                               ]
    }
    { MEMW(ptrX) = isq                      //[                                     ]
      IF (p1) JUMP .RETURN                  //[                                     ]
      k = SUB(n, j)                         //[                                     ]
      pr = MPY(isq, br)                     //[ [0,1]                               ]
    }
    { LOOP0(.SCAL_PROD, k)                  //[ for(i=n-j-2 downto 0                ]
      pi = MPY(isq, bi)                     //[ [0,1]                               ]
    }
    { j = ADD(j, #1)                        //[                                     ]
      pr   = ASR(pr, exp)                   //[ [0,2]                               ]
      pi   = ASR(pi, exp)                   //[ [0,2]                               ]
      bi_br = MEMD(hptri++M0)               //[ [1,0]                               ]
    }
    { sumi_sumr = COMBINE(pil, prl)         //[ [0,3]                               ]
      pr = MPY(isq, br)                     //[ [1,1]                               ]
      pi = MPY(isq, bi)                     //[ [1,1]                               ]
      ptrX = ADD(ptrX, n3)
    }
    .falign
.SCAL_PROD:
    { MEMD(sptr++#8) = zero                 //[ [0,4]                               ]
      pr   = ASR(pr, exp)                   //[ [1,2]                               ]
      pi   = ASR(pi, exp)                   //[ [1,2]                               ]
      bi_br = MEMD(hptri++M0)               //[ [2,0]                               ]
    }
    { MEMD(hptr++M0) = sumi_sumr            //[ [0,5]                               ]
      sumi_sumr = COMBINE(pil, prl)         //[ [1,3]                               ]
      pr = MPY(isq, br)                     //[ [2,1]                               ]
      pi = MPY(isq, bi)                     //[ [2,1]                               ]
    }:endloop0

    { i = #1
      hptr = ADD(ptrX, #0)                  //[                                     ]
      optr = ADD(ptrX, #8)                  //[                                     ]
      LOOP1(.OUTER_PROD, k)                 //[ for(k=n-j-2 downto 0                ]
    }
    .falign
.OUTER_PROD:
    { ai_ar = MEMD(hptr++M0)                //[ [0,0]                               ]
      bi_br = MEMD(ptrX)                    //[ [0,0]                               ]
      LOOP0(.INNER_PROD, i)                 //[ for(i=k+1 downto 0                  ]
    }
    { pr = MPY(ar, br)                      //[ [0,1]                               ]
      pi = MPY(ar, bi)                      //[ [0,1]                               ]
      i = ADD(i, #1)
      sptr = optr
    }
    { pr += MPY(ai, bi)                     //[ [0,2]                               ]
      pi -= MPY(ai, br)                     //[ [0,2]                               ]
      vptr = ADD(ptrX, n3)                  //[                                     ]
      optr = ADD(optr, n3)                  //[ OL                                  ]
    }
    .falign
.INNER_PROD:
    { pr   = ADD(pr, pr)                    //[ [0,3]                               ]
      pi   = ADD(pi, pi)                    //[ [0,3]                               ]
      sumi_sumr = MEMD(sptr)                //[ [0,3]                               ]
      bi_br = MEMD(vptr++M0)                //[ [1,0]                               ]
    }
    { sumr = SUB(sumr, prh)                 //[ [0,4]                               ]
      sumi = ADD(sumi, pih)                 //[ [0,4]                               ]
      pr = MPY(ar, br)                      //[ [1,1]                               ]
      pi = MPY(ar, bi)                      //[ [1,1]                               ]
    }
    { MEMD(sptr++#8) = sumi_sumr            //[ [0,5]                               ]
      pr += MPY(ai, bi)                     //[ [1,2]                               ]
      pi -= MPY(ai, br)                     //[ [1,2]                               ]
    }:endloop0:endloop1

    { ptrX = ADD(ptrX, #8)                  //[                                     ]
      br = MEMW(ptrX+#8)                    //[                                     ]
      JUMP .FOR_J                           //[ jump to j-loop                      ]
    }
.RETURN:
    { R17:16 = MEMD(sp++#16)                //[ restore callee-saved registers      ]
      R19:18 = MEMD(sp+#8)                  //[ restore callee-saved registers      ]
    }
    { R21:20 = MEMD(sp++#8)                 //[ restore callee-saved registers      ]
      JUMPR    R31                          //[ return                              ]
    }
    .size cholesky, .-cholesky
