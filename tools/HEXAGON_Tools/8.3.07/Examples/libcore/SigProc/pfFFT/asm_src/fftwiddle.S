/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:05 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:05 CST 2008 
****************************************************************************/ 

    .file    "fftwiddle.S"

    /*-----------------------------------------------------------------------------*/
    /*                 Twiddle factors  for 8,16,32, 64-point FFT                  */
    /*-----------------------------------------------------------------------------*/
    .globl FFTtwiddles
    .data
    .p2align 3
    .type    FFTtwiddles, @object
    .size    FFTtwiddles, 192
FFTtwiddles:
    .word  0x00007fff, 0x80000000, 0x80000000 
    .word  0x80000000, 0xa57ea57e, 0x5a82a57e 
    .word  0xa57e5a82, 0x89bfcf05, 0xcf0589bf 
    .word  0xa57ea57e, 0xcf0589bf, 0x764230fc 
    .word  0xcf057642, 0x8276e708, 0x9593b8e4 
    .word  0x89bfcf05, 0xb8e49593, 0x7d8ae708 
    .word  0x89bf30fc, 0x9593b8e4, 0x18f98276 
    .word  0xcf0589bf, 0xe7088276, 0x471d6a6e 
    .word  0xe7087d8a, 0x809ef375, 0x8583dad8 
    .word  0x8276e708, 0xaecd9d0e, 0x70e3c3aa 
    .word  0x9593471d, 0x8f1ec3aa, 0xf375809e 
    .word  0xb8e49593, 0xdad88583, 0x62f25134 
    .word  0xb8e46a6e, 0x8583dad8, 0xaecd9d0e 
    .word  0x9593b8e4, 0xc3aa8f1e, 0x7f620c8c 
    .word  0x827618f9, 0x9d0eaecd, 0x3c578f1e 
    .word  0xe7088276, 0xf375809e, 0x25287a7d 
