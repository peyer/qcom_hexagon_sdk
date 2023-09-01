/**=============================================================================
@file
    qhdsp_fir_h.S

@brief
    FIR filtering on a bulk of data.

    [=============================================================================]
    [  Assumptions:                                                               ]
    [           - length and Taps are multiple of 4                               ]
    [           - length > 0 * Taps >= 8 (callee-checked)                         ]
    [           - Input/Output array is aligned by 8                              ]
    [           - Coefficient array is aligned by 8                               ]
    [           - Coefficients arranged in reverse order                          ]
    [=============================================================================]
    [  Inputs     : r0 : int16_t *in_samples   -- pointer to input buffer         ]
    [               r1 : int16_t *coefs -- pointer to Coefficients                ]
    [               r2 : uint32_t taps      -- Number of Taps                     ]
    [               r3 : uint32_t length    -- Number of Samples                  ]
    [               r4 : int16_t *out_samples  -- pointer to output buffer        ]
    [=============================================================================]
    [  Register Usage: r0-r23, r28                                                ]
    [  Hardware Loops affected: loop0, loop1                                      ]
    [                                                                             ]
    [  Stack Memory Frame Allocated (in Bytes): 56                                ]
    [=============================================================================]
    [  Implementation:                                                            ]
    [           - process 4 samples per iteration in inner loop                   ]
    [           - generate 4 outputs per iteration in outer loop                  ]
    [=============================================================================]
    [  Cycle Count:                                                               ]
    [           - (N/4)*[3*(T/4)+4] + 9                                           ]
    [                                                                             ]
    [=============================================================================]

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

    .file   "qhdsp_fir_h.S"

    .text
    .p2align 2
    .p2align 4,,15
    .globl qhdsp_fir_h
    .type    qhdsp_fir_h, @function

/*============================================================================*/

#define L2FETCH_CONFIG    0x0100FF00    // initial L2FETCH config [stride = 256 : width = 255 : height = 0]

/*============================================================================*/

qhdsp_fir_h:
{
    p0 = !cmp.eq(r0, #0)                       // check NULL pointer - in_samples
    p0 = !cmp.eq(r1, #0)                       // check NULL pointer - coefs
    p0 = cmp.gtu(r3, #0)                       // check length > 0
    p0 = !cmp.eq(r4, #0)                       // check NULL pointer - out_samples
    if (!p0.new) jump:nt .L_input_error
}
{
    r12 = #1                                   //[ minimum lines to prefetch           ]
    r13 = lsr(r3, #7)                          //[ calculate lines to prefetch for in_samples (length/(256/sizeof(int16_t))) ]
    r14 = lsr(r2, #7)                          //[ calculate lines to prefetch for coefs (length/(256/sizeof(int16_t)))      ]
    p0 = cmp.gtu(r2,#7)                        //[ p0 = t/4 > 1                        ]
    if !p0.new jump:nt .L_input_error          //[ if !p0 return                       ]
}
{
    r13 = max(r13, r12)                        //[ avoid zero lines to prefetch        ]
    r14 = max(r14, r12)                        //[ avoid zero lines to prefetch        ]
    allocframe(#6*8)                           //[ reserve stack - implicitly allocates 8bytes extra for R31:30
}
{
    r13 = add(r13, ##L2FETCH_CONFIG)           //[ add lines to prefetch to initial L2FETCH config ]
    r14 = add(r14, ##L2FETCH_CONFIG)           //[ add lines to prefetch to initial L2FETCH config ]
}
{
    r28 = add(r1,#0)                           //[ save coefs in r28                   ]
    l2fetch(r0, r13)
}
{
    p3 = cmp.gt(r2,r2)                         //[ clean p3                            ]
    l2fetch(r1, r14)
}
{
    memd(r29+#0) = r1:0                        //[ save input params                   ]
    memd(r29+#8) = r3:2                        //[ save input params                   ]
    r3:2  = vasrw(r3:2, #2)                    //[ n/4 : t/4
}
{
    r2 = add(r2,#-2)                           //[ lc0 = t/4-2                         ]
    memd(r29+#16) = r17:16                     //[ callee-saved registers              ]
    r7:6 = memd(r0++#8)                        //[ r7:6 = x3|x2|x1|x0                  ]
}
{
    memd(r29+#24) = r19:18                     //[ callee-saved registers              ]
    r10  = memw(r0+#0)                         //[ r10 = x5|x4                         ]
    loop1(.L_qhdsp_fir_h_mainloop,r3)          //[ setup outer loop:lc1 = n/4          ]
}
{
    memd(r29+#32) = r21:20                     //[ callee-saved registers              ]
    r9:8 = combine(r10,r7)                     //[ r9:8 = x5|x4|x3|x2,                 ]
    r15:14 = memd(r1++#8)                      //[ load coefficients                   ]
    r3 = add(r0,#8)                            //[ set read pointer                    ]
}
{
    memd(r29+#40) = r23:22                     //[ callee-saved registers              ]
    r17:16 = vrmpyh(r7:6,r15:14)               //[ [p] sum0 += coefs[] * x[]           ]
    r19:18 = vrmpyh(r9:8,r15:14)               //[ [p] sum2 += coefs[] * x[]           ]
    r12 = memw(r0+#4)                          //[ [p] r12 = x7|x6                     ]
}
    .falign
.L_qhdsp_fir_h_mainloop:
{
    if p3 memd(r4++#8) = r21:20                //[ [e1]save yout[i] -- yout[i+4]       ]
    r7:6 = combine(r12,r10)                    //[ r7:6 = x7|x6|x5|x4                  ]
    r11:10 = valignb(r11:10,r7:6,#2)           //[ r11:10 = x4|x3|x2|x1                ]
    loop0(.L_qhdsp_fir_h_innerloop,r2)         //[ setup inner loop: lc0 = t/4-2       ]
}
{
    r13:12 = valignb(r13:12,r9:8,#2)           //[ r13:12 = x6|x5|x4|x3                ]
    r21:20 = vrmpyh(r11:10,r15:14)             //[ sum1 += coefs[] * x[]               ]
    r9 = memw(r3++#4)                          //[ r8 = x9|x8                          ]
    p0 = cmp.gt(r2,#0)                         //[ p0 = (lc0 > 0)                      ]
}
{
    r15:14 = memd(r1++#8)                      //[ load coefficients                   ]
    r23:22 = vrmpyh(r13:12,r15:14)             //[ sum3 += coefs[] * x[]               ]
    r8 = r7                                    //[ r9:8 = x9|x8|x7|x6                  ]
    if !p0 jump .L_qhdsp_fir_h_innerlpend      //[ if !(lc0>0) skip loop               ]
}
    .falign
.L_qhdsp_fir_h_innerloop:
{
    r10 = r9                                   //[[1]r10 = x5|x4                       ]
    r12 = memw(r3++#4)                         //[[1]r12 = x7|x6                       ]
    r17:16 += vrmpyh(r7:6,r15:14)              //[[2]sum0 += coefs[] * x[]             ]
    r19:18 += vrmpyh(r9:8,r15:14)              //[[2]sum2 += coefs[] * x[]             ]
}
{
    r11:10 = valignb(r11:10,r7:6,#2)           //[[1]r11:10 = x4|x3|x2|x1              ]
    r13:12 = valignb(r13:12,r9:8,#2)           //[[1]r13:12 = x6|x5|x4|x3              ]
    r7:6 = combine(r12,r10)                    //[[1]r7:6 = x7|x6|x5|x4                ]
    r9 = memw(r3++#4)                          //[[1]r8 = x9|x8                        ]
}
{
    r21:20 += vrmpyh(r11:10,r15:14)            //[[1]sum1 += coefs[] * x[]             ]
    r23:22 += vrmpyh(r13:12,r15:14)            //[[1]sum3 += coefs[] * x[]             ]
    r8 = r7                                    //[[1]r9:8 = x9|x8|x7|x6                ]
    r15:14 = memd(r1++#8)                      //[[1]load coefficients                 ]
}:endloop0
    .falign
.L_qhdsp_fir_h_innerlpend:
{
    r17:16 += vrmpyh(r7:6,r15:14)              //[ sum0 += coefs[] * x[]               ]
    r19:18 += vrmpyh(r9:8,r15:14)              //[ sum2 += coefs[] * x[]               ]
    r10 = r9                                   //[ r10 = x5|x4                         ]
    r12 = memw(r3++#4)                         //[ r12 = x7|x6                         ]
}
{
    r11:10 = valignb(r11:10,r7:6,#2)           //[ r11:10 = x4|x3|x2|x1                ]
    r13:12 = valignb(r13:12,r9:8,#2)           //[ r13:12 = x6|x5|x4|x3                ]
    r7:6 = memd(r0++#8)                        //[ [o1] r7:6 = x3|x2|x1|x0             ]
    r9 = memw(r0+#8)                           //[ [o1] r9 = x5|x4                     ]
}
{
    r21:20 += vrmpyh(r11:10,r15:14)            //[ sum1 += coefs[] * x[]               ]
    r23:22 += vrmpyh(r13:12,r15:14)            //[ sum3 += coefs[] * x[]               ]
    r3 = add(r0,#4)                            //[ [o1]                                ]
    r1 = r28                                   //[ [o1]set r1 = coeff                  ]
}
{
    r17:16 = asr(r17:16,#15)                   //[ sum0 >>= 15                         ]
    r21:20 = asr(r21:20,#15)                   //[ sum1 >>= 15                         ]
    r8 = r7                                    //[ [o1] r9:8 = x5|x4|x3|x2             ]
    r10 = r9                                   //[ [o1] r10 = x5|4                     ]
}
{
    r19:18 = asr(r19:18,#15)                   //[ sum2 >>= 15                         ]
    r23:22 = asr(r23:22,#15)                   //[ sum3 >>= 15                         ]
    r15:14 = memd(r1++#8)                      //[ [o1] load coefficients              ]
}
{
    r21:20 = combine(r20,r16)                  //[ pack to sum1:sum0                   ]
    r23:22 = combine(r22,r18)                  //[ pack to sum3:sum2                   ]
    r17:16 = vrmpyh(r7:6,r15:14)               //[ [o1] sum0 += coefs[] * x[]          ]
    r19:18 = vrmpyh(r9:8,r15:14)               //[ [o1] sum2 += coefs[] * x[]          ]
}
{
    r20 = vsatwh(r21:20)                       //[ saturate to 16-bit                  ]
    r21 = vsatwh(r23:22)                       //[ saturate to 16-bit                  ]
    p3 = cmp.eq(r2,r2)                         //[ set p3 (for conditional write)      ]
    r12 = memw(r3++#4)                         //[ [o1] r12 = x7|x6                    ]
}:endloop1
{
    memd(r4) = r21:20                          //[[e1]save output                      ]
}
{
    r3:2 = memd(r29+#8)                        //[ restore input params                ]
    r21:20 = memd(r29+#32)                     //[ restore callee-saved registers      ]
}
{
    r3 = add(r3,#-1)                           // length-1
    r23:22 = memd(r29+#40)                     //[ restore callee-saved registers      ]
    r1:0 = memd(r29+#0)                        //[ restore input params                ]
}
{
    r19:18 = memd(r29+#24)                     //[ restore callee-saved registers      ]
    r17:16 = memd(r29+#16)                     //[ restore callee-saved registers      ]
    r1 = addasl(r0,r2,#1)                      // in_samples[taps]
    r2 = asl(r3,#1)                            // (length-1)*sizeof(int16_t)
}
{
    // memmove((void *)in_samples, (const void *)&in_samples[taps], (length-1)*sizeof(int16_t));
    call memmove
}
{
    r0 = #0
    dealloc_return                             //[ pop stack and return                ]
}
    .falign
.L_input_error:
{
    r0 = #-1
    jumpr r31
}
    .size    qhdsp_fir_h, .-qhdsp_fir_h
