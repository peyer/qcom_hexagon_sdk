/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


/*-------------------------------------------------------------------
 *				F8 - Confidentiality Algorithm
 *-------------------------------------------------------------------
 *
 *	A sample implementation of f8, the 3GPP Confidentiality algorithm.
 *
 *	This has been coded for clarity, not necessarily for efficiency.
 *
 *	This will compile and run correctly on both Intel (little endian)
 *  and Sparc (big endian) machines. (Compilers used supported 32-bit ints)
 *
 *	Version 1.0		05 November  1999
 *
 *-------------------------------------------------------------------*/
#include <stdio.h>
#include "typedef.h"
#include "Kasumi.h"

/*----- a 64-bit structure to help with endian issues -----*/

typedef union {
	UWord32 b32[2];
	UWord16 b16[4];
	UWord8  b8[8];
} REGISTER64;

/*---------------------------------------------------------
 * f8()
 *		Given key, count, bearer, direction,  data,
 *		and bit length  encrypt the bit stream
 *---------------------------------------------------------*/
void f8( UWord8 *key, int count, int bearer, int dir, UWord8 *data, int length )
{
	REGISTER64 A;		/* the modifier			*/
	REGISTER64 temp;		/* The working register	*/
	int i, n;
	UWord8  ModKey[16];		/* Modified key		*/
	UWord16 blkcnt;			/* The block counter */

	/* Start by building our global modifier */

	temp.b32[0]  = temp.b32[1]  = 0;
	A.b32[0]     = A.b32[1]     = 0;

	/* initialise register in an endian correct manner*/

	A.b8[0]  = (UWord8) (count>>24);
	A.b8[1]  = (UWord8) (count>>16);
	A.b8[2]  = (UWord8) (count>>8);
	A.b8[3]  = (UWord8) (count);
	A.b8[4]  = (UWord8) (bearer<<3);
	A.b8[4] |= (UWord8) (dir<<2);

	/* Construct the modified key and then "kasumi" A */

	for( n=0; n<16; ++n )
		ModKey[n] = (UWord8)(key[n] ^ 0x55);
	KeySchedule( ModKey );

	Kasumi( A.b8 );	/* First encryption to create modifier */

	/* Final initialisation steps */

	blkcnt = 0;
	KeySchedule( key );

	/* Now run the block cipher */

	while( length > 0 )
	{
		/* First we calculate the next 64-bits of keystream */

		/* XOR in A and BLKCNT to last value */

		temp.b32[0] ^= A.b32[0];
		temp.b32[1] ^= A.b32[1];
		temp.b8[7]  ^= blkcnt;

		/* KASUMI it to produce the next block of keystream */

		Kasumi( temp.b8 );

		/* Set <n> to the number of bytes of input data	*
		 * we have to modify.  (=8 if length <= 64)		*/

		if( length >= 64 )
			n = 8;
		else
			n = (length+7)/8;

		/* XOR the keystream with the input data stream */

		for( i=0; i<n; ++i )
			*data++ ^= temp.b8[i];
		length -= 64;	/* done another 64 bits	*/
		++blkcnt;		/* increment BLKCNT		*/
	}
}

/*-----------------------------------------------------------
 *			e n d    o f    f 8 . c
 *-----------------------------------------------------------*/
