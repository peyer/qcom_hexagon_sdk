/**=============================================================================
@file
    qhmath_sigmoid_h.S

@brief
    Sigmoid function for 16-bit floating-point values.

    Function prototype

        int16_t qhmath_sigmoid_h(int16_t x);

    Reference C code

        int16_t qhmath_sigmoid_h(int16_t x)
        {
            int16_t val;
            int32_t index;
            int16_t a, b;
            int16_t fract;
            int16_t xx;
            int16_t fix = 0x4000; // 0.5

            bool negative = x < 0;

            xx = negative ? -x : x; // :sat !

            // Find nearest index
            index = (uint32_t)xx >> 6;

            // Calculation of fractional value
            fract = (xx - (index << 6)) << 9;

            // Read two nearest values of input value from the sigmoid_LUT
            a = sigmoid_LUT[index];
            b = sigmoid_LUT[index + 1];

            // Linear interpolation process
            val = (int32_t)(0x8000 - fract) * a >> 16;
            val = (int16_t)((((int32_t)val << 16) + ((int32_t)fract * b)) >> 15);

            fix += negative ? -val : val;

            return fix;
        }

@note
    Sigmoid function is calculated as following:
    sigmoid(x) =  1.0/(e^(-x)+1.0)

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_sigmoid_h
    .type qhmath_sigmoid_h,@function

/*============================================================================*/

qhmath_sigmoid_h:
{
    p3 = tstbit(r0,#15)                        // input is negative
    r0 = zxth(r0)                              // clear higher half
}
{
    r2 = add(pc,##sigmoid_LUT@PCREL)           // table address
    r1:0 = vabsh(r1:0):sat                     // half absoulte, absoulte(x)
}
{
    r3 = asr(r0,#6)                            // Find nearest index
}
{
    r0 = and(#0x7FFF,asl(r0,#9))               // Calculation of fractional value
    r2 = addasl(r2, r3, #1)                    // table address with index
}
{
                                               // Linear interpolation process
    r4 = sub(#0x8000, r0)                      // 0x8000 - fract
                                               // Read two nearest values of input value from the sigmoid_LUT
    r1 = memh(r2)                              // a = sigmoid_LUT[index];
    r3 = memh(r2 + #2)                         // b = sigmoid_LUT[index + 1];
}
{
    r4 = mpyi(r0, r3)                          // Linear interpolation process
    r3 = mpyi(r1, r4)
}
{
    r2 = add(r4,r3)
}
{
    r2 = asr(r2,#14):rnd

    r1 = #0x4000                               // half, 0.5
}
{
    if (!p3) r0 = add(r1,r2)
    if (p3) r0 = sub(r1,r2)

    jumpr r31
}
    .size qhmath_sigmoid_h, .-qhmath_sigmoid_h

/*============================================================================*/

    .type	    sigmoid_LUT,@object
    .section	.rodata,"a",@progbits
    .p2align	3
sigmoid_LUT:
    .half 0x0000
    .half 0x007F, 0x00FF, 0x017F, 0x01FF, 0x027F, 0x02FF, 0x037F, 0x03FE, 0x047E, 0x04FD, 0x057C, 0x05FB, 0x067A, 0x06F8, 0x0777, 0x07F5
    .half 0x0873, 0x08F0, 0x096E, 0x09EB, 0x0A68, 0x0AE4, 0x0B60, 0x0BDC, 0x0C57, 0x0CD2, 0x0D4D, 0x0DC7, 0x0E41, 0x0EBB, 0x0F34, 0x0FAC
    .half 0x1024, 0x109C, 0x1113, 0x118A, 0x1200, 0x1275, 0x12EB, 0x135F, 0x13D3, 0x1447, 0x14B9, 0x152C, 0x159D, 0x160E, 0x167F, 0x16EF
    .half 0x175E, 0x17CD, 0x183B, 0x18A8, 0x1915, 0x1981, 0x19EC, 0x1A57, 0x1AC1, 0x1B2A, 0x1B93, 0x1BFA, 0x1C62, 0x1CC8, 0x1D2E, 0x1D93
    .half 0x1DF7, 0x1E5B, 0x1EBE, 0x1F20, 0x1F81, 0x1FE2, 0x2041, 0x20A1, 0x20FF, 0x215D, 0x21B9, 0x2215, 0x2271, 0x22CB, 0x2325, 0x237E
    .half 0x23D6, 0x242E, 0x2484, 0x24DA, 0x2530, 0x2584, 0x25D8, 0x262A, 0x267D, 0x26CE, 0x271E, 0x276E, 0x27BD, 0x280C, 0x2859, 0x28A6
    .half 0x28F2, 0x293D, 0x2987, 0x29D1, 0x2A1A, 0x2A62, 0x2AAA, 0x2AF1, 0x2B37, 0x2B7C, 0x2BC0, 0x2C04, 0x2C47, 0x2C8A, 0x2CCB, 0x2D0C
    .half 0x2D4D, 0x2D8C, 0x2DCB, 0x2E09, 0x2E46, 0x2E83, 0x2EBF, 0x2EFB, 0x2F35, 0x2F6F, 0x2FA9, 0x2FE1, 0x3019, 0x3051, 0x3087, 0x30BD
    .half 0x30F3, 0x3128, 0x315C, 0x318F, 0x31C2, 0x31F5, 0x3226, 0x3257, 0x3288, 0x32B8, 0x32E7, 0x3316, 0x3344, 0x3372, 0x339F, 0x33CB
    .half 0x33F7, 0x3422, 0x344D, 0x3477, 0x34A1, 0x34CA, 0x34F3, 0x351B, 0x3543, 0x356A, 0x3590, 0x35B6, 0x35DC, 0x3601, 0x3626, 0x364A
    .half 0x366D, 0x3691, 0x36B3, 0x36D6, 0x36F7, 0x3719, 0x373A, 0x375A, 0x377A, 0x379A, 0x37B9, 0x37D8, 0x37F6, 0x3814, 0x3831, 0x384F
    .half 0x386B, 0x3888, 0x38A4, 0x38BF, 0x38DA, 0x38F5, 0x3910, 0x392A, 0x3943, 0x395D, 0x3976, 0x398E, 0x39A7, 0x39BF, 0x39D6, 0x39ED
    .half 0x3A04, 0x3A1B, 0x3A31, 0x3A47, 0x3A5D, 0x3A72, 0x3A88, 0x3A9C, 0x3AB1, 0x3AC5, 0x3AD9, 0x3AED, 0x3B00, 0x3B13, 0x3B26, 0x3B38
    .half 0x3B4B, 0x3B5D, 0x3B6E, 0x3B80, 0x3B91, 0x3BA2, 0x3BB3, 0x3BC3, 0x3BD4, 0x3BE4, 0x3BF3, 0x3C03, 0x3C12, 0x3C21, 0x3C30, 0x3C3F
    .half 0x3C4D, 0x3C5C, 0x3C6A, 0x3C78, 0x3C85, 0x3C93, 0x3CA0, 0x3CAD, 0x3CBA, 0x3CC6, 0x3CD3, 0x3CDF, 0x3CEB, 0x3CF7, 0x3D03, 0x3D0F
    .half 0x3D1A, 0x3D25, 0x3D30, 0x3D3B, 0x3D46, 0x3D51, 0x3D5B, 0x3D65, 0x3D6F, 0x3D79, 0x3D83, 0x3D8D, 0x3D96, 0x3DA0, 0x3DA9, 0x3DB2
    .half 0x3DBB, 0x3DC4, 0x3DCD, 0x3DD5, 0x3DDE, 0x3DE6, 0x3DEE, 0x3DF6, 0x3DFE, 0x3E06, 0x3E0E, 0x3E15, 0x3E1D, 0x3E24, 0x3E2C, 0x3E33
    .half 0x3E3A, 0x3E41, 0x3E47, 0x3E4E, 0x3E55, 0x3E5B, 0x3E62, 0x3E68, 0x3E6E, 0x3E75, 0x3E7B, 0x3E81, 0x3E86, 0x3E8C, 0x3E92, 0x3E97
    .half 0x3E9D, 0x3EA2, 0x3EA8, 0x3EAD, 0x3EB2, 0x3EB7, 0x3EBC, 0x3EC1, 0x3EC6, 0x3ECB, 0x3ED0, 0x3ED4, 0x3ED9, 0x3EDE, 0x3EE2, 0x3EE6
    .half 0x3EEB, 0x3EEF, 0x3EF3, 0x3EF7, 0x3EFB, 0x3EFF, 0x3F03, 0x3F07, 0x3F0B, 0x3F0F, 0x3F12, 0x3F16, 0x3F1A, 0x3F1D, 0x3F21, 0x3F24
    .half 0x3F28, 0x3F2B, 0x3F2E, 0x3F31, 0x3F35, 0x3F38, 0x3F3B, 0x3F3E, 0x3F41, 0x3F44, 0x3F47, 0x3F49, 0x3F4C, 0x3F4F, 0x3F52, 0x3F54
    .half 0x3F57, 0x3F5A, 0x3F5C, 0x3F5F, 0x3F61, 0x3F64, 0x3F66, 0x3F68, 0x3F6B, 0x3F6D, 0x3F6F, 0x3F72, 0x3F74, 0x3F76, 0x3F78, 0x3F7A
    .half 0x3F7C, 0x3F7E, 0x3F80, 0x3F82, 0x3F84, 0x3F86, 0x3F88, 0x3F8A, 0x3F8C, 0x3F8D, 0x3F8F, 0x3F91, 0x3F93, 0x3F94, 0x3F96, 0x3F98
    .half 0x3F99, 0x3F9B, 0x3F9C, 0x3F9E, 0x3F9F, 0x3FA1, 0x3FA2, 0x3FA4, 0x3FA5, 0x3FA7, 0x3FA8, 0x3FA9, 0x3FAB, 0x3FAC, 0x3FAD, 0x3FAE
    .half 0x3FB0, 0x3FB1, 0x3FB2, 0x3FB3, 0x3FB5, 0x3FB6, 0x3FB7, 0x3FB8, 0x3FB9, 0x3FBA, 0x3FBB, 0x3FBC, 0x3FBD, 0x3FBE, 0x3FBF, 0x3FC0
    .half 0x3FC1, 0x3FC2, 0x3FC3, 0x3FC4, 0x3FC5, 0x3FC6, 0x3FC7, 0x3FC8, 0x3FC9, 0x3FC9, 0x3FCA, 0x3FCB, 0x3FCC, 0x3FCD, 0x3FCE, 0x3FCE
    .half 0x3FCF, 0x3FD0, 0x3FD1, 0x3FD1, 0x3FD2, 0x3FD3, 0x3FD3, 0x3FD4, 0x3FD5, 0x3FD5, 0x3FD6, 0x3FD7, 0x3FD7, 0x3FD8, 0x3FD9, 0x3FD9
    .half 0x3FDA, 0x3FDA, 0x3FDB, 0x3FDB, 0x3FDC, 0x3FDD, 0x3FDD, 0x3FDE, 0x3FDE, 0x3FDF, 0x3FDF, 0x3FE0, 0x3FE0, 0x3FE1, 0x3FE1, 0x3FE2
    .half 0x3FE2, 0x3FE3, 0x3FE3, 0x3FE3, 0x3FE4, 0x3FE4, 0x3FE5, 0x3FE5, 0x3FE6, 0x3FE6, 0x3FE6, 0x3FE7, 0x3FE7, 0x3FE8, 0x3FE8, 0x3FE8
    .half 0x3FE9, 0x3FE9, 0x3FE9, 0x3FEA, 0x3FEA, 0x3FEA, 0x3FEB, 0x3FEB, 0x3FEB, 0x3FEC, 0x3FEC, 0x3FEC, 0x3FED, 0x3FED, 0x3FED, 0x3FED
    .half 0x3FEE, 0x3FEE, 0x3FEE, 0x3FEE, 0x3FEF, 0x3FEF, 0x3FEF, 0x3FF0, 0x3FF0, 0x3FF0, 0x3FF0, 0x3FF0, 0x3FF1, 0x3FF1, 0x3FF1, 0x3FF1
    .half 0x3FF2, 0x3FF2, 0x3FF2, 0x3FF2, 0x3FF2, 0x3FF3, 0x3FF3, 0x3FF3, 0x3FF3, 0x3FF3, 0x3FF4, 0x3FF4, 0x3FF4, 0x3FF4, 0x3FF4, 0x3FF5
    .size	sigmoid_LUT, 1026
