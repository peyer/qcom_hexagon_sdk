/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file   "crc.S"
    /*[*****************************************************************************]*/
    /*[  Function   : UWord32 crc32()                                               ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Bit-by-bit implementation of CRC encoding                     ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - The order of the generator polynomial G(x) must be              ]*/
    /*[             NOT greater than 32                                             ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8  *input                                           ]*/
    /*[               R1 : int     nbits                                            ]*/
    /*[               R2 : UWord32 poly                                             ]*/
    /*[                                                                             ]*/
    /*[  Returns    : R0 : the MSB of the UWord32 are CRC bits                      ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R6                                                      ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 13*(nbits/8) +5 +2*(nbits%8 + 1)  (~1.625 cycles/bit)           ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15 
    .globl crc32
    .type    crc32, @function
crc32:
    { P0 = CMP.GT(R1,#7)                    //[ P0 = (nbits>7)                      ]
      P1 = CMP.GT(R2,#-1)                   //[ IF !(G.31==0)                       ]
      R3 = ASL(R2,#1)                       //[ G<<1                                ]
      R6 = ASR(R1,#3)                       //[ LC1 = nbits/8                       ]
    }
    { IF !P1 R3 = XOR(R3,R2)                //[ poly1 = G ^ (G<<1)                  ]
      IF !P0 JUMP .CRC32_restBits           //[ if (nbits <=7) jump CRC32_restBits  ]
      R4 = #0                               //[ crc = 0                             ]
      R5 = MEMUB(R0++#1)                    //[[p]*data++                           ]
    }
    { LOOP1(.CRC32_ByteLOOP,R6)             //[ setup loop: lc1 = nbits/8           ]
    }

    .falign
.CRC32_ByteLOOP:
    { R4 ^= ASL(R5,#24)                     //[ crc ^= (*data++ << 24)              ]
      LOOP0(CRC32_InnerLOOP,#4)             //[ setup inner loop                    ]
      R5 = MEMUB(R0++#1)                    //[ *data++                             ]
      R1 = ADD(R1,#-8)                      //[ nbits -= 8                          ]
    }
    .falign
CRC32_InnerLOOP:
    { P0 = CMP.GT(R4,#-1)                   //[ P0 = (crc.31 == 0)                  ]
      P1 = TSTBIT(R4,#30)                   //[ P1 = (crc.30 == 1)                  ]
      R4 = ASL(R4,#2)                       //[ crc << 2                            ]
    }
    { IF !P0 R4 = XOR(R4,R3)                //[ if (crc.31 !=0) crc=(crc << 2)^poly1]
    }
    { IF  P1 R4 = XOR(R4,R2)                //[ if (crc.30 !=0) crc ^= poly         ]
    }:endloop0:endloop1 

    .falign
.CRC32_restBits:
    { P0 = CMP.GT(R1,#0)                    //[ P0 = (nbits > 0)                    ]
      IF !P0.new JUMP:t .CRC32_END          //[ if !P0 DONE                         ]
    }
    { R4 ^= ASL(R5,#24)                     //[ crc ^= (*data++ << 24)              ]
      LOOP0(.CRC32_restLOOP,R1)             //[ setup loop0: lc0 = nbits            ]
    }

    .falign
.CRC32_restLOOP:
    { P0 = CMP.GT(R4,#-1)                   //[ P0 = !(crc & 0x80000000)            ]
      R4 = ASL(R4,#1)                       //[ crc << 1                            ]
    }
    { IF !P0 R4 = XOR(R4,R2)                //[ if !P0 crc = (crc<<1) ^ poly        ]
    }:endloop0

.CRC32_END:
    { R0 = R4                               //[ return value                        ]
      JUMPR R31                             //[ return                              ]
    }
    .size    crc32, .-crc32




    /*[*****************************************************************************]*/
    /*[  Function   : UWord32 crc16()                                               ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Bit-by-bit implementation of CRC encoding                     ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - The order of the generator polynomial G(x) must be              ]*/
    /*[             NOT greater than 24                                             ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8  *input                                           ]*/
    /*[               R1 : int     nbits                                            ]*/
    /*[               R2 : UWord32 poly                                             ]*/
    /*[                                                                             ]*/
    /*[  Returns    : R0 : the MSB of the UWord32 are CRC bits                      ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R7                                                      ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - nbits + 2*ceil(nbits/8) + 4       (~1.25  cycles/bit)           ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl crc16
    .type    crc16, @function
crc16:
    { P0 = CMP.GT(R1,#0)                    //[ P0 = (nbits>0)                      ]
      R4 = #0                               //[ crc = 0                             ]
      IF !P0.new JUMP:nt .CRC16_END         //[ IF !P0 return crc=0                 ]
      R3 = ADD(R1,#7)                       //[ (nbits+7)                           ]
    }    
    { R2 = LSR(R2,#1)                       //[ poly << 1                           ]
      R3 = ASR(R3,#3)                       //[ LC1 = (nbits+7)/8                   ]
      R6 = #8                               //[ R6 = constant 8                     ]
    }
    { LOOP1(.CRC16_ByteLOOP,R3)             //[ setup loop1: lc1=ceil(nbits/8)      ]
    }

    .falign
.CRC16_ByteLOOP:
    { R5 = MEMUB(R0++#1)                    //[ *data++                             ]
      R3 = R2                               //[ spoly                               ]
      R4 = ASL(R4,#8)                       //[ crc <<=8                            ]
      R7 = MIN(R1,R6)                       //[ R7 = (nbits>8) ? 8 : nbits          ]
    }
    { R4 ^= ASL(R5,#24)                     //[ crc ^= (*data++ << 24)              ]
      R5 = #31                              //[ R5 = (31-i), where i=0              ]
      R1 = ADD(R1,#-8)                      //[ nbits -= 8                          ]
      LOOP0(.CRC16_InnerLOOP,R7)            //[ setup loop0: lc0 = min(8, nbits)    ]
    }
    .falign
.CRC16_InnerLOOP:
    { P0 = TSTBIT(R4,R5)                    //[ if (crc>>(31-i)) & 1                ]
      IF P0.new R4 = XOR(R4,R3)             //[ then crc ^= spoly                   ]
      R5 = ADD(R5,#-1)                      //[ R5 = (31-i)                         ]
      R3 = LSR(R3,#1)                       //[ spoly >>=1                          ]
    }:endloop0:endloop1 

.CRC16_END:
    { R0 = ASL(R4,R7)                       //[ return value                        ]
      JUMPR R31                             //[ return                              ]
    }
    .size    crc16, .-crc16





    /*[*****************************************************************************]*/
    /*[  Function   : UWord32 crc32_lut()                                           ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Lookup table implementation of CRC encoding                   ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - The lenght of message is a multiple of 8-bits                   ]*/
    /*[           - The order of the generator polynomial G(x) must be              ]*/
    /*[             NOT greater than 32                                             ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8  *data                                            ]*/
    /*[               R1 : int     nbyte                                            ]*/
    /*[               R2 : UWord32 *crctab                                          ]*/
    /*[                                                                             ]*/
    /*[  Returns    : R0 : the MSB of the UWord32 are CRC bits                      ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R5                                                      ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 3*nbytes +3                       (~0.375 cycles/bit)           ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl crc32_lut
    .type    crc32_lut, @function
crc32_lut:
    { P0 = CMP.GT(R1,#0)                    //[ P0 = (nbyte>0)                      ]
      R3 = #0                               //[ crc = 0                             ]
      IF !P0.new JUMP:nt .CRC32LUT_END      //[ IF !P0 return crc = 0               ]
    }
    { LOOP0(.CRC32LUT_LOOP,R1)              //[ setup loop0: lc0 = nbytes           ]
      R5 = MEMUB(R0++#1)                    //[ *dat++                              ]
    }

    .falign
.CRC32LUT_LOOP:
    { R5 = MEMUB(R0++#1)                    //[[1] *data++                          ]
      R4 = ADDASL(R2,R5,#2)                 //[[2] &crctab[(crc>>24)^ *data++]      ]
      R3 = ASL(R3,#8)                       //[[2] crc <<8                          ]
    }
    { R5 ^= LSR(R3,#24)                     //[[1] for crc>>24 ^ *data++            ]
      R1 = MEMW(R4)                         //[[2] load crctab[(crc>>24)^ *data++]  ]
    }
    { R5 ^= LSR(R1,#24)                     //[[1] crc>>24 ^ (*data++)              ]
      R3 = XOR(R3,R1)                       //[[2] crc = (crc<<8) ^crctab[]         ]
    }:endloop0 

.CRC32LUT_END:
    { R0 = R3                               //[ return value                        ]
      JUMPR R31                             //[ return                              ]
    }
    .size    crc32_lut, .-crc32_lut




    /*[*****************************************************************************]*/
    /*[  Function   : UWord32 crc8_lut()                                            ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Lookup table implementation of CRC encoding                   ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - The lenght of message is a multiple of 8-bits                   ]*/
    /*[           - The order of the generator polynomial G(x) must be              ]*/
    /*[             NOT greater than 8                                              ]*/
    /*[           - crctab MUST be aligned by 256bytes                              ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8  *data                                            ]*/
    /*[               R1 : int     nbyte                                            ]*/
    /*[               R2 : UWord32 *crctab                                          ]*/
    /*[                                                                             ]*/
    /*[  Returns    : R0 : CRC bits                                                 ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R4                                                      ]*/
    /*[  Hardware Loops affected: Loop0                                             ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - 2*nbytes +3                       (~0.25 cycles/bit)            ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl crc8_lut
    .type    crc8_lut, @function
crc8_lut:
    { P0 = CMP.GT(R1,#0)                    //[ P0 = (nbyte>0)                      ]
      R3 = #0                               //[ crc = 0                             ]
      IF !P0.new JUMP:nt .CRC8LUT_END       //[ IF !P0 return crc = 0               ]
    }
    { LOOP0(.CRC8LUT_LOOP,R1)               //[ setup loop0: lc0 = nbytes           ]
      R1 = MEMUB(R0++#1)                    //[[p] *data++                          ]
      R4 = R2                               //[ R4 = crctab                         ]
    }

    .falign
.CRC8LUT_LOOP:
    { R4 ^= XOR(R1,R3)                      //[ &crctab[crc ^ (*data++)]            ]
      R1 = MEMUB(R0++#1)                    //[ *data++                             ]
    }
    { R3 = MEMUB(R4)                        //[ crc = crctab[]                      ]
      R4 = R2                               //[ R4 = crctab                         ]
    }:endloop0 

.CRC8LUT_END:
    { R0 = R3                               //[ return value                        ]
      JUMPR R31                             //[ return                              ]
    }
    .size    crc8_lut, .-crc8_lut




    /*[*****************************************************************************]*/
    /*[  Function   : UWord32 crc32_blk()                                           ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Block update implementation of CRC encoding                   ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - The lenght of message is a multiple of 8-bits                   ]*/
    /*[           - nbytes >= 8                                                     ]*/
    /*[           - data MUST be aligned by 8bytes                                  ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8  *data                                            ]*/
    /*[               R1 : int     nbyte                                            ]*/
    /*[               R2 : UWord32 *crctab                                          ]*/
    /*[               R3 : int     mG                                               ]*/
    /*[                                                                             ]*/
    /*[  Returns    : R0 : CRC bits                                                 ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R10                                                     ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - (mG+2)*(nbytes/8) +5                                            ]*/
    /*[             e.g., ~0.15625 cycles/bit for mG=8                              ]*/
    /*[                                                                             ]*/
    /*[*****************************************************************************]*/
    .text
    .p2align 2
    .p2align 4,,15
    .globl crc32_blk
    .type    crc32_blk, @function
crc32_blk:
    { P0 = CMP.GT(R1,#7)                    //[ P0 = (nbyte/8 >0)                   ]
      R0 = #0                               //[ crc = 0                             ]
      R1 = ASR(R1,#3)                       //[ LC1 = nbytes/8                      ]
      R6 = R0                               //[ R6 = data                           ]
    }
    { IF !P0 JUMP .CRC32BLK_END             //[ IF !P0 return                       ]
      R10 = #-1                             //[ cancell epsilog                     ]
      R3 = ADD(R3,#-2)                      //[ LC0 = mG-2                          ]
    }
    { LOOP1(.CRC32BLK_OuterLOOP,R1)         //[ setup loop1: lc1 = nbytes/8         ]
    }

    .falign
.CRC32BLK_OuterLOOP:
    { R5:4 = MEMD(R6++#8)                   //[ load *data++ for i=0, 1, ..7        ]
      R7 = PARITY(R5:4,R9:8)                //[[O2] parity(d, crctab[i])            ]
      R0 |= ASL(R7,R10)                     //[[O2] crc |= parity() << n            ]
      R10 = ADD(R10,#-1)                    //[[O2] n--                             ]
    }
    { R4 = SWIZ(R4)                         //[ endian swap                         ]
      R10 = #31                             //[                                     ]
      R1 = R2                               //[ &crctab                             ]
      R0 |= ASL(R7,R10)                     //[[O2] crc |= parity() << n            ]
    }
    { R9:8 = MEMD(R1++#8)                   //[ crctab[0]                           ]
      R4 = SWIZ(R5)                         //[ endian swap                         ]
      R5 = XOR(R4,R0)                       //[ d ^= (UWord64)crc << 32             ]
      R0 = #0                               //[ crc = 0                             ]
    }
    { R9:8 = MEMD(R1++#8)                   //[ crctab[1]                           ]
      R7 = PARITY(R5:4,R9:8)                //[[p]parity(d,crctab[i])               ]
      LOOP0(.CRC32BLK_InnerLOOP,R3)         //[ setup loop0: lc0 = mG-2             ]
    }

    .falign
.CRC32BLK_InnerLOOP:
    { R9:8 = MEMD(R1++#8)                   //[[1] crctab[i]                        ]
      R7 = PARITY(R5:4,R9:8)                //[[2] parity(d, crctab[i])             ]
      R0 |= ASL(R7,R10)                     //[[3] crc |= parity() << n             ]
      R10 = ADD(R10,#-1)                    //[[3] n--                              ]
    }:endloop0:endloop1 

    { R7 = PARITY(R5:4,R9:8)                //[[e] parity(d, crctab[i])             ]
      R0 |= ASL(R7,R10)                     //[[e] crc |= parity() << n             ]
      R10 = ADD(R10,#-1)                    //[[e] n--                              ]
    }

.CRC32BLK_END:
    { R0 |= ASL(R7,R10)                     //[[e] crc |= parity() << n             ]
      JUMPR R31                             //[ return                              ]
    }
    .size    crc32_blk, .-crc32_blk




    /*[*****************************************************************************]*/
    /*[  Function   : UWord32 crc32_clut()                                          ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Block update implementation of CRC encoding                   ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - data MUST be aligned by 8bytes                                  ]*/
    /*[           - nbytes must be multiple of 16                                   ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8  *data                                            ]*/
    /*[               R1 : int     nbytes                                           ]*/
    /*[               R2 : UWord32 *crctab                                          ]*/
    /*[               R3 : UWord32 *delaycrc -  compression table with 16,15,14,13  ]*/
    /*[                                         delay                               ]*/
    /*[                                                                             ]*/
    /*[  Returns    : R0 : CRC bits                                                 ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R25                                                     ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 40                                ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - (11/16)*nbytes + 48              (~0.0859 cycles/bit)           ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  Revision History:                                                          ]*/
    /*[  ----------------                                                           ]*/
    /*[  Author           Date                Comments                              ]*/
    /*[  -------------------------------------------------------------------        ]*/
    /*[  DJH              07/15/09       created for V2 first version               ]*/
    /*[                                  currently ptr % 8 and n % 16 but           ]*/
    /*[                                  can be modified for arbitrary              ]*/
    /*[*****************************************************************************]*/


    /* --------------------------------------------------------------------------- */
    /*   The method utilizes the fact that a polynomial is congruent to the        */
    /*   polynomial less an arbitrary polynomial modulo a second polynomial.       */
    /*                                                                             */
    /*   M(x) is congruent to M'(x) if  M(x) % G(x) = M'(x) % G(x)                 */
    /*                                                                             */
    /*   this means that there exists Q(x) such that:                              */
    /*                                                                             */
    /*   M(x) - M'(x) = Q(x).G(x)                                                  */
    /*                                                                             */
    /*   if M(x) is the original message then it's CRC is M(x) % G(x) if the       */
    /*    message is added to a multiple of the generator polynomial then:         */
    /*                                                                             */
    /*     (M(x) - Q(x).G(x)) % G(x) = M(x) % G(x)                                 */
    /*                                                                             */
    /*   i.e. removing an arbitrary piece of the message modulo the CRC poly       */
    /*   G(x) and taking the CRC of the remaining message will produce the         */
    /*   same CRC as the original message. The original and modified are           */
    /*   congruent.                                                                */
    /*                                                                             */
    /*   To utilize this result we take the leading byte of the message & n        */
    /*   trailing zeros & take the CRC of this quantity, i.e. B.x^n this can       */
    /*   be stored in a lookup table. The delay n can be made large so it          */
    /*   doesn't affect the next few operations and so generates enough            */
    /*   temporary parallelism to allow the max number of lookups to be            */
    /*   generated in parallel.                                                    */
    /*                                                                             */
    /*   The future number is accumulated into a value which is then used to       */
    /*   add to the remaining message, this modified message is again crc'd        */
    /*   into the future and recursively the message collapses into a short        */
    /*   residue. The last few bytes of the modified message are CRC'd by          */
    /*   the usual slower lookup table method.                                     */
    /*                                                                             */
    /*   4 tables are used with differing delays so the CRCs are generated         */
    /*   at the same part of the message and this simplifies the code for          */
    /*   max performance, no need for shifts. Here delays of 16,15,14,13 are       */
    /*   ideal. There are 2 coders running in parallel independently as the        */
    /*   recurrence bound of the loop is the limiting factor.                      */
    /*                                                                             */
    /*   The disadvantage of this method is a large lookup table, table            */
    /*   useage is 5(1 basic+4delays) x 256(8bits per lookup) x 4(crc size)        */
    /*   bytes = 5120 bytes.                                                       */
    /*                                                                             */
    /* --------------------------------------------------------------------------- */


    /* =========================== variable declarations ==========================*/
                          //          1
                          //01234567890
#define data       R0     //x1111111111
#define nbyte      R1     //-----------
#define crctable   R2     //-----------
#define delaycrc   R3     //-----------
#define n16        R1     //-----------
#define crcsize    R12    //-----------
#define contab0    R3     //xx111x1x111
#define contab1    R4     //-----------
#define contab2    R5     //-----------
#define contab3    R1     //-----------
#define d7to0      R9:8   //-----------
#define d3to0      R8     //-----------
#define d7to4      R9     //-----------
#define dfto8      R11:10 //-----------
#define dbto8      R10    //-----------
#define dftoc      R11    //-----------
#define v40        R13:12 //--111------
#define v51        R7:6   //--11111111-
#define v0         R12    //--111------
#define v1         R6     //--11111111- 
#define v4         R13    //---11------
#define v5         R7     //---1111111- 
#define v51_       R13:12 //----------1
#define v62        R17:16 //----111----
#define v73        R19:18 //----1111111
#define v2         R16    //----111----
#define v3         R18    //----1111111
#define v6         R17    //-----11----
#define v7         R19    //-----111111
#define vc8        R13:12 //-------111-
#define vd9        R15:14 //-------1111
#define v8         R12    //-------111-
#define v9         R14    //-------1111 
#define vc         R13    //---------1-
#define vd         R15    //---------11 
#define vea        R17:16 //--------111
#define vfb        R21:20 //11111------
#define va         R16    //--------11-
#define vb         R20    //--------111
#define ve         R17    //----------1
#define vf         R21    //----------1
#define d7to0_     R23:22 //-111111111-
#define dfto8_     R25:24 //-1111111111
#define crc        R0     //-----------
#define lut        R6     //-----------
#define mask       R12    //-----------
#define par_state  R7     //-----------
#define ptr        R6     //-----------

    .text
    .p2align 2
    .p2align 4,,15
    .globl crc32_clut
    .type   crc32_clut, @function
crc32_clut:
    {  n16 = lsr(nbyte, #4)                 //[                                     ]
       crcsize = #256                       //[                                     ]
       sp = add(sp, #-40)                   //[                                     ]
       p0 = cmp.eq(nbyte,#0)                //[check for empty message              ]
    }
    {  n16 = add(n16, #-1)                  //[                                     ]
       if(!p0) memd(sp+#0) = r17:16         //[ callee-saved registers              ]
       if( p0) sp = add(sp, #40)            //[                                     ]
    }
    {  if(p0) crc = #0                      //[                                     ]
       if(!p0) memd(sp+#8) = r19:18         //[ callee-saved registers              ]
       if(p0) jumpr r31                     //[ return early                        ]
    }
    {  memd(sp+#16) = r21:20                //[ callee-saved registers              ]
       contab1 = addasl(delaycrc,crcsize,#2)//[                                     ]
       loop0(.Lcompress, n16)               //[                                     ]
    }
    {  memd(sp+#24) = r23:22                //[ callee-saved registers              ]
       contab2 = addasl(contab1,crcsize,#2) //[                                     ]
       p0 = cmp.eq(n16, #0)                 //[if(n16) for(i=0; i < n16; i++) {     ]
    }
    {  memd(sp+#32) = r25:24                //[ callee-saved registers              ]
       vfb =#0                              //[ 2                                   ]
       contab3 = addasl(contab2,crcsize, #2)//[                                     ]
    }
    {  d7to0 = memd(data++#16)              //[ 1                                   ]
       dfto8 = memd(data+#8)                //[ 2                                   ]
       if(p0) jump .Ltailonly               //[                                     ]
    }

    .falign
.Lcompress:
    {  contab0 = tableidxw(d3to0,#8,#0)     //[ 1                                   ]
       contab1 = tableidxw(d3to0,#8,#8)     //[ 1                                   ]
       d7to0_ = memd(data++#16)             //[ 1                                   ]
       dfto8_ = memd(data+#8)               //[ 2                                   ]
    }
    {  v0  = memw(contab0)                  //[ 1 crc lands 16+0bytes ahead         ]
       v1  = memw(contab1)                  //[ 1 crc lands 15+1bytes ahead         ]
       contab2 = tableidxw(d3to0,#8,#16)    //[ 1                                   ]
       contab3 = tableidxw(d3to0,#8,#24)    //[ 1                                   ]
    }
    {  v2  = memw(contab2)                  //[ 1 crc lands 14+2bytes ahead         ]
       v3  = memw(contab3)                  //[ 1 crc lands 13+3bytes ahead         ]
       contab0 = tableidxw(d7to4,#8,#0)     //[ 1                                   ]
       contab1 = tableidxw(d7to4,#8,#8)     //[ 1                                   ]
    }
    {  v4  = memw(contab0)                  //[ 1 crc lands 16+4bytes ahead         ]
       v5  = memw(contab1)                  //[ 1 crc lands 15+5bytes ahead         ]
       contab2 = tableidxw(d7to4,#8,#16)    //[ 1                                   ]
       contab3 = tableidxw(d7to4,#8,#24)    //[ 1                                   ]
    }
    {  v6  = memw(contab2)                  //[ 1 crc lands 14+6bytes ahead         ]
       v7  = memw(contab3)                  //[ 1 crc lands 13+7bytes ahead         ]
       v51 = xor(v51, v40)                  //[ 1                                   ]
       dfto8 = xor(dfto8, vfb)              //[ 2                                   ]
    }
    {  v7 = xor(v7, v6)                     //[ 1                                   ]
       v3 = xor(v3, v2)                     //[ 1                                   ]
       contab0 = tableidxw(dbto8,#8,#0)     //[ 2                                   ]
       contab1 = tableidxw(dbto8,#8,#8)     //[ 2                                   ]
    }
    {  v8  = memw(contab0)                  //[ 2 crc lands 16+8bytes ahead         ]
       v9  = memw(contab1)                  //[ 2 crc lands 15+9bytes ahead         ]
       contab2 = tableidxw(dbto8,#8,#16)    //[ 2                                   ]
       contab3 = tableidxw(dbto8,#8,#24)    //[ 2                                   ]
    }
    {  va  = memw(contab2)                  //[ 2 crc lands 14+10bytes ahead        ]
       vb  = memw(contab3)                  //[ 2 crc lands 13+11bytes ahead        ]
       contab0 = tableidxw(dftoc,#8,#0)     //[ 2                                   ]
       contab1 = tableidxw(dftoc,#8,#8)     //[ 2                                   ]
    }
    {  vc  = memw(contab0)                  //[ 2 crc lands 16+12bytes ahead        ]
       vd  = memw(contab1)                  //[ 2 crc lands 15+13bytes ahead        ]
       contab2 = tableidxw(dftoc,#8,#16)    //[ 4                                   ]
       contab3 = tableidxw(dftoc,#8,#24)    //[ 2                                   ]
    }
    {  ve  = memw(contab2)                  //[ 2 crc lands 14+14bytes ahead        ]
       vf  = memw(contab3)                  //[ 2 crc lands 13+15bytes ahead        ]
       vd9 = xor(vd9, vc8)                  //[ 2                                   ]
       v51_= xor(v51, d7to0_)               //[ 1 combine crc sum with next data    ]
    }
    {  d7to0 = xor(v73, v51_)               //[ 1                                   ]
       vb    = xor(vb, va)                  //[ 2                                   ]
       vf    = xor(vf, ve)                  //[ 2                                   ]
       dfto8 = xor(vd9, dfto8_)             //[ 2 combine crc sum with next data    ]
    }:endloop0

.Ltailonly:
    {  dfto8 = xor(dfto8, vfb)              //[ 2                                   ]
       crc = #0                             //[ crc = 0                             ]
       mask = #0x00ff                       //[                                     ]
       loop0(.Ltail_loop0,#7)               //[                                     ]
    }
    {  par_state = and(d3to0, mask)         //[                                     ]
       d7to0 = lsr(d7to0, #8)               //[                                     ]
       r17:16 = memd(sp++#16)               //[ restore callee-saved registers      ]
       r19:18 = memd(sp+#8)                 //[ restore callee-saved registers      ]
    }

    .falign
.Ltail_loop0:
    {  ptr = addasl(crctable,par_state,#2)  //[ [1] &crctab[(crc>>24)^ *data++]     ]
       crc = asl(crc,#8)                    //[ [1] crc <<8                         ]
       par_state = and(d3to0, mask)         //[                                     ]
    }
    {  lut = memw(ptr)                      //[                                     ]
       par_state ^= lsr(crc,#24)            //[ [2]crc>>24 ^ *data++                ]
    }
    {  crc = xor(crc, lut)                  //[ [1] crc = (crc<<8) ^crctab[]        ]
       par_state ^= lsr(lut, #24)           //[ [2] crc>>24 ^ (*data++)             ]
       d7to0 = lsr(d7to0, #8)               //[                                     ]
    }:endloop0

    {  loop0(.Ltail_loop1,#9)               //[                                     ]
       r21:20 = memd(sp++#16)               //[ restore callee-saved registers      ]
       r23:22 = memd(sp+#8)                 //[ restore callee-saved registers      ]
    }

    .falign
.Ltail_loop1:
    {  ptr = addasl(crctable,par_state,#2)  //[ [1] &crctab[(crc>>24)^ *data++]     ]
       crc = asl(crc,#8)                    //[ [1] crc <<8                         ]
       par_state = and(dbto8, mask)         //[                                     ]
    }
    {  lut = memw(ptr)                      //[                                     ]
       par_state ^= lsr(crc,#24)            //[ [2]crc>>24 ^ *data++                ]
    }
    {  crc = xor(crc, lut)                  //[ [1] crc = (crc<<8) ^crctab[]        ]
       par_state ^= lsr(lut, #24)           //[ [2] crc>>24 ^ (*data++)             ]
       dfto8 = lsr(dfto8, #8)               //[                                     ]
    }:endloop0

    {  r25:24 = memd(sp++#8)                //[ restore callee-saved registers      ]
       JUMPR R31                            //[ return                              ]
    }
    .size   crc32_clut, .-crc32_clut





#if __HEXAGON_ARCH__ >= 4

    /*[*****************************************************************************]*/
    /*[  Function   : UWord32 crc32poly()                                           ]*/
    /*[*****************************************************************************]*/
    /*[  Description: Block implementation of CRC encoding                          ]*/
    /*[=============================================================================]*/
    /*[  Assumptions:                                                               ]*/
    /*[           - data MUST be aligned by 8bytes                                  ]*/
    /*[           - nbytes must be multiple of 16                                   ]*/
    /*[=============================================================================]*/
    /*[  Inputs     : R0 : UWord8  *data                                            ]*/
    /*[               R1 : int     nbytes                                           ]*/
    /*[               R2 : UWord32 poly    -field polynomial                        ]*/
    /*[               R3 : UWord32 ipoly   -inverse field polynomial                ]*/
    /*[               R4 : UWord32 alpha64 -root alpha raised to power 64(poly*poly)]*/
    /*[                                         delay                               ]*/
    /*[                                                                             ]*/
    /*[  Returns    : R0 : CRC bits                                                 ]*/
    /*[=============================================================================]*/
    /*[  Register Usage: R0-R15                                                     ]*/
    /*[  Hardware Loops affected: Loop0, Loop1                                      ]*/
    /*[                                                                             ]*/
    /*[  Stack Memory Frame Allocated (in Bytes): 0                                 ]*/
    /*[=============================================================================]*/
    /*[  Cycle Count:                                                               ]*/
    /*[           - if((int) ptr_data % 4 == 0)                                     ]*/
    /*[                 cycles  = 14 + (nbyte + (nbyte % 8))/2                      ]*/
    /*[           - else                                                            ]*/
    /*[                 cycles <= 28 + nbyte/2                                      ]*/
    /*[                                                                             ]*/
    /*[=============================================================================]*/
    /*[  Revision History:                                                          ]*/
    /*[  ----------------                                                           ]*/
    /*[  Author           Date                Comments                              ]*/
    /*[  -------------------------------------------------------------------        ]*/
    /*[  DJH              05/04/10       created for V4 first version               ]*/
    /*[*****************************************************************************]*/

    /* --------------------------------------------------------------------------- */
    /* TECHNIQUE                                                                   */
    /*                                                                             */
    /*  The crc equation is of the form crc = (alpha*crc ^ data)*alpha             */
    /*  this equation can be mapped into a galois field operation, repeated        */
    /*  use of the function yields the equation                                    */
    /*                                                                             */
    /*              crc = (alpha^nbits '*' crc ^ data_nbits) '*' alpha^nbits       */
    /*                                                                             */
    /*  '*' is a galois field multiply in GF(2^N) in this case N = 32              */
    /*  V4 has a polynomial multiply instruction, this is used to implement        */
    /*  the GF(2^N) multiplication. Normally this is implemented by                */
    /*                                                                             */
    /*             a '*' b = (a . b) % g                                           */
    /*                                                                             */
    /*  . denotes polynomial multiply and % is a modulo of the product by          */
    /*  the field polynomial g.                                                    */
    /*                                                                             */
    /*  if P(x) = a(x) . b(x) then P(x) / G(x) is                                  */
    /*                                                                             */
    /*  P(x) / G(x) = D(x) + R(x)/G(x)  where D(x) is the ratio of the 2 -         */
    /*  the remainde, and R(x) is the modulo residue therefore we can              */
    /*  rewrite the multiply as:                                                   */
    /*                                                                             */
    /*       R(x) = P(x) + D(x).G(x)                                               */
    /*                                                                             */
    /*  thebits in R(x) above N-1 are by defintion zero. By euqating the 2         */
    /*  sides we can write                                                         */
    /*                                                                             */
    /*       D(x).G(x) = P(x)                                                      */
    /*  powers of x > N-1 this can be re written as a matrix equation as           */
    /*                                                                             */
    /*        |1    0    0  .. 0    0    0| |dn-2|  |p2n-2|                        */
    /*        |gn-1 1    0  .. 0    0    0| |dn-3|  |p2n-3|                        */
    /*        |gn-2 gn-1 1  .. 0    0    0| |dn-4|  |p2n-4|                        */
    /*        |:    :    :     :    :    :|.| :  | =| :   |                        */
    /*        |:    :    :     :    :    :| | :  |  | :   |                        */
    /*        |g3   g4   g5 .. gn-1 1    0| |d1  |  |pn+1 |                        */
    /*        |g2   g3   g4 .. g    gn-1 1| |d0  |  |pn   |                        */
    /*                                                                             */
    /*  The G matrix is simply a toeplitz of the upper N-2 bits of the             */
    /*  generator polynomial. The ratio can be solved by inverting the             */
    /*  matrix g. As this is a toeplitz of leading diagonal 1 its inverse          */
    /*  is also toeplitz with diagonal 1. As the poly p(x) is a constant it        */
    /*  can be solved offline the product of the inverse generator                 */
    /*  polynomial and a value can be performed using a polynomial multiply.       */
    /*  The inverse can be generated by passing the polynomial through an          */
    /*  lfsr and reading out the result. The code to do this is given here:        */
    /*                                                                             */
    /*        u32 qpoly_i(u32 poly) {                                              */
    /*          int k;                                                             */
    /*          u64 ipoly = 0x8000000000000000LL;                                  */
    /*          u64 gpoly = (u64)poly;                                             */
    /*                                                                             */
    /*          gpoly <<= 32;                                                      */
    /*          for(k=0; k < 32; k++)                                              */
    /*            ipoly = Q6_P_lfs_PP(ipoly, gpoly);                               */
    /*          ipoly = Q6_P_brev_P(ipoly) ;                                       */
    /*          return(ipoly);                                                     */
    /*        }                                                                    */
    /*                                                                             */
    /*  If we define  P(x) = Pa(x) + Pb(x).x^N  where Pa(x) = P(x) % x^N           */
    /*  and Pb(x) = (P(x) + Pa(x))/x^N                                             */
    /*  then the galois multiply operation can be defined as:                      */
    /*                                                                             */
    /*         Q'(x) = x.Q(x) + x^N                                                */
    /*         P(x) = A(x).B(x)                                                    */
    /*         D(x) = Pb(x).Q'(x).x                                                */
    /*         R(x) = Pa(x) + D(b(x).G(x) % x^N                                    */
    /*                                                                             */
    /*  Q(x) is the quotient polynomial generated by qpoly_i()                     */
    /*  Implementing the operations using the pmpyw instruction yields the         */
    /*  following pseudo assembly code for the crc operation:                      */
    /*                                                                             */
    /*         rr0 = pmpyw(alpha^N, crc)                                           */
    /*                                                                             */
    /*       { crc = rr0 ^ byterev(data)                                           */
    /*         rr0 ^= pmpyw(ipoly, hi(rr0))                                        */
    /*       }                                                                     */
    /*         crc ^= pmpyw(hi(r0), poly)                                          */
    /*                                                                             */
    /*  the incoming data needs to be byte reversed due to the message             */
    /*  stream being big endian. The alpha^N is the number of incoming bits        */
    /*  each time. For any field GF(2^N) alpha^N is always the lower 32bits        */
    /*  of the field polynomial.                                                   */
    /*  In the code below the inner loop, that computes crc of blocks of 8         */
    /*  bytes needs to be unrolled so alpha^64 is required, this is computed       */
    /*  offline and passed into the function using the code below:                 */
    /*                                                                             */
    /*       u32 gmpy(u32 poly, u32 ipoly)                                         */
    /*       {                                                                     */
    /*          u64 p0, r0;                                                        */
    /*                                                                             */
    /*          r0 = Q6_P_pmpyw_RR(poly, poly);                                    */
    /*          p0 = r0; r0 = Q6_P_pmpywxacc_RR(r0, ipoly, hi(r0));                */
    /*          p0 = Q6_P_pmpywxacc_RR(p0, hi(r0), poly) ;                         */
    /*                                                                             */
    /*          return(lo(p0));                                                    */
    /*       }                                                                     */
    /*  The performance of QDSP V4 is then 2 32x32 bit galois field                */
    /*  polynomial multpilies every 3 cycles or 1.5 cycles per multiply,           */
    /*  however due to byte reversal this drops to 64bits every 4 cycles.          */
    /*                                                                             */
    /* --------------------------------------------------------------------------- */

    /* =========================== variable declarations ==========================*/
#define ptr_data r0     //message pointer
#define length   r1     //number of bytes in message
#define poly     r2     //GF(2&^32) polynomial
#define ipoly    r3     //quotient polynomial
#define alpha64  r4     //poly*poly
#define b7654    r15    //upper 4 bytes
#define b3210    r5     //lower 4 bytes
#define b0       r5     //1 byte
#define lcrc_e   r7:6   //even crc for innner loop
#define crc_e    r6     //even crc for innner loop
#define rr0      r9:8   //temp poly multiply value
#define rr0_h    r9     //temp poly multiply value
#define rr0_l    r8     //temp poly multiply value
#define lcrc_o   r11:10 //odd crc for inner loop and across calculation
#define crc_o    r10    //odd crc for inner loop and across calculation
#define rr1      r13:12 //temp poly multiply value
#define rr1_h    r13    //temp poly multiply value
#define rr1_l    r12    //temp poly multiply value
#define alphai   r1     //honer rule coefficient x = a^n * x + y
#define start    r12    //num bytes in prolog pre 4byute aligned block
#define end      r14    //num bytes in prolog post 4byute aligned block
#define mid      r15    //num dwords in inner kernel
#define midend   r15:14 //calculate mid and end at same time
#define mask8    r13    //temporary mask for calculating pre and post bytes
#define lzeros   r11    //log2 nearest power of 2 above length 

    .text
    .p2align 2
    .p2align 4,,15
    .globl crc32poly
    .type  crc32poly, @function
crc32poly:
    { start = sub(#0, ptr_data)             //[ get prolog length                   ]
      lzeros = cl0(length)                  //[                                     ]
    }
    { start = and(start,#3)                 //[ prolog length num bytes up front    ]
      mask8 = CONST32(0x7fffffff)           //[                                     ]
    }
    { start &= asr(mask8, lzeros)           //[                                     ]
    }
    { mid = sub(length, start)              //[                                     ]
      p1 = cmp.eq(start, #0)                //[ is start empty                      ]
      loop0(.L_prolog, start)               //[ set up start loop                   ]
    }
    { midend = bitsplit(mid, #3)            //[ num bytes at end/dwords in kernel   ]
      if(p1) jump .L_startkernel            //[ if no start skip to kernel          ]
      crc_o = #0                            //[ initialize crc to 0                 ]
      crc_e = #0                            //[ set even crc to 0                   ]
    }

    .falign
.L_prolog:
    { b0 = memub(ptr_data++#1)              //[ load next byte                      ]
      rr0 = pmpyw(crc_o, alphai)            //[ [0,p]                               ]
      lcrc_o = pmpyw(crc_o, alphai)         //[ [0,p]                               ]
      alphai.L = #0x100                     //[ [2]set next horner multiply by alph8]
    }
    { crc_o ^= asl(b0, #24)                 //[ combine with current crc            ]
      rr0 ^= pmpyw(ipoly, rr0_h)            //[ [1,p]                               ]
    }
    { lcrc_o ^= pmpyw(rr0_h, poly)          //[ [2,p]                               ]
      alphai.H = #0x00                      //[ [2]set next horner multiply by alph8]
    }:endloop0

.L_startkernel:
    { p3 = cmp.gtu(end, #3)                 //[ check if 4bytes at end              ]
      p1 = cmp.eq(mid, #0)                  //[ no kernel x8                        ]
      if(p1.new) jump:NT .L_endkernel       //[ if no kernel skip                   ]
    }
    { loop0(.L_kernel, mid)                 //[ setup loop0                         ]
    }

    .falign
.L_kernel:
    { rr0 = pmpyw(crc_o, alphai)            //[ [0,0]untroll loop to remove         ]
      rr1 = pmpyw(crc_e, alphai)            //[ [0,0]recursion bound                ]
      b3210 = memw(ptr_data++#8)            //[ [0,0]kernel data can be 4b aligned  ]
      b7654 = memw(ptr_data+#4)             //[ [0,0]but multiple of 8              ]
    }
    { b3210 = swiz(b3210)                   //[ [0,1]                               ]
      b7654 = swiz(b7654)                   //[ [0,1]                               ]
      alphai = alpha64                      //[ [0,3]crc = alpha^64.crc + even/odd  ]
    }
    { crc_o = xor(rr0_l, b3210)             //[ [0,2]                               ]
      crc_e = xor(rr1_l, b7654)             //[ [0,2]                               ]
      rr0 ^= pmpyw(ipoly, rr0_h)            //[ [0,2]                               ]
      rr1 ^= pmpyw(ipoly, rr1_h)            //[ [0,2]                               ]
    }
    { lcrc_o ^= pmpyw(rr0_h, poly)          //[ [0,3]
      lcrc_e ^= pmpyw(rr1_h, poly)          //[ [0,3]
    }:endloop0
   
    { rr0 = pmpyw(crc_o, poly)              //[ [0,e]combine odd and even crcs      ]
      alphai = poly                         //[                                     ]
    }
    { crc_o = xor(crc_e, rr0_l)             //[ [0,e]                               ]
      rr0 ^= pmpyw(ipoly, rr0_h)            //[ [0,e]                               ]
    }
    { lcrc_o ^= pmpyw(rr0_h, poly)          //[ [0,e]                               ]
    }
.L_endkernel:
    { end = and(end, #3)                    //[ number of bytes in epilog 0-3       ]
      if(!p3)jump .L_noword                 //[ if no last qword skip               ]
      if(p3)b3210=memw(ptr_data++#4)        //[ load word little endian             ]
    }
    { b3210 = swiz(b3210)                   //[ [0,1] swap to big endian            ]
      rr0 = pmpyw(crc_o, alphai)            //[ [0,e]                               ]
    }
    { crc_o = xor(b3210, rr0_l)             //[ add in next word                    ]
      rr0 ^= pmpyw(ipoly, rr0_h)            //[ [0,e]                               ]
    }
    { lcrc_o ^= pmpyw(rr0_h, poly)          //[ [0,e]                               ]
      alphai = poly                         //[ horner rule with alpha^32           ]
    }

.L_noword:
    { p1 = cmp.eq(end, #0)                  //[                                     ]
      if(p1.new) jump:NT .L_end             //[ if no byte epilog skip              ]
    }
    { loop0(.L_epilog, end)                 //[                                     ]
    }

    .falign
.L_epilog:
    { b0 = memub(ptr_data++#1)              //[ load next byte                      ]
      rr0 = pmpyw(crc_o, alphai)            //[ [0,e]                               ]
      lcrc_o = pmpyw(crc_o, alphai)         //[ [0,e]                               ]
      alphai.L = #0x100                     //[ [2]set next horner multiply by alph8]
    }
    { crc_o ^= asl(b0, #24)                 //[ shift left instead of swapping      ]
      rr0 ^= pmpyw(ipoly, rr0_h)            //[ [0,e]                               ]
    }
    { lcrc_o ^= pmpyw(rr0_h, poly)          //[ [0,e]                               ]
      alphai.H = #0x00                      //[ [2]set next horner multiply by alph8]
    }:endloop0

.L_end:
    { rr0 = pmpyw(crc_o, alphai)            //[ [0,e] * alphai                      ]
    }
    { r0 = rr0_l                            //[ [0,e] final * alphai                ]
      rr0 ^= pmpyw(ipoly, rr0_h)            //[ [0,e]                               ]
    }
    { r1:0 ^= pmpyw(rr0_h, poly)            //[ [0,e]                               ]
      JUMPR R31                             //[ return                              ]
    }
    .size crc32poly, .-crc32poly
#endif

