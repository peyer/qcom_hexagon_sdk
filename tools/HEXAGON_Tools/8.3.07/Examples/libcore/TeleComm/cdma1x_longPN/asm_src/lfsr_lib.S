/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file   "lfsr_lib.S"
    /*[*****************************************************************************]*/
    /*[  Function   : UWord64 calc_dec()                                            ]*/
    /*[*****************************************************************************]*/
    /*[  Description: decimated 42bit LFSR generator for cdma1X long PN code        ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord64 seed                                             ]*/
    /*[               R1 : UWord64 *decimate                                        ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R9                                                      ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 44                                                              ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  Revision History:                                                          ]*/
    /*[  ----------------                                                           ]*/
    /*[  Author           Date                Comments                              ]*/
    /*[  -------------------------------------------------------------------        ]*/
    /*[  DJH              11/9/09       asm completed model created                 ]*/
    /*[  DJH              11/10/09      added state split incorporating             ]*/
    /*[                                 idea from Mao                               ]*/
    /*[*****************************************************************************]*/
#define seedi   r1:0
#define dec_table   r2
#define i       r3
#define tap     r7:6
#define fib64   r1:0
#define fib64h  r1
#define fib64l  r0
#define bit     r5:4
#define bith    r5
#define bitl    r4
#define _seed   r9:8

    .text
    .p2align 2
    .p2align 4,,15
    .global calc_dec_asm
    .type   calc_dec_asm, @function
calc_dec_asm:
    /* --------------------------------------------------------------------------- */
    /*  apply power matrix in "Decimation of m-Sequences leading to any            */
    /*  desired phase offset - B Arazi - Electronics Letters 31st March 1977"      */
    /*  to fibonacci state equivalent, uses input table of 0th row of              */
    /*  power matrices for alpha^(i*decimation) *                                  */
    /*                                                                             */
    /*  u64 calc_dec(u64 seed, u64 * decimate)                                     */
    /*  {                                                                          */
    /*     int i;                                                                  */
    /*     u64 fib64 = 0;                                                          */
    /*     for(i=0; i < N; i+=1) {                                                 */
    /*         fib64 |= ((u64)Q6_R_parity_PP(decimate[i], seed))<<i;               */
    /*     }                                                                       */
    /*     return(fib64);                                                          */
    /*  }                                                                          */
    /* --------------------------------------------------------------------------- */

    { tap= MEMD(dec_table++#8)              //[ [0,0]                               ]
      LOOP0(.DecLoop, #39)                  //[ setup loop0: lc0 = 39               ]
      _seed = seedi                         //[ copy seedin                         ]
    }
    { fib64h = #0                           //[                                     ]
      fib64l= PARITY(_seed, tap)            //[ [1,0]                               ]
      tap= MEMD(dec_table++#8)              //[ [0,1]                               ]
    }
    { i = #1                                //[ [2,0]                               ]
      bith = #0                             //[                                     ]
      bitl= PARITY(_seed, tap)              //[ [1,1]                               ]
      tap= MEMD(dec_table++#8)              //[ [0,1]                               ]
    }
    .falign
.DecLoop:
    { fib64 |= ASL(bit, i)                  //[ [2,0]                               ]
      i = ADD(i, #1)                        //[ [2,0]                               ]
      bitl= PARITY(_seed, tap)              //[ [1,1]                               ]
      tap= MEMD(dec_table++#8)              //[ [0,1]                               ]
    }:endloop0

    { fib64 |= ASL(bit, i)                  //[ [2,1]                               ]
      bitl= PARITY(_seed, tap)              //[ [1,2]                               ]
      i = ADD(i, #1)                        //[ [2,0]                               ]
    }
    { fib64 |= ASL(bit, i)                  //[ [2,2]                               ]
      JUMPR R31                             //[ return                              ]
    }
    .size   calc_dec_asm, .-calc_dec_asm




    /*[*****************************************************************************]*/
    /*[  Function   : void long_code_gen_f_asm()                                    ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Generate output lfsr sequence takes odd and even decimated    ]*/
    /*[               sequences and interleaves them to double bandwdith so         ]*/
    /*[               decimate by 64 is 2 decimated sequences of 128 interleaved,   ]*/
    /*[               one delayed by 64                                             ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord64 *state0                                          ]*/
    /*[               R1 : UWord64 *state1                                          ]*/
    /*[               R2 : UWord64 *long_code                                       ]*/
    /*[               R3 : int     num_64chips                                      ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R13                                                     ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 34*num_64chips/64 + 5                                           ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  Revision History:                                                          ]*/
    /*[  ----------------                                                           ]*/
    /*[  Author           Date                Comments                              ]*/
    /*[  -------------------------------------------------------------------        ]*/
    /*[  DJH              11/9/09       asm completed model created                 ]*/
    /*[  DJH              11/10/09      added state split incorporating             ]*/
    /*[                                 idea from Mao                               ]*/
    /*[*****************************************************************************]*/
#define  state0      r0
#define  state1      r1
#define  code_ptr    r2
#define  num_64chips r3
#define  s0          r5:4
#define  s1          r7:6
#define  fpoly        r9:8
#define  fpolyh       r9
#define  fpolyl       r8
#define  decbits     r11:10
#define  b0          r11:10
#define  b0l         r10
#define  b1          r13:12
#define  b1l         r12

    .text
    .p2align 2
    .p2align 4,,15
    .global long_code_gen_f_asm
    .type   long_code_gen_f_asm, @function
long_code_gen_f_asm:
    /* --------------------------------------------------------------------------- */
    /*  void long_code_gen_f( u64 *state0, u64 *state1, u64 *long_code,            */
    /*                        int num_64chips)                                     */
    /*  {                                                                          */
    /*    int i1, i2;                                                              */
    /*    u64 s0,s1;                                                               */
    /*    u64 poly = (0x0000040a8e6f04efLL)<<22;                                   */
    /*    u64 * ptr = (u64 *) long_code;                                           */
    /*                                                                             */
    /*    s0 = *state0<<22;                                                        */
    /*    s1 = *state1<<22;                                                        */
    /*    for(i1 = 0; i1 < num_64chips; i1++){                                     */
    /*       ptr[i1]=Q6_P_interleave_P(((s1>>22) << 32) | ((s0>>22)&0xFFFFFFFFLL));*/
    /*       for(i2 = 0; i2 < 32; i2++){                                           */
    /*             s0 = Q6_P_lfs_PP(s0, poly);                                     */
    /*             s1 = Q6_P_lfs_PP(s1, poly);                                     */
    /*       }                                                                     */
    /*    }                                                                        */
    /*    *state0 = s0;                                                            */
    /*    *state1 = s1;                                                            */
    /*    return;                                                                  */
    /*  }                                                                          */
    /* --------------------------------------------------------------------------- */
    { s0 = MEMD(state0)                     //[ SU                                  ]
      s1 = MEMD(state1)                     //[ SU                                  ]
      LOOP1(.BitLoop, num_64chips)          //[ SU                                  ]
      fpolyl.l = #lo(0x3BC00000)            //[                                     ]
    }
    { s0 = ASL(s0, #22)                     //[                                     ]
      s1 = ASL(s1, #22)                     //[                                     ]
      fpolyl.h = #hi(0x3BC00000)            //[ set polynomial                      ]
      fpolyh.l = #lo(0x02A39BC1)            //[ set polynomial                      ]
    }
    .falign
.BitLoop:
    { b1 = LSR(s1, #22)                     //[                                     ]
      b0 = LSR(s0, #22)                     //[                                     ]
      fpolyh.h = #hi(0x02A39BC1)            //[                                     ]
    }
    { decbits = COMBINE(b1l,b0l)            //[                                     ]
      s0 = LFS(s0, fpoly)                   //[                                     ]
      s1 = LFS(s1, fpoly)                   //[                                     ]
    }
    { decbits = INTERLEAVE(decbits)         //[ OL                                  ]
      LOOP0(.lfsrLoop, #30)                 //[ setup loop0: lc0 = 30               ]
    }
    { memd(code_ptr++#8) = decbits          //[ [3,1]                               ]
      s0 = LFS(s0, fpoly)                   //[                                     ]
      s1 = LFS(s1, fpoly)                   //[                                     ]
    }
    .falign
.lfsrLoop:
    { s0 = LFS(s0, fpoly)                   //[                                     ]
      s1 = LFS(s1, fpoly)                   //[                                     ]
    }:endloop0:endloop1

    { b0 = LSR(s0, #22)                     //[                                     ]
      b1 = LSR(s1, #22)                     //[                                     ]
    }
    { MEMD(state0) = b0                     //[ SU                                  ]
    }
    { MEMD(state1) = b1                     //[ SU                                  ]
      JUMPR R31                             //[ return                              ]
    }
    .size   long_code_gen_f_asm, .-long_code_gen_f_asm




    /*[*****************************************************************************]*/
    /*[  Function   : void split_states()                                           ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Advance decimated states by 42 cycles to get next 42 deicmated]*/
    /*[               state bits, deinterleave them to get input for generator.     ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord64 *state                                           ]*/
    /*[               R1 : int     n                                                ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R16                                                     ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 52                                                              ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
#define  states      r0
#define  n           r1
#define  zeror       r1
#define  st0          r3:2
#define  st1          r5:4
#define  st2          r7:6
#define  st3          r9:8
#define  spoly       r11:10
#define  spolyh      r11
#define  spolyl      r10
#define  d0          r13:12
#define  d0h         r13
#define  d0l         r12
#define  d2          r15:14
#define  d2h         r15
#define  d2l         r14
#define  d1          r11:10
#define  d1h         r11
#define  d1l         r10
#define  d3          r9:8
#define  d3h         r9
#define  d3l         r8
#define  c2r21       r16

    .text
    .p2align 2
    .p2align 4,,15 
    .global split_states_asm
    .type   split_states_asm, @function
split_states_asm:
    /* --------------------------------------------------------------------------- */
    /*  void split_states( u64 * states, int n)                                    */
    /*  {                                                                          */
    /*    int i;                                                                   */
    /*    u64 s0, s1, s2, s3, d0, d1, d2, d3;                                      */
    /*    u64 poly = (0x0000040a8e6f04efLL)<<22;                                   */
    /*                                                                             */
    /*    s0 = states[0];                                                          */
    /*    s2 = states[2];                                                          */
    /*    s1 = s0<<22;                                                             */
    /*    s3 = s2<<22;                                                             */
    /*    d0 = Q6_P_deinterleave_P(s0);                                            */
    /*    d2 = Q6_P_deinterleave_P(s2);                                            */
    /*    for(i = 0; i < n; i++) {                                                 */
    /*       s1 = Q6_P_lfs_PP(s1, poly);                                           */
    /*       s3 = Q6_P_lfs_PP(s3, poly);                                           */
    /*    }                                                                        */
    /*    s1 >>= 22;                                                               */
    /*    s3 >>= 22;                                                               */
    /*    d1 = Q6_P_deinterleave_P(s1);                                            */
    /*    d3 = Q6_P_deinterleave_P(s3);                                            */
    /*    s0 = _lo32(d1)<<21 | _lo32(d0);                                          */
    /*    s1 = _hi32(d1)<<21 | _hi32(d0);                                          */
    /*    s2 = _lo32(d3)<<21 | _lo32(d2);                                          */
    /*    s3 = _hi32(d3)<<21 | _hi32(d2);                                          */
    /*    states[0] = s0;                                                          */
    /*    states[1] = s1;                                                          */
    /*    states[2] = s2;                                                          */
    /*    states[3] = s3;                                                          */
    /*    return;                                                                  */
    /*  }                                                                          */
    /* --------------------------------------------------------------------------- */
    { M0 = R16                              //[ save R16 in M0                      ]
    }
    { st0 = MEMD(states)                    //[ SU                                  ]
      st2 = MEMD(states+#16)                //[ SU                                  ]
      LOOP0(.SplitLoop, n)                  //[ SU                                  ]
      zeror= #0                             //[ constant                            ]
    }
    { st1 = ASL(st0, #22)                   //[                                     ]
      st3 = ASL(st2, #22)                   //[                                     ]
      spolyl.l = #lo(0x3BC00000)            //[                                     ]
      spolyh.l = #lo(0x02A39BC1)            //[                                     ]
    }
    { d0 = DEINTERLEAVE(st0)                //[                                     ]
      d2 = DEINTERLEAVE(st2)                //[                                     ]
      spolyl.h = #hi(0x3BC00000)            //[                                     ]
      spolyh.h = #hi(0x02A39BC1)            //[                                     ]
    }

    .falign
.SplitLoop:
    { st1 = LFS(st1, spoly)                 //[                                     ]
      st3 = LFS(st3, spoly)                 //[                                     ]
    }:endloop0

    { st1 = LSR(st1, #22)                   //[                                     ]
      st3 = LSR(st3, #22)                   //[                                     ]
      c2r21.l = #lo(0x00200000)             //[                                     ]
    }
    { d1 = DEINTERLEAVE(st1)                //[                                     ]
      d3 = DEINTERLEAVE(st3)                //[                                     ]
      c2r21.h = #hi(0x00200000)             //[                                     ]
      st0 = COMBINE(zeror,d0l)              //[                                     ]
    }
    { st0 += MPY(d1l, c2r21)                //[                                     ]
      st1 = COMBINE(zeror,d0h)              //[                                     ]
    }
    { MEMD(states    ) = st0                //[ SU                                  ]
      st1 += MPY(d1h, c2r21)                //[                                     ]
      st2 = COMBINE(zeror,d2l)              //[                                     ]
    }
    { MEMD(states+#8 ) = st1                //[ SU                                  ]
      st2 += MPY(d3l,c2r21)                 //[                                     ]
      st1 = COMBINE(zeror,d2h)              //[                                     ]
    }
    { MEMD(states+#16) = st2                //[ SU                                  ]
      st1 += MPY(d3h,c2r21)                 //[                                     ]
      R16 = M0                              //[ restore R16                         ]
    }
    { MEMD(states+#24) = st1                //[ SU                                  ]
      JUMPR R31                             //[ return                              ]
    }
    .size   split_states_asm, .-split_states_asm



    /*[*****************************************************************************]*/
    /*[  Function   : void mask_gen_c()                                             ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Generate initial state based on seed and mask                 ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - None                                                            ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord64 *fib_1                                           ]*/
    /*[               R1 : UWord64 *fib0                                            ]*/
    /*[               R2 : UWord64 state                                            ]*/
    /*[               R3 : UWord64 mask                                             ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R12                                                     ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 192                                                             ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/

#define fib_1   r0
#define fib0    r1
#define state   r3:2
#define stateh  r3
#define statel  r2
#define mask    r5:4
#define fib42   r7:6
#define fib42h  r7
#define fib42l  r6
#define bit0    r9:8
#define bit0h   r9
#define bit0l   r8
#define mpoly   r11:10
#define mpolyh  r11
#define mpolyl  r10
#define f10     r11:10
#define f1      r11
#define f0      r10
#define j       r12

    .text
    .p2align 2
    .p2align 4,,15
    .global mask_gen_asm
    .type   mask_gen_asm, @function
mask_gen_asm:
    /* --------------------------------------------------------------------------- */
    /*   void mask_gen_c(u64 *fib_1, u64 *fib0, u64 state, u64 mask)               */
    /*   {                                                                         */
    /*    int i, j;                                                                */
    /*    u64 fib42, bit;                                                          */
    /*    u64 poly = 0x0000040a8e6f04efLL;                                         */
    /*                                                                             */
    /*    fib42 = 0;                                                               */
    /*    for(i = 0; i < N; i++){                                                  */
    /*      bit = parity(mask, state);                                             */
    /*      fib42 = (fib42<<1) | bit;                                              */
    /*      state = (state << 1); if (state & 0x40000000000LL) state ^= poly;      */
    /*    }                                                                        */
    /*    *fib_1 = fib42;                                                          */
    /*    bit = parity(mask, state);                                               */
    /*    fib42 = (fib42<<1) | bit;                                                */
    /*    *fib0 = fib42;                                                           */
    /*                                                                             */
    /*    return ;                                                                 */
    /*  }                                                                          */
    /* --------------------------------------------------------------------------- */
    { mpolyh.l = #lo(0x0000040a)            //[ [P0]poly=0x0000040a8e6f04efLL       ]
      mpolyl.l = #lo(0x8e6f04ef)            //[ [P0]                                ]
      bit0h = #0                            //[                                     ]
      j = #22                               //[                                     ]
    }
    { fib42 = #0                            //[                                     ]
      LOOP0(.MaskGen, #42)                  //[ for(i = 0; i < N; i++){             ]
      mpolyh.h = #hi(0x0000040a)            //[ [P0]                                ]
      mpolyl.h = #hi(0x8e6f04ef)            //[ [P0]                                ]
    }
    .falign
.MaskGen:
    { bit0l = PARITY(mask, state)           //[                                     ]
      p0 = CMP.GTU(stateh, #0x1FF)          //[                                     ]
      state = ASL(state, #1)                //[                                     ]
    }
    { fib42 |= ASL(bit0, j)                 //[                                     ]
      IF (P0) stateh = XOR(stateh, mpolyh)  //[                                     ]
      IF (P0) statel = XOR(statel, mpolyl)  //[                                     ]
      j = add(j, #1)                        //[ j++                                 ]
    }:endloop0

    { f1 = BREV(fib42l)                     //[                                     ]
      f0 = BREV(fib42h)                     //[                                     ]
    }
    { MEMD(fib_1) = f10                     //[                                     ]
      bit0l = PARITY(mask, state)           //[                                     ]
    }
    { bit0 |= ASL(f10, #1)                  //[                                     ]
    }
    { MEMD(fib0) = bit0                     //[                                     ]
      JUMPR R31                             //[ return                              ]
    }
    .size   mask_gen_asm, .-mask_gen_asm

    /* --------------------------------------------------------------------------- */
    /*                     end of lfsr library file                                */
    /* --------------------------------------------------------------------------- */
