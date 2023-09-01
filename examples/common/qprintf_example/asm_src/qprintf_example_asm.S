/**=============================================================================

 @file
 qprintf_example_asm.h

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

#include "qprintf_asm.h"

   .text
   .p2align 4                         
   .globl qprintf_example_asm           
   .type      qprintf_example_asm, @function
qprintf_example_asm:

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
#if (__HEXAGON_ARCH__ >= 60)
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

   // Now we set each words of V18 to its respective index
   { loop0 (splat_loop,#32)
     R0 = #32
     R1 = #124	 }
splat_loop:
   { V18.w=vinsert(R0)   }
   { V18 = vror(V18,R1) }
   { R0 = add(R0,#-1) }:endloop0
#endif
  
  // 1. Scalar register display
  qprintf("qprintf extends printf support to assembly.");
  qprintf("You can use it for regular text including tabs\t and line breaks\n or even nothing!");
  qprintf("");
  qprintf("Unlike printf, qprintf begins each statement with the file name and line number and ends them with a return character.");
  qprintf("You can also print scalar registers in any format you want. For example, R20 is set to -1 and you can express it as %%d: %d",r20);
  qprintf("Or any printf format really: %%u: %u, %%x: %x, %%23d: %23d, %%+.6d: %+.6d. Etc. ",r20,r20,r20,r20);
  qprintf("qprintf also extends new formats to display registers as two 16-bit values.");
  qprintf("E.g. %%dd for two signed 16-bit: %dd. Or %%ud for unsigned-signed: %ud, and so on... %%du: %du, %%uu: %uu",r20, r20, r20, r20);
  qprintf("And same with four 8-bit values. For example, %%dddd: %dddd, %%duud: %duud, %%uddu: %uddu, etc.",r20, r20, r20);
  R22 = #4
  R22 = convert_w2sf(R22) 
  qprintf("We support float format too. r22 (now 4.0 in float) as %%e:%e or %%5.2f: %5.2f (different from %%d): %d",r22,r22,r22);

#if (__HEXAGON_ARCH__ >= 60)
  // 2. HVX support
  R0 = #-1
  V0 = VSPLAT(R0) 
  qprintf("You can print hvx registers as well. v0 as %%11d: %11d et voila",v0);
  qprintf("Note: The end of a formatter for a v register needs to be identified with <end of string>, <space>, ',', ';', '|', '\\', or ':'");
  qprintf("I.e. you can't do %%dthis but you can do %%d that or %%08x|%%+11u,or end the string with %%u");
  qprintf("The 16-bit and 8-bit display format is also supported for hvx vectors. E.g. v0 as %%du: %du",v0);
  qprintf("v0 as %%duud: %duud",v0);
  qprintf("You can also specify the number of columns per row to display with (n). For example, v0 as %%(3)+.6d: %(3)+.6d",v0);
  qprintf("And you use a mask (set previously from C) if you only care about some v0 values.");
  qprintf("\nHere the mask has been set in C as\n  qprintf_set_mask(QPRINTF_MASK_ODD_32,QPRINTF_MASK_EVEN_32);\nthus selecting the 8 even words from the lowest 64 bytes and the 8 odd words from the upper 64 bytes with v0 as %%md: %md",v0);
  qprintf("v0 as %%mdd: %mdd",v0);
  qprintf("v0 as %%m(5)uu: %m(5)uu",v0);
  qprintf("v0 as %%mdddd: %mdddd",v0);
  qprintf("v0 as %%m(12)uuuu: %m(12)dddu",v0);
  V22 = VSPLAT(R22) 
  qprintf("v22 (4.0 in float splat across the entire register) as %%05.4f: %05.4f",v22);

  // 3. Predicate support
  qprintf("You can also print p registers in any format. p0: %x, p1: %08x",p0,p1);
  Q1=vcmp.gt(V18.w,V19.w)
  qprintf("You can also print vector predicate registers. E.g. word-wise comparison between %(32)2d and %(32)2d is\n  %d",v18,v19,q1);
  Q3=vcmp.gt(V18.b,V19.b)
  qprintf("while byte-wise comparison between %(8)dddd and %(8)dddd is\n  %x",v18,v19,q3);
  // Please note formatter is ignored when displaying vector predicate registers. We always display in hex.
#endif

  // 4. Register dump support  
  qprintf("And finally you can print all scalar or v registers. For example, r as %%8x returns: %8x",r);
#if (__HEXAGON_ARCH__ >= 60)
  qprintf("And v as %%(16)x returns: %(16)x",v);
#endif

    { r20=asl(r2,#2)
    if (!p0.new) r4=#6
    p0=cmp.gtu(r2,#7); if (p0.new) jump:nt label }  
    
label:
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
   .size      qprintf_example_asm, .-qprintf_example_asm
