/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:19:54 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:19:54 CST 2008 
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

#include "hexagon_sim_timer.h"

#include "typedef.h"
#include "Kasumi.h"

#include "testset.h"

int main()
{
 int nset, i, k;
 UWord8 mask;
 UWord8 *data, *data_ref;
 int start_time, Total_cycles;

 for(nset =0; nset < 6; nset++)
 {
    data = input[nset];
   
	start_time = hexagon_sim_read_pcycles();
 
	f8( &Key[nset][0], Count[nset], Bearer[nset], Direction[nset], data, Length[nset] );

        Total_cycles = hexagon_sim_read_pcycles() - start_time;

	printf("Cycle Count: %d\t-- ", Total_cycles);
	
	data_ref = output[nset];	
	
	for(i=0; i< Length[nset]/8; i++)
	{
		if( data[i] != data_ref[i] )
		{
		   printf("Error with Test Set #%d\n", nset+1);
		   printf("**** Error at Byte #%d:  output= %x   ref= %x\n", i, data[i], data_ref[i]);
		   return 1;
		}
	}
	
	k = 8 - (Length[nset]& 0x07);
	if( k != 8 )
	{
		mask = ~((1<<k) - 1);
		if( (mask & data[i]) != data_ref[i] )
		{
		   printf("Error with Test Set #%d\n", nset+1);
		   printf("**** Error at Last Byte:  output= %x   ref= %x\n", data[i], data_ref[i]);
		   return 1;
		}		
	}
	printf("PASS Test Set #%d !\n\n", nset+1);	
 }
 printf("*** PASS!\n");
 return 0;
	
}
