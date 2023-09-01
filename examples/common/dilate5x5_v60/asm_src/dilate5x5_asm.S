    /* ============================================================================ */
    /*  QUALCOMM TECHNOLOGIES, INC.                                                 */
    /*                                                                              */
    /*  HEXAGON HVX Image/Video Processing Library                                  */
    /*                                                                              */
    /* ---------------------------------------------------------------------------- */
    /*            Copyright (c) 2016 QUALCOMM TECHNOLOGIES Incorporated.            */
    /*                             All Rights Reserved.                             */
    /*                    QUALCOMM Confidential and Proprietary                     */
    /* ============================================================================ */

    /*[*****************************************************************************]*/
    /*[  FUNCTION   : void dilate5x5Per2Row()                                       ]*/
    /*[*****************************************************************************]*/
    /*[  DESCRIPTION: Performs 5x5 dilation of an image block                       ]*/
    /*[=============================================================================]*/
    /*[  INPUTS     : R0 : unsigned char *src  -- pointer to input image            ]*/
    /*[               R1 : int stride_i        -- stride of image input             ]*/
    /*[               R2 : int width           -- image width                       ]*/
    /*[               R3 : unsigned char *dst  -- pointer to output buffer          ]*/
    /*[               R4 : int stride_o        -- stride of image output            ]*/
    /*[=============================================================================]*/
    /*[  IMPLEMENTATION:                                                            ]*/
    /*[           - process two rows                                                ]*/
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
    /*[   6.0.0         05/18/2016              created for HVX evaluation          ]*/
    /*[  -------------------------------------------------------------------------  ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define VLEN 128
#define LOG2VLEN 7
    
#define stride_i            R1
#define outp0               R3
#define stride_o            R4
#define outp1               R5
#define outp2               R6
#define outp3               R7
#define inp0                R8
#define inp1                R9
#define inp2                R0
#define inp3                R10
#define inp4                R11
#define inp5                R12
#define inp6                R13
#define inp7                R14

/* ============================================================ */
#define sVmax0v0            V0
#define sVmax0v1            V1
#define sVmax1v0            V2
#define sVmax1v1            V3
#define sVmax2v0            V4
#define sVmax2v1            V5
#define sVmax3v0            V6
#define sVmax3v1            V7

#define sLine0              V8
#define sLine1              sLine0
#define sLine2              V9
#define sLine3              sLine2
#define sLine4              sLine0
#define sLine5              V10
#define sLine6              sLine0
#define sLine7              sLine0

#define sVmax0n2            sLine5
#define sMaxl134            V11
#define sVmax0n1            sLine5
#define sMaxl1l4            V12
#define sVmax1n2            V13
#define sVmax1n1            V14
#define sVmax2n2            V15
#define sMaxl136            sVmax1n1
#define sVmax0p1            V16
#define sVmax0p2            sMaxl1l4
#define sVmax2n1            sVmax1n2
#define sVmax1p1            sMaxl1l4
#define sVmax1p2            sMaxl1l4
#define sOut2               sVmax1n2
#define sVmax2p1            sMaxl1l4
#define sVmax3n2            sMaxl1l4
#define sVmax3v1t           sVmax1n1
#define sVmax3n1            sMaxl1l4
#define sOut3               V18
#define sVmax2p2            sMaxl1l4
#define sVmax3p1            sMaxl1l4
#define sVmax3p2            sLine0

#define sOut0               V17
#define sOut1               sMaxl134
#define sVmax00             V10
#define sVmax10             V11
#define sVmax02             V12
#define sVmax12             V13
#define sMaxl1l2            V14
#define sVmax01             sVmax0v0
#define sVmax11             sVmax1v0
/* ============================================================ */
    .text
    .p2align 2
    .p2align 4,,15
    .globl dilate5x5Per2Row
    .type    dilate5x5Per2Row, @function
dilate5x5Per2Row:
    { outp1 = ADD(outp0,stride_o)                   //
      inp1 = SUB(inp2,stride_i)                     //
      inp3 = ADD(inp2,stride_i)                     //
      R2 = ADD(R2,#VLEN-1-VLEN)                     //
    }
    { outp2 = ADD(outp1,stride_o)                   //
      inp0 = SUB(inp1,stride_i)                     //
      inp4 = ADD(inp3,stride_i)                     //
      R2 = ASR(R2,#LOG2VLEN)                        //
    }
    { outp3 = ADD(outp2,stride_o)                   //
      inp5 = ADD(inp4,stride_i)                     //
      P3 = SP1LOOP0(.dilate5x5RowLOOP,R2)           //
      sLine3 = VMEM(inp3++#1):NT                    //
    }
    { inp6 = ADD(inp5,stride_i)                     //
      sLine4.cur = VMEM(inp4++#1)                   //
      sMaxl134.ub = VMAX(sLine3.ub,sLine4.ub)       //
    }
    { inp7 = ADD(inp6,stride_i)                     //
      sLine2.cur = VMEM(inp2++#1):NT                //
      sMaxl1l4.ub = VMAX(sLine2.ub,sMaxl134.ub)     //
    }
    { sLine1.cur = VMEM(inp1++#1):NT                //
      sMaxl1l4.ub = VMAX(sLine1.ub,sMaxl1l4.ub)     //
    }
    { sLine0.cur = VMEM(inp0++#1):NT                //
      sVmax0v1.ub = VMAX(sLine0.ub,sMaxl1l4.ub)     //
      sVmax0v0 = #0                                 //
      sVmax1v0 = #0                                 //
    }
    { sLine5.cur = VMEM(inp5++#1)                   //
      sMaxl136.ub = VMAX(sLine5.ub,sMaxl134.ub)     //
      sVmax2v0 = #0                                 //
      sVmax3v0 = #0                                 //
    }
    { sLine6.cur = VMEM(inp6++#1)                   //
      sMaxl136.ub = VMAX(sLine6.ub,sMaxl136.ub)     //
      sVmax1v1.ub = VMAX(sMaxl1l4.ub,sLine5.ub)     //
    }
    { sLine7.cur = VMEM(inp7++#1)                   //
      sVmax2v1.ub = VMAX(sLine2.ub,sMaxl136.ub)     //
      sVmax3v1.ub = VMAX(sMaxl136.ub,sLine7.ub)     //
    }
    .falign
.dilate5x5RowLOOP:
    { sLine3 = VMEM(inp3++#1):NT                    //[1]
      sVmax0n2 = VLALIGN(sVmax0v1,sVmax0v0,#2)      //[1]
      sOut3.ub = VMAX(sOut3.ub,sVmax3p2.ub)         //[2]
    }
    { sLine4.cur = VMEM(inp4++#1)                   //[1]
      sMaxl134.ub = VMAX(sLine3.ub,sLine4.ub)       //[1]
      sVmax0n1 = VLALIGN(sVmax0v1,sVmax0v0,#1)      //[1]
      sOut0.ub = VMAX(sVmax0n2.ub,sVmax0v1.ub)      //[1]
    }
    { sLine2.cur = VMEM(inp2++#1):NT                //[1]
      sMaxl1l4.ub = VMAX(sLine2.ub,sMaxl134.ub)     //[1]
      sOut0.ub = VMAX(sVmax0n1.ub,sOut0.ub)         //[1]
      sVmax1n2 = VLALIGN(sVmax1v1,sVmax1v0,#2)      //[1]
    }
    { sLine1.cur = VMEM(inp1++#1):NT                //[1]
      sMaxl1l4.ub = VMAX(sLine1.ub,sMaxl1l4.ub)     //[1]
      sVmax0v0 = sVmax0v1                           //[1]
      sVmax1n1 = VLALIGN(sVmax1v1,sVmax1v0,#1)      //[1]
    }
    { sLine0.cur = VMEM(inp0++#1):NT                //[1]
      sVmax0v1.ub = VMAX(sLine0.ub,sMaxl1l4.ub)     //[1]
      sVmax1v0 = sVmax1v1                           //[1]
      sVmax2n2 = VLALIGN(sVmax2v1,sVmax2v0,#2)      //[1]
    }
    { sLine5.cur = VMEM(inp5++#1)                   //[1]
      sMaxl136.ub = VMAX(sLine5.ub,sMaxl134.ub)     //[1]
      sVmax0p1 = VALIGN(sVmax0v1,sVmax0v0,#1)       //[1]
      sOut1.ub = VMAX(sVmax1n1.ub,sVmax1v0.ub)      //[1]
    }
    { sLine6.cur = VMEM(inp6++#1)                   //[1]
      sMaxl136.ub = VMAX(sLine6.ub,sMaxl136.ub)     //[1]
      sVmax0p2 = VALIGN(sVmax0v1,sVmax0v0,#2)       //[1]
      sVmax1v1.ub = VMAX(sMaxl1l4.ub,sLine5.ub)     //[1]
    }
    { sOut0.ub = VMAX(sOut0.ub,sVmax0p1.ub)         //[1]
      sVmax2n1 = VLALIGN(sVmax2v1,sVmax2v0,#1)      //[1]
      sOut1.ub = VMAX(sVmax1n2.ub,sOut1.ub)         //[1]
      IF P3 VMEM(outp3++#1):NT = sOut3              //[2]
    }
    { sOut0.ub = VMAX(sOut0.ub,sVmax0p2.ub)         //[1]
      sVmax1p1 = VALIGN(sVmax1v1,sVmax1v0,#1)       //[1]
      sVmax2v0 = sVmax2v1                           //[1]
      sVmax2v1.ub = VMAX(sLine2.ub,sMaxl136.ub)     //[1]
    }
    { sVmax1p2 = VALIGN(sVmax1v1,sVmax1v0,#2)       //[1]
      sOut1.ub = VMAX(sOut1.ub,sVmax1p1.ub)         //[1]
      VMEM(outp0++#1):NT = sOut0                    //[1]
      sOut2.ub = VMAX(sVmax2n1.ub,sVmax2v0.ub)      //[1]
    }
    { sOut1.ub = VMAX(sOut1.ub,sVmax1p2.ub)         //[1]
      VMEM(outp1++#1):NT = sOut1.new                //[1]
      sVmax2p1 = VALIGN(sVmax2v1,sVmax2v0,#1)       //[1]
      sOut2.ub = VMAX(sVmax2n2.ub,sOut2.ub)         //[1]
    }
    { sVmax3n2 = VLALIGN(sVmax3v1,sVmax3v0,#2)      //[1]
      sOut2.ub = VMAX(sOut2.ub,sVmax2p1.ub)         //[1]
      sLine7.cur = VMEM(inp7++#1)                   //[1]
      sVmax3v1t.ub = VMAX(sMaxl136.ub,sLine7.ub)    //[1]
    }
    { sVmax3n1 = VLALIGN(sVmax3v1,sVmax3v0,#1)      //[1]
      sOut3.ub = VMAX(sVmax3n2.ub,sVmax3v1.ub)      //[1]
      sVmax3v0 = sVmax3v1                           //[1]
      sVmax3v1 = sVmax3v1t                          //[1]
    }
    { sVmax2p2 = VALIGN(sVmax2v1,sVmax2v0,#2)       //[1]
      sOut3.ub = VMAX(sVmax3n1.ub,sOut3.ub)         //[1]
    }
    { sVmax3p1 = VALIGN(sVmax3v1,sVmax3v0,#1)       //[1]
      sOut2.ub = VMAX(sOut2.ub,sVmax2p2.ub)         //[1]
    }
    { sVmax3p2 = VALIGN(sVmax3v1,sVmax3v0,#2)       //[1]
      sOut3.ub = VMAX(sOut3.ub,sVmax3p1.ub)         //[1]
      VMEM(outp2++#1):NT = sOut2                    //[1]
    }:endloop0
    { //sLine3 = VMEM(inp3++#1)                     //[1]
      sVmax0n2 = VLALIGN(sVmax0v1,sVmax0v0,#2)      //[1]
      sOut3.ub = VMAX(sOut3.ub,sVmax3p2.ub)         //[2]
    }
    { //sLine4.cur = VMEM(inp4++#1)                 //[1]
      //sMaxl134.ub = VMAX(sLine3.ub,sLine4.ub)     //[1]
      sVmax0n1 = VLALIGN(sVmax0v1,sVmax0v0,#1)      //[1]
      sOut0.ub = VMAX(sVmax0n2.ub,sVmax0v1.ub)      //[1]
    }
    { //sLine2.cur = VMEM(inp2++#1)                 //[1]
      //sMaxl1l4.ub = VMAX(sLine2.ub,sMaxl134.ub)   //[1]
      sOut0.ub = VMAX(sVmax0n1.ub,sOut0.ub)         //[1]
      sVmax1n2 = VLALIGN(sVmax1v1,sVmax1v0,#2)      //[1]
    }
    { //sLine1.cur = VMEM(inp1++#1)                 //[1]
      //sMaxl1l4.ub = VMAX(sLine1.ub,sMaxl1l4.ub)   //[1]
      sVmax0v0 = sVmax0v1                           //[1]
      sVmax1n1 = VLALIGN(sVmax1v1,sVmax1v0,#1)      //[1]
    }
    { //sLine0.cur = VMEM(inp0++#1)                 //[1]
      //sVmax0v1.ub = VMAX(sLine0.ub,sMaxl1l4.ub)   //[1]
      sVmax1v0 = sVmax1v1                           //[1]
      sVmax2n2 = VLALIGN(sVmax2v1,sVmax2v0,#2)      //[1]
    }
    { //sLine5.cur = VMEM(inp5++#1)                 //[1]
      //sMaxl136.ub = VMAX(sLine5.ub,sMaxl134.ub)   //[1]
      sVmax0p1 = VALIGN(sVmax0v1,sVmax0v0,#1)       //[1]
      sOut1.ub = VMAX(sVmax1n1.ub,sVmax1v0.ub)      //[1]
    }
    { //sLine6.cur = VMEM(inp6++#1)                 //[1]
      //sMaxl136.ub = VMAX(sLine6.ub,sMaxl136.ub)   //[1]
      sVmax0p2 = VALIGN(sVmax0v1,sVmax0v0,#2)       //[1]
      //sVmax1v1.ub = VMAX(sMaxl1l4.ub,sLine5.ub)   //[1]
    }
    { sOut0.ub = VMAX(sOut0.ub,sVmax0p1.ub)         //[1]
      sVmax2n1 = VLALIGN(sVmax2v1,sVmax2v0,#1)      //[1]
      sOut1.ub = VMAX(sVmax1n2.ub,sOut1.ub)         //[1]
      IF P3 VMEM(outp3++#1) = sOut3                 //[2]
    }
    { sOut0.ub = VMAX(sOut0.ub,sVmax0p2.ub)         //[1]
      sVmax1p1 = VALIGN(sVmax1v1,sVmax1v0,#1)       //[1]
      sVmax2v0 = sVmax2v1                           //[1]
      //sVmax2v1.ub = VMAX(sLine2.ub,sMaxl136.ub)   //[1]
    }
    { sVmax1p2 = VALIGN(sVmax1v1,sVmax1v0,#2)       //[1]
      sOut1.ub = VMAX(sOut1.ub,sVmax1p1.ub)         //[1]
      VMEM(outp0++#1):NT = sOut0                    //[1]
      sOut2.ub = VMAX(sVmax2n1.ub,sVmax2v0.ub)      //[1]
    }
    { sOut1.ub = VMAX(sOut1.ub,sVmax1p2.ub)         //[1]
      VMEM(outp1++#1):NT = sOut1.new                //[1]
      sVmax2p1 = VALIGN(sVmax2v1,sVmax2v0,#1)       //[1]
      sOut2.ub = VMAX(sVmax2n2.ub,sOut2.ub)         //[1]
    }
    { sVmax3n2 = VLALIGN(sVmax3v1,sVmax3v0,#2)      //[1]
      sOut2.ub = VMAX(sOut2.ub,sVmax2p1.ub)         //[1]
      //sLine7.cur = VMEM(inp7++#1)                 //[1]
      //sVmax3v1t.ub = VMAX(sMaxl136.ub,sLine7.ub)  //[1]
    }
    { sVmax3n1 = VLALIGN(sVmax3v1,sVmax3v0,#1)      //[1]
      sOut3.ub = VMAX(sVmax3n2.ub,sVmax3v1.ub)      //[1]
      sVmax3v0 = sVmax3v1                           //[1]
      //sVmax3v1 = sVmax3v1t                        //[1]
    }
    { sVmax2p2 = VALIGN(sVmax2v1,sVmax2v0,#2)       //[1]
      sOut3.ub = VMAX(sVmax3n1.ub,sOut3.ub)         //[1]
    }
    { sVmax3p1 = VALIGN(sVmax3v1,sVmax3v0,#1)       //[1]
      sOut2.ub = VMAX(sOut2.ub,sVmax2p2.ub)         //[1]
    }
    { sVmax3p2 = VALIGN(sVmax3v1,sVmax3v0,#2)       //[1]
      sOut3.ub = VMAX(sOut3.ub,sVmax3p1.ub)         //[1]
      VMEM(outp2++#1):NT = sOut2                    //[1]
    }
    { sOut3.ub = VMAX(sOut3.ub,sVmax3p2.ub)         //[2]
      VMEM(outp3++#1):NT = sOut3.new                //[2]
    }
    { JUMPR R31                                     // return
    }
    .size    dilate5x5Per2Row, .-dilate5x5Per2Row
