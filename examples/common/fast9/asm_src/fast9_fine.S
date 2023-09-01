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
    /*[  FUNCTION   : int fast9_detect_fine                                         ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: Fine detection and score                                      ]*/
    /*[                                                                             ]*/
    /*[  The pixels in the FAST9 circle are numbered as shown below.                ]*/
    /*[  At the beginning the pointer points to c15. We take advantage of the two   ]*/
    /*[  post-incrementing modifier registers to save explicit adds on the pointer. ]*/
    /*[  In the order of top row to bottom, the pointer points to c15, c14, c13,    ]*/
    /*[  ref, c5, c6, and c7. Other pixels on the same row have known offsets to the]*/
    /*[  pointer at compile time.                                                   ]*/
    /*[                                                                             ]*/
    /*[         15  0  1                                                            ]*/
    /*[      14           2                                                         ]*/
    /*[   13                 3                                                      ]*/
    /*[   12       ref       4                                                      ]*/
    /*[   11                 5                                                      ]*/
    /*[      10           6                                                         ]*/
    /*[          9  8  7                                                            ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned char *im       -- pointer to input image        ]*/
    /*[               R1 : int num_pixel32         -- number of pixels in word      ]*/
    /*[               R2 : unsinged int stride     -- stride of image input         ]*/
    /*[               R3 : int barrier             -- barrier (0~255)               ]*/
    /*[               R4 : unsigned int   *bitmask -- pointer to input bitmask      ]*/
    /*[               R5 : unsigned short *xpos    -- pointer to output             ]*/
    /*[          (R29+0) : int             start   -- start point                   ]*/
    /*[=============================================================================]*/
    /*[  OUTUTS    : R0 : number of features                                        ]*/
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
    /*[   6.0.0         12/12/2014              created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define STACK_ALLOCATE_SIZE             48
/* ============================================================ */
#define REFADDR             R28
#define STRIDEPLUS3         R2
#define BARRIER             R3
#define MASK_ADDR_CURR      R4
#define CORNER_ADDR         R5
#define PIXADDR0            R0
#define PIXADDR1            R1
#define BRIGHT_THRESHOLD    R7:6
#define DARK_THRESHOLD      R9:8
#define BARRIER_VEC_TMP     R9:8
#define PIX11TO4_RD         R11:10
#define PIX3TO12_RD         R13:12
#define PIX11TO4            R15:14
#define PIX3TO12            R17:16
#define PIXREF              R19:18
#define BITMASK             R20
#define BITPOS              R21
#define NUM_CORNERS         R22
#define PIXREF_RD           R23
#define XCOR                R24
#define XSTART              R25
#define I0                  R26
#define XOFFSET             R27
#define TEMP                R14
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl fast9_detect_fine
    .type  fast9_detect_fine, @function
fast9_detect_fine:
    { R6 = ADD(R2,#-1)                                          // stride - 1
      R7 = SUB(#1,R2)                                           // -stride + 1
      R29 = ADD(R29,#-STACK_ALLOCATE_SIZE)                      // reserve stacks
      MEMD(R29+#(0-STACK_ALLOCATE_SIZE)) = R25:24               // callee-saved regs
    }
    { MEMD(R29+#8) = R23:22                                     // callee-saved regs
      XSTART = MEMW(R29+#(0+STACK_ALLOCATE_SIZE))               // xstart
      BARRIER = VSPLATB(R3)                                     // BARRIER = |b|b|b|b|
      LOOP0(.search_candidate_set,R1)                           // setup loop with lc0 = num_pixels32
    }
    { MEMD(R29+#16) = R21:20                                    // callee-saved regs
      MEMD(R29+#24) = R19:18                                    // callee-saved regs
      C7:6 = R7:6                                               // M0 = stride-1, M1=-(stride-1)
      REFADDR = ADD(R0,XSTART)                                  // REFADDR = &image + xstart
    }
    { MEMD(R29+#32) = R17:16                                    // callee-saved regs
      MEMD(R29+#40) = R27:26                                    // callee-saved regs
      STRIDEPLUS3 = ADD(R2,#3)                                  // stride + 3
      I0 = #0                                                   // i0 = 0
    }
    { BRIGHT_THRESHOLD = #-1                                    // prevent 1st iteration from false detection
      DARK_THRESHOLD   = #0                                     // prevent 1st iteration from false detection
      NUM_CORNERS      = #0                                     // keep count of detected corners
      BITMASK = MEMW(MASK_ADDR_CURR++#4)                        // read BITMASK of the next candidate set
    } 
    { PIX11TO4_RD = PIXREF                                      //
      PIX3TO12_RD = PIXREF                                      //
    }

    .falign
.search_candidate_set:
    { IF(BITMASK==#0) JUMP:T .get_next_candidate_set            // load next candidate set if this one's empty
      PIXADDR1 = SUB(REFADDR,STRIDEPLUS3)                       //
      BITPOS = CT0(BITMASK)                                     // BITPOS = offset of the next candidate
    } 
    { XOFFSET = ADD(I0,BITPOS)                                  // [1]convert i to offset
    }
    { XOFFSET = INSERT(XOFFSET,#3,#LOG2VLEN)                    // [1]convert i to offset
      TEMP    = EXTRACTU(XOFFSET,#LOG2VLEN,#3)                  // [1]convert i to offset
    }

    .falign
.process_candidates:
    { XOFFSET = INSERT(TEMP,#LOG2VLEN,#0)                       // [2]offset to xstart
      PIX11TO4 = PIX11TO4_RD                                    // [3]c[(i+4)%16], i=0..7
      PIX3TO12 = PIX3TO12_RD                                    // [3]c[(i+12)%16], i=0..7
    } 
    { PIXADDR0 = ADD(REFADDR, XOFFSET)                          // [2]PIXADDR = ref address of the next candidate
      PIXADDR1 = ADD(PIXADDR1,XOFFSET)                          // [2]C13ADDR = PIXADDR- stride - 3
      BITMASK = CLRBIT(BITMASK,BITPOS)                          // [2]clear bit at BITPOS in BITMASK
      DARK_THRESHOLD   = VSUBUB(PIXREF,BARRIER_VEC_TMP):sat     // [3]c_b[i] = *p-b, i=0..7
    }
    { PIX11TO4_RD = MEMB_FIFO(PIXADDR0+#3)                      // [2]load&pack c4
      PIX3TO12_RD = MEMB_FIFO(PIXADDR0+#-3)                     // [2]load&pack c12
      P1= VCMPB.GTU(PIX3TO12,BRIGHT_THRESHOLD)                  // [3]compare to cb  	
      P0= VCMPB.GTU(PIX11TO4,BRIGHT_THRESHOLD)                  // [3]
    }
    { PIXREF_RD = MEMUB(PIXADDR0+#0)                            // [2]load reference pixel
      PIXADDR0 = ADD(PIXADDR0,STRIDEPLUS3)                      // [2]c5ADDR = ref+stride + 3
      P3= VCMPB.GTU(DARK_THRESHOLD,PIX3TO12)                    // [3]compare to c_b
      P2= VCMPB.GTU(DARK_THRESHOLD,PIX11TO4)                    // [3]
    }
    { PIX11TO4_RD = MEMB_FIFO(PIXADDR0++M0)                     // [2]load&pack c5
      PIX3TO12_RD = MEMB_FIFO(PIXADDR1++M1)                     // [2]load&pack c13
      P1 = !FASTCORNER9(P1,P0)                                  // [3]P1 = (not a corner)
      P1 = !FASTCORNER9(P3,P2)                                  // [3]
    }
    { PIX11TO4_RD = MEMB_FIFO(PIXADDR0++M0)                     // [2]load&pack c6
      PIX3TO12_RD = MEMB_FIFO(PIXADDR1++M1)                     // [2]load&pack c14
      PIXREF_RD = VSPLATB(PIXREF_RD)                            // [2]duplicate *p
      IF P1 JUMP .process_candidates_CONT                       // [3]if not a corner, then jump
    }

    .falign
.fast_fine_found_a_corner:
    { MEMH(CORNER_ADDR++#2) = XCOR                              // [3]store x coordinate to corner output
      NUM_CORNERS = ADD(NUM_CORNERS,#1)                         // [3]NUM_CORNERS++
      IF P1 JUMP .fast9_detect_fine_exit                        // [3](for epsilog)if not a corner, exit
    }

    .falign
.process_candidates_CONT:
    { PIX11TO4_RD = MEMB_FIFO(PIXADDR0++#-1)                    // [2]load&pack c7
      PIX3TO12_RD = MEMB_FIFO(PIXADDR1++#1)                     // [2]load&pack c15
      BARRIER_VEC_TMP = COMBINE(BARRIER,BARRIER)                // [2]BARRIER_VEC_TMP[i]=barrier, i=0..7
      XCOR = ADD(XSTART,XOFFSET)                                // [2]calculate X coordinate of the next candidate
    } 
    { BITPOS = CT0(BITMASK)                                     // [1]BITPOS = offset of the next candidate
      PIX11TO4_RD = MEMB_FIFO(PIXADDR0++#-1)                    // [2]load&pack c8
      PIX3TO12_RD = MEMB_FIFO(PIXADDR1++#1)                     // [2]load&pack c0
      PIXREF = COMBINE(PIXREF_RD,PIXREF_RD)                     // [2]PIXREF[i] = *p, i=0..7
    } 
    { XOFFSET = ADD(I0,BITPOS)                                  // [1]convert i to offset
      PIX11TO4_RD = MEMB_FIFO(PIXADDR0++M1)                     // [2]load&pack c9
      PIX3TO12_RD = MEMB_FIFO(PIXADDR1++M0)                     // [2]load&pack c1
      BRIGHT_THRESHOLD = VADDUB(PIXREF,BARRIER_VEC_TMP):sat     // [2]cb[i]  = *p+b, i=0..7
    } 
    { XOFFSET = INSERT(XOFFSET,#3,#LOG2VLEN)                    // [1]convert i to offset
      TEMP    = EXTRACTU(XOFFSET,#LOG2VLEN,#3)                  // [1]convert i to offset
      NOP; NOP                                                  //
    }
    { PIX11TO4_RD = MEMB_FIFO(PIXADDR0+#-2)                     // [2]load&pack c10
      PIX3TO12_RD = MEMB_FIFO(PIXADDR1+#2)                      // [2]load&pack c2
      PIXADDR0 = SUB(PIXADDR0,STRIDEPLUS3)                      // [2]C11ADDR
      PIXADDR1 = ADD(PIXADDR1,STRIDEPLUS3)                      // [2]C3ADDR
    } 
    { PIXADDR1 = SUB(REFADDR,STRIDEPLUS3)                       // [1]
      PIX11TO4_RD = MEMB_FIFO(PIXADDR0+#0)                      // [2]load&pack c11
      PIX3TO12_RD = MEMB_FIFO(PIXADDR1+#0)                      // [2]load&pack c3
      IF (BITMASK!=#0) JUMP:t .process_candidates               // [2]if any candidates, continue process
    } 

    .falign
.get_next_candidate_set:
    { I0 = ADD(I0,#32)                                          // i += 32
      BITMASK = MEMW(MASK_ADDR_CURR++#4)                        // read BITMASK of the next candidate set
    }:endloop0
   

    .falign 
.flush_candidate_pipeline:
    { DARK_THRESHOLD   = VSUBUB(PIXREF,BARRIER_VEC_TMP):sat     //[e]c_b[i] = *p-b, i=0..7
      PIX11TO4 = PIX11TO4_RD                                    //[e]c[(i+4)%16], i=0..7
      PIX3TO12 = PIX3TO12_RD                                    //[e]c[(i+12)%16], i=0..7
    }
    { P1= VCMPB.GTU(PIX3TO12,BRIGHT_THRESHOLD)                  //[e]compare to cb  	
      P0= VCMPB.GTU(PIX11TO4,BRIGHT_THRESHOLD)                  //[e]
    } 
    { P3= VCMPB.GTU(DARK_THRESHOLD,PIX3TO12)                    //[e]compare to c_b
      P2= VCMPB.GTU(DARK_THRESHOLD,PIX11TO4)                    //[e]
    } 
    { P0 = !FASTCORNER9(P1,P0)                                  //[e]P0 = (not a corner)
      P0 = !FASTCORNER9(P3,P2)                                  //[e]
    }
    { P1 = CMP.EQ(R0,R0)                                        //[e] set P1 (for termination)
      IF !P0 JUMP .fast_fine_found_a_corner                     //[e] if a corner, jump
    }

    .falign
.fast9_detect_fine_exit:
    { R0 = NUM_CORNERS                                          // return value
      R25:24 = MEMD(R29+#0)                                     // restore callee-saved regs
      R23:22 = MEMD(R29+#8)                                     // restore callee-saved regs
    }
    { R21:20 = MEMD(R29+#16)                                    // restore callee-saved regs
      R19:18 = MEMD(R29+#24)                                    // restore callee-saved regs
    }
    { R17:16 = MEMD(R29+#32)                                    // restore callee-saved regs
      R27:26 = MEMD(R29+#40)                                    // restore callee-saved regs
      R29 = ADD(R29,#STACK_ALLOCATE_SIZE)                       // pop stacks
      JUMPR R31                                                 // return
    }
    .size fast9_detect_fine, .-fast9_detect_fine
