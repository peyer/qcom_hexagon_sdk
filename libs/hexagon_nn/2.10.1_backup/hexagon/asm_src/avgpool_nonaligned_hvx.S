
/*
 * Copyright (c) 2016-2017, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *    * Neither the name of The Linux Foundation nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/*
 */
#if 0
              for (z = 0; z < out_depth; z++) {
                /* foreach window y * foreach window x */
                sum = 0;
                for (in_y = start_y; in_y < end_y; in_y++) {
                  for (in_x = start_x; in_x < end_x; in_x++) {
                    uint32_t data = in0[z + in_depth * in_x + in_depth * in_width *  in_y];
                    sum += data;
                  }
                }
                out0[z] = (sum / count);
              }
#endif

          .global avgpool_nonaligned_hvx
          .type   avgpool_nonaligned_hvx, @function
          .balign 32
avgpool_nonaligned_hvx: 
/* ============================================================================ */
#define dsto        r0 //dest ptr
#define srco        r1 //src ptr
#define image_depth r2 //num bytes
#define win_width   r3
#define win_height  r4
#define image_width r5
#define scale       r6

#define stride      r7
#define stride0     r8
#define c0101       r9
#define src         r10
#define width       r11  //write width
#define dalign      r12
#define mdsto       r13

#define z1z0        v1:0
#define z0          v0
#define z1          v1
#define x0          v2
#define y0          v3
#define z2          v4
#define z3          v5
#define vzero       v6
#define d0          v7

#define qprolog     q0
#define qepilog     q1
/* ============================================================================ */
 
   {
     scale = memw(sp+#0)
     M0 = image_depth
     stride  = sub(image_width, win_width)
   } {
     stride = mpyi(stride, image_depth)
     c0101 = ##0x01010101
     scale = combine(scale.L, scale.L)
   } {
     loop1(.L_vert, win_height)
     vzero = #0
     src = srco
     srco = add(srco, #128)
   } {
     loop0(.L_horz, win_width)
     z1z0 = vcombine(vzero, vzero)
   }
/* ============================================================================ */
   .balign 32
.L_vert:
.L_horz:
   { 
     x0 = vmemu(src++M0)              //+in_depth* in_x
   } {
     nop
   } {
     z1z0.uh += vmpy(x0.ub, c0101.ub) //multiply vy 1 to uh 
   }:endloop0
   { 
     src = add(src, stride) 
     loop0(.L_horz, win_width)
   }:endloop1 
   {
     z2.h = vmpy(z0.h, scale.h):<<1:rnd:sat
     image_depth = add(image_depth, #-128) //160
     loop1(.L_vert, win_height)
   } {
     z3.h = vmpy(z1.h, scale.h):<<1:rnd:sat
     p0 = cmp.gt(image_depth, #0)          //0
     width =add(image_depth, #128)         //
     dalign = and(dsto, #127)              //0
   } {
     qprolog = vsetq(dsto)                 //vmem(dsto++#1) = y0 onaligned  //76543210
     if(p0) width = #128                   //128
   } {
     mdsto = sub(#0, dsto)
     y0.ub = vsat(z3.h, z2.h)              //
     dalign = add(dalign, width)           //128
   } {
     qepilog = vsetq(dalign)               //________
   } {
     d0 = vror(y0, mdsto)                  //54321076
     p1 = cmp.gt(dalign, #127)             //0 is block not less than 128 bytes
     if(p1.new) jump:nt .L_gt
   } 
   {
     qepilog = and(qepilog, !qepilog)      //________
     qprolog = or(qprolog, !qepilog)       //________
   }
.L_gt:
   {
     z0 = #0
     if( qepilog) vmem(dsto+#1) = d0       //________
     src = srco
   } {
     z1 = #0
     srco = add(srco, #128)
     if(!qprolog) vmem(dsto++#1) = d0      //76543210
     if(p0) jump .L_vert
   }
     jumpr r31
.L_end:
/*==============================================================================*/
      .size avgpool_nonaligned_hvx, .L_end-avgpool_nonaligned_hvx
