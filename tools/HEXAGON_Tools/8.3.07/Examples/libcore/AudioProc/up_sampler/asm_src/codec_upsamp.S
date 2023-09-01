/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:20:10 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:20:10 CST 2008 
****************************************************************************/ 

#include "asm_macros.h"

	.file	"codec_upsamp.c"

/***************************************************************/
/* codec_upsamp_filt : Filter to do upsampling by a factor of 8*/ 
/* Inputs     : R0 : in -> Pointer to input signal.            */
/*              R1 : filter memory                             */
/*              R2 : size of input buffer                      */
/*              R3 : mode                                      */
/*              R4 : out -> Pointer to output signal (i)       */
/* Registers Affected: R0-R20                                  */
/* Hardware Loops affected: Loop0, Loop1                       */
/* Assumptions:                                                */
/*            - filter_memory is aligned to 8-byte boundary    */
/*            - the most current is the first in filtMem       */
/*                                                             */
/* Stack Memory Frame Allocated (in Bytes): 32                 */
/*                                                             */
/* Cycle Count:                                                */
/*                                                             */
/*            Author         Date          Comments            */
/*           --------       ------        ----------           */
/*                                                             */
/***************************************************************/            
	.text
	.p2align 4
	.globl codec_upsamp_filt_mono
	.type	codec_upsamp_filt_mono, @function
codec_upsamp_filt_mono:

	__saveonentry_32

	{ P0 = CMP.GTU(R3,#0)				// P0 = (mode!=0)
	  P1 = CMP.GT(R2,#0)				// P1 = (nsamples>0)
	  IF  P0.new R18 = #4				// step = (mode)? 2 : 1
	  IF !P0.new R18 = #2				// step = (mode)? 2 : 1
	}
	{ 
          IF !P1 JUMP codec_upsamp_return		// if(nsamples<=0) return
	  R20 = CONST32(#codec_upsamp_coefs)		// r20 clobbered but callee save 
	  M0 = R18
	  R19 = #0					// r19 clobbered but callee save 
	}

	.falign
codec_upsamp_outerloop:
	{ R15 = MEMH(R0++M0)				// cur=inputBuffer[i]
	  R3 = ADD(R20,#16)				// codec_upsampe_coefs+8
	  R19 = #0					// to make first write dummy
	  LOOP1(codec_upsamp_polyphase_filt,#8)
	}

	.falign
codec_upsamp_polyphase_filt:
	{ R8 = MEMH(R20++#2)				// load cur
	  R5 = R1					// R5 = filtMem
	  R17:16 = ASR(R7:6,#13)
	  NOP
	}
	{ R7:6 = MPY(R8.L,R15.L)			// cur * c[j][0]
	  R11:10 = MEMD(R5++#8)				// load filtMem[0-3]
	  R9:8 = MEMD(R3++#8)				// load c[j][1-4]
	  LOOP0(codec_upsamp_innerloop,#5)
	}

	.falign
codec_upsamp_innerloop:
	{ R7:6 += VRMPYH(R9:8,R11:10)			// lTemp += c[j][k] * filtMem[k]
	  R9:8 = MEMD(R3++#8)				// load c[j][k]
	  R11:10 = MEMD(R5++#8)				// load filtMem[k]
	}:endloop0

	{ R7:6 += VRMPYH(R9:8,R11:10)
	  R9:8 = MEMD(R3++#8)
	  R13:12 = MEMD(R5)
	  R16 = SATH(R16)
	}
	{ MEMH(R4) = R16
	  R4 = ADD(R4,R19)
	  R19 = R18
	  R7:6 += VRMPYH(R9:8,R13:12)
	}:endloop1

	{ R17:16 = ASR(R7:6,#13)
	  R15 = ASLH(R15)
	  R2 = ADD(R2,#-1)				// update LC for outloop
	  LOOP0(codec_upsamp_update_fileMEM,#5)
	}
	{ R16 = SATH(R16)
	  R9:8 = VALIGNB(R13:12,R11:10,#6)
	  R13:12 = R11:10
	  R11:10 = MEMD(R5+#-16)
	}
	MEMH(R4++M0) = R16

	.falign
codec_upsamp_update_fileMEM:
	{ MEMD(R5++#-8) = R9:8
	  R9:8 = VALIGNB(R13:12,R11:10,#6)
	  R13:12 = R11:10
	  R11:10 = MEMD(R5+#-24)
	}:endloop0

	{ MEMD(R5++#-8) = R9:8
	  R9:8 = VALIGNB(R13:12,R15:14,#6)
	  P0 = CMP.GT(R2,#0)
	}
	{ MEMD(R5) = R9:8
	  R20 = ADD(R20,#-16)				// adjust R20 to codec_upsamp_coefs
	  IF P0 JUMP codec_upsamp_outerloop
	}
	.falign
codec_upsamp_return:
        __restoreonexit_32

	.globl codec_upsamp_coefs
	.section	.rodata
	.p2align 3
	.type	codec_upsamp_coefs, @object
	.size	codec_upsamp_coefs, 464
codec_upsamp_coefs:
	.hword  1, 1, 1, 0, -2, -6, -10, -15			// coefs for curr input

	.hword	-20, 9, 33, -42					// filter #0
	.hword	96, -142, 208, -261, 313, -349, 344, -200
	.hword	-304, 7386, 1667, -1078, 798, -573, 405, -277
	.hword	178, -92, 42, 4, -4, 35, -26, 1

	.hword	-24, 22, 16, -22				// filter #1
	.hword	75, -127, 207, -287, 382, -487, 597, -651
	.hword	599, 7552, 599, -651, 597, -487, 382, -287
	.hword	207, -127, 75, -22, 16, 22, -24, 1

	.hword	-26, 35, -4, 4					// filter #2
	.hword	42, -92, 178, -277, 405, -573, 798, -1078
	.hword	1667, 7386, -304, -200, 344, -349, 313, -261
	.hword	208, -142, 96, -42, 33, 9, -20, 1

	.hword	-27, 46, -24, 33				// filter #3
	.hword	1, -42, 124, -228, 376, -588, 913, -1424
	.hword	2837, 6904, -993, 220, 74, -179, 209, -204
	.hword	184, -139, 105, -54, 47, -4, -15, 0

	.hword	-26, 54, -41, 61				// filter #4
	.hword	-43, 18, 50, -145, 293, -526, 920, -1631
	.hword	4029, 6141, -1442, 566, -178, -1, 88, -126
	.hword	139, -119, 100, -58, 55, -14, -10, 0

	.hword	-21, 57, -53, 85				// filter #5
	.hword	-85, 81, -37, -35, 163, -385, 804, -1649
	.hword	5158, 5158, -1649, 804, -385, 163, -35, -37
	.hword	81, -85, 85, -53, 57, -21, -6, 0

	.hword	-14, 55, -58, 100				// filter #6
	.hword	-119, 139, -126, 88, -1, -178, 566, -1442
	.hword	6141, 4029, -1631, 920, -526, 293, -145, 50
	.hword	18, -43, 61, -41, 54, -26, -2, 0

	.hword	-4, 47, -54, 105				// filter #7
	.hword	-139, 184, -204, 209, -179, 74, 220, -993
	.hword	6904, 2837, -1424, 913, -588, 376, -228, 124
	.hword	-42, 1, 33, -24, 46, -27, 0, 0


	.size	codec_upsamp_filt_mono, .-codec_upsamp_filt_mono
	.ident	"GCC: (GNU) 3.4.2-BT_070309_0000_cyg"
