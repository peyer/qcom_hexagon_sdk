/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>
#include "typedef.h"
#include <hexagon_protos.h>


void GenCrc32Tab(UWord32 GenPoly, UWord32 *crctab)
{
   UWord32 crc;
   int i, j;

   for (i = 0; i < 256; i++) {
       crc = i<<24;
       for (j = 0; j <8; j++) {
           crc = (crc << 1) ^ (crc & 0x80000000 ? GenPoly : 0);
       }
       crctab[i] = crc;
   }
}


void GenCrc8Tab(UWord8 GenPoly, UWord8 *crctab)
{
   UWord8 crc;
   int i, j;

   for (i = 0; i < 256; i++) {
       crc = i;
       for (j = 0; j <8; j++) {
           crc = (crc << 1) ^ (crc & 0x80 ? GenPoly : 0);
       }
       crctab[i] = crc;
   }
}



// Generate transfer matrix for n-bits shift 
void GenCrc32blkTab(UWord32 GenPoly, int mG, int nbits, UWord64 *crctab) {
   static UWord32 gen[64];
   UWord32 crc;
   UWord64 value; 
   int i, j;

   // Generate remainder for X^63, X^62, ...  
   for (i=0; i<nbits; i++) {
       crc = 1<< 31; 
       for (j=0; j< (64-i); j++) {
             crc = (crc << 1) ^ (crc &0x80000000 ? GenPoly : 0);
       }
       gen[i] = crc;	
   }

   // Construct table from remainders
   for (i=0; i< mG; i++) {
       value = 0;
       for (j=0; j< nbits; j++) {
           value = (value << 1) | ((gen[j] >> (31-i)) & 1);
       }
       crctab[i] = value;
   }
}

//Generate Congruent compression table for crc32_clut function
//builds 4x1024byte block must be aligned to 1024bytes

#define MAXDELAY 16
void GenCrc32ComTab(UWord32 GenPoly, UWord32 *crccomtab)
{
   int i, j, k, l;
   unsigned char v[MAXDELAY];
   unsigned int crc;
   int delay = 16;

   for(j=0; j < 4; j++) {
     for (i = 0; i < 256; i++) {
       for(k=1; k < (delay-j); k++) v[k] = 0; v[0] = i;

       crc = 0;
       for(k=0; k < delay-j; k++) {
         crc  ^= v[k] << 24;
         for (l=0; l < 8; ++l) {
           crc = (crc << 1) ^ (crc & 0x80000000 ? GenPoly : 0);
         }
       }
       crc = ((crc >> 24) & 0xff)     | ((crc >> 16) & 0xff) <<  8 |
             ((crc >>  8) & 0xff)<<16 | ((crc >>  0) & 0xff) << 24 ;

       crccomtab[i+j*256] = crc ;
     }
   }
}


#if __HEXAGON_ARCH__ >= 4
//perform galois field multiply for V4

unsigned int gmpy(unsigned int x, unsigned int y,
                        unsigned int poly, unsigned int ipoly)
{
    unsigned long long pp0, rr0;

    rr0 = Q6_P_pmpyw_RR(x, y);
    pp0 = rr0; 
    rr0 = Q6_P_pmpywxacc_RR(rr0, ipoly, (unsigned int) (rr0>>32));
    pp0 = Q6_P_pmpywxacc_RR(pp0, (unsigned int) (rr0>>32), poly) ;

    return((unsigned int)pp0);
}

unsigned int qpoly(unsigned int poly) {
   int k;
   unsigned long long ipoly = 0x8000000000000000LL;
   unsigned long long gpoly = poly;

   gpoly <<= 32;
   for(k=0; k < 32; k++) {
     ipoly = Q6_P_lfs_PP(ipoly, gpoly); //fibonacci lfsr is used to get the inverse poly
   }
   ipoly = Q6_P_brev_P(ipoly) ;
   return(ipoly);
}
#endif


