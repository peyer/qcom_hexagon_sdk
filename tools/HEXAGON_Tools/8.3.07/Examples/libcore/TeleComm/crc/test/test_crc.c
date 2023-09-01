/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>
#include <string.h>
#include <hexagon_sim_timer.h>
#include "typedef.h"
#include "crc.h"


#define POLY8  0x07		// G(x) = x^8 + x^2 + x + 1
#define POLY16 0x10210000	// G(x) = x^16 + x^12 + x^5 +1
#define POLY32 0x04C11DB7	// G(x) = x^32+x^26+x^23+x^22+x^16+x^12
                                // +x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1


UWord32 Crc32Tab[256]; 
UWord32 Crc32ComTab[1024] __attribute__((aligned(1024))); 
uword8_8B_align Crc8Tab[256] __attribute__((aligned(256)));

unsigned char str[] __attribute__((aligned(8))) = 
"This is a test for CRC.\n";

unsigned char str1[] __attribute__((aligned(8))) = 
"This is another test for a CRC.\n";

UWord32 ipoly, alpha64;

int main()
{
   UWord32 crcbits;
   int len, len1;
   long long start_time; 
   int Total_cycles;

   len  = strlen((const char*)str);
   len1 = strlen((const char*)str1);

   //-------------------------------------------------------------------
   //test crc32() 
   start_time = hexagon_sim_read_pcycles();
   crcbits = crc32( str, 8*len-1, POLY32);
   Total_cycles = hexagon_sim_read_pcycles() - start_time;

   if ( crcbits != 0x9a622849) {
       printf("### crc32 FAILED!!!\n");
       return 1;
   }
   printf("Cycle-count: crc32     -- %d\n", Total_cycles); 

   //-------------------------------------------------------------------
   //test crc16() 
   start_time = hexagon_sim_read_pcycles();
   crcbits = crc16( str, 8*len-1, POLY16);
   Total_cycles = hexagon_sim_read_pcycles() - start_time;

   if ( crcbits != 0x98340000) {
       printf("### crc16 FAILED!!!\n");
       return 1;
   }

   printf("Cycle-count: crc16     -- %d\n", Total_cycles); 

   //-------------------------------------------------------------------
   // generate lookup table
   GenCrc32Tab(POLY32, Crc32Tab);

   start_time = hexagon_sim_read_pcycles();
   crcbits = crc32_lut( str, len, Crc32Tab);
   Total_cycles = hexagon_sim_read_pcycles() - start_time;

   if ( crcbits != 0x30054d25) {
       printf("### crc32 FAILED!!!\n");
       return 1;
   }
   printf("Cycle-count: crc32_lut -- %d\n", Total_cycles); 

   //-------------------------------------------------------------------
   // generate compression lookup table
   GenCrc32ComTab(POLY32, Crc32ComTab);

   start_time = hexagon_sim_read_pcycles();
   crcbits = crc32_clut( str1, len1, Crc32Tab, Crc32ComTab);
   Total_cycles = hexagon_sim_read_pcycles() - start_time;

   if ( crcbits != 0x971466D4) {
       printf("### crc32 FAILED!!!\n");
       printf(" %08X \n", (int)crcbits);
       return 1;
   }
   if(crc32_lut(str1, len1, Crc32Tab) != crcbits) {
       printf("### crc32 FAILED!!!\n");
       return 1;
   }
   printf("Cycle-count: crc32_clut -- %d\n", Total_cycles); 


   //-------------------------------------------------------------------
   GenCrc8Tab(POLY8, Crc8Tab);

   start_time = hexagon_sim_read_pcycles();
   crcbits = crc8_lut( str, 23, Crc8Tab);
   Total_cycles = hexagon_sim_read_pcycles() - start_time;

   if ( crcbits != 0x42) {
       printf("### crc8_lut FAILED!!!\n");
       return 1;
   }
   printf("Cycle-count: crc8_lut -- %d\n", Total_cycles); 

   //-------------------------------------------------------------------
   GenCrc32blkTab(POLY32, 32, 64, (UWord64 *)Crc8Tab);

   start_time = hexagon_sim_read_pcycles();
   crcbits = crc32_blk( str, len, (UWord64 *)Crc8Tab, 32);
   Total_cycles = hexagon_sim_read_pcycles() - start_time;

   if ( crcbits != 0x30054d25) {
       printf("### crc32_blk FAILED!!!\n");
       return 1;
   }
   printf("Cycle-count: crc32_blk -- %d\n", Total_cycles); 

   //-------------------------------------------------------------------
#if __HEXAGON_ARCH__ >= 4
   ipoly = qpoly(POLY32);
   alpha64 = gmpy(POLY32, POLY32, POLY32, ipoly);

   start_time = hexagon_sim_read_pcycles();
   crcbits = crc32poly(str, len, POLY32, ipoly, alpha64);
   Total_cycles = hexagon_sim_read_pcycles() - start_time;

   if ( crcbits != 0x30054d25) {
       printf("### crc32poly FAILED!!!\n");
       return 1;
   }
   printf("Cycle-count: crc32poly -- %d\n", Total_cycles); 
#endif

   printf("*** PASS!\n");
   return 0;
}

