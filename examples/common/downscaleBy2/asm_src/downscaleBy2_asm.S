    /**************************************************************************************************
    * Copyright (c) Date: 2013-2014 Qualcomm Technologies INCORPORATED                                *
    * All Rights Reserved                                                                             *
    ***************************************************************************************************/

   .file       "downscaleBy2_asm.S"

    /*================================================================================================*/
    /*  FUNCTION     : down2                                                                          */
    /*================================================================================================*/
    /*  CYCLE-COUNT  : Inner loop: 32x2 pixels downsampled to 16x1 pixels in 10 cycles                */
    /*  (ideal cache)                                                                                 */
    /*================================================================================================*/
    /*  PARAMETERS   :                                                                                */
    /*  ============                                                                                  */
    /* R0 : srcImg (4 byte aligned)                                                                   */
    /* R1 : srcWidth (multiple of 2, >= 16)                                                           */
    /* R2 : srcHeight (multiple of 2)                                                                 */
    /* R3 : srcStride (4 byte multiple, >= 16)                                                        */
    /* R4 : dstImg  (8 byte multiple)                                                                 */
    /* R5 : dstStride (8 byte multiple)                                                               */
    /*================================================================================================*/
    /***************************************************************************************************
    * IMPLEMENTATION SUMMARY                                                                           *
    * ======================                                                                           *
    *   Down by 2 downscaling is done as follows. Each destination pixel is an average of 4            *
    *   source pixels. For example, in the picture below, Q1 = (A1 + A2 + B1 + B2) / 4.                *
    *                                                                                                  *
    *   |---------|---------|---------|---------|---------|---------|---------|---------|              *
    *   | A1 | A2 | C1 | C2 | E1 | E2 | G1 | G2 | I1 | I2 | K1 | K2 | M1 | M2 | O1 | O2 |              *
    *   |---------|---------|---------|---------|---------|---------|---------|---------|              *
    *   | B1 | B2 | D1 | D2 | F1 | F2 | H1 | H2 | J1 | J2 | L1 | L2 | N1 | N2 | P1 | P2 |              *
    *   |---------|---------|---------|---------|---------|---------|---------|---------|              *
    *                                                                                                  *
    *        |     |----|----|----|----|----|----|----|----|                                           *
    *        |     | Q1 | Q2 | Q3 | Q4 | R1 | R2 | R3 | R4 |                                           *
    *        V     |----|----|----|----|----|----|----|----|                                           *
    *                                                                                                  *
    ****************************************************************************************************/
    /*================================================================================================*/
    /*  STACK USAGE                                                                                   */
    /*================================================================================================*/
#define DOWN2_CALLEE_SAVED_REG_SIZE    7 * 8
#define DOWN2_CALLEE_SAVED_REG_OFFSET  0
#define DOWN2_STACK_SIZE               (DOWN2_CALLEE_SAVED_REG_OFFSET + DOWN2_CALLEE_SAVED_REG_SIZE)

   .text
   .p2align 4                          // ensures 16-byte alignment of first packet
   .globl down2                        // makes function have global scope
   .type       down2, @function
down2:
   {
     ALLOCFRAME(#DOWN2_STACK_SIZE)     // prepare stack
     P2 = CMP.GT(R1,#31)               // (srcWidth >= 32)
   }{
     MEMD(R29+#(2*8)) = R21:20         // save callee-saved regs
     MEMD(R29+#(3*8)) = R23:22         // save callee-saved regs
     R22 = ASR(R1,#5)                  // srcWidth / 32
     R23 = ASR(R1,#1)                  // dstWidth
   }{
     MEMD(R29+#(4*8)) = R25:24         // save callee-saved regs
     MEMD(R29+#(5*8)) = R27:26         // save callee-saved regs
     R26 = SUB(R3,R1)                  // srcStride - srcWidth
     R24 = #4                          // L2FETCH height = 4
   }{
     MEMD(R29+#(0*8)) = R17:16         // save callee-saved regs
     MEMD(R29+#(1*8)) = R19:18         // save callee-saved regs
     R24 |= ASL(R1,#16)                // L2FETCH width = srcWidth
     R1 = AND(R1,#31)                  // srcWidth % 32
   }{
     R28 = ADD(R0,R3)                  // srcImg + srcStride (first odd row)
     R2 = ASR(R2,#1)                   // srcHeight / 2
     R25 = R3                          // L2FETCH stride
     MEMD(R29+#(6*8)) = R31:30         // save callee-saved regs
   }{
     LOOP1(.down2_outerLOOP, R2)       // loop (srcHeight / 2)
     R26 = ADD(R26,R3)                 // 2 * srcStride - srcWidth
     R1 = ASR(R1,#1)                   // (srcWidth % 32) / 2
     P2 = CMP.EQ(R1,#0)                // (srcWidth % 32 == 0)
   }
   .falign
.down2_outerLOOP:
   {
     R30 = ADD(R4,#31)                 // dst + 31
     R31 = ADD(R4,R23)                 // dst + dstWidth
     L2FETCH(R0,R25:24)                // during processing of src rows (N, N+1), fetch rows N..N+3
   }{
     R30 = AND(R30,#-32)               // left edge of first full cache line in dst row
     DCFETCH(R0)                       // fetch current even row cache line
   }{
     R31 = SUB(R31,R30)                // number of bytes in dst row to right of first cache line edge
     DCFETCH(R28)                      // fetch odd row current cache line
   }{
     P0 = CMP.GT(R31,#31)              // at least 1 full cache line in dst row?
     IF (!P0.NEW) JUMP:NT .down2_outerLOOP_part2 // skip dczeroa loop
     R31 = ASR(R31,#5)                 // number of full cache lines in dst row
   }{
     LOOP0(.down2_dczeroa, R31)        // loop (number of cache lines)
   }
   .falign
.down2_dczeroa:
   {
     DCZEROA(R30)                      // allocate cache line
     R30 = ADD(R30,#32)                // advance to next cache line
   }:endloop0
   .falign
.down2_outerLOOP_part2:
   {
     P3 = SP1LOOP0(.down2_innerLOOP,R22) // inner loop per 32 bytes of row
   }
   .falign
.down2_innerLOOP:
   {
     DCFETCH(R0+#56)                   // [1] fetch upcoming even src row cache line
     R7:6 = MEMUBH(R0++#4)             // [1] load A1 A2 C1 C2
     R11:10 = VRADDUB(R15:14,R11:10)   // [2] I1+J1+I2+J2 | K1+L1+K2+L2
     R13:12 = VRADDUB(R17:16,R13:12)   // [2] M1+N1+M2+N2 | O1+P1+O2+P2
   }{
     DCFETCH(R28+#56)                  // [1] fetch upcoming odd src row cache line
     R9:8 = MEMUBH(R0++#4)             // [1] load E1 E2 G1 G2
     R16 = VASRW(R11:10,#2)            // [2] R1 | R2
     R17 = VASRW(R13:12,#2)            // [2] R3 | R4
   }{
     R11:10 = MEMUBH(R28++#8)          // [1] load B1 B2 D1 D2
     R13:12 = MEMUBH(R28+#4)           // [1] load F1 F2 G1 G2
     R21 = VTRUNEHB(R17:16)            // [2] packed Q1 | Q2 | Q3 | Q4
     R20 = VTRUNEHB(R19:18)            // [2] packed R1 | R2 | R3 | R4
   }{
     R15:14 = MEMUBH(R0++#8)           // [1] load I1 I2 K1 K2
     R17:16 = MEMUBH(R0+#4)            // [1] load M1 M2 O1 O2
     R7:6 = VRADDUB(R7:6,R11:10)       // [1] A1+B1+A2+B2 | C1+D1+C2+D2
     R9:8 = VRADDUB(R9:8,R13:12)       // [1] E1+F1+E2+F2 | G1+H1+G2+H2
   }{
     R11:10 = MEMUBH(R28++#8)          // [1] load J1 J2 L1 L2
     R13:12 = MEMUBH(R28+#4)           // [1] load N1 N2 P1 P2
     R18 = VASRW(R7:6,#2)              // [1] Q1 | Q2
     R19 = VASRW(R9:8,#2)              // [1] Q3 | Q4
   }{
     IF (P3) MEMD(R4++#8) = R21:20     // [2] store dst Q1 | Q2 | Q3 | Q4
     R11:10 = VRADDUB(R15:14,R11:10)   // [1] I1+J1+I2+J2 | K1+L1+K2+L2
     R13:12 = VRADDUB(R17:16,R13:12)   // [1] M1+N1+M2+N2 | O1+P1+O2+P2
     R7:6 = MEMUBH(R0++#4)             // [1] load A1 A2 C1 C2
   }{
     R9:8 = MEMUBH(R0++#4)             // [1] load E1 E2 G1 G2
     R16 = VASRW(R11:10,#2)            // [1] R1 | R2
     R17 = VASRW(R13:12,#2)            // [1] R3 | R4
     R13:12 = MEMUBH(R28+#4)           // [1] load F1 F2 G1 G2
   }{
     R11:10 = MEMUBH(R28++#8)          // [1] load B1 B2 D1 D2
     R21 = VTRUNEHB(R17:16)            // [1] packed Q1 | Q2 | Q3 | Q4
     R20 = VTRUNEHB(R19:18)            // [1] packed R1 | R2 | R3 | R4
     R17:16 = MEMUBH(R0+#4)            // [1] load M1 M2 O1 O2
   }{
     R15:14 = MEMUBH(R0++#8)           // [1] load I1 I2 K1 K2
     R7:6 = VRADDUB(R7:6,R11:10)       // [1] A1+B1+A2+B2 | C1+D1+C2+D2
     R9:8 = VRADDUB(R9:8,R13:12)       // [1] E1+F1+E2+F2 | G1+H1+G2+H2
     R13:12 = MEMUBH(R28+#4)           // [1] load N1 N2 P1 P2
   }{
     R11:10 = MEMUBH(R28++#8)          // [1] load J1 J2 L1 L2
     MEMD(R4++#8) = R21:20             // [1] store dst Q1 | Q2 | Q3 | Q4
     R18 = VASRW(R7:6,#2)              // [1] Q1 | Q2
     R19 = VASRW(R9:8,#2)              // [1] Q3 | Q4
   }:endloop0

   {
     R11:10 = VRADDUB(R15:14,R11:10)   // [2] I1+J1+I2+J2 | K1+L1+K2+L2
     R13:12 = VRADDUB(R17:16,R13:12)   // [2] M1+N1+M2+N2 | O1+P1+O2+P2
   }{
     R16 = VASRW(R11:10,#2)            // [2] R1 | R2
     R17 = VASRW(R13:12,#2)            // [2] R3 | R4
   }{
     R21 = VTRUNEHB(R17:16)            // [2] packed Q1 | Q2 | Q3 | Q4
     R20 = VTRUNEHB(R19:18)            // [2] packed R1 | R2 | R3 | R4
   }{
     MEMD(R4++#8) = R21:20             // [2] store dst Q1 | Q2 | Q3 | Q4
     IF (P2) JUMP .down2_outerLOOP_end // if (srcWidth % 32 == 0) skip tail processing
     P3 = SP1LOOP0(.down2_outerLOOP_tail, R1) // tail processing loop setup
   }
   .falign
.down2_outerLOOP_tail:
   {
     R6 = MEMUBH(R0++#2)               // [1] read 2 pixels from even row
     R7 = MEMUBH(R28++#2)              // [1] read 2 pixels from odd row
     R8 = ASR(R6,#3)                   // [2] avg = (2 * sum_of_4_pixels) / 8
   }{
     R6 = VRADDUH(R7:6,R7:6)           // [1] sum of 4 pixels
     IF(P3) MEMB(R4++#1) = R8          // [2] store dst pixel
   }:endloop0
   {
     R8 = ASR(R6,#3)                   // avg = (2 * sum_of_4_pixels) / 8
     MEMB(R4++#1) = R8.NEW             // store dst pixel
   }
   .falign
.down2_outerLOOP_end:
   {
     R4 += SUB(R5,R23)                 // dst += (dstStride - dstWidth)
     R0 = ADD(R0,R26)                  // src_even += (2 * dstStride - dstWidth)
     R28 = ADD(R28,R26)                // src_odd += (2 * dstStride - dstWidth)
   }:endloop1
   .falign
.down2_END:
   {
     R17:16 = MEMD(R29+#(0*8))         // restore caller registers
     R19:18 = MEMD(R29+#(1*8))         // restore caller registers
   }{
     R21:20 = MEMD(R29+#(2*8))         // restore caller registers
     R23:22 = MEMD(R29+#(3*8))         // restore caller registers
   }{
     R25:24 = MEMD(R29+#(4*8))         // restore caller registers
     R31:30 = MEMD(R29+#(6*8))         // restore caller registers
   }{
     R27:26 = MEMD(R29+#(5*8))         // restore caller registers
     DEALLOC_RETURN                    // return
   }
   .size       down2, .-down2

    /*================================================================================================*/
    /*  FUNCTION     : down2_hvx                                                                      */
    /*================================================================================================*/
    /*  CYCLE-COUNT  : Inner loop: 4 packets to consume 4xVLEN input pixels and produce VLEN          */
    /*                 output pixels (ideal cache)                                                    */
    /*================================================================================================*/
    /*  PARAMETERS   :                                                                                */
    /*  ============                                                                                  */
    /* R0 : srcImg (4 byte aligned)                                                                   */
    /* R1 : srcWidth (multiple of 2, >= 16)                                                           */
    /* R2 : srcHeight (multiple of 2)                                                                 */
    /* R3 : srcStride (4 byte multiple, >= 16)                                                        */
    /* R4 : dstImg  (8 byte multiple)                                                                 */
    /* R5 : dstStride (8 byte multiple)                                                               */
    /* stack : VLEN                                                                                   */
    /*================================================================================================*/
    /***************************************************************************************************
    * IMPLEMENTATION SUMMARY                                                                           *
    * ======================                                                                           *
    *   Down by 2 downscaling is done as follows. Each destination pixel is an average of 4            *
    *   source pixels. For example, in the picture below, Q1 = (A1 + A2 + B1 + B2) / 4.                *
    *                                                                                                  *
    *   |---------|---------|---------|---------|---------|---------|---------|---------|              *
    *   | A1 | A2 | C1 | C2 | E1 | E2 | G1 | G2 | I1 | I2 | K1 | K2 | M1 | M2 | O1 | O2 |              *
    *   |---------|---------|---------|---------|---------|---------|---------|---------|              *
    *   | B1 | B2 | D1 | D2 | F1 | F2 | H1 | H2 | J1 | J2 | L1 | L2 | N1 | N2 | P1 | P2 |              *
    *   |---------|---------|---------|---------|---------|---------|---------|---------|              *
    *                                                                                                  *
    *        |     |----|----|----|----|----|----|----|----|                                           *
    *        |     | Q1 | Q2 | Q3 | Q4 | R1 | R2 | R3 | R4 |                                           *
    *        V     |----|----|----|----|----|----|----|----|                                           *
    *                                                                                                  *
    ****************************************************************************************************/
    /*================================================================================================*/
    /*  STACK USAGE                                                                                   */
    /*================================================================================================*/
#define DOWN2_HVX_VLEN_OFFSET             (0)

   .text
   .p2align 4                          // ensures 16-byte alignment of first packet
   .globl down2_hvx                    // makes function have global scope
   .type       down2_hvx, @function
down2_hvx:
#if (__HEXAGON_ARCH__ < 60)
   JUMP down2                          // call scalar version, in case caller errantly called hvx version on non-HVX targets.
#else
   {
     R15 = MEMW(R29)                   // VLEN
     R9 = ##0x40404040                 // byte splat (64)
     R12 = #4                          // L2FETCH height = 4
   }{
     R14 = CT0(R15)                    // log2(VLEN)
     R12 |= ASL(R1,#16)                // L2FETCH width = srcWidth
     R13 = R3                          // L2FETCH stride
     R28 = R0                          // src
   }{
     R10 = ADD(R14,#1)                 // log2(2*VLEN)
     R6 = ASR(R2,#1)                   // srcHeight / 2
   }{
     R11 = ASR(R1,R10)                 // number of full pairs of vectors in width
     LOOP1(.down2_hvx_outerLOOP, R6)   // loop (srcHeight / 2)
   }{
     R1 -= ASL(R11,R10)                // remaining width beyond pairs of full vectors
   }{
     P2 = CMP.GT(R1,R15)               // remaining width > VLEN?
     R7 = ASR(R1,#1)                   // remaining dst width
     P1 = CMP.EQ(R1,#0)                // width is a multiple of 2*VLEN?
   }{
     Q0 = VSETQ(R7)                    // write mask for remaining dst bytes
   }
   .falign
.down2_hvx_outerLOOP:
   {
#if 1
    R2 = USR                           // check USR register for outstanding L2 fetch. Let it finish before proceeding.
    }{
    P0 = TSTBIT(R2,#31);IF(P0.NEW) JUMP:T .down2_hvx_outerLOOP
   }{
#endif
     L2FETCH(R28,R13:12)               // during processing of src rows (N, N+1), fetch rows N..N+3
     R2 = R0                           // src row 0
     R3 = ADD(R0,R13)                  // src row 1
     R0 += ADD(R13,R13)                // next rows
   }{
     R6 = ADD(R6,#-1)                  // decrement loop counter
     P0 = CMP.GT(R6,#2)                // remaining loop count > 2?
     IF (P0.NEW) R28 = ADD(R0,R13)     // advance L2FETCH pointer
     R12.L = #2                        // L2FETCH height
   }{
     IF (P0) R28 = ADD(R28,R13)        // advance L2FETCH pointer
     P3 = !CMP.EQ(R0,R0)               // FALSE (in case jump is taken)
     IF (R11==#0) JUMP:NT .down2_hvx_tail // skip main loop
     R8 = R4                           // dst
   }{
     P3 = SP1LOOP0(.down2_hvx_innerLOOP,R11) // loop over pairs of full vectors
     R7 = #0                           // loopcount = 0
   }
   .falign
.down2_hvx_innerLOOP:
   {
     V0 = VMEM(R2++#1):NT              // row0[0]
     V10.h += VDMPY(V2.ub,R9.b)        // [2] pairs of row0[0] + pairs of row1[0], shifted left by 6
     P0 = CMP.EQ(R7,#1)                // [2] (loopcount==1)
     R7 = ADD(R7,#1)                   // loopcount++
   }{
     V1 = VMEM(R2++#1):NT              // row0[1]
     V11.h += VDMPY(V3.ub,R9.b)        // [2] pairs of row0[1] + pairs of row1[1], shifted left by 6
     IF (P0) R8 = SUB(R8,R15)          // [2] rewind dst in 2nd iteration to overwrite 1st iteration garbage
   }{
     V2 = VMEM(R3++#1):NT              // row1[0]
     V10.h = VDMPY(V0.ub,R9.b)         // add pairs of elements from row0[0] and shift left by 6
     V4.b = VPACKO(V11.h,V10.h)        // [2] pack dst vector
   }{
     VMEM(R8++#1):NT = V4                  // [2] store dst (first iteration writes garbage)
     V3 = VMEM(R3++#1):NT              // row1[1]
     V11.h = VDMPY(V1.ub,R9.b)         // add pairs of elements from row0[1] and shift left by 6
   }:endloop0
   {
     V10.h += VDMPY(V2.ub,R9.b)        // [2] pairs of row0[0] + pairs of row1[0], shifted left by 6
     P0 = CMP.EQ(R7,#1)                // [2] (loopcount==1)
   }{
     V11.h += VDMPY(V3.ub,R9.b)        // [2] pairs of row0[1] + pairs of row1[1], shifted left by 6
     IF (P0) R8 = SUB(R8,R15)          // [2] rewind dst in 2nd iteration to overwrite 1st iteration garbage
   }
   .falign
.down2_hvx_tail:
   {
     V4.b = VPACKO(V11.h,V10.h)        // [2] pack dst vector
     IF (P3) VMEM(R8++#1):NT = V4.NEW  // [2] store dst
     IF (P1) JUMP .down2_hvx_tail_end  // skip tail processing
   }{
     V0 = VMEM(R2++#1):NT              // row0[0]
   }{
     V2 = VMEM(R3++#1):NT              // row1[0]
     IF (!P2) JUMP .down2_hvx_tail2    // remaining width <= VLEN
   }{
     V1 = VMEM(R2++#1):NT              // row0[1]
   }{
     V3 = VMEM(R3++#1):NT              // row1[1]
   }
   .falign
.down2_hvx_tail2:
   {
     V10.h = VDMPY(V0.ub,R9.b)         // add pairs of elements from row0[0] and shift left by 6
     V11.h = VDMPY(V1.ub,R9.b)         // add pairs of elements from row0[1] and shift left by 6
   }{
     V10.h += VDMPY(V2.ub,R9.b)        // pairs of row0[0] + pairs of row1[0], shifted left by 6
     V11.h += VDMPY(V3.ub,R9.b)        // pairs of row0[1] + pairs of row1[1], shifted left by 6
   }{
     V0.b = VPACKO(V11.h,V10.h)        // pack dst vector
   }{
     IF (Q0) VMEM(R8++#1) = V0         // bytewise-enabled store dst
   }
   .falign
.down2_hvx_tail_end:
   {
     R4 = ADD(R4,R5)                   // dst += dstStride
   }:endloop1
   {
     JUMPR R31                         // return
   }
#endif                                 // __HEXAGON_ARCH__ < 60
   .size       down2_hvx, .-down2_hvx

