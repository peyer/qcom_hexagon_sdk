/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file "dsplib2.S"

    /*[*****************************************************************************]*/
    /*[  Function   : result_scale_t approx_uinv_linlut8()                          ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Approximate unsigned inverse using linear 8-entry lookup table]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord32 input                                            ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R3                                                      ]*/
    /*[  Hardware Loops affected: None                                              ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 5                                                               ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl approx_uinv_linlut8
    .type   approx_uinv_linlut8, @function
approx_uinv_linlut8:
    { R1 = CL0(R0)                          //[ R1 = #leading 0's of input          ]
      P0 = CMP.EQ(R0,#0)                    //[ p0 = (input==0)?                    ]
      IF P0.new JUMP:nt .approx_uinv_linlut8_0
                                            //[ handle input==0 case                ]
      R3.H = #HI(approx_uinv_linlut8_LUT)   //[ R3=start addr. of LUT               ]
    }
    { R3.L = #LO(approx_uinv_linlut8_LUT)   //[ R3=start addr. of LUT               ]
      R0 = ASL(R0,R1)                       //[ R0=d ..norm. input in range [.5,1)  ]
      R1 = SUB(#30, R1)                     //[ scale_factor=30-cl0 ..in [-1,30]    ]
    }
    { R3 = TABLEIDXW(R0,#3 ,#28 )           //[ r3=LUT addr+4*ind..ind=d[30:28]     ]
      R2 = EXTRACTU(R0,#15 ,#13 )           //[ r2=dn= d[27:13]..15 bits ..fraction ]
                                            //[ left after extracting the LUT       ]
                                            //[ index for the segment.              ]
                                            //[ d:||1 |ind     |dn      | ignored ||]
                                            //[     31 30-28(3) 27-13(15) 12-0(13)  ]
    }
    { R0 = MEMW(R3)                         //[ init. result=r0= c<<16 + m          ]
      R2 = ADD(R2, #1)                      //[ Add 1 to dn, as m is already added  ]
                                            //[ into the result.
                                            //[ Thus, result=(c<<16 + m) -m*(dn+1)  ]
                                            //[             = c<<16 - m*dn          ]
                                            //[ Q of terms:-   1st: Q30(=14+16)     ]
                                            //[                2nd: Q30(=15+15)     ]
    }
    { R0 -= MPYU(R0.L,R2.L)                 //[ result = c<<16 - m*dn    Q30        ]
      JUMPR R31                             //[ return                              ]
    }
.approx_uinv_linlut8_0:
    { R1:0 = COMBINE(#0,#-1)                //[ scale_factor=0,result=0xFFFFFFFF    ]
      JUMPR R31                             //[ return                              ]
    }
    .size   approx_uinv_linlut8, .-approx_uinv_linlut8

    /*-----------------------------------------------------------------------------*/
    /*            8-entry lookup table for approx_uinv_linlut8                     */
    /*-----------------------------------------------------------------------------*/
    .data
    .p2align 5
    .type   approx_uinv_linlut8_LUT, @object
    .size   approx_uinv_linlut8_LUT, 32
approx_uinv_linlut8_LUT:
    /*--------------------------------------*/
    /*   ||  m  || c  ||                    */
    /*--------------------------------------*/
    .hword  7273, 32712                     
    .hword  5820, 29087
    .hword  4762, 26185
    .hword  3969, 23809
    .hword  3359, 21828
    .hword  2879, 20151
    .hword  2496, 18714
    .hword  2184, 17467



    /*[*****************************************************************************]*/
    /*[  Function   : result_scale_t approx_uinv_cub8()                             ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Approximate unsigned inverse using cubic polynomial approx.   ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord32 input                                            ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R6                                                      ]*/
    /*[  Hardware Loops affected: None                                              ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 7                                                               ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl approx_uinv_cubelut8
    .type   approx_uinv_cubelut8, @function
approx_uinv_cubelut8:
    { P0 = CMP.EQ(R0,#0)                    //[ p0 = (input==0)?                    ]
      IF P0.new JUMP:nt .approx_uinv_cubelut8_0  
                                            //[ handle input==0 case                ]
      R6 = CL0(R0)                          //[ r6 = #leading 0s in input           ]
      R4.H = #HI(approx_uinv_cubelut8_coefs)//[ r4=start addr. of LUT               ]
    }
    { R0 = ASL(R0,R6)                       //[ R0=d ..norm. input in range [.5,1)  ]
      R4.L = #LO(approx_uinv_cubelut8_coefs)//[ r4=start addr. of LUT               ]
      R2 = #0                               //[ clear r2 = dn for insert op         ]
    }
    { R4 = TABLEIDXD(R0, #3, #28)           //[ R4=LUT addr+8*ind..ind=d[31:28]     ]
      R2 = INSERT(R0, #28, #1)              //[ R2 =dn=2*d[27:0]..Q29               ]
                                            //[ r2=dn=2*d[27:0] in Q31..fraction    ]
                                            //[ left after extracting the LUT index ]
                                            //[ for the segment. offset of 3 gives  ]
                                            //[ * by 8.                             ]
                                            //[ d:   ||1 |   ind     |  dn     ||   ]
                                            //[        31    30-28(3)  27-0(28)     ]
    }
    { R5:4 = MEMD(R4)                       //[ Load coefficients c0 through c3 as  ]
                                            //[ c0,c1,c3,c2 i.e. c0 in lowest hw, c1]
                                            //[ in the next, c3 in the penultimate  ]
                                            //[ and c2 in highest hw                ]
      R3 = MPY(R2,R2)                       //[ dn^2                 Q26            ]
    }
    { R1:0 = VMPYWOH(R3:2,R5:4):rnd:sat     //[ R1  = c2*dn^2    Q30(=20+26-16)     ]
                                            //[ R0  = c1*dn^1    Q30(=17+29-16)     ] 
      R3 = MPY(R2,R3)                       //[ dn^3                 Q23            ]
      R2 = #-32768                          //[ 1..Q15                              ]
    }
    { R0 += MPYU(R2.L,R4.L)                 //[ c0<<15 + c1*dn^1..Q30(=15+15)       ]
      R2  = MPY(R3, R5.L):<<1:rnd:sat       //[ r2=c3*dn^3  Q30(=22+23-16+1)        ]
    }
    { R0+= ADD(R1,R2)                       //[ Calculate result=r0 =r0+r1+r2       ]
                                            //[ =c0<<16 +c1*dn^1 +c2*dn^2 +c3*dn^3  ]
      R1 = SUB(#30, R6)                     //[ scale_factor=30-cl0 ..in [-1,30]    ]
      JUMPR R31                             //[ return                              ]
    }
.approx_uinv_cubelut8_0:
    { R1:0 = COMBINE(#0,#-1)                //[ scale_factor=0,result=0xFFFFFFFF    ]
      JUMPR R31                             //[ return                              ]
    }
    .size   approx_uinv_cubelut8, .-approx_uinv_cubelut8


    /*-----------------------------------------------------------------------------*/
    /*                 lookup table for approx_uinv_cube8                          */
    /*-----------------------------------------------------------------------------*/
    .data
    .p2align 6
    .type   approx_uinv_cubelut8_coefs, @object
    .size   approx_uinv_cubelut8_coefs, 64
approx_uinv_cubelut8_coefs:
    /*--------------------------------------*/
    /*       c0      c1      c3     c2      */
    /*       Q15     Q17     Q22    Q20     */
    /*--------------------------------------*/
    .hword  65535, -32748, -12909, 32241
    .hword  58254, -25881,  -8266, 22715
    .hword  52429, -20969,  -5536, 16597
    .hword  47663, -17332,  -3846, 12491
    .hword  43691, -14564,  -2754,  9635
    .hword  40330, -12409,  -2024,  7586
    .hword  37449, -10698,  -1520,  6079
    .hword  34953,  -9323,  -1164,  4946






    /*[*****************************************************************************]*/
    /*[  Function   : result_scale_t approx_uinv_newt()                             ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Approximate unsigned inverse using Newton-Raphson interative  ]*/
    /*[               method.                                                       ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord32 input                                            ]*/
    /*[               R1 : UWord32 iter                                             ]*/ 
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R9                                                      ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 4 + 2*niter                                                     ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl approx_uinv_newt
    .type   approx_uinv_newt, @function
approx_uinv_newt:
    { P0 = CMP.EQ(R0,#0)                    //[ check if input=0                    ]
      IF P0.new JUMP:nt .approx_uinv_newt_0 //[ handle input==0 case                ]
      R6 = CL0(R0)                          //[ #leading 0s in input                ]
      R3.H = #HI(1963229551)                //[ x.uw[1]  .9142     Q31              ]
    }
    {
      R3.L = #LO(1963229551)                //[ x.uw[1]                             ]
      R2 = #0                               //[ x.uw[0]                             ]
      LOOP0(.newtons_LOOP,R1)               //[ loop setup                          ]
    }
    {
      R0 = ASL(R0,R6)                       //[ r0=d=norm. input in range [.5,1)    ]
      R3-= ASL(R0,R6)                       //[ d in Q32 can be interpreted as 2*d  ]
                                            //[ if d were in Q31. Use the trick     ]
                                            //[  2-x=-x for x in Q31. Thus,         ]
                                            //[  r3= x= 2.9142 -2*d                 ]
                                            //[       = 0.9142-2*d in Q31           ]
      R8 = #0                               //[ init. loop counter n                ]
    }
    .falign
.newtons_LOOP:
    { R7 = MPYU(R0,R3)                      //[ Calculate a= (d *x.uw[1])>> 32  ..  ]
                                            //[           in Q31-n(=32 +31-n -32)   ]
      R5:4 = ASL(R3:2,R8)                   //[ xp = x<<n     Q31(=31-n +n)         ]
    }
    { R3:2-= MPYU(R7,R5)                    //[ x=2*x-d*x*xp  Q31-n-1(=31-n+31-32)  ]
      R8 = ADD(R8, #1)                      //[ n++                                 ]
      R9 = R8                               //[(n-1)++                              ]
    }:endloop0                              //[  start=newtons_LOOP                 ]
    
    { R1 = SUB(#30,R6)                      //[ scale_factor=30-cl0 ..in [-1,30]    ]
      R0 = ASL(R3,R9):sat
                                            //[ For explanation of use of asl:sat op] 
                                            //[ refer prototype details under       ]
                                            //[ loop instability                    ]
      JUMPR R31                             //[ return                              ]
    }
.approx_uinv_newt_0:
    { R1:0 = COMBINE(#0,#-1)                //[ scale_factor=0,result=0xFFFFFFFF    ]
      JUMPR R31                             //[ return                              ]
    }
    .size   approx_uinv_newt, .-approx_uinv_newt




    /*[*****************************************************************************]*/
    /*[  Function   : result_scale_t approx_uinv_tylr()                             ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Approximate unsigned inverse using Taylor's series expansion  ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - hi_pow > 2                                                      ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord32 input                                            ]*/
    /*[               R1 : UWord32 hi_pow                                           ]*/ 
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R9                                                      ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 6 + 1*hi_pow                                                    ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl approx_uinv_tylr
    .type   approx_uinv_tylr, @function
approx_uinv_tylr:
    { R9 = CL0(R0)                          //[ #leading 0s in input                ]
      R3.H = #HI(1430224110)                //[ c =.666  Q31 (1/d= x ~= c -m*d)     ]
      R8.H = #HI(3818258432)                //[ m =1.778 Q31 (1/d= x ~= c -m*d)     ]
      R2 = #0
    }
    { R0 = ASL(R0,R9)                       //[ r0 =d=norm. input in range [.5,1)   ]
      R3.L = #LO(1430224110)                //[ c                                   ]
      R8.L = #LO(3818258432)                //[ m, slope of init. linear approx of  ]
                                            //[     1/d                             ]
      R7.H = #HI(2147483648)                //[ 0x80000000L    setup for a=1-d*x    ]
    }
    { R3:2-= MPYU(R8, R0)                   //[ d in Q32 can be interpreted as 2*d  ]
                                            //[ if d were in Q31. Use the trick     ]
                                            //[ 2-x=-x for x in Q31. Thus,          ]
                                            //[ r3:2=x =2.666 -1.778*d              ]
                                            //[        =.666 -1.778*d in Q31        ]
      R6 = #0                               //[ setup for a=1-d*x                   ]
      R7.L = #LO(2147483648)                //[ 0x80000000L                         ]
    }
    { R7:6 -= MPYU(R0,R3)                   //[ a = 1-d*x           Q31             ]
      R1 = ADD(R1,#-2)                      //[ unrolled 2 loops at start           ]
      P0 = CMP.EQ(R0,#0)                    //[ check if input=0                    ]
      IF P0.new JUMP:nt .approx_uinv_tylr_0 //[ handle input==0 case                ]
    }
    { R7:6 = ASL(R7:6, #1)                  //[ convert a in Q31 to Q32 to avoid    ]
                                            //[ 1 bit loss per higher power of a    ]
      LOOP0(.taylors_LOOP,R1)               //[ loop setup                          ]
    }
    { R7:6+=MPYU(R7, R7)                    //[ sum+=a^2            Q32             ]
      R5 = MPYU(R7,R7)                      //[ a^2                                 ]
      R4 = R7                               //[ save R4 = a = 1-d*x                 ]
      R1 = SUB(#31,R9)                      //[ scale_factor=31-cl0 ..in [0,31]     ]
    }

    .falign
.taylors_LOOP:
    {
      R7:6 += MPYU(R5,R4)                   //[ sum+=a^k  k in [3,hi_pow] Q32       ]
      R5    = MPYU(R5,R4)                   //[ a^k       k in [3,hi_pow]           ]
    }:endloop0                              //[ start = taylors_LOOP                ]

    { R7:6 = MPYU(R3,R7)                    //[ sum1= x*sum               Q31       ]
    }
    { R0 = ADD(R3,R7)                       //[ 1/d=x/(1-d*x)=x + sum1    Q31       ]
      JUMPR R31                             //[ return                              ]
    }
.approx_uinv_tylr_0:
    { R1:0 = COMBINE(#0,#-1)                //[ scale_factor=0,result=0xFFFFFFFF    ]
      JUMPR R31                             //[ return                              ]
    }
    .size   approx_uinv_tylr, .-approx_uinv_tylr




    /*[*****************************************************************************]*/
    /*[  Function   : result_scale_t approx_dB_to_ulin_linlut8()                    ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Approximate dB to linear range conversion using linear        ]*/
    /*[               8-entry lookup table.                                         ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word32 input                                             ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R4                                                      ]*/
    /*[  Hardware Loops affected: None                                              ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 5                                                               ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl approx_dB_to_ulin_linlut8
    .type   approx_dB_to_ulin_linlut8, @function
approx_dB_to_ulin_linlut8:
    { R2.L = #LO(43541)                     //[ r2 = .1*log2(10)=.3321928..Q17      ]
      R4.H = #HI(approx_dB_to_ulin_linlut8_LUT)
                                            //[ r4=start addr. of LUT               ]
    }                                                                             
    { R3:2 = VMPYWEUH(R1:0, R3:2):sat       //[ Calculate d = i+f. r2 = d           ]
                                            //[   = input*.3321928..Q25(=24+17-16)  ]
                                            //[ use vmpy to do s32 x u16.           ]
      R4.L = #LO(approx_dB_to_ulin_linlut8_LUT)   
                                            //[ r4=start addr. of LUT               ]
    }
    {                                       //[ in assembly we use 32x16u for       ]
                                            //[ computation of d, so d is in Q25    ]
                                            //[ and all bit fields are offseted by  ]
                                            //[ "1" comparing to C reference code   ]
                                            //[            <-------f--------------> ]
                                            //[ d:|  i    | ind    | fn    |ignored ]
                                            //[   31:25(7) 24:22(3) 21:7(15) 6:0(7) ]
      R4 = TABLEIDXW(R2,#3 ,#22 )           //[ R4=LUT addr+4*ind..ind=d[24:22]     ]
      R3 = EXTRACTU(R2,#15, #7 )            //[ R3=fn = d[21:7]..Q15                ]
    }
    { R0 = MEMW(R4)                         //[ Initialize the result=r0 with       ]
                                            //[  c<<16 + m..Q31(15+16)              ]
      R1 = ASR(R2, #25)                     //[ scale = i+1..i=d[31:25]. scale lies ]
                                            //[ in range [-31, 32]                  ]
      R3 = ADD(R3, #-1)                     //[ Calculate fn-1 to compensate for m  ]
                                            //[ already loaded into the result      ]
    }                                                                             
    { R0 += MPY(R0.L,R3.L)                  //[ Calculate result=c<<16 + m*fn       ]
                                            //[           =(c<<16 + m) + m*(fn-1)   ]
      R1 = SUB(#-1, R1)                     //[ scale=-1-i..in [-31,32]             ]
      JUMPR R31                             //[ return                              ]
    }
    .size   approx_dB_to_ulin_linlut8, .-approx_dB_to_ulin_linlut8


    /*-----------------------------------------------------------------------------*/
    /*                 lookup table for approx_dB_to_ulin_linlut8                  */
    /*-----------------------------------------------------------------------------*/
    .data
    .p2align 5
    .type   approx_dB_to_ulin_linlut8_LUT, @object
    .size   approx_dB_to_ulin_linlut8_LUT, 32
approx_dB_to_ulin_linlut8_LUT:
    /*--------------------------------------*/
    /*   ||  m  ||  c   ||                  */
    /*       16     15                      */
    /*--------------------------------------*/
    .hword  5931, 32752
    .hword  6467, 35717
    .hword  7053, 38949
    .hword  7691, 42474
    .hword  8387, 46319
    .hword  9146, 50511
    .hword  9974, 55082
    .hword 10877, 60068



    /*[*****************************************************************************]*/
    /*[  Function   : result_scale_t approx_dB_to_ulin_cubefit()                    ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Approximate dB to linear range conversion using a single      ]*/
    /*[               cubic polynomail fit.                                         ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - cubic coefs c0:c3:- 32764, 22833, 7346, 2588                    ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word32 input                                             ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R4                                                      ]*/
    /*[  Hardware Loops affected: None                                              ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 5                                                               ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl approx_dB_to_ulin_cubefit
    .type	approx_dB_to_ulin_cubefit, @function
approx_dB_to_ulin_cubefit:
    { R2 = CONST32(#5573270)                //[ R2 = .1*log2(10)= .3321928..Q24     ]
      R1.L = #32764                         //[ R1.l = c0..Q15                      ]
    }
    {                                       //[ d:   ||  i      |  f       ||       ]
                                            //[        31:16(16)   15:0(16)         ]
      R2 = MPY(R0, R2)                      //[ r2=  d= i+f= input*.3321928..Q16    ]
      R0 = ASLH(R1)                         //[ Init. accumulator to                ]
                                            //[       c0<<16..Q31(=15+16)           ]
      R3.L = #22833                         //[ r3.l = c1..Q15                      ]
    }
    {
      R0 += MPYU(R3.L,R2.L)                 //[ r0+= c1*f..Q31(=15+16)              ]
      R1  = MPYU(R2.L,R2.L)                 //[ r1 = f^2..Q32(=2*16)                ]
      R3.L = #7346                          //[ r3.l = c2..Q15                      ]
      R4 = ASRH(R2)                         //[ r4 = i = d[31:16]                   ]
    }
    {
      R0 += MPYU(R3.L,R1.H)                 //[ r0+= c2*f^2..Q31(=15+16)            ]
      R1  = MPYU(R1.H,R2.L)                 //[ r1 = f^3..Q32(=2*16)                ]
      R3.L = #2588                          //[ r3.l = c3..Q15                      ]
    }
    { R0 += MPYU(R3.L,R1.H)                 //[ result =r0= c0<<16 +c1*f^1 +c2*f^2  ]
                                            //[              +c3*f^3..Q31(=15+16)   ] 
      R1  = SUB(#-1, R4)                    //[ scale=-1-i..in [-31,32]             ]
      JUMPR R31                             //[ return                              ]
    }
    .size	approx_dB_to_ulin_cubefit, .-approx_dB_to_ulin_cubefit




    /*[*****************************************************************************]*/
    /*[  Function   : result_scale_t approx_dB_to_ulin_quadlut8()                   ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Approximate dB to linear range conversion using 2nd degree    ]*/
    /*[               8-segment lookup table.                                       ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : Word32 input                                             ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R5                                                      ]*/
    /*[  Hardware Loops affected: None                                              ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 6                                                               ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl approx_dB_to_ulin_quadlut8
    .type   approx_dB_to_ulin_quadlut8, @function
approx_dB_to_ulin_quadlut8:
    { R2 = CONST32(#1426757212)             //[ .1*log2(10)=.3321928..Q32           ]
      R4.H = #HI(approx_dB_to_ulin_quadlut8_coefs)   
                                            //[ r4=start addr. of LUT               ]
    }
    { R5 = MPY(R0, R2)                      //[ r5=d=input*.3321928..Q24(=24+32-32) ]
      R4.L = #LO(approx_dB_to_ulin_quadlut8_coefs)   
                                            //[ r4=start addr. of LUT               ]
      R3 = #0                               //[ clear r3 for insert op              ]
    }
    {                                       //[             <-------f----------->   ]
                                            //[ d:|| i      |ind     |fn     ||     ]
                                            //[     31-24(8) 23-21(3) 20-0(21)      ]
      R4 = TABLEIDXD(R5,#3 ,#21 )           //[ r4=LUT addr+8*ind..ind=d[23:21]     ]
      R3 = INSERT(R5, #21, #8)              //[ r3=fn = d[20:0]<<8..Q29             ]
    }
    { R2 = MPY(R3,R3)                       //[ r2=fn^2..Q26(=2*29-32)              ]
      R1 = MEMW(R4)                         //[ r1=c0..Q31(=15+16)                  ]
      R4 = ASR(R5, #24)                     //[ r4 = i=d[31:24]                     ]
      R5 = MEMW(R4+#4)                      //[ load coefs..r5.l=c1 r5.h=c2         ]
    }
    { R1:0 += VMPYWEUH(R3:2,R5:4):rnd:sat   //[    r5:4= | c2  |  c1  | i         | ]
                                            //[    r3:2= |   fn       |  fn^2     | ]
                                            //[  =>r1:0= |  c1*fn^1 + | don't care| ]
                                            //[             c0<<16                  ]
                                            //[    Qs  =    Q31(=18+29-16)          ]
      R2 = MPY(R2,R5.H):<<1:rnd:sat         //[ c2*fn^2..Q31(=20+26-16+1)           ]
    }
    { R0 = ADD(R1,R2):sat                   //[ Calculate result=r0 =r1+r2 =c0<<16  ]
                                            //[             +c1*fn^1 +c2*fn^2..Q31  ]
      R1 = SUB(#-1, R4)                     //[ scale=-1-i..in [-31,32]             ]
      JUMPR R31                             //[ return                              ]
    }
    .size   approx_dB_to_ulin_quadlut8, .-approx_dB_to_ulin_quadlut8


    /*-----------------------------------------------------------------------------*/
    /*                 lookup table for approx_dB_to_ulin_quadlut8                 */
    /*-----------------------------------------------------------------------------*/
    .data
    .p2align 6
    .type   approx_dB_to_ulin_quadlut8_coefs, @object
    .size   approx_dB_to_ulin_quadlut8_coefs, 64
approx_dB_to_ulin_quadlut8_coefs:
    /*--------------------------------------*/
    /*     c0.l  c0.h     c1     c2         */
    /*     <----Q31--->   Q18    Q20        */
    /*--------------------------------------*/
    .hword  6222, 32768, 22697, 4110   
    .hword 56918, 35733, 24751, 4483   
    .hword  4425, 38968, 26991, 4889   
    .hword 63455, 42494, 29434, 5332   
    .hword  7301, 46341, 32097, 5818   
    .hword 23046, 50535, 35002, 6344   
    .hword 12892, 55109, 38170, 6917   
    .hword   381, 60097, 41625, 7542




    /*[*****************************************************************************]*/
    /*[  Function   : Word32 approx_ulin_to_dB_linlut8()                            ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Approximate linear range to dB conversion using linear        ]*/
    /*[               8-entry lookup table.                                         ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord32 input                                            ]*/
    /*[             : R1 : Word32 Qi                                                ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R6                                                      ]*/
    /*[  Hardware Loops affected: None                                              ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 6                                                               ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl approx_ulin_to_dB_linlut8
    .type   approx_ulin_to_dB_linlut8, @function
approx_ulin_to_dB_linlut8:
    { R2 = CL0(R0)                          //[ #leading 0's of input               ]
      P0 = CMP.EQ(R0,#0)                    //[ p0 = (input==0)?                    ]
      IF P0.new JUMP:nt .approx_ulin_to_dB_linlut8_0
                                            //[ handle input=0                      ]
      R3.H = #HI(approx_ulin_to_dB_linlut8_LUT)
                                            //[ r3=start addr. of LUT               ]
    }
    { R0 = ASL(R0,R2)                       //[ r0=norm.input in Q31. range=[1,2)   ]
      R3.L = #LO(approx_ulin_to_dB_linlut8_LUT) 
                                            //[ r3=start addr. of LUT               ]
      R6.L = #LO(50504453)                  //[ 3.0103 in Q24                       ]
      R1 += ADD(R2, #-31)                   //[ r1=Qi + cl0 -31                     ]
    }
    {                                       //[ d:   ||1 |   ind     |  dn     ||   ]
                                            //[        31    30-28(3)  27-0(28)     ]
      R3 = TABLEIDXH(R0,#3 ,#28 )           //[ r3=LUT addr+2*ind..ind=d[30:28]     ]
      R4 = EXTRACTU(R0,#28 ,#0 )            //[ r4 = dn=d[27:0] in Q28              ]
      R6.H = #HI(50504453)                  //[ 3.0103 in Q24                       ]
      R0 = #0                               //[ init. accumulator                   ]
    }
    { R2 = MEMH(R3+#2)                      //[ r2=c[ind+1] in Q12                  ]
      R3 = MEMH(R3+#0)                      //[ r3=c[ind] in Q12                    ]
      R5 = R4                               //[ r5 =r4 = dn in Q28                  ]
      R1 = MPYI(R1, R6)                     //[ norm_offset =3.0103*(Qi+cl0-31) Q24 ]
    }
    { R1:0 += VMPYWEH(R5:4,R3:2):rnd:sat    //[   r3:2=| 0 | c[ind] |0 | c[ind+1] | ]
                                            //[   r5:4=|  dn        |   dn        | ]
                                            //[ =>r1:0=|c[ind]*dn + |c[ind+1]*dn  | ]
                                            //[          norm_offset                ]
                                            //[   Qs: =24(=12+28-16)  24(=12+28-16) ]
      R3 = ASL(R3, #12)                     //[ r3 = c[ind] ..Q12 to Q24(=12+12)    ]
    }
    { R0 += SUB(R3, R1)                     //[ result= r0+r3-r1                    ]
                                            //[       = c[ind+1]*dn + c[ind] -      ]
                                            //[         (c[ind]*dn +norm_offset)    ]
                                            //[       = c[ind+1]*dn + c[ind]*(1-dn) ]
                                            //[         -norm_offset..in Q24        ]
      JUMPR R31                             //[ return                              ]

    }
.approx_ulin_to_dB_linlut8_0:
    { R0.H = #0x8000                        //[ result=0x80000000 for input==0      ]
                                            //[ (only need setup R0.H because R0 =0)]
      JUMPR R31                             //[ return                              ]
    }
    .size   approx_ulin_to_dB_linlut8, .-approx_ulin_to_dB_linlut8

    /*-----------------------------------------------------------------------------*/
    /*                 lookup table for approx_ulin_to_dB_linlut8                  */
    /*-----------------------------------------------------------------------------*/
    .data
    .p2align 5
    .type   approx_ulin_to_dB_linlut8_LUT, @object
    .size   approx_ulin_to_dB_linlut8_LUT, 18
approx_ulin_to_dB_linlut8_LUT:
    /*--------------------------------------*/
    /* c ~= 10*log10( 1 + d[30:28]/2^3 ).Q12*/
    /*--------------------------------------*/
    .hword    16, 2108, 3980,  5674
    .hword  7220, 8643, 9961, 11187
    .hword 12336





    /*[*****************************************************************************]*/
    /*[  Function   : Word32 approx_ulin_to_dB_cubelut8()                           ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Approximate linear range to dB conversion using linear        ]*/
    /*[               32-segment lookup tabl.                                       ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord32 input                                            ]*/
    /*[             : R1 : Word32 Qi                                                ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R4                                                      ]*/
    /*[  Hardware Loops affected: None                                              ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 5                                                               ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
	.text
	.p2align 2
	.p2align 4,,15
	.globl approx_ulin_to_dB_cubelut8
	.type	approx_ulin_to_dB_cubelut8, @function
approx_ulin_to_dB_cubelut8:
    { R2 = CL0(R0)                          //[ #leading 0s in input                ]
      P0 = CMP.EQ(R0,#0)                    //[ p0 = (input==0)?                    ]
      R6.H = #HI(-50504453)                 //[ -3.0103 in Q24                      ]
      R4.H = #HI(approx_ulin_to_dB_cubelut8_coefs)   
                                            //[ r4=start addr. of LUT               ]
    }
    { R0 = ASL(R0, R2)                      //[ r0=d ..norm. input in range [1,2)   ]
      R1+= ADD(R2, #-31)                    //[ r1 = Qi + cl0 -31                   ]
      R8.H = #HI(0x2000000)                 //[ r8 = 1 ..Q25                        ]
      R4.L = #LO(approx_ulin_to_dB_cubelut8_coefs)   
                                            //[ r4=start addr. of LUT               ]
    }
    {                                       //[ d: | 1 |ind     |dn      |ignored | ]
                                            //[      31 30:28(3) 27:2(26)  1:0(2)   ]
      R4 = TABLEIDXD(R0, #3, #28)           //[ r4=LUT addr+8*ind..ind=d[30:28]     ]
      R2 = EXTRACTU(R0, #26, #2 )           //[ r2 = dn =d[27:2] in Q26.            ]
      R6.L = #LO(-50504453)                 //[ -3.0103 in Q24                      ]
      R8.L = #LO(0x2000000)                 //[ r8 = 1 ..Q25                        ]
    }
    { R5:4 = MEMD(R4)                       //[ Load cubic poly. coefficients in the]
                                            //[ order c0, c1, c3, & c2., i.e., c0   ]
                                            //[ in lowest hword, c1 in the next,    ]
                                            //[ then c3 and c2 in the highest hword ]
      R3 = MPY(R2,R2)                       //[ r3 = dn^2..Q20(=26*2-32)            ]
      R1 = MPYI(R1,R6)                      //[ Init the accumulator to -norm_offset]
      R0 = #0                               //[      =-3.0103*(Qi+cl0-31)           ]
                                            //[ r1 = -norm_offset in Q24            ]
                                            //[ r0 = 0                              ]
    }
    { R1:0+=VMPYWOH(R3:2,R5:4):<<1:rnd:sat  //[    r5:4= |  c2 | c3   |  c1 | c0  | ]
                                            //[    r3:2= |   dn^2     |    dn     | ]
                                            //[ => r1:0= | c2*dn^2 -  | c1*dn^1   | ]
                                            //[            norm_offset              ]
                                            //[Qs= Q24(=19+20-16+1)|Q24(=13+26-16+1)]
      
      R3 = MPY(R3,R2.H):<<1:rnd:sat         //[ dn^3..Q15(=26+20-32+1)              ]
      R2 = R8                               //[ r2 =1 in Q25                        ]
    }
    { R1:0+=VMPYWEUH(R3:2,R5:4):<<1:rnd:sat //[    r5:4= |  c2 | c3   |   c1 | c0 | ]
                                            //[    r3:2= |   dn^3     |    1      | ]
                                            //[ => r1:0= | c3*dn^3 +  | c0 +      | ]
                                            //[            c2*dn^2 -     c1*dn^1    ]
                                            //[             norm_offset
                                            //[Qs= Q24(=24+15-16+1)|Q24(=14+25-16+1)]
    }
    { R0 = ADD(R1,R0):sat                   //[ r0 =result= r1+r0= c0 + c1*dn^1 +   ]
                                            //[ c2*dn^2 + c3*dn^3 - norm_offset(Q24)]
      IF !P0 JUMPR R31                      //[ return                              ]
    }
approx_ulin_to_dB_cubelut8_0:
    { R0 = ASL(R8, #6)                      //[ result=0x80000000 for input==0      ]
      JUMPR R31                             //[ return                              ]
    }
    .size   approx_ulin_to_dB_cubelut8, .-approx_ulin_to_dB_cubelut8



    /*-----------------------------------------------------------------------------*/
    /*                 lookup table for approx_ulin_to_dB_cubelut8                 */
    /*-----------------------------------------------------------------------------*/
	.data
	.p2align 6
	.type	approx_ulin_to_dB_cubelut8_coefs, @object
	.size	approx_ulin_to_dB_cubelut8_coefs, 64
approx_ulin_to_dB_cubelut8_coefs:
    /*--------------------------------------*/
    /*       (c0 & c3 are unsigned )        */
    /*       c0      c1     c3        c2    */
    /*       Q14     Q13    Q24       Q19   */
    /*--------------------------------------*/
    .hword      0,  4447,  39647,  -17639
    .hword   8381,  3953,  28385,  -13960 
    .hword  15877,  3558,  21015,  -11321 
    .hword  22660,  3234,  15992,   -9365 
    .hword  28850,  2965,  12450,   -7875 
    .hword  34546,  2737,   9882,   -6714 
    .hword  39820,  2541,   7974,   -5791 
    .hword  44728,  2372,   6527,   -5047



