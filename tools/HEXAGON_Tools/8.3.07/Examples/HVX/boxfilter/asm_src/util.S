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
    .file    "util.S"

#include "hvx.cfg.h"
    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void Vmemset()                                                ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION:                                                               ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : void           *src                                      ]*/
    /*[               R1 : unsigned char  value                                     ]*/
    /*[               R2 : int            size                                      ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
    /*[           -                                                                 ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  ASSUMPTIONS:                                                               ]*/
    /*[           - input array is aligned by VLEN                                  ]*/
    /*[           - size is multiple of VLEN                                        ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  REVISION HISTORY                                                           ]*/
    /*[  ----------------                                                           ]*/
    /*[  Version        Date                    Comments                            ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[   6.0.0         08/24/2016              created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl Vmemset
    .type	Vmemset, @function
Vmemset:
    { R1 = VSPLATB(R1)                      // splat value in R-register
      R2 = LSR(R2,#LOG2VLEN)                // size/VLEN
    }
    { P0 = CMP.GT(R2,#0)                    //
      IF !P0.new JUMPR:nt R31               //
    }
    { LOOP0(.Vmemset_LOOP,R2)               //
      V0 = VSPLAT(R1)                       // splat value in V-register
    }

    .falign
.Vmemset_LOOP:
    { VMEM(R0++#1) = V0                     //src[i] = value
    }:endloop0

    { JUMPR R31                             // return
    }
    .size	Vmemset, .-Vmemset

