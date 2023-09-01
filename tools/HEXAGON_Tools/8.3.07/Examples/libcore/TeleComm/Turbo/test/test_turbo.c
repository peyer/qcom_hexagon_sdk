#include <stdio.h>
#include <stdlib.h>
#include "data_macros.h"
#include "turbo.h"
#include "test_turbo.h"

#include "hexagon_sim_timer.h"

#ifndef DATA_DIR
#define DATA_DIR "./data"
#endif


int main( int argc, char *argv[] )
{
    int i, j;
    TCDEC tc;
    short int *refOut;
    FILE *fp;

    long long start_time, total_cycles;
        
    //=====================================
    // ALLOCATE MEMORY FOR DECODER
    //=====================================
    if((tc.xs = (short *) malloc((FRAME_SIZE+K-1)*sizeof(short))) == NULL)
    {
      printf( "Cannot allocate xs.\n" );
      exit( 1 );
    }
    if((tc.xLin = (short *) malloc( (FRAME_SIZE+K-1)*sizeof(short))) == NULL)
    {
      printf( "Cannot allocate xLin.\n" );
      exit( 1 );
    }
    if((tc.xp1 = (short *) malloc( (FRAME_SIZE+K-1)*sizeof(short))) == NULL)
    {
      printf( "Cannot allocate xp1.\n" );
      exit( 1 );
    }
    if((tc.xp2 = (short *) malloc( (FRAME_SIZE+K-1)*sizeof(short))) == NULL)
    {
      printf( "Cannot allocate xp2.\n" );
      exit( 1 );
    }
    if((tc.tail1 = (short *) malloc((K-1)*sizeof(short))) == NULL)
    {
      printf( "Cannot allocate tail1.\n" );
      exit( 1 );
    }
    if((tc.tail2 = (short *) malloc((K-1)*sizeof(short))) == NULL)
    {
      printf( "Cannot allocate tail2.\n" );
      exit( 1 );
    }
    if((tc.alpha = (short *) malloc((8*FRAME_SIZE)*sizeof(short))) == NULL)
    {
      printf( "Cannot allocate alpha.\n" );
      exit( 1 );
    }
    if((tc.L1 = (short *) malloc((FRAME_SIZE)*sizeof(short))) == NULL)
    {
      printf( "Cannot allocate L1.\n" );
      exit( 1 );
    }
    if((tc.L2 = (short *) malloc((FRAME_SIZE)*sizeof(short))) == NULL)
    {
      printf( "Cannot allocate L2.\n" );
      exit( 1 );
    }
    if((tc.intrlve = (int *) malloc((FRAME_SIZE)*sizeof(int))) == NULL)
    {
      printf( "Cannot allocate intrlve.\n" );
      exit( 1 );
    }
    if((tc.dec_bits = (short *) malloc(FRAME_SIZE*sizeof(short))) == NULL)
    {
      printf( "Cannot allocate dec_bits.\n" );
      exit( 1 );
    }


    //=====================================
    // READ IN THE INTERLEAVER FILE
    //=====================================
    if ((fp = fopen(FULL_FILE_NAME("umts640.txt"), "r" )) == NULL)
    {
      printf( "Could not open interleave file\n");
      exit( 1 );
    }

    for ( i = 0; i < FRAME_SIZE; i++ ) {
      fscanf( fp, "%d", &tc.intrlve[i] );
      tc.intrlve[i] -= 1;
    }
    fclose( fp );

    //=====================================
    // READ INPUT DATA FROM FILE
    //=====================================
    if ((fp = fopen(FULL_FILE_NAME("indata.bin"), "rb")) == NULL) {
       printf("Error: can not open indata.bin\n");
       exit( 1 );
    }

    if ( fread( tc.xs,  sizeof(short), (FRAME_SIZE+K-1), fp)!= (FRAME_SIZE+K-1) )
    {
       printf("Error: Unable to read xs\n");
       exit( 1 );
    }

    if ( fread( tc.xp1, sizeof(short), (FRAME_SIZE+K-1), fp)!= (FRAME_SIZE+K-1) )
    {
       printf("Error: Unable to read xp1\n");
       exit( 1 );
    }

    if ( fread( tc.xp2, sizeof(short), (FRAME_SIZE+K-1), fp)!= (FRAME_SIZE+K-1) )
    {
       printf("Error: Unable to read xp2\n");
       exit( 1 );
    }

    if ( fread( tc.tail1, sizeof(short), (K-1), fp)!= (K-1) )
    {
       printf("Error: Unable to read tail1\n");
       exit( 1 );
    }

    if ( fread( tc.tail2, sizeof(short), (K-1), fp)!= (K-1) )
    {
       printf("Error: Unable to read tail2\n");
       exit( 1 );
    }

    fclose(fp);

    //=====================================
    // SET DECODER PARAMETERS
    //=====================================
    tc.num_iter   = NUM_ITER; 
    tc.nFrameSize = FRAME_SIZE;
    tc.nTails     = K;

    //=====================================
    // Call Turbo Decoder
    //=====================================
    start_time = hexagon_sim_read_pcycles();
    turbo_dec(&tc);
    total_cycles = hexagon_sim_read_pcycles() - start_time;

    //=====================================
    // COMPARE OUTPUTS
    //=====================================
    refOut = tc.xLin;    // just to buffer reference outputs

    if ((fp = fopen(FULL_FILE_NAME("outdata.bin"), "rb")) == NULL) {
       printf("Error: can not open outdata.bin\n");
    }
    
    //=====================================
    // COMPARE ALPHA
    //=====================================
    for ( i = 0; i < 8; i++ ) {

      if ( fread( refOut, sizeof(short), FRAME_SIZE, fp ) != FRAME_SIZE ) {
         printf("Error: Unable to read alpha\n");
         exit( 1 );
      }

      for ( j = 0; j < FRAME_SIZE; j++ ) {
          if ( tc.alpha[i*FRAME_SIZE+j] != refOut[j] ) {
             printf("FAILED!!! alpha\n");
             exit( 1 );
          }
      }
    }
    //=====================================
    // COMPARE L1
    //=====================================
    if ( fread( refOut, sizeof(short), FRAME_SIZE, fp ) != FRAME_SIZE ) {
         printf("Error: Unable to L1\n");
         exit( 1 );
    }
        
    for ( i = 0; i < FRAME_SIZE; i++ ) {
       if ( tc.L1[i] != refOut[i] ) {
           printf("FAILED!!! L1\n");
           exit( 1 );
       }
    }
    //=====================================
    // COMPARE L2
    //=====================================
    if ( fread( refOut, sizeof(short), FRAME_SIZE, fp ) != FRAME_SIZE ) {
         printf("Error: Unable to read L2\n");
         exit( 1 );
    }
    
    for ( i = 0; i < FRAME_SIZE; i++ ) {
       if ( tc.L2[i] != refOut[i] ) {
           printf("FAILED!!! L2\n");
           exit( 1 );
       }
    }
    fclose(fp);

    printf("Cycle-count: %.1f /bit/iteration\n", (float)total_cycles/FRAME_SIZE/NUM_ITER);

    printf("*** PASS!\n");

    //=====================================
    // FREE MEMORY
    //=====================================
    free( tc.xs );
    free( tc.xp1 );
    free( tc.xp2 );
    free( tc.tail1 );
    free( tc.tail2 );
    free( tc.L1 );
    free( tc.L2 );
    free( tc.intrlve );
    free( tc.dec_bits );
    free( tc.alpha );
    return  0;
}
