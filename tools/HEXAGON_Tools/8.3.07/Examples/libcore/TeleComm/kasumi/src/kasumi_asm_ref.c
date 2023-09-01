/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


/*-----------------------------------------------------------------------
 *						Kasumi.c
 *-----------------------------------------------------------------------
 *
 *	A sample implementation of KASUMI, the core algorithm for the
 *	3GPP Confidentiality and Integrity algorithms.
 *
 *	This has been coded for clarity, not necessarily for efficiency.
 *
 *	This will compile and run correctly on both Intel (little endian)
 *	and Sparc (big endian) machines. (Compilers used supported 32-bit ints).
 *
 *	Version 1.1		08 May 2000
 *
 *-----------------------------------------------------------------------*/
#include <stdio.h>
#include "typedef.h"
#include "Kasumi.h"

typedef union __attribute__((packed)) {
	UWord32 b32;
	UWord16 b16[2];
	UWord8  b8[4];
} DWORD;

/*--------- 16 bit rotate left ------------------------------------------*/
#define ROL16(a,b) (UWord16)((a<<b)|(a>>(16-b)))


/*-------- globals: The subkey arrays -----------------------------------*/

static UWord16 KLi[16];
static UWord16 KOIi[48];


/*---------------------------------------------------------------------
 *	FI()
 *		The FI function (fig 3).  It includes the S7 and S9 tables.
 *		Transforms a 16-bit value.
 *---------------------------------------------------------------------*/
static UWord8 S7[] = {
		54, 50, 62, 56, 22, 34, 94, 96, 38, 6, 63, 93, 2, 18,123, 33,
		55,113, 39,114, 21, 67, 65, 12, 47, 73, 46, 27, 25,111,124, 81,
		53, 9,121, 79, 52, 60, 58, 48,101,127, 40,120,104, 70, 71, 43,
		20,122, 72, 61, 23,109, 13,100, 77, 1, 16, 7, 82, 10,105, 98,
		117,116, 76, 11, 89,106, 0,125,118, 99, 86, 69, 30, 57,126, 87,
		112, 51, 17, 5, 95, 14, 90, 84, 91, 8, 35,103, 32, 97, 28, 66,
		102, 31, 26, 45, 75, 4, 85, 92, 37, 74, 80, 49, 68, 29,115, 44,
		64,107,108, 24,110, 83, 36, 78, 42, 19, 15, 41, 88,119, 59, 3};

static UWord16 S9[] = {
		167,239,161,379,391,334,  9,338, 38,226, 48,358,452,385, 90,397,
		183,253,147,331,415,340, 51,362,306,500,262, 82,216,159,356,177,
		175,241,489, 37,206, 17,  0,333, 44,254,378, 58,143,220, 81,400,
		 95,  3,315,245, 54,235,218,405,472,264,172,494,371,290,399, 76,
		165,197,395,121,257,480,423,212,240, 28,462,176,406,507,288,223,
		501,407,249,265, 89,186,221,428,164, 74,440,196,458,421,350,163,
		232,158,134,354, 13,250,491,142,191, 69,193,425,152,227,366,135,
		344,300,276,242,437,320,113,278, 11,243, 87,317, 36, 93,496, 27,
		487,446,482, 41, 68,156,457,131,326,403,339, 20, 39,115,442,124,
		475,384,508, 53,112,170,479,151,126,169, 73,268,279,321,168,364,
		363,292, 46,499,393,327,324, 24,456,267,157,460,488,426,309,229,
		439,506,208,271,349,401,434,236, 16,209,359, 52, 56,120,199,277,
		465,416,252,287,246,  6, 83,305,420,345,153,502, 65, 61,244,282,
		173,222,418, 67,386,368,261,101,476,291,195,430, 49, 79,166,330,
		280,383,373,128,382,408,155,495,367,388,274,107,459,417, 62,454,
		132,225,203,316,234, 14,301, 91,503,286,424,211,347,307,140,374,
		 35,103,125,427, 19,214,453,146,498,314,444,230,256,329,198,285,
		 50,116, 78,410, 10,205,510,171,231, 45,139,467, 29, 86,505, 32,
		 72, 26,342,150,313,490,431,238,411,325,149,473, 40,119,174,355,
		185,233,389, 71,448,273,372, 55,110,178,322, 12,469,392,369,190,
		  1,109,375,137,181, 88, 75,308,260,484, 98,272,370,275,412,111,
		336,318,  4,504,492,259,304, 77,337,435, 21,357,303,332,483, 18,
		 47, 85, 25,497,474,289,100,269,296,478,270,106, 31,104,433, 84,
		414,486,394, 96, 99,154,511,148,413,361,409,255,162,215,302,201,
		266,351,343,144,441,365,108,298,251, 34,182,509,138,210,335,133,
		311,352,328,141,396,346,123,319,450,281,429,228,443,481, 92,404,
		485,422,248,297, 23,213,130,466, 22,217,283, 70,294,360,419,127,
		312,377,  7,468,194,  2,117,295,463,258,224,447,247,187, 80,398,
		284,353,105,390,299,471,470,184, 57,200,348, 63,204,188, 33,451,
		 97, 30,310,219, 94,160,129,493, 64,179,263,102,189,207,114,402,
		438,477,387,122,192, 42,381,  5,145,118,180,449,293,323,136,380,
		 43, 66, 60,455,341,445,202,432, 8,237, 15,376,436,464, 59,461};



/*---------------------------------------------------------------------
 * FOI()
 *		The FOI() function.
 *		Transforms a 32-bit value.  Uses <index> to identify the
 *		appropriate subkeys to use.
 *---------------------------------------------------------------------*/
static UWord32 FOI( UWord32 in, UWord16 *KOIitable )
{
    UWord32 x, y, temp;
    UWord32 nine1, seven1, nine2, seven2, nine3, seven3, subkey;
    int i;
    
    /* Split the input into two 16-bit words */

    x  = (UWord16)(in>>16);
    y = (UWord16) in;

    /* Now apply the same basic transformation three times         */

    for(i = 0; i < 3; i++)
    {
	x ^= (*KOIitable);	
	KOIitable = KOIitable + 1;
	
	subkey = *KOIitable;
	KOIitable = KOIitable + 1;
	
	// FI function
	seven1 = x & 0x7F;
	nine1  = S9[x>>7] ^ seven1;

	nine2  =  S9[x>>7]   ^ seven1 ^ (subkey&0x1FF);
	seven2 = (S7[seven1] ^ nine1 ^ (subkey >> 9)) & 0x7F;
	
	nine3  = S9[nine2]  ^ seven2;
	seven3 = S7[seven2] ^ (nine3 & 0x7F);

	temp = (seven3<<9)  + nine3;
	x = y;
	y ^= temp;    	
    	
    }

    in = (x<<16)| y;
    return( in );
}

/*---------------------------------------------------------------------
 * FL()
 *		The FL() function.
 *		Transforms a 32-bit value.  Uses <index> to identify the
 *		appropriate subkeys to use.
 *---------------------------------------------------------------------*/

static UWord32 FL( UWord32 in, UWord16 *KLitable )
{
	UWord16 l, r, a, b;

	/* split out the left and right halves */

	l = (UWord16)(in>>16);
	r = (UWord16)(in);

	/* do the FL() operations			*/

	a  = l & (*KLitable);	
	KLitable = KLitable + 1;
	r ^= ROL16(a,1);
		
	b  = r | (*KLitable);
	l ^= ROL16(b,1);

	/* put the two halves back together */
	in = ((UWord32)l<<16) | r;

	return( in );
}



/*---------------------------------------------------------------------
 * Kasumi()
 *		the Main algorithm (fig 1).  Apply the same pair of operations
 *		four times.  Transforms the 64-bit input.
 *---------------------------------------------------------------------*/
void Kasumi( UWord8 *data )
{
	UWord32 left, right, temp;
	DWORD *d;
	UWord16 *KLitable = KLi; 
	UWord16 *KOIitable = KOIi;
	int n;

	/* Start by getting the data into two 32-bit words (endian corect) */
	d = (DWORD*)data;
	left  = (((UWord32)d[0].b8[0])<<24)+(((UWord32)d[0].b8[1])<<16)
		+(d[0].b8[2]<<8)+(d[0].b8[3]);
	right = (((UWord32)d[1].b8[0])<<24)+(((UWord32)d[1].b8[1])<<16)
		+(d[1].b8[2]<<8)+(d[1].b8[3]);

	for(n = 0; n < 4; n++)
	{
		temp = FL( left, KLitable );
		temp = FOI( temp, KOIitable );
		right ^= temp;
		KOIitable += 6;
		KLitable += 2;

		temp = FOI( right, KOIitable );
		temp = FL( temp,  KLitable );
		left ^= temp;	
		KLitable += 2;
		KOIitable += 6;
	}

	/* return the correct endian result */
	d[0].b8[0] = (UWord8)(left>>24);		d[1].b8[0] = (UWord8)(right>>24);
	d[0].b8[1] = (UWord8)(left>>16);		d[1].b8[1] = (UWord8)(right>>16);
	d[0].b8[2] = (UWord8)(left>>8);		d[1].b8[2] = (UWord8)(right>>8);
	d[0].b8[3] = (UWord8)(left);		d[1].b8[3] = (UWord8)(right);
}

/*---------------------------------------------------------------------
 * KeySchedule()
 *		Build the key schedule.  Most "key" operations use 16-bit
 *		subkeys so we build UWord16-sized arrays that are "endian" correct.
 *---------------------------------------------------------------------*/
void KeySchedule( UWord8 *k )
{
	static UWord16 C[] = {
		0x0123,0x4567,0x89AB,0xCDEF, 0xFEDC,0xBA98,0x7654,0x3210 };
	UWord16 key[8], Kprime[8];
	int n;

	/* Now build the K'[] keys */
	for( n=0; n<8; ++n )
	{
		key[n] = (UWord16)((k[2*n]<<8) + (k[2*n+1]));
		Kprime[n] = (UWord16)(key[n] ^ C[n]);
	}

	/* Finally construct the various sub keys */

	for( n=0; n<8; ++n )
	{
		KLi[2*n   ] = ROL16(key[n],1);
		KLi[2*n+1 ] = Kprime[(n+2)&0x7];
		
		KOIi[6*n  ] = ROL16(key[(n+1)&0x7],5);
		KOIi[6*n+1] = Kprime[(n+4)&0x7];
		KOIi[6*n+2] = ROL16(key[(n+5)&0x7],8);
		KOIi[6*n+3] = Kprime[(n+3)&0x7];
		KOIi[6*n+4] = ROL16(key[(n+6)&0x7],13);
		KOIi[6*n+5] = Kprime[(n+7)&0x7];
	}
}


/*---------------------------------------------------------------------
 *				e n d    o f    k a s u m i . c
 *---------------------------------------------------------------------*/

