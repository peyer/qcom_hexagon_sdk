/**=============================================================================
@file
    qhmath_inv_h.S

@brief
    Computes fixed-point inversion.

    Function prototype

        result_scale_t qhmath_inv_h(int32_t); where

        result_scale_t
        {
            Word32 result;
            Word32 scale;
        }

    Reference C code

        result_scale_t qhmath_inv_h(Word32)
        {
            UWord32 L_num, L_denom;
            Word16 nshft, result;
            int iteration;
            result_scale_t rval;

            if (input <= 0) {
                rval.result = -1;
                rval.scale  = -1;
                return rval;
            }

            L_denom = (UWord32)input << 16;
            nshft = norm_l(L_denom);
            L_denom = L_shl(L_denom, nshft);

            L_num = 0x80000000;
            result = 0;

            for (iteration = 0; iteration < 15; iteration++) {

                result <<= 1;

                if (L_num >= L_denom)
                {
                    L_num -= L_denom;
                    result++;
                }
                L_num <<= 1;
            }

            nshft = 29 - nshft;

            rval.result = result;
            rval.scale  = nshft;
            return rval;
        }

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

/*============================================================================*/

    .p2align 2
    .p2align 4,,15
    .global qhmath_inv_h
    .type    qhmath_inv_h, @function

/*============================================================================*/

qhmath_inv_h:
{
    R2 = #0x4000                               // constant
    R3 = ASLH(R0)                              // L_denom = input<<16
    R4.H = #0x8000                             // L_num = 0x80000000
    R0 = #0                                    // initialize result=0
}
{
    P0 = CMP.GT(R3,#0)                         // P0 = (input >0)
    IF !P0.new JUMP:nt .L_invert_le0           // if (input<=0) jump invert_le0
    R4.L = #0                                  // L_num = 0x80000000
    R5 = NORMAMT(R3)                           // nshft = norm_l(L_denom)
}
{
    R3 = ASL(R3,R5)                            // L_denom <<= nshft
    R1 = SUB(#29,R5)                           // nshft = 29 - nshft
    LOOP0(.L_invert_loop,#14)                  // setup loop (unroll once)
}
    .falign
.L_invert_loop:
{
    P0 = CMP.GTU(R3,R4)                        // if (L_num>= L_denom)
    IF !P0.new R4 = SUB(R4,R3)                 // then L_num -= L_denom
    IF !P0.new R0 = ADD(R0,R2)                 // then  result++
    R3:2 = VLSRW(R3:2,#1)                      // equivalent to L_denom <<1, result<<1
}:endloop0

{
    P0 = CMP.GTU(R3,R4)                        // if (L_num>= L_denom)
    IF !P0.new R0 = ADD(R0,R2)                 // then  result++
    JUMPR R31                                  // return
}
.L_invert_le0:
{
    R1:0 = COMBINE(#-1,#-1)                    // R1:0 = -1 : -1
    JUMPR R31                                  // return
}
    .size    qhmath_inv_h, .-qhmath_inv_h
