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
    .file	"getxpos.S"

#include "hvx.cfg.h"
    /*[*****************************************************************************]*/
    /*[  FUNCTION   : int get_xpos()                                                ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: get x positions of corners                                    ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned int *img      -- pointer to masks               ]*/
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
    /*[   6.0.0         12/12/2014              created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl get_xpos
    .type	get_xpos, @function
get_xpos:
    { R6 = ##8*VLEN                                 // constant
      R5:4 = COMBINE(#0,R0)                         // R4 = mask; R5 = k%(VLEN/4) where k = 0
      R2 = ADD(R2,#(2-2*(VLEN/4)))                  // (k/(VLEN/4))*8*VLEN -2*(VLEN/4)+xstart+2;, k=0
    }
    { LOOP1(.get_xposLOOP,R1)                       // setup loop
      R0 = #0                                       // numcorners = 0
    }

    .falign
.get_xposLOOP:
    { R1 = MEMW(R4++#4)                             // load mask[k] 
      IF (CMP.EQ(R1.new,#0)) JUMP:nt .get_xpos_LPCONT   
                                                    // P0 = (mask[k]==0)
      R9 = ADD(R2,R5)                               // x0  
      R5 = ADD(R5,#1)                               // k++
    }
    { R8 = CT0(R1)                                  // bitpos = ct0(mask[k])
    }

    .falign
.get_xpos_Check32:
    { R7 = ADDASL(R9,R8,#LOG2VLEN-2)                // x0 + bitpos*(VLEN/4)
      MEMH(R3++#2) = R7.new                         // *(xpos++) = x0 + bitpos*(VLEN/4)
      R0 = ADD(R0,#1)                               // numcorners++
      R1 = TOGGLEBIT(R1,R8)                         // mask[k] ^= 1<<bitpos
    }
    { R8 = CT0(R1)                                  // bitpos = ct0(mask[k])
      P0 = CMP.EQ(R1,#0)
      IF !P0.new JUMP:t .get_xpos_Check32           // P0 = (mask[k]==0)
    }

    .falign
.get_xpos_LPCONT:
    { P0 = CMP.EQ(R5,#VLEN/4)                       //
      IF P0.new R5 = #0                             // k%(VLEN/4)
      IF P0.new R2 = ADD(R2,R6)                     // update (k/(VLEN/4))*8*VLEN -2*(VLEN/4)+xstart+2
    }:endloop1

    { JUMPR R31                                     // return numcorners
    }
    .size	get_xpos, .-get_xpos
