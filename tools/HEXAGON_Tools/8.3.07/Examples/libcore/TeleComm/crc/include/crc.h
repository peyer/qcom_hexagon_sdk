/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:00 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:00 CST 2008 
****************************************************************************/ 


/*! \file crc.h
    \brief Brief description of file
*/

typedef unsigned char __attribute__((aligned(8)))  uword8_8B_align;

/*! 
Computes a CRC remainder vector of 31-bits or less.

\param data pointer to input bit-stream buffer
\param nbits number of input bits
\param poly generator polynomial word (left-aligned)

\details
\return
CRC bits 

\b Assembly \b Assumptions
 - None

\b Cycle-Count
 - 13*(nbits/8) + 2*(nbits&7) + 6  (~1.625 cycles/bit)

\b Notes
 - Bit-by-bit implementation
 - Output CRC bits are left-aligned in MSB of return value

*/

UWord32 crc32(unsigned char *data, int nbits, UWord32 poly);


/*! 
Computes a CRC remainder vector of 15-bits or less.

\param data pointer to input bit-stream buffer
\param nbits number of input bits
\param poly generator polynomial word (left-aligned)

\details
\return
CRC bits 

\b Assembly \b Assumptions
 - None

\b Cycle-Count
 - nbits + 2*ceil(nbits/8) + 4  (~1.25 cycles/bit)

\b Notes
 - Bit-by-bit implementation
 - Output CRC bits are left-aligned in MSB of return value

*/

UWord32 crc16(unsigned char *data, int nbits, UWord32 poly);


/*! 
Computes a CRC remainder vector of 31-bits or less.

\param data pointer to input bit-stream buffer
\param nbytes number of input bytes
\param crctab pointer to look-up table

\details
\return
CRC bits 

\b Assembly \b Assumptions
 - None

\b Cycle-Count
 - 3*nbytes + 3

\b Notes
 - LUT implementation
 - Output CRC bits are left-aligned in MSB of return value

*/


UWord32 crc32_lut(unsigned char *data,int nbytes, UWord32 *crctab);


/*! 
Computes a CRC remainder vector of 31-bits or less.

\param data pointer to input bit-stream buffer
\param nbytes number of input bytes
\param crctab pointer to look-up table
\param crccomtab pointer to compression look-up table

\details
\return
CRC bits 

\b Assembly \b Assumptions
 - None table must be aligned to 1024bytes
 - data must be aligned to 8bytes
 - nbyte must be multiple of 16bytes

\b Cycle-Count
 - 11*nbytes/16 + 48

\b Notes
 - LUT and special implementation
 - Output CRC bits are left-aligned in MSB of return value
 - with some simple additional code can be modified to all sizes and alignments 
*/

UWord32 crc32_clut(unsigned char *data,int nbytes, UWord32 *crctab, UWord32 *crccomtab);


/*! 
Computes a CRC remainder vector of 7-bits or less.

\param data pointer to input bit-stream buffer
\param nbytes number of input bytes
\param crctab pointer to look-up table

\details
\return
CRC bits 

\b Assembly \b Assumptions
 - LUT is aligned by 256

\b Cycle-Count
 - 2*nbytes + 3

\b Notes
 - LUT implementation
 - Output CRC bits are left-aligned in MSB of return value

*/

UWord8  crc8_lut(unsigned char *data,int nbytes, UWord8 *crctab);



/*! 
Computes a CRC remainder vector of 31-bits or less.

\param data pointer to input bit-stream buffer
\param nbytes number of input bytes
\param crctab pointer to look-up table
\param mG order of generator polynomial

\details
\return
CRC bits 

\b Assembly \b Assumptions
 - input array is aligned by 8

\b Cycle-Count
 - (mG+2)*ceil(nbytes/8) + 5  (~1.25 cycles/byte for mG=8)

\b Notes
 - block (64-bit) implementation proposed by Remi Gurski
 - Output CRC bits are left-aligned in MSB of return value

*/

UWord32  crc32_blk(unsigned char *data,int nbytes, UWord64 *crctab, int mG);


/*! 
Generates LUT for crc32_lut() function

\param GenPoly generator polynomail
\param crctab pointer to look-up table

*/

void GenCrc32Tab(UWord32 GenPoly, UWord32 *crctab);

/*! 
Generates LUT for crc32_clut() function

\param GenPoly generator polynomial
\param crctab pointer to compression look-up table

*/

void GenCrc32ComTab(UWord32 GenPoly, UWord32 *crccomtab);

/*! 
Generates LUT for crc8_lut() function

\param GenPoly generator polynomail
\param crctab pointer to look-up table

*/

void GenCrc8Tab(UWord8 GenPoly, UWord8 *crctab);


/*! 
Generates LUT for crc32_blk() function

\param GenPoly generator polynomail
\param mG order of the generator polynomial
\param nbits number of bits to be processed once
\param crctab pointer to look-up table

*/

void GenCrc32blkTab(UWord32 GenPoly, int mG, int nbits, UWord64 *crctab);

/*!
Generated Quotient polynomial
\param poly field polynomial leading 1 dropped

*/

unsigned int qpoly(unsigned int poly) ;

/*! 
Generates polynomial multiplication 

\param x  multiplicand 1
\param y  multiplicand 2
\param poly field polynomial leading 1 dropped
\param ipoly quotient polynomial

*/

unsigned int gmpy(unsigned int x, unsigned int y,
                  unsigned int poly, unsigned int ipoly);


/*! 
Generates crc using polynomial multiplication 

\param ptr8_data pointer to the message byte aligned
\param len number of bytes in message
\param poly field polynomial leading 1 dropped
\param ipoly quotient polynomial
\param alpha64 root to the power 64 or poly * poly

*/

unsigned int crc32poly(unsigned char *ptr8_data, int len,
                       unsigned int poly, unsigned int ipoly, unsigned int alpha64);
 
