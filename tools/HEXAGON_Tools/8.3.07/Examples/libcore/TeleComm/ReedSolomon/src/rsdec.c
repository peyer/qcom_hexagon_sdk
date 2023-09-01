#include "typedef.h"
#include "rs.h"
#include "gf_tab.h"



/************************************************************************/
/*  Basic operations used for GF_MPY                                    */
/*                                                                      */
/************************************************************************/
static Word32 modwrap( Word32 x, Word32 n)
{
   if ( x >= n)
      x -=  n;
   return x;
}

static Word32 addNN( Word32 x, Word32 y)
{
   Word32 k;
   k = x + y;
   k = ( k >511) ? 511 : k;
   return k;
}


       
/************************************************************************/
/*  Calculate sydrome                                                   */
/*        r[0]*alph^(RS_K+RS_2T-1)+.....+r[RS_K+RS_2T-1]                */
/*                                                                      */
/************************************************************************/
int calculate_syndrome( UWord8 *rv_symbol, UWord8 *syndrome )
{   
    int i,j ;
    int flag = 0;
    UWord8 acc;
    Word32 ak;

    for(i=1; i<=RS_2T; i++)
    {  
        ak = i;
        acc = rv_symbol[RS_K + RS_2T-1];

        for(j=(RS_K+RS_2T-2); j>=0; j--)
        {
            acc ^= GF_EXPTAB[addNN(GF_LOGTAB[rv_symbol[j]], ak)];
            ak = modwrap(ak +i, 255);
        }

        syndrome[i-1] = acc;
        if (acc !=0) flag = 1;
    }
   
    return flag;
}



/************************************************************************/
/*  Belerkamp_massey algorithm                                          */
/*                                                                      */
/************************************************************************/
int belerkamp_massey(UWord8 *syndrome, UWord16 *logLambda) 
{                                                                    
    UWord16 logB[RS_T], logT[RS_T];
    UWord8 delta, lambda[RS_T];
    UWord16 logInvDelta;
    int i,j, L;

    for (i = 0; i < RS_T; i++)                                        
    {                                                                  
      lambda[i] = 0;                                                   
      logB[i] = 511;                                                        
    }                                                                  
                                                                       
    L = 0;                                                             
    logB[0] = 0;

    delta = syndrome[0];                                                    

    for (i = 1; i <= RS_2T; i++)                                        
    {                                                                  
      if ( (i>2*L) && (delta!=0) ) 
      { 
        L = i - L;                                            
        logInvDelta = 255 - GF_LOGTAB[delta];

        for (j=0; j<RS_T; j++) 
        {
            if (lambda[j] !=0)
            {
                logT[j] = modwrap(GF_LOGTAB[lambda[j]] + logInvDelta, 255);                  
            }
            else
            {
                 logT[j] = 511;
            }
        }
      }
      else
      {
        logInvDelta = 511;

        for (j=0; j<RS_T; j++) logT[j] = logB[j];
            
      }

      if (delta !=0 )
      {
         for (j = 0; j < RS_T; j++) 
         {
            lambda[j] ^= GF_EXPTAB[addNN(GF_LOGTAB[delta],logB[j])];     
         }
      }

      logB[0] = logInvDelta;
      for (j = 1; j < RS_T; j++) logB[j] = logT[j-1];                       

      delta = syndrome[i];                                                    

      for (j = 1; j <= L; j++)                                       
        delta ^= GF_EXPTAB[addNN(GF_LOGTAB[lambda[j-1]], GF_LOGTAB[syndrome[i-j]])];                       
    }                                                                  

    for (i = 0; i < RS_T; i++)
    {
        logLambda[i] = GF_LOGTAB[lambda[i]];
    }
    return L;
}                                                                    

       
/************************************************************************/
/*  Chien Search                                                        */
/*                                                                      */
/************************************************************************/
int chien_search(UWord16 *logLambda, UWord8 *roots)
{
    Word32 num_coefs, num_roots;
    UWord8 logGama[RS_T], jk[RS_T];
    UWord8 acc; 
    int i, k;

    num_coefs   = 0;

    for (i = 1; i <= RS_T; i++)
    {
      // find only non-zero coefficients
      if (logLambda[i-1] !=511) 
      {
         logGama[num_coefs] = logLambda[i-1];
         jk[num_coefs] = i;
         num_coefs++;
      }

    }

    num_roots = 0;

    for (i = 0; i < 255; i++)
    {
       acc = 1;

       for (k = 0; k< num_coefs; k++)
       {
          acc ^= GF_EXPTAB[logGama[k]];
          logGama[k] = modwrap(logGama[k] + jk[k], 255);
       }

       if( acc == 0 )
       {
          roots[num_roots++] = i;
          if (num_roots == jk[num_coefs-1])
             break;
       }

    }

    return num_roots;
}



/************************************************************************/
/*  Forney's Algorithm                                                  */
/*                                                                      */
/************************************************************************/
void forney( UWord8 *s, UWord16 *logLambda, UWord8 *roots, Word32 nerrors, UWord8 *error_values )
{
    UWord16 logOmega[RS_T];
    UWord16 xk, x, x2;
    UWord8 nominator, denom, acc;
    int deg_omega;
    int i, j;

    deg_omega = 0;

    /* Determine the error evaluator polynomial Omega(x) */
    for (i = 1; i <= RS_T; i++)
    {
        acc = s[i-1] ^ GF_EXPTAB[logLambda[i-1]];

        for (j = i-2; j >= 0; j--)
        {
            acc ^= GF_EXPTAB[addNN(GF_LOGTAB[s[j]],logLambda[i-j-2])];
        }

        logOmega[i-1] = GF_LOGTAB[acc];

        if (acc !=0) 
           deg_omega = i;
    }

    /* Compute the error values */
    for (i = 0; i < nerrors; i++)
    {

        x = roots[i];
        xk = roots[i];

        nominator = 1;

        for (j = 0; j < deg_omega; j++)
        {
            nominator ^= GF_EXPTAB[addNN(logOmega[j], xk)];
            xk = modwrap(xk + x, 255);
        }

        xk = x;
        x2  = modwrap(x + x, 255);

        denom = 0;

        for (j = 0; j < nerrors; j+=2)
        {
            denom ^= GF_EXPTAB[addNN(logLambda[j], xk)];
            xk = modwrap(xk + x2, 255);
        }

        error_values[i] = GF_EXPTAB[addNN(GF_LOGTAB[nominator], 255- GF_LOGTAB[denom])];
    }
}
