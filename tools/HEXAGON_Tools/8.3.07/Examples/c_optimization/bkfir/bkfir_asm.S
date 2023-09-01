/*****************************************************************
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
*****************************************************************/
/************************************************************************/
/*                                                                      */
/*                       QUALCOMM  DSP DIVISION                         */
/*                                                                      */
/*                       Copyright (c) 2005                             */
/*                       All Rights Reserved                            */
/*                                                                      */ 
/************************************************************************/
/*  FUNCTIONS      : Real Block FIR                                     */
/*  ARCHITECTURE   : Hexagon                                            */
/*  VERSION        :                                                    */
/*                                                                      */ 
/************************************************************************/
/*                                                                      */
/*  REVISION HISTORY:                                                   */
/*  =================                                                   */
/*                                                                      */
/*  Author              Date           Comments                         */
/*  -------------------------------------------------------------       */
/*  MZ                  12/16/04       created for prototype            */
/*  MZ                  01/06/05       updated: use vdmpy               */
/*  MZ                  01/14/05       Verified under simulator         */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/*   PEFORMANCE SUMMARY:                                                */
/*   ===================                                                */
/*                                                                      */
/*   N = number of samples                                              */
/*   T = number of taps                                                 */
/*   for this table:  N = 40, T = 16                                    */
/*                                                                      */
/*   ================================================================   */
/*              |  Cycle Count Formulas             | Cycle Counts      */
/*   -----------+-----------------------------------+----------------   */
/*   Kernel     |  (T+3)N/4 + 7                     |                   */
/*   ================================================================   */
/*                                                                      */
/*                                                                      */
/*   ================================================================   */
/*                     |   Code Size              Data Size             */
/*   ------------------+---------------------------------------------   */
/*   Kernel            |                                                */
/*   ================================================================   */
/*                                                                      */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/*  DESCRIPTIONS:                                                       */
/*  =============                                                       */ 
/*   - FORMAT:                                                          */
/*      o INPUT/OUTPUT/COEFFICIENT: in Half-Word                        */
/*   - ARRARY STRUCTURE:                                                */
/*      o Coefficients arranged in reverse order                        */
/*                                                                      */
/*   - ASSUMPTIONS:                                                     */
/*      o N, T are multiples of 4                                       */
/*      o Input/Output array is aligned by 8                            */
/*      o Coefficient array is aligned by 4                             */ 
/*                                                                      */
/*   - IMPLEMENTATION:                                                  */
/*      o process 4 samples per iteration in inner loop                 */
/*      o generate 4 outputs per iteration in outer loop                */
/*                                                                      */
/************************************************************************/	


	.text

/************************************************************************/
/*                                                                      */
/*                Block FIR FUNCTION                                    */
/*                ==================                                    */
/*   r0:  pointer to Input buffer                                       */
/*   r1:  pointer to Coefficients                                       */
/*   r2:  pointer to output buffer                                      */
/*   r4:  Number of Taps                                                */
/*   r5:  Number of Samples                                             */
/*                                                                      */
/************************************************************************/

	.p2align 2
	.globl FIR
FIR:
// Hexagon v3 has a different ABI
// We need to save/restore R16 - R27, but R22 - R27 aren't used here
#if __HEXAGON_ARCH__ >= 3 || __HEXAGON_ARCH__ >=3
	ALLOCFRAME(#24)
	MEMD(SP) = R17:16
	MEMD(SP+#8) = R19:18
	MEMD(SP+#16) = R21:20
#endif
	{ R10 = MEMW(R1++#4)		// load coeficients
	  R21 = R1			// save the coefficent pointer in r21
	  R3 = ADD(R0, #8)		// set pointer for load
	  R5  = ASR(R5, #2)		// N/4
	}
        LOOP1(MAIN_LOOP_START,R5)
	{ R7:6 = MEMD(R0++#8)		// R7:6 = X(4-T) || X(3-T)|| X(2-T)||X(1-T)
	  R11 = R10
	  R19:18 = VSUBW(R19:18,R19:18)	// zero out for dummy write
	  R4  = ASR(R4, #2)		// T/4
	}
	{ R4  = ADD(R4, #-1)		// inner loop counter = T/4 -1
	  NOP
	  R15:14 = VSUBW(R15:14,R15:14)	// reset accumulators R14:R15
	  R17:16 = VSUBW(R17:16,R17:16)	// reset accumulators R16:R17
	}

MAIN_LOOP_START:
	{ MEMD(R2++#8) = R19:18		// Save output; the first is a dummy write
          R8  = MEMH(R3++#2)		// R8 = X(n+4)
	  NOP
          LOOP0(INNER_LOOP_START,R4)
	}

INNER_LOOP_START:
	
	{ R8  = MEMH(R3++#2)		// R8 = X(n+5)
	  R12 = MEMW(R1++#4)		// load coefficients
	  R15:14 += VDMPY(R11:10,R7:6):<<1:sat
	  R7:6 = VALIGNB(R9:8,R7:6,#2)
					// R7:R6 = X(n+4)||X(n+3)||X(n+2)||X(n+1)
	}
	{ R8 = MEMH(R3++#2)		// R8 = X(n+6)
	  R13 = R12			// duplicate the coefficents
	  R17:16 += VDMPY(R11:10,R7:6):<<1:sat
	  R7:6 = VALIGNB(R9:8,R7:6,#2)
					// R7:R6 = X(n+5)||X(n+4)||X(n+3)||X(n+2)
	}

	{ R8  = MEMH(R3++#2)		// R8 = X(n+7)
	  R10 = MEMW(R1++#4)		// load coefficients
	  R15:14 += VDMPY(R13:12,R7:6):<<1:sat
	  R7:6 = VALIGNB(R9:8,R7:6,#2)
					// R7:R6 = X(n+6)||X(n+5)||X(n+4)||X(n+3)
	}
	{ R8 = MEMH(R3++#2)		// R8 = X(n+8)
	  R11 = R10			// duplicate the coefficents
	  R17:16 += VDMPY(R13:12,R7:6):<<1:sat
	  R7:6 = VALIGNB(R9:8,R7:6,#2)
					// R7:R6 = X(n+7)||X(n+6)||X(n+5)||X(n+4)
	}:endloop0

	{ R8  = MEMH(R3++#2)		// unroll inner loop once to reduce overhead 
	  R12 = MEMW(R1++#4)		 
	  R15:14 += VDMPY(R11:10,R7:6):<<1:sat
	  R7:6 = VALIGNB(R9:8,R7:6,#2)
	}
	{ R8 = MEMH(R3++#2)		
	  R13 = R12			
	  R17:16 += VDMPY(R11:10,R7:6):<<1:sat
	  R7:6 = VALIGNB(R9:8,R7:6,#2)
	}
	{ R1 = R21
	  R3 = ADD(R0, #8)
	  R15:14 += VDMPY(R13:12,R7:6):<<1:sat
	  R7:6 = VALIGNB(R9:8,R7:6,#2)
	}
	{ R10 = MEMW(R1++#4)		// load coeficients
	  NOP
	  R17:16 += VDMPY(R13:12,R7:6):<<1:sat
	  R15:14 = VASRW(R15:14,#3)	// Scaling output
	}
	{ R7:6 = MEMD(R0++#8)		// R7:6 = X(n'+3) || X(n'+2)||X(n'+1)||X(n')
	  R11 = R10
	  NOP
	  R17:16 = VASRW(R17:16,#3)	// Scaling output
	}
	{ R18 = COMBINE(R16.H, R14.H)
	  R19 = COMBINE(R17.H, R15.H)
	  R15:14 = VSUBW(R15:14,R15:14)	// reset accumulators R14:R15
	  R17:16 = VSUBW(R17:16,R17:16)	// reset accumulators R16:R17
	}:endloop1

#if __HEXAGON_ARCH__ >= 3 || __HEXAGON_ARCH__ >=3
	{ MEMD(R2+#0) = R19:18
	  R17:16 = MEMD(SP)
	}
	{
	  R21:20 = MEMD(SP+#16)
	  R19:18 = MEMD(SP+#8)
	}
	DEALLOCFRAME
	JUMPR R31			// Return
#else
	{ MEMD(R2+#0) = R19:18
	  JUMPR R31			// Return
	}
#endif

