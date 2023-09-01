/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:02 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:02 CST 2008 
****************************************************************************/ 


#include <stdio.h>
#include "data_macros.h"
#include "typedef.h"

#include "hexagon_sim_timer.h"

/* prototype for upsampling filter */
#include "upsamp.h"


#define INPUT_SIZE 128

#ifndef DATA_DIR
#define DATA_DIR "./data"
#endif

/* Allotting the needed buffers as globals */
Word16 in[INPUT_SIZE];
Word16 out[8*INPUT_SIZE] __attribute__((aligned(8)));

Word16 filt_memory[28*2] __attribute__((aligned(8))) ;

Word16 expected_out[8*INPUT_SIZE];

int main ()
{
    int i, numsamp;
    int start_time, Total_cycles;

    FILE *Infile, *Outfile;


    // initialization 
    for(i=0;i<32;i++)
        filt_memory[i]=0;

    // load input and reference output
    if((Infile = fopen(FULL_FILE_NAME("indata.bin"),"rb")) == NULL) {
       printf("Error: can not open indata.bin\n");
       return 1;
    }

    if((Outfile = fopen(FULL_FILE_NAME("outdata.bin"),"rb")) == NULL) {
       printf("Error: can not open outdata.bin\n");
       return 1;
    }

    if (fread(in,  sizeof(Word16), INPUT_SIZE, Infile) != INPUT_SIZE ) {
       printf("Error, Unable to read to indata.bin\n");
       return 1;
    }

    if (fread(expected_out, sizeof(Word16), 8*INPUT_SIZE, Outfile) != (8*INPUT_SIZE)) {
         printf("Error, Unable to read to rgb.out\n");
         return 1;
    }

    fclose(Infile);
    fclose(Outfile);

    // call function
    start_time = hexagon_sim_read_pcycles();
    // printf("%d\n", start_time);
    // For mono
    // numsamp = INPUT_SIZE;
    // upsamp8(in, filt_memory, INPUT_SIZE, 0, out);

    // For stereo
    numsamp = INPUT_SIZE/2;
    upsamp8(in  , filt_memory,    INPUT_SIZE/2, 1, out  );
    upsamp8(in+1, filt_memory+28, INPUT_SIZE/2, 1, out+1);

    Total_cycles = hexagon_sim_read_pcycles() - start_time;

    //extra call, zero length for complete coverage of fall through case
    upsamp8(in+1, filt_memory+28, 0, 1, out+1);


    // compare results
    for (i=0; i<8*INPUT_SIZE; i++) {
        if( out[i] != expected_out[i]) {
           printf("### FAILED !!!!\n");
           return 1;
        }
    }
    printf("*** PASS!!\nCycle Count:  %d PER SAMPLES\n", Total_cycles/INPUT_SIZE );
    return 0;
}
