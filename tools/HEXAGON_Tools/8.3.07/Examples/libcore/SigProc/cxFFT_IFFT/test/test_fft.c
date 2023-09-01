/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "data_macros.h"
#include "typedef.h"
#include "q6basic_op.h"
#include "hexagon_sim_timer.h"

#include "fft.h"
#include "fftwiddles.h"

#undef PRECISION_TEST

#ifndef DATA_DIR
#define DATA_DIR "./data"
#endif

CWord2x16 Input[256]   __attribute__((aligned(1<<16)));
CWord2x16 Output[256]  __attribute__((aligned(8)));
CWord2x16 Expected_results[256];

#ifndef PRECISION_TEST
int main()
{
    FILE *Infile;
    int i, Nfft; 
    long long start_time; 
    int overhead, Total_cycles;

   /* --------------------------------------------------------- */
   /*                     Test FFT                              */
   /* --------------------------------------------------------- */
    start_time = hexagon_sim_read_pcycles();
    fft16x16(Input, 0 , Wtwiddles, Output);
    overhead = (int)(hexagon_sim_read_pcycles() - start_time) -1;

   /* --------------------------------------------------------- */
   /* read inputs from file                                     */
   /* --------------------------------------------------------- */
   if((Infile = fopen(FULL_FILE_NAME("FFTin0.bin"),"rb")) == NULL) {
     printf("Error: can not open FFTin(0).bin\n");
     return 1;
   }
   if (fread(Input,  sizeof(CWord2x16), 256, Infile) != 256 ) 
       printf("Error, Unable to read from FFTin(0).bin\n");
   fclose(Infile);


   /* --------------------------------------------------------- */
   /* CALL 128-point FFT                                        */
   /* --------------------------------------------------------- */
    Nfft = 128;
    printf("testing %d-point  FFT ....", Nfft);

    start_time = hexagon_sim_read_pcycles();
    fft16x16(Input,Nfft, Wtwiddles, Output);
    Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;

   /* --------------------------------------------------------- */
   /* read expected results from file                           */
   /* --------------------------------------------------------- */
   if((Infile = fopen(FULL_FILE_NAME("FFT128out.bin"),"rb")) == NULL) {
     printf("Error: can not open (scale)FFT(cr)128out.bin\n");
     return 1;
   }
   if (fread(Expected_results, sizeof(CWord2x16), Nfft, Infile) != Nfft ) 
       printf("Error: Unable to read from (scaled)FFT(cr)128.bin\n");
   fclose(Infile);

   /* --------------------------------------------------------- */
   /*  compare results                                          */
   /* --------------------------------------------------------- */
    for (i=0;i<Nfft;i++) {
        if (Output[i] != Expected_results[i]) {
            printf("FAILED!!!\n");
            return 1;
        }

    }
    printf("PASSED!   Cycle-count: %d\n", Total_cycles);


   /* --------------------------------------------------------- */
   /* CALL 256-point FFT                                        */
   /* --------------------------------------------------------- */
    Nfft = 256;
    printf("testing %d-point  FFT ....", Nfft);

    start_time = hexagon_sim_read_pcycles();
    fft16x16(Input,Nfft, Wtwiddles, Output);
    Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;

   /* --------------------------------------------------------- */
   /* read expected results from file                           */
   /* --------------------------------------------------------- */
   if((Infile = fopen(FULL_FILE_NAME("FFT256out.bin"),"rb")) == NULL) {
     printf("Error: can not open (scale)FFT(cr)256out.bin\n");
     return 1;
   }
   if (fread(Expected_results, sizeof(CWord2x16), Nfft, Infile) != Nfft ) 
       printf("Error: Unable to read from (scaled)FFT(cr)256.bin\n");
   fclose(Infile);

   /* --------------------------------------------------------- */
   /*  compare results                                          */
   /* --------------------------------------------------------- */
    for (i=0;i<Nfft;i++) {
        if (Output[i] != Expected_results[i]) {
            printf("FAILED!!! \n");
            return 1;
        }
    }
    printf("PASSED!   Cycle-count: %d\n", Total_cycles);

   /* --------------------------------------------------------- */
   /*                     Test scaled FFT                       */
   /* --------------------------------------------------------- */

   /* --------------------------------------------------------- */
   /* read inputs from file                                     */
   /* --------------------------------------------------------- */
   if((Infile = fopen(FULL_FILE_NAME("FFTin.bin"),"rb")) == NULL) {
     printf("Error: can not open FFTin(0).bin\n");
     return 1;
   }
   if (fread(Input,  sizeof(CWord2x16), 256, Infile) != 256 ) 
       printf("Error, Unable to read from FFTin(0).bin\n");
   fclose(Infile);


   /* --------------------------------------------------------- */
   /* CALL 128-point scaled FFT                                 */
   /* --------------------------------------------------------- */
    Nfft = 128;
    printf("testing %d-point sFFT ....", Nfft);

    start_time = hexagon_sim_read_pcycles();
    sfft16x16(Input,Nfft, Wtwiddles, Output);
    Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;

   /* --------------------------------------------------------- */
   /* read expected results from file                           */
   /* --------------------------------------------------------- */
#ifdef TWOS_COMPLEMENT_ROUNDING
   if((Infile = fopen(FULL_FILE_NAME("scaleFFT128out.bin"),"rb")) == NULL) {
#else
   if((Infile = fopen(FULL_FILE_NAME("scaleFFTcr128out.bin"),"rb")) == NULL) {
#endif 
     printf("Error: can not open (scale)FFT(cr)128out.bin\n");
     return 1;
   }
   if (fread(Expected_results, sizeof(CWord2x16), Nfft, Infile) != Nfft ) 
       printf("Error: Unable to read from (scaled)FFT(cr)128.bin\n");
   fclose(Infile);

   /* --------------------------------------------------------- */
   /*  compare results                                          */
   /* --------------------------------------------------------- */
    for (i=0;i<Nfft;i++) {
        if (Output[i] != Expected_results[i]) {
            printf("FAILED!!!\n");
            return 1;
        }

    }
    printf("PASSED!   Cycle-count: %d\n", Total_cycles);


   /* --------------------------------------------------------- */
   /* CALL 256-point scaled FFT                                 */
   /* --------------------------------------------------------- */
    Nfft = 256;
    printf("testing %d-point sFFT ....", Nfft);

    start_time = hexagon_sim_read_pcycles();
    sfft16x16(Input,Nfft, Wtwiddles, Output);
    Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;

   /* --------------------------------------------------------- */
   /* read expected results from file                           */
   /* --------------------------------------------------------- */
#ifdef TWOS_COMPLEMENT_ROUNDING
   if((Infile = fopen(FULL_FILE_NAME("scaleFFT256out.bin"),"rb")) == NULL) {
#else
   if((Infile = fopen(FULL_FILE_NAME("scaleFFTcr256out.bin"),"rb")) == NULL) {
#endif
     printf("Error: can not open (scale)FFT(cr)256out.bin\n");
     return 1;
   }
   if (fread(Expected_results, sizeof(CWord2x16), Nfft, Infile) != Nfft ) 
       printf("Error: Unable to read from (scaled)FFT(cr)256.bin\n");
   fclose(Infile);

   /* --------------------------------------------------------- */
   /*  compare results                                          */
   /* --------------------------------------------------------- */
    for (i=0;i<Nfft;i++) {
        if (Output[i] != Expected_results[i]) {
            printf("FAILED!!! \n");
            return 1;
        }
    }
    printf("PASSED!   Cycle-count: %d\n", Total_cycles);


   /* --------------------------------------------------------- */
   /*                     Test iFFT                             */
   /* --------------------------------------------------------- */

   /* --------------------------------------------------------- */
   /* read inputs from file                                     */
   /* --------------------------------------------------------- */
   if((Infile = fopen(FULL_FILE_NAME("FFTin.bin"),"rb")) == NULL) {
     printf("Error: can not open FFTin.bin\n");
     return 1;
   }
   if (fread(Input,  sizeof(CWord2x16), 256, Infile) != 256 ) 
       printf("Error, Unable to read from FFTin.bin\n");
   fclose(Infile);


   /* --------------------------------------------------------- */
   /* CALL 128-point iFFT                                       */
   /* --------------------------------------------------------- */
    Nfft = 128;
    printf("testing %d-point iFFT ....", Nfft);

    start_time = hexagon_sim_read_pcycles();
    ifft16x16(Input,Nfft, Wtwiddles, Output);
    Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;

   /* --------------------------------------------------------- */
   /* read expected results from file                           */
   /* --------------------------------------------------------- */
#ifdef TWOS_COMPLEMENT_ROUNDING
   if((Infile = fopen(FULL_FILE_NAME("iFFT128out.bin"),"rb")) == NULL) {
#else
   if((Infile = fopen(FULL_FILE_NAME("iFFTcr128out.bin"),"rb")) == NULL) {
#endif 

     printf("Error: can not open iFFT(cr)128out.bin\n");
     return 1;
   }
   if (fread(Expected_results, sizeof(CWord2x16), Nfft, Infile) != Nfft ) 
       printf("Error: Unable to read from iFFT(cr)128.bin\n");
   fclose(Infile);

   /* --------------------------------------------------------- */
   /*  compare results                                          */
   /* --------------------------------------------------------- */
    for (i=0;i<Nfft;i++) {
        if (Output[i] != Expected_results[i]) {
            printf("FAILED!!!\n");
            return 1;
        }

    }
    printf("PASSED!   Cycle-count: %d\n", Total_cycles);


   /* --------------------------------------------------------- */
   /* CALL 256-point iFFT                                       */
   /* --------------------------------------------------------- */
    Nfft = 256;
    printf("testing %d-point iFFT ....", Nfft);

    start_time = hexagon_sim_read_pcycles();
    ifft16x16(Input,Nfft, Wtwiddles, Output);
    Total_cycles = (int)(hexagon_sim_read_pcycles() - start_time) - overhead;

   /* --------------------------------------------------------- */
   /* read expected results from file                           */
   /* --------------------------------------------------------- */
#ifdef TWOS_COMPLEMENT_ROUNDING
   if((Infile = fopen(FULL_FILE_NAME("iFFT256out.bin"),"rb")) == NULL) {
#else
   if((Infile = fopen(FULL_FILE_NAME("iFFTcr256out.bin"),"rb")) == NULL) {
#endif

     printf("Error: can not open iFFT(cr)256out.bin\n");
     return 1;
   }
   if (fread(Expected_results, sizeof(CWord2x16), Nfft, Infile) != Nfft ) 
       printf("Error: Unable to read from iFFT(cr)256.bin\n");
   fclose(Infile);

   /* --------------------------------------------------------- */
   /*  compare results                                          */
   /* --------------------------------------------------------- */
    for (i=0;i<Nfft;i++) {
        if (Output[i] != Expected_results[i]) {
            printf("FAILED!!! \n");
            return 1;
        }
    }

    printf("PASSED!   Cycle-count: %d\n", Total_cycles);
    printf("*** PASS!\n");
    return 0;
}

#else

void PrecisionCheck( CWord2x16 *Input, CWord2x16 *FFTOut, 
                     int npoint, int fftmode, int scale  );
#define NTEST     1

int main()
{
    int i, j, Nfft;

    printf("Attn: These Tests are slow\n");

    srand48(8888888);

    for (j = 0; j < NTEST; j++) 
    {

        for (i=0; i<256; i++)
        {
           Input[i] = (CWord2x16)mrand48();
        }
        
       /* --------------------------------------------*/
       /* test precision of 128-point scaled FFT      */
       /* ------------------------------------------- */
        Nfft = 128;
        sfft16x16(Input, Nfft, Wtwiddles, Output);
        PrecisionCheck( Input, Output, Nfft, 1, 1);

       /* --------------------------------------------*/
       /* test precision of 256-point scaled FFT      */
       /* ------------------------------------------- */
        Nfft = 256;
        sfft16x16(Input, Nfft, Wtwiddles, Output);
        PrecisionCheck( Input, Output, Nfft, 1, 1);
    }

    return 0;
}
#endif
