/**=============================================================================
@file
    qhmath_tan_h.S

@brief
    Computes the tangent of 16-bit fixed-point argument.
    
    Function is implemented using 9th order polynomial approximation:
    y = 1 + T1*x + T2*x^3 + T3*x^5 + T4*x^7 + T5*x^9
    for input angle range [0,pi/4] and 1/y for range (pi/4, pi/2)

    Function prototype
    
        int16_t qhmath_tan_h(int16_t input_angle)

    Reference C code

        int16_t qhmath_tan_h(int16_t input_angle)
        {
            int16_t sign = 0;
            int16_t inverse = 0;
            int16_t abs_angle = angle;
            int32_t y = 0;
            uint32_t x2;
            result_scale_t res_inv;

            // Check range of angle
            if ((angle < 0) || (angle > 25736))
            {
                y = 0xFFFF;
                return  y;
            }

            // Check whether the angle between 2nd and 3rd quadrant
            if ((angle > 6434) && (angle <= 3*6434))
            {
                sign = angle - 2*6434;
                abs_angle = abs(angle - 2*6434);
            }

            // Check whether the angle in 4th quadrant
            if (angle > 3*6434)
            {
                sign = -1;
                abs_angle = 25736 - angle;
            }

            // Check whether the angle in 1st pi/8 or in 2nd pi/8
            if (abs_angle > 3217)
            {
                inverse = 1;
                abs_angle = 6434 - abs_angle;

                // Check whether the distance beetwen angle and pi/2 less than 64 in Q3.12 fmt
                if (abs_angle <= 64)
                {
                    y = 0x7FFF;
                    return  y;
                }
            }

            x2 = ((((uint32_t)abs_angle) * ((uint32_t)abs_angle))>>9);

            // Calculating polynomial approximation
            y = ((T3_FIXED_PT_SFT + T4_FIXED_PT*x2)>>15);
            y = ((T2_FIXED_PT_SFT + y*x2)>>15);
            y = ((T1_FIXED_PT_SFT + y*x2)>>15);
            y = ((T0_FIXED_PT_SFT + y*x2)>>15);
            y = ((y*abs_angle) >> 16);

            // Check whether is needed to calculate inverse value of polynomial
            if (inverse == 1)
            {
                res_inv = qhmath_inv_h(y);
                y = res_inv.result >> (10 - res_inv.scale);
            }

            // Check sign of return value
            if (sign < 0)
            {
                return -(y>>1);
            }
            else
            {
                return (y>>1);
            }
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global  qhmath_tan_h
    .type qhmath_tan_h,@function

/*============================================================================*/

#define neg_result    r3
#define x2            r4
#define sign          r6
#define inverse       r7
#define abs_angle     r8
#define y_result      r9
#define tmp           r10
#define inf_range_reg r11
#define abs_angle_inv r12
#define reg_pi        r13
#define reg_3pi_2     r9
#define sht_reg       r12
#define pi3_2         19302
#define pi_2          6434
#define pi            12868
#define pi2           25736
#define pi_4          3217
#define true          1
#define false         0
#define inf_range     64
#define negative      -1
#define t6_coeff      308
#define t5_coeff      102
#define t4_coeff      801
#define t3_coeff      1750
#define t2_coeff      4371
#define t1_coeff      10922
#define t0_coeff      32768
#define t3_sft_coeff  28672000
#define t2_sft_coeff  71614464
#define t1_sft_coeff  178946048
#define t0_sft_coeff  536870912

/*============================================================================*/

qhmath_tan_h:
{
    p0 = cmp.gtu(r0,##25736)
    r2 = ##65535
}
{
    inf_range_reg = #inf_range
    reg_3pi_2 = #pi3_2
    if (p0) jump:nt .L_out_of_range
    allocframe(#8)
}

.L_check_2_3_quadrant:
{
    inverse = #false
    reg_pi = #pi
    abs_angle = r0
}
{
    p0 = cmp.gt(r0, #pi_2)
    p0 = cmp.lt(r0, reg_3pi_2)
    if (p0.new) jump:t .L_2_3_quadrant
}

.L_check_4_quadrant:
{
    p0 = cmp.gt(r0, #pi3_2)
    if (p0.new) jump:nt .L_4_quadrant
}
{
    jump .L_determine_octant
}

.L_4_quadrant:
{
    sign = #negative
    abs_angle = sub(#pi2, r0)
    jump .L_determine_octant
}

.L_2_3_quadrant:
{
    sign = sub(r0,r13)
}
{
    abs_angle = abs(sign)
}

.L_determine_octant:
{
    p0 = cmp.gt(abs_angle, #pi_4)
    abs_angle_inv = sub(#pi_2, abs_angle)
}
{
    if (!p0) jump:t .L_poly
}

.L_prepare_inverse:
{
    abs_angle = abs_angle_inv
    inverse = #true
    p0 = cmp.lt(abs_angle_inv,inf_range_reg)
    if (!p0.new) jump:t .L_poly
}

.L_out_of_range:
{
    r0 = sxth(r2)
    dealloc_return
}

    /* Calculating polynomial of 9th order */
.L_poly:
{
    r2 = sxth(abs_angle)
    r3 = #t4_coeff
}
{
    x2 = mpyi(r2,r2)
}
{
    r4 = lsr(r4,#9)
}
{
    r3 = add(##t3_sft_coeff,mpyi(x2,r3))
}
{
    r3 = lsr(r3,#15)
}
{
    r3 = add(##t2_sft_coeff,mpyi(r3,x2))
}
{
    r3 = lsr(r3,#15)
}
{
    r3 = add(##t1_sft_coeff,mpyi(r3,x2))
}
{
    r3 = lsr(r3,#15)
}
{
    r3 = add(##t0_sft_coeff,mpyi(r3,x2))
}
{
    r3 = lsr(r3,#15)
}
{
    r2 = mpyi(r3,r2)
}
{
    tmp = #0
    r0 = asr(r2, #12)
    p0 = cmp.eq(inverse, #true)
    if (!p0.new) jump:t .L_sht_q9
}
.L_apply_inverse:
{
    sht_reg = #23
    call qhmath_inv_h
}
{
    r2 = sub(r1,sht_reg)
}
{
    r0 = asr(r0,r2)
    jump .L_check_sign
}
.L_sht_q9:
{
    r0 = asr(r0, #5)
}
.L_check_sign:
{
    neg_result = neg(r0)
    p0 = cmp.lt(sign, tmp)
    if (!p0.new) jump:t .L_ret
}
{
    r0 = neg_result
}
.L_ret:
{
    dealloc_return
}

    .size  qhmath_tan_h, .-qhmath_tan_h
