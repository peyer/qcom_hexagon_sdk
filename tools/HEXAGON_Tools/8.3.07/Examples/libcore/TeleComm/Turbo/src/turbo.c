#include <stdio.h>
#include "typedef.h"
#include "q6basic_op.h"
#include "turbo.h"

/*======================================================*/
/*  LogMAP Turbo decoder                                */
/*======================================================*/

//
// DEFINITION of MAX*
//
#define THRESHOLD         22

signed char TurboTable[THRESHOLD+1] = 
{ 6, 5, 5, 4, 4, 3, 3, 3, 3, 2,
  2, 2, 2, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 0
};


#ifndef HEXAGON_ASM
static Word16 max_star( Word16 x, Word16 y )
{  
   Word16 z, sxy;
   sxy = x -y;
   z = abs_s(sxy);

   if ( z > THRESHOLD )
       return (   sxy < 0 ? (y) : (x)              );
   else    
       return ( ( sxy < 0 ? (y) : (x) ) + TurboTable[z] );
}


//
// Define Functions
//

void turbo_dec_kernel( Word16 *xLin, Word16 *p, Word16 *alpha, int nFrameSize, int nTails, Word16 *Lout  )
{
  
   int j;
   Word16 tmp_a, tmp_b, tmp_c, tmp_d;
   Word16 beta[8], beta_curr[8]; 
   Word16 lambda[4];                         // LAMBDAS -- BRANCH METRICS 
   Word16 max_over_ones, max_over_zeros;     // USED TO COMPUTE EXTRINSIC DATA

   //
   // FORWARD RECURSION FOR ALPHAS
   //
   alpha[0] = 1024;
   alpha[1] = 0;
   alpha[2] = 0;
   alpha[3] = 0;
   alpha[4] = 0;
   alpha[5] = 0;
   alpha[6] = 0;
   alpha[7] = 0;

   for ( j=0; j<nFrameSize-1; j++ ) {
       //
       // COMPUTE BRANCH METRICS (I.E, GAMMAS)
       //
       lambda[3] = 0;
       lambda[2] = p[j];
       lambda[1] = xLin[j] ;
       lambda[0] = saturate( (int)xLin[j] + (int)p[j] );

       //
       // UPDATE ALPHAS
       //
       alpha[8*(j+1)+0] = max_star(alpha[8*j+0]+lambda[0],alpha[8*j+4]+lambda[3]);
       alpha[8*(j+1)+1] = max_star(alpha[8*j+0]+lambda[3],alpha[8*j+4]+lambda[0]);
       alpha[8*(j+1)+2] = max_star(alpha[8*j+1]+lambda[1],alpha[8*j+5]+lambda[2]);
       alpha[8*(j+1)+3] = max_star(alpha[8*j+1]+lambda[2],alpha[8*j+5]+lambda[1]);
       alpha[8*(j+1)+4] = max_star(alpha[8*j+2]+lambda[2],alpha[8*j+6]+lambda[1]);
       alpha[8*(j+1)+5] = max_star(alpha[8*j+2]+lambda[1],alpha[8*j+6]+lambda[2]);
       alpha[8*(j+1)+6] = max_star(alpha[8*j+3]+lambda[3],alpha[8*j+7]+lambda[0]);
       alpha[8*(j+1)+7] = max_star(alpha[8*j+3]+lambda[0],alpha[8*j+7]+lambda[3]);

   }

   //
   // BACKWARD RECURSION FOR TAIL BITS.
   // DON'T NEED TO COMPUTE EXTRINSIC INFORMATION FOR TAIL BITS.
   //
   beta[0] = 1024;
   beta[1] = 0;
   beta[2] = 0;
   beta[3] = 0;
   beta[4] = 0;
   beta[5] = 0;
   beta[6] = 0;
   beta[7] = 0;

   for ( j=nFrameSize+nTails-2; j>=nFrameSize; j-- ) {
       //
       // COMPUTE BRANCH METRICS (I.E, GAMMAS)
       //
       
       lambda[3] = 0;
       lambda[2] = p[j];
       lambda[1] = xLin[j];
       lambda[0] = saturate( (int)xLin[j] + (int)p[j] ) ;

       //
       // UPDATE BETAS
       //
       beta_curr[0] = max_star(beta[0]+lambda[0],beta[1]+lambda[3]);
       beta_curr[1] = max_star(beta[2]+lambda[1],beta[3]+lambda[2]);
       beta_curr[2] = max_star(beta[4]+lambda[2],beta[5]+lambda[1]);
       beta_curr[3] = max_star(beta[6]+lambda[3],beta[7]+lambda[0]);
       beta_curr[4] = max_star(beta[0]+lambda[3],beta[1]+lambda[0]);
       beta_curr[5] = max_star(beta[2]+lambda[2],beta[3]+lambda[1]);
       beta_curr[6] = max_star(beta[4]+lambda[1],beta[5]+lambda[2]);
       beta_curr[7] = max_star(beta[6]+lambda[0],beta[7]+lambda[3]);
       //
       //
       beta[0] = beta_curr[0];
       beta[1] = beta_curr[1];
       beta[2] = beta_curr[2];
       beta[3] = beta_curr[3];
       beta[4] = beta_curr[4];
       beta[5] = beta_curr[5];
       beta[6] = beta_curr[6];
       beta[7] = beta_curr[7];
   }

   //
   // BACKWARD RECURSION AND EXTRINSIC DATA COMPUTATION
   // FOR INFORMATION BITS
   //
   //
   for ( j=nFrameSize-1; j>=0; j-- ) {
       //
       // COMPUTE BRANCH METRICS (I.E, GAMMAS)
       //
       lambda[3] = 0;
       lambda[2] = p[j];
       lambda[1] = xLin[j];
       lambda[0] = saturate( (int)xLin[j] + (int)p[j] );

       //
       // COMPUTE EXTRINSIC INFORMATION
       //

        tmp_a = max_star(beta[0]+alpha[8*j+0],beta[1]+alpha[8*j+4]);
        tmp_b = max_star(beta[2]+alpha[8*j+1],beta[3]+alpha[8*j+5]);
        tmp_c = max_star(beta[4]+alpha[8*j+6],beta[5]+alpha[8*j+2]);
        tmp_d = max_star(beta[6]+alpha[8*j+7],beta[7]+alpha[8*j+3]);

        tmp_a = max_star(tmp_a,tmp_d);
        tmp_c = max_star(tmp_b,tmp_c);   

        max_over_ones = max_star(tmp_a+lambda[0],tmp_c+lambda[1]);
        
        tmp_a = max_star(beta[0]+alpha[8*j+4],beta[1]+alpha[8*j+0]);
        tmp_b = max_star(beta[2]+alpha[8*j+5],beta[3]+alpha[8*j+1]);
        tmp_c = max_star(beta[4]+alpha[8*j+2],beta[5]+alpha[8*j+6]);
        tmp_d = max_star(beta[6]+alpha[8*j+3],beta[7]+alpha[8*j+7]);

        tmp_a = max_star(tmp_a,tmp_d);
        tmp_c = max_star(tmp_b,tmp_c);
        
        max_over_zeros = max_star(tmp_a+lambda[3],tmp_c+lambda[2]);
   
       //
       // REMOVE A PRIORI INFORMATION.
       // LEAVE EXTRINSIC INFORMATION.
       //
       Lout[j] = max_over_ones - max_over_zeros-xLin[j];

       //
       // UPDATE BETAS
       //
       beta_curr[0] = max_star(beta[0]+lambda[0],beta[1]+lambda[3]);
       beta_curr[1] = max_star(beta[2]+lambda[1],beta[3]+lambda[2]);
       beta_curr[2] = max_star(beta[4]+lambda[2],beta[5]+lambda[1]);
       beta_curr[3] = max_star(beta[6]+lambda[3],beta[7]+lambda[0]);
       beta_curr[4] = max_star(beta[0]+lambda[3],beta[1]+lambda[0]);
       beta_curr[5] = max_star(beta[2]+lambda[2],beta[3]+lambda[1]);
       beta_curr[6] = max_star(beta[4]+lambda[1],beta[5]+lambda[2]);
       beta_curr[7] = max_star(beta[6]+lambda[0],beta[7]+lambda[3]);
       //
       //
       beta[0] = beta_curr[0];
       beta[1] = beta_curr[1];
       beta[2] = beta_curr[2];
       beta[3] = beta_curr[3];
       beta[4] = beta_curr[4];
       beta[5] = beta_curr[5];
       beta[6] = beta_curr[6];
       beta[7] = beta_curr[7];
   }
}



int turbo_dec( TCDEC *tc )
{
   int i, main_loop;
   Word16 *L_write;
   Word16 *xs    = tc->xs;  
   Word16 *tail1 = tc->tail1;
   Word16 *tail2 = tc->tail2;
   Word16 *xLin  = tc->xLin;            
   Word16 *L1    = tc->L1; 
   Word16 *L2    = tc->L2;     
   int *intrlve = tc->intrlve; 

   //
   // INITIALIZE LAMBDAS
   //
   for ( i = 0; i < tc->nFrameSize; i++ ) {
       L1[i] = 0;
       L2[i] = 0;
   }
        
   //
   // TURBO DECODE
   //
   for ( main_loop=0; main_loop<tc->num_iter; main_loop++ ) {

         // CALCULATE INPUTS
         for ( i=0; i<tc->nFrameSize; i++)
         {
            xLin[i] = saturate( (int)xs[i] + (int)L1[i] );
         }

         // APPEND TAILS
         for ( i=0; i<tc->nTails-1; i++ ) 
         {   xLin[tc->nFrameSize+i] = tail1[i];
         }

         L_write = L2;
         turbo_dec_kernel(xLin, tc->xp1, tc->alpha, tc->nFrameSize, tc->nTails, L_write );

         // CALCULATE AND INTERLEAVE INPUTS
         for ( i=0; i<tc->nFrameSize; i++)
         {
            int j = intrlve[i];
            xLin[i] = saturate( (int)xs[j] + (int)L2[j] );
         }

         // APPEND TAILS
         for ( i=0; i<tc->nTails-1; i++ ) 
         {   xLin[tc->nFrameSize+i] = tail2[i];
         }

         L_write = xLin;
         turbo_dec_kernel(xLin, tc->xp2, tc->alpha, tc->nFrameSize, tc->nTails, L_write );

         // DE-INTERLEAVE
         for ( i=0; i<tc->nFrameSize; i++)
         {
            int j = intrlve[i];
            L1[j] = L_write[i];
         }
   }

   //
   // DECODE THE BITS
   //
   for ( i=0; i<tc->nFrameSize; i++ ) {
         tc->dec_bits[i] = L1[i] + L2[i] + xs[i] > 0 ? 0 : 1;
   }

   return 0;
}


#else
// Declare functions in assembly
//
void turbo_dec_alpha         ( Word16 *xLin, 
                               Word16 *p, 
                               Word16 *alpha, 
                               int nFrameSize );

void turbo_dec_beta_tail     ( Word16 *xLin, 
                               Word16 *p, 
                               Word16 *beta, 
                               int nTails, 
                               int nFrameSize );

void turbo_dec_beta_extrinsic( Word16 *xLin, 
                               Word16 *p, 
                               Word16 *alpha,
                               Word16 *Lout, 
                               Word16 *beta, 
                               int nFrameSize );

void cal_input1              ( Word16 *xs, 
                               Word16 *L, 
                               int nFrameSize, 
                               Word16 *xLin   );

void cal_input2              ( Word16 *xs, 
                               Word16 *L, 
                               int *intrlve, 
                               int nFrameSize, 
                               Word16 *xLin   );

void deinterleave            ( Word16 *Lin, 
                               int *intrlve, 
                               int nFrameSize, 
                               Word16 *Lout   );


//
// Define Functions
//
void turbo_dec_kernel( Word16 *xLin, Word16 *p, Word16 *alpha, int nFrameSize, int nTails, Word16 *Lout )
{
   Word16 beta[8];
   turbo_dec_alpha         ( xLin, p, alpha, nFrameSize             );
   turbo_dec_beta_tail     ( xLin, p, beta, nTails, nFrameSize      );
   turbo_dec_beta_extrinsic( xLin, p, alpha, Lout, beta, nFrameSize );
}


int turbo_dec( TCDEC *tc )
{
   int i, main_loop;
   Word16 *L_write;
   Word16 *xs    = tc->xs;  
   Word16 *tail1 = tc->tail1;
   Word16 *tail2 = tc->tail2;
   Word16 *xLin  = tc->xLin;            
   Word16 *L1    = tc->L1; 
   Word16 *L2    = tc->L2;     
   int *intrlve = tc->intrlve; 

   //
   // INITIALIZE LAMBDAS
   //
   for ( i = 0; i < tc->nFrameSize; i++ ) {
       L1[i] = 0;
       L2[i] = 0;
   }
        
   //
   // TURBO DECODE
   //
   for ( main_loop=0; main_loop<tc->num_iter; main_loop++ ) {

         // CALCULATE INPUTS
         cal_input1(xs, L1, tc->nFrameSize, xLin);

         // APPEND TAILS
         for ( i=0; i<tc->nTails-1; i++ ) 
         {   xLin[tc->nFrameSize+i] = tail1[i];
         }

         L_write = L2;
         turbo_dec_kernel(xLin, tc->xp1, tc->alpha, tc->nFrameSize, tc->nTails, L_write );

         cal_input2(xs, L2, intrlve, tc->nFrameSize, xLin);

         // APPEND TAILS
         for ( i=0; i<tc->nTails-1; i++ ) 
         {   xLin[tc->nFrameSize+i] = tail2[i];
         }

         L_write = xLin;
         turbo_dec_kernel(xLin, tc->xp2, tc->alpha, tc->nFrameSize, tc->nTails, L_write );

         deinterleave(L_write, intrlve, tc->nFrameSize, L1);
   }

   //
   // DECODE THE BITS
   //
   for ( i=0; i<tc->nFrameSize; i++ ) {
         tc->dec_bits[i] = L1[i] + L2[i] + xs[i] > 0 ? 0 : 1;
   }

   return 0;
}
#endif

