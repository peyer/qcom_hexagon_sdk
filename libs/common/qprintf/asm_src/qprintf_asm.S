/**=============================================================================

@file
   qprintf_asm.S

@brief
   Assembly implementation saving all registers on the stack to pass on to
   higher-level C function for post-processing.
      
Copyright (c) 2017 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

.file       "qprintf_asm.S"

#include "qprintf_asm.h"

.set STACK_SIZE, (16*8 + 2*8 + 128 + 32*128)
    .text
    .p2align 2
    .p2align 4,,15
    .globl qprintf_asm
    .type    qprintf_asm, @function
qprintf_asm:
   {
   ALLOCFRAME(#(28*4 + 4*4 + 128 + 32*128 + 4*128)) // r0-r27 + p0-p3 + alignment + v0-31 + q0-3
   } {
   // user data already on stack:
   //   r29:28 @ fp[8]
   //   r31:30 @ fp[32]  (8 + 24 = alloc size of wrapper macro)
   //   msg @ fp[16]
   //   arg @ fp[20]
   //   line number @ fp[24]
   memd(r29 + #0) = r1:0
   memd(r29 + #8) = r3:2
   } {
   memd(r29 + #16) = r5:4
   memd(r29 + #24) = r7:6
   } {
   r6 = add(r29,#(28*4 + 4*4 + 127))
   memd(r29 + #32) = r9:8
   memd(r29 + #40) = r11:10
   } {
   r6 = and(r6,#~127)
   memd(r29 + #48) = r13:12
   memd(r29 + #56) = r15:14
   } {
   memd(r29 + #64) = r17:16
   memd(r29 + #72) = r19:18
   } {
   memd(r29 + #80) = r21:20
   memd(r29 + #88) = r23:22
   } {
   memd(r29 + #96) = r25:24
   memd(r29 + #104) = r27:26
   } 
#if (__HEXAGON_ARCH__ >= 60)
   { vmem(r6++#1) = V0 } { vmem(r6++#1) = V1 } { vmem(r6++#1) = V2 } { vmem(r6++#1) = V3  } 
   { vmem(r6++#1) = V4 } { vmem(r6++#1) = V5 } { vmem(r6++#1) = V6 } { vmem(r6++#1) = V7  } 
   { vmem(r6++#1) = V8 } { vmem(r6++#1) = V9 } { vmem(r6++#1) = V10} { vmem(r6++#1) = V11 } 
   { vmem(r6++#1) = V12} { vmem(r6++#1) = V13} { vmem(r6++#1) = V14} { vmem(r6++#1) = V15 } 
   { vmem(r6++#1) = V16} { vmem(r6++#1) = V17} { vmem(r6++#1) = V18} { vmem(r6++#1) = V19 } 
   { vmem(r6++#1) = V20} { vmem(r6++#1) = V21} { vmem(r6++#1) = V22} { vmem(r6++#1) = V23 } 
   { vmem(r6++#1) = V24} { vmem(r6++#1) = V25} { vmem(r6++#1) = V26} { vmem(r6++#1) = V27 } 
   { vmem(r6++#1) = V28} { vmem(r6++#1) = V29} { vmem(r6++#1) = V30} { vmem(r6++#1) = V31 } 
   {
   r7 = #-1 
   } {
   V0 = vand(Q0,r7)
   } {
   vmem(r6++#1) = V0
   V0 = vand(Q1,r7)
   } {
   vmem(r6++#1) = V0
   V0 = vand(Q2,r7)
   } {
   vmem(r6++#1) = V0
   V0 = vand(Q3,r7)
   } {
   vmem(r6++#1) = V0
   }
 #endif
   {
   r6 = p0
   r7 = p1
   } {
   memd(r29 + #112) = r7:6
   } {
   r6 = p2
   r7 = p3
   r2 = memw(r29 + #(STACK_SIZE + 8))    
   } {
   memd(r29 + #120) = r7:6
   R0 = r29  // stack pointer from current call where registers are stored
   R1 = r30  // stack pointer from previous call where arguments to printf are stored
   call qprintf_c
   }  {
   r0 = add(r29,#(28*4 + 4*4 +127))
   } {
   r0 = and(r0,#~127)
   }
#if (__HEXAGON_ARCH__ >= 60)
   { v0  = vmem(r0++#1) } { v1  = vmem(r0++#1) } { v2  = vmem(r0++#1) } { v3  = vmem(r0++#1) }
   { v4  = vmem(r0++#1) } { v5  = vmem(r0++#1) } { v6  = vmem(r0++#1) } { v7  = vmem(r0++#1) }
   { v8  = vmem(r0++#1) } { v9  = vmem(r0++#1) } { v10 = vmem(r0++#1) } { v11 = vmem(r0++#1) }
   { v12 = vmem(r0++#1) } { v13 = vmem(r0++#1) } { v14 = vmem(r0++#1) } { v15 = vmem(r0++#1) }
   { v16 = vmem(r0++#1) } { v17 = vmem(r0++#1) } { v18 = vmem(r0++#1) } { v19 = vmem(r0++#1) }
   { v20 = vmem(r0++#1) } { v21 = vmem(r0++#1) } { v22 = vmem(r0++#1) } { v23 = vmem(r0++#1) }
   { v24 = vmem(r0++#1) } { v25 = vmem(r0++#1) } { v26 = vmem(r0++#1) } { v27 = vmem(r0++#1) }
   { v28 = vmem(r0++#1) } { v29 = vmem(r0++#1) } { v30 = vmem(r0++#1) } { v31 = vmem(r0++#1) }
   // TODO: restore Q0-Q3
#endif
   {
   r1:0 = memd(r29 + #112)
   r3:2 = memd(r29 + #120)
   } {
   p0 = r0
   p1 = r1
   } {
   p2 = r2
   p3 = r3
   } {
   r1:0 = memd(r29 + #0)
   r3:2 = memd(r29 + #8)
   } {
   r5:4 = memd(r29 + #16)
   r7:6 = memd(r29 + #24)
   } {
   r9:8 = memd(r29 + #32)
   r11:10 = memd(r29 + #40)
   } {
   r13:12 = memd(r29 + #48)
   r15:14 = memd(r29 + #56)
   } {
   r17:16 = memd(r29 + #64)
   r19:18 = memd(r29 + #72)
   } {
   r21:20 = memd(r29 + #80)
   r23:22 = memd(r29 + #88)
   } {
   r25:24 = memd(r29 + #96)
   r27:26 = memd(r29 + #104)
   } {
   DEALLOC_RETURN
   }
    .size    qprintf_asm, .-qprintf_asm
    
   .text
    .p2align 2
    .p2align 4,,15
    .globl qprintf_V_all
    .type    qprintf_V_all, @function
qprintf_V_all:
  qprintf_macro "", "0", "%x", v
  jumpr r31
.size    qprintf_V_all, .-qprintf_V_all


  .text
    .p2align 2
    .p2align 4,,15
    .globl qprintf_R_all
    .type    qprintf_R_all, @function
qprintf_R_all:
  qprintf_macro "", "0", "%08x", r
  jumpr r31
.size    qprintf_R_all, .-qprintf_R_all


