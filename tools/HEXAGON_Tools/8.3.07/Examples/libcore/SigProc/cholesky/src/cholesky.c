/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED
* All Rights Reserved
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008
****************************************************************************/

#include <stdio.h>
#include "typedef.h"
#include "q6basic_op.h"
#include "table.h"
#include "cholesky.h"

void invSqrt(int inputin, int *mantPtr, int *expPtr){                      
        const int SQRT_ONE_HALF = 0x5A82799A;  /* ~ sqrt(1/2) */                                  
        int   bitcount;                                                                           
        int   seed_x, seed_x_707, input, tseed_x; 
        long long int x2, ax3;   
        int   Odd, offset;                                                                             
        /* Number of redundant sign bits */       
        bitcount = norm_l(inputin);   
                                                                                                  
        /* Normalize the recip_sqrt value to the range 0.5-1.0 */                             
        input = inputin << bitcount;                                                              
        bitcount += 2;      

        /* extract bit 1 for even/odd test of exponent */    
        Odd = bitcount & 1;                  

        /* divide exponent by 2 */                           
        bitcount = bitcount >> 1;   

        *expPtr = 31-bitcount;                                                                       
        offset = (input >> 22) & 0x0FF;
                                                                                                  
        tseed_x= (unsigned int)(invSqrtTable[offset]);                                            

        seed_x = 0x40000000 + (tseed_x << 22); //0x100 < 7

        x2   = LL_imult32(seed_x, seed_x);  /* x[n]^2 */    
        ax3  = 2*LL_imult32(input, x2>>31);        
        ax3  = 0x6000000000000000LL - ax3;
        x2   = LL_imult32(seed_x, ax3>>32);    /* x[n+1]=2*x[n]*(0.75-a*x[n]^2) */
        seed_x = (Word32) (x2 >> 30);

        x2   = LL_imult32(seed_x, seed_x);  /* x[n]^2 */    
        ax3  = 2*LL_imult32(input, x2>>31);        
        ax3  = 0x6000000000000000LL - ax3;
        x2   = LL_imult32(seed_x, ax3>>32);    /* x[n+1]=2*x[n]*(0.75-a*x[n]^2) */
        seed_x = (Word32) (x2 >> 30);

        /* if exponent is Odd multiply by sqrt(2) */                                              
        seed_x_707 = (Word32) (LL_imult32(seed_x, SQRT_ONE_HALF) >> 30);                                  
        if (Odd) seed_x = seed_x_707;                                                       
        *mantPtr = seed_x; /* Return Mantisa */                                          
        return;                                                                                   
}                          

/*=========================================================================*/
/* cholesky - fixed point c code                                           */
/*=========================================================================*/

void cholesky(Word32 * ptrX, int n){
    int i, j, k;
    Word32 isq;
    Word32 ar, ai, br, bi;
    Word32 sumr, sumi;
    long long pr, pi;
    int exp;
    int * vptr, * hptr, *hptri, * sptr, *optr;

    for (j = 1; j < n; j++) {
        br = *ptrX; 
        invSqrt(br, &isq, &exp);                
        ptrX[0] =isq;  //only need recip for later inversion
        ptrX[1] =exp;                          

        hptri = hptr= ptrX + 2*n; 
        sptr =ptrX + 2; 
        for (k = 0; k < n-j; k++){                     
            br = hptri[0] ;
            bi = hptri[1] ;
            sumr = (Word32)(LL_imult32(isq, br)>>exp); 
            sumi = (Word32)(LL_imult32(isq, bi)>>exp);  hptri += 2*n;

            hptr[0] = (Word32) sumr; hptr[1] = (Word32) sumi; hptr += 2*n;
            sptr[0] = 0;           sptr[1] = 0;           sptr += 2;
        }                                                   
        hptr = ptrX + 2*n;  
        optr = hptr + 2; 
        sptr = optr;
        for(k = 0; k < n-j; k++) {                                       
            vptr = ptrX + 2*n;  
            ar = hptr[0]; ai = hptr[1]; hptr += 2*n;
            for(i = 0; i < k+1; i++) {                                 
               br = vptr[0]; bi = vptr[1] ; vptr += 2*n;  

               pr  = LL_imult32(ar, br);
               pr += LL_imult32(ai, bi);    
               pi  = LL_imult32(ar, bi);    
               pi -= LL_imult32(ai, br);
               pr = pr + pr;
               pi = pi + pi;

               sumr = sptr[0]; sumi = sptr[1];
               sumr = sumr - ((Word32)(pr>>32)) ; 
               sumi = sumi + ((Word32)(pi>>32)) ;
               *sptr++ = sumr ;
               *sptr++ = sumi ;
            }                       
            optr += 2*n;
            sptr = optr;
        }                          
        ptrX += 2*(n+1);
    }                             
    br = *ptrX;
    invSqrt(br, &isq, &exp);                
    ptrX[0] = isq;
    ptrX[1] = exp;  
}
/* ========================================================= */
