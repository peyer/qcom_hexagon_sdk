/**=============================================================================
@file
    qhmath_div_h.S

@brief
    Division of 16-bit fixed-point numbers. Divides numerator by denominator.

    Function prototype

        int16_t qhmath_div_h(int16_t n, int16_t d)

    Reference C code

        int16_t qhmath_div_h(int16_t n, int16_t d)
        {
            bool sign, neg_sign;
            int16_t abs_num, abs_den;
            int16_t norm_num, norm_den;
            int16_t interpol;
            uint32_t sht_den;
            int16_t ret_val;
            int16_t index;
            int16_t shift_factor;

            if (d == 0)
            {
                // Saturation value
                return 0x7FFF;
            }

            // Determine sign for output result
            sign = ((n ^ d) & (0x80000)) >> 15;
            neg_sign = ~sign;
            abs_num = abs(n)
            abs_den = abs(d)

            // Find number of MSB ones or zeros
            norm_den = norm_l(abs_den)
            sht_den = L_shl(abs_den, norm_den);
            norm_den = norm_den - 30;
            index = sht_den >> 27;
            sht_den = sht_den >> 11;
            interpol = sht_den % (1 << 16);
            ret_val = (inv_tab[index] << 16) + interpol * (inv_table[index + 1] - inv_table[index]);
            norm_num = norm_l(abs_num)
            shift_factor = norm_den - norm_num;
            ret_value = ret_val >> (shift_factor - 8);

            // Apply sign calculation
            ret_val = (ret_val ^ neg_sign) + sign;

            if (ret_val > 0x8000)
            {
                ret_val = 0x8000;
            }

            return ret_val;
        }

@note
    Output value is in Q7.8 signed format. That means that the high 8 bits represents integer
    part while the low 8 bits represents fractional part.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/* ========================================================================== */

    .p2align 2
    .p2align 4,,15
    .global qhmath_div_h
    .type qhmath_div_h, @function

/* ========================================================================== */

#define num                   r0
#define den                   r1
#define inv_table_addr        r2
#define abs_num               r8
#define abs_den               r9
#define sign                  r10
#define neg_sign              r11
#define shift_fac             r12
#define num_den               r7
#define ret_val               r0
#define norm_num              r2
#define norm_den              r3
#define scale                 r3
#define table_idx_0           r5
#define table_idx_1           r6
#define diff_idx_1_0          r5
#define shift_idx_0           r4
#define s_den                 r1

/* ========================================================================== */

qhmath_div_h:
{
    sign = #0x8000
    abs_num  = abs(num)
    abs_den  = abs(den)
}
{
    p0 = cmp.eq(den, #0)
    if (p0.new) jump:nt .L_ret
    norm_den = normamt(abs_den)
    inv_table_addr = add(pc,#inv_table@PCREL)
}
{
    sign &= xor(num, den)
    s_den = asl(abs_den, norm_den)
    norm_den = add(norm_den,#-30)
}
{
    inv_table_addr = tableidxh(s_den,#3,#27)
    sign = asr(sign, #15)
}
{
    s_den = asr(s_den, #11)
    shift_idx_0.l = #0x8000
    table_idx_0 = memh(inv_table_addr)
    table_idx_1 = memh(inv_table_addr+#2)
}
{
    neg_sign = sub(#0, sign)
    shift_idx_0 = combine(table_idx_0.l, shift_idx_0.l)
    diff_idx_1_0 = sub(table_idx_0, table_idx_1)
    norm_num = normamt(abs_num)
}
{
    shift_fac = sub(norm_den, norm_num)
    shift_idx_0 -= mpyu(s_den.l,diff_idx_1_0.l)
    ret_val = asl(abs_num, norm_num)
}
{
    ret_val = mpy(ret_val, shift_idx_0.h):<<1:rnd:sat
    shift_fac = sub(#-8, shift_fac)
}
{
    ret_val = asr(ret_val, shift_fac)
}
{
    p0 = cmp.gtu(ret_val, #0x8000)
    ret_val = xor(ret_val, neg_sign)
}
{
    ret_val = add(ret_val, sign)
}
.L_ret:
{
    if (p0) ret_val = #0x7FFF
    jumpr r31
}
    .size qhmath_div_h, .-qhmath_div_h

    .globl inv_table
    .section .rodata
    .p2align 5
    .type    inv_table, @object
inv_table:
    .hword    32690,29066,26171,23798,21820,20145,18709,17463
    .hword    16373
    .size    inv_table, 18
