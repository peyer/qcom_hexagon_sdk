/**=============================================================================

 @file
 qdebug_sample_asm.h

 @brief
 An assembly file illustrating some of the ways the QD macros may be used to
 display or process register contents.

 Copyright (c) 2017 Qualcomm Technologies Incorporated.
 All Rights Reserved Qualcomm Proprietary

 Export of this technology or software is regulated by the U.S.
 Government. Diversion contrary to U.S. law prohibited.

 All ideas, data and information contained in or disclosed by
 this document are confidential and proprietary information of
 Qualcomm Technologies Incorporated and all rights therein are expressly reserved.
 By accepting this material the recipient agrees that this material
 and the information contained therein are held in confidence and in
 trust and will not be used, copied, reproduced in whole or in part,
 nor its contents revealed in any manner to others without the express
 written permission of Qualcomm Technologies Incorporated.

 =============================================================================**/

#include "qdebug_asm.h"

	.text
   .p2align 4                         
   .globl test           
   .type       test, @function
test:
	{ r0 = add(r0,r0) }
	{ JUMPR R31 }

   .size       test, .-test
 
   .text
   .p2align 4                         
   .globl qdebug_sample_asm           
   .type       qdebug_sample_asm, @function
qdebug_sample_asm:

   // First, we simply set all R and V registers to their index value
   {
   ALLOCFRAME(#(8*8 + 2*8)) // r + p  
   } {
   memd(r29 + #0) = r17:16
   memd(r29 + #8) = r19:18
   } {
   memd(r29 + #16) = r21:20
   memd(r29 + #24) = r23:22
   } {
   memd(r29 + #32) = r25:24
   memd(r29 + #40) = r27:26
   } {
   memw(r29 + #48) = r28
   memd(r29 + #56) = r31:30
   } {
   r6 = p0
   r7 = p1
   } {
   memd(r29 + #64) = r7:6
   r6 = p2
   r7 = p3
   } {
   memd(r29 + #72) = r7:6
   }
   {
   r0=#0
   r1=#1
   r2=#2
   r3=#3
   }
   {
   r4=#4
   r5=#5
   r6=#6
   r7=#7
   }
   {
   r8=#8
   r9=#9
   r10=#10
   r11=#11
   }
   {
   r12=#12
   r13=#13
   r14=#14
   r15=#15
   }
   {
   r16=#16
   r17=#17
   r18=#18
   r19=#19
   }
   {
   r20=#20
   r21=#21
   r22=#22
   r23=#23
   }
   {
   r24=#24
   r25=#25
   r26=#26
   r27=#27
   }
   {
   r28=#28
   }
   { V0 = VSPLAT(R0) }
   { V1 = VSPLAT(R1) }
   { V2 = VSPLAT(R2) }
   { V3 = VSPLAT(R3) }
   { V4 = VSPLAT(R4) }
   { V5 = VSPLAT(R5) }
   { V6 = VSPLAT(R6) }
   { V7 = VSPLAT(R7) }
   { V8 = VSPLAT(R8) }
   { V9 = VSPLAT(R9) }
   
   { V10 = VSPLAT(R10) }
   { V11 = VSPLAT(R11) }
   { V12 = VSPLAT(R12) }
   { V13 = VSPLAT(R13) }
   { V14 = VSPLAT(R14) }
   { V15 = VSPLAT(R15) }
   { V16 = VSPLAT(R16) }
   { V17 = VSPLAT(R17) }
   { V18 = VSPLAT(R18) }
   { V19 = VSPLAT(R19) }

   { V20 = VSPLAT(R20) }
   { V21 = VSPLAT(R21) }
   { V22 = VSPLAT(R22) }
   { V23 = VSPLAT(R23) }
   { V24 = VSPLAT(R24) }
   { V25 = VSPLAT(R25) }
   { V26 = VSPLAT(R26) }
   { V27 = VSPLAT(R27) }
   { V28 = VSPLAT(R28) }

   { R0 = #29 }   
   { V29 = VSPLAT(R0) }
   { R0 = #30 }   
   { V30 = VSPLAT(R0) }
   { R0 = #31 }   
   { V31 = VSPLAT(R0) }
   { R0 = #0 }
   
   // Now we show how to display contents of all R and V registers in various ways
   QD print_int V_all   "Displaying all HVX registers as int"
   QD print_int R_all   "Displaying all R registers as int"
   QD print_int R20   "Displaying register contents as int"
   QD print_short R20   "Displaying register contents as short"
   QD print_hex R20   "Displaying register contents as hex"
   QD print_byte R20   "Displaying register contents as byte"
   QD print_ubyte R20   "Displaying register contents as ubyte"
   QD print_float R20   "Displaying register contents as float... doesn't make much sense here"

   QD cust0 R18   "Applying some custom processing to a specific scalar register"
   QD cust1 R18   "Applying some other custom processing to a specific scalar register"
   QD cust0 V20   "Or V register (this string may be ignored by custom function)"

   // Now we set each words of V20 to its respective index to illustrate how masks can
   // display parts of vector
   { loop0 (splat_loop,#32)
     R0 = #32
     R1 = #124	 }
splat_loop:
   { V18.w=vinsert(R0)   }
   { V18 = vror(V18,R1) }
   { R0 = add(R0,#-1) }:endloop0
   { R1 = #4 }
   { V18 = vror(V18,R1) }

   // See how the display of this V register is affected depending on how the display
   // mask is setup in C with qd_set_mask  
   QD print_int V18   "Displaying V20 as int elements"
   
   { R20 = #4 }
   { R20 = convert_w2sf(R20) }
   { V20 = VSPLAT(R20) }

   QD print_float R20   "But it does after setting R20 to 4.0f..."
   QD print_float V20   "... and splatting it into V20"
   
   {
   r1:0 = memd(r29 + #64)
   r3:2 = memd(r29 + #72)
   } {
   p0 = r0
   p1 = r1
   } {
   p2 = r2
   p3 = r3
   } {
   r17:16 = memd(r29 + #0)
   r19:18 = memd(r29 + #8)
   } {
   r21:20 = memd(r29 + #16)
   r23:22 = memd(r29 + #24)
   } {
   r25:24 = memd(r29 + #32)
   r27:26 = memd(r29 + #40)
   } {
   r28 = memw(r29 + #48)
   } {
   DEALLOC_RETURN
   }
   .size       qdebug_sample_asm, .-qdebug_sample_asm
