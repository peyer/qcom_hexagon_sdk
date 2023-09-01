/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 
#include <stdio.h>
#include "q6basic_op.h"
#include "fft_util.h"
#include "fft.h"


/***************************************************************/
/* Functio: 5 point FFT                                        */
/***************************************************************/
/* - scaling factor = 1/5                                      */
/***************************************************************/
void fft5(CWord2x16 *input, CWord2x16 *output) 
{
  CWord2x16 W1 = 0xCF4E0FD2; 		// W1*2/5
  CWord2x16 W2 = 0xE1E8D694;		// W2*2/5
  CWord2x16 C15 = 0x0199A;		// 1/5
  CWord2x16 C45 = 0x06666;		// 4/5

  CWord2x16 x0, x1, x2, x3, x4, XS0;
  CWord2x16 A, B, C, D, E, F, G, H; 
  CWord2x16 Y0, Y1, Y2, Y3, Y4;

  int tempr, tempi;

     x0 = input[0]; 
     x1 = input[1]; 
     x2 = input[2]; 
     x3 = input[3]; 
     x4 = input[4]; 

     A =  cavg(x1,x4);
     B =  cavg(x2,x3);
     C = cnavg(x1,x4);
     D = cnavg(x2,x3);

     tempr = L_add(L_mult(real(A), real(W1)), L_mult(real(B), real(W2)));
     tempi = L_add(L_mult(imag(A), real(W1)), L_mult(imag(B), real(W2)));
     E = complex(rnd(tempr), rnd(tempi)); 

     tempr = L_add(L_mult(real(A), real(W2)), L_mult(real(B), real(W1)));
     tempi = L_add(L_mult(imag(A), real(W2)), L_mult(imag(B), real(W1)));
     F = complex(rnd(tempr), rnd(tempi)); 

     tempr = L_add(L_mult(imag(C), -imag(W1)), L_mult(imag(D), -imag(W2)));
     tempi = L_add(L_mult(real(C),  imag(W1)), L_mult(real(D),  imag(W2)));
     G = complex(rnd(tempr), rnd(tempi)); 

     tempr = L_add(L_mult(imag(C), -imag(W2)), L_mult(imag(D),  imag(W1)));
     tempi = L_add(L_mult(real(C),  imag(W2)), L_mult(real(D), -imag(W1)));
     H = complex(rnd(tempr), rnd(tempi)); 

     Y0 =  cmult_r(cavg_nr(A,B), C45);
     Y1 = cadd(E,G);
     Y2 = cadd(F,H);
     Y4 = csub(E,G);
     Y3 = csub(F,H);

     XS0 = cmult_r(x0, C15);

     output[0] = cadd(XS0,Y0);
     output[1] = cadd(XS0,Y1);
     output[2] = cadd(XS0,Y2);
     output[3] = cadd(XS0,Y3);
     output[4] = cadd(XS0,Y4);
}



/***************************************************************/
/* Function: 320-point scaled complex FFT                      */
/***************************************************************/
/* - scaling factor = 1/320                                    */
/***************************************************************/
void fft320(CWord2x16 *input, CWord2x16 *output) 
{
  CWord2x16 x[5], y[5];
  CWord2x16 xb1[5][64], xb2[5][64];
  int i, j;

     // 5-point FFT
     for(i=0; i<64; i++) {
        for(j=0; j<5; j++) {
           x[j] = input[(5*i+64*j)%320];
        } 

        fft5(x, y);

        for(j=0; j<5; j++) {
           xb1[j][i] = y[j];
        }
     } 
  

     // 64-point FFT on rows
     for(j=0; j<5; j++) {
        sfft16x16(xb1[j], 64, FFTtwiddles, xb2[j]) ;
     }
   
     // re-order outputs
     for(j=0; j<5; j++) {
        for(i=0; i<64; i++) {
           output[(65*i + 256*j)%320] = xb2[j][i];
        }
     }
}




/***************************************************************/
/* DSP implementation of 9-point FFT                           */
/***************************************************************/
/* - two stages of radix-3 implementation                      */
/* - scaling factor 1/9                                        */
/***************************************************************/
void ButterflyRadix3A ( CWord2x16 *x ) 
{
  CWord2x16 W39x12 = 0xC893E000;

  CWord2x16 A, B; 
  CWord2x16 X0, XS0, Y0, Y1, Y2;
  int tempr, tempi;

     A  =  cavg(x[1],x[2]);
     B  = cnavg(x[1],x[2]);

     X0 =  cavg(x[0], 0);
     XS0 = cavg_nr(X0, 0);

     Y0  = cavg_nr(X0, A); 
     x[0] = Y0;

     tempr = L_add(L_mult(real(A), real(W39x12)), L_mult(imag(B), -imag(W39x12)));
     tempi = L_add(L_mult(imag(A), real(W39x12)), L_mult(real(B),  imag(W39x12)));
     Y1 = complex( rnd(tempr), rnd(tempi) ); 
     x[1] = cadd(XS0,Y1);

     tempr = L_add(L_mult(real(A), real(W39x12)), L_mult(imag(B),  imag(W39x12)));
     tempi = L_add(L_mult(imag(A), real(W39x12)), L_mult(real(B), -imag(W39x12)));
     Y2 = complex( rnd(tempr), rnd(tempi) ); 
     x[2] = cadd(XS0,Y2);
  
}

void ButterflyRadix3B ( CWord2x16 *x, CWord2x16 *w ) 
{
  CWord2x16 W39 = 0x9126C000;
  CWord2x16 C49 = 0x00038E4;

  CWord2x16 A, B; 
  CWord2x16 XS0, X1, X2, Y0, Y1, Y2;
  int tempr, tempi;

     X1 = cmult_r(x[1],w[0]);
     X2 = cmult_r(x[2],w[1]);

     A = cadd(X1,X2);
     B = csub(X1,X2);

     XS0 = cmult_r(x[0], C49);

     Y0 = A;

     tempr = L_add(L_mult(real(A), real(W39)), L_mult(imag(B), -imag(W39)));
     tempi = L_add(L_mult(imag(A), real(W39)), L_mult(real(B),  imag(W39)));
     Y1 = complex( rnd(tempr), rnd(tempi) ); 

     tempr = L_add(L_mult(real(A), real(W39)), L_mult(imag(B),  imag(W39)));
     tempi = L_add(L_mult(imag(A), real(W39)), L_mult(real(B), -imag(W39)));
     Y2 = complex( rnd(tempr), rnd(tempi) ); 

     x[0] = cadd(XS0,Y0);
     x[1] = cadd(XS0,Y1);
     x[2] = cadd(XS0,Y2);
  
}

void fft9(CWord2x16 *input, CWord2x16 *output) 
{
  CWord2x16 Wk9[3][2] = { { 0x000038E4, 0x000038E4 },    // W09*4/9, W09*4/9
                          { 0xDB6F2B94, 0xC7FA09E1 },	 // W19*4/9, W29*4/9
                          { 0xC7FA09E1, 0xEC8BCA8B }};   // W29*4/9, W49*4/9

  CWord2x16 x[3], g[9];

  int i;

     // stage 1
     for (i=0; i<3; i++) {
        x[0] = input[i+0]; 
        x[1] = input[i+3]; 
        x[2] = input[i+6]; 

        ButterflyRadix3A(x);

        g[i+0] = x[0];
        g[i+3] = x[1];
        g[i+6] = x[2];
     }


     // stage 2
     for (i=0; i<3; i++) {
        x[0] = g[3*i+0]; 
        x[1] = g[3*i+1]; 
        x[2] = g[3*i+2]; 

        ButterflyRadix3B(x, Wk9[i]);

        output[i+0] = x[0];
        output[i+3] = x[1];
        output[i+6] = x[2];
     }
}




/***************************************************************/
/* Function: 360-point scaled complex FFT                      */
/***************************************************************/
/* - scaling factor = 1/360                                    */
/***************************************************************/
void fft360(CWord2x16 *input, CWord2x16 *output) 
{
  CWord2x16 x[9], y[9];
  CWord2x16 xb1[5][9][8], xb2[5][9][8];
  int i, j, k;

     // 5-point FFT
     for(i=0; i<9; i++) {
        for(j=0; j<8; j++) {
           for(k=0; k<5; k++) {
              x[k] = input[(40*i+45*j+72*k)%360];
           }

           fft5(x, y);

           for(k=0; k<5; k++) {
              xb1[k][i][j] = y[k];
           } 
        }
     } 
  

     // 8-point FFTs 
     for(k=0; k<5; k++) {
        for(i=0; i<9; i++) {
           sfft16x16(xb1[k][i], 8, FFTtwiddles, xb2[k][i]) ;
        }
     }


     // 9-point FFTs 
     for(k=0; k<5; k++) {
        for(j=0; j<8; j++) {
           for(i=0; i<9; i++) {
              x[i] = xb2[k][i][j];
           }

           fft9(x,y);   

           for(i=0; i<9; i++) {
              xb2[k][i][j] = y[i];
           }
        }
     }

     // re-order outputs
     for(k=0; k<5; k++) {
        for(i=0; i<9; i++) {
           for(j=0; j<8; j++) {
              output[ (216*k + 280*i + 225*j)%360] = xb2[k][i][j];
           }
        }
     }
}


