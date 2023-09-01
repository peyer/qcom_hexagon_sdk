/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008 
****************************************************************************/ 


/*! \file rs.h
    \brief Brief description of file
*/

// Parameter for Galois field. 
#define GF_GEN_POLY 0x1d        // g(x) = x^8 + x^4 + x^3 + x^2 + 1
// Parameters for RS code
#define RS_K         188
#define RS_T         8
#define RS_2T        (2*RS_T)

/*! 
Calculates syndromes

\param received_symbol pointer to input buffer for received symbols
\param syndrome pointer to output buffer for syndromes

\details

\return
    return 0 if all syndromes are zero, else return 1

\b Assembly \b Assumptions
 - \a if T is not a multiple of 2, the size of output buffer must be extended by 1-byte. 

\b Cycle-Count
 - (N*6 - 4) * ceil(T/2) + 11, where N = K + 2*T

*/
int calculate_syndrome( UWord8 *rv_symbol, 
                        UWord8 *syndrome        );



/*! 
Computes coefficients of error polynomial by using berlekamp-massey algorithm

\param syndrome pointer to input buffer for syndromes
\param logLambda pointer to output buffer for logarithm of coefficients 

\details

\return
    the order of error polynomial

\b Assembly \b Assumptions
 - \a 16-bits are used for logLamda[i] due to 0-element.

\b Cycle-Count
 - T*(11*T/2 + 6*ceil(T/2) + 3*ceil(T/4)) + 67*ceil(T/2) + ceil(T/4) + 1 (worst case)

*/

int belerkamp_massey(   UWord8  *s, 
                        UWord16 *logLambda      );


/*! 
Searchs roots of the error polynomial by Chien search algorithm

\param logLambda pointer to input buffer for logarithm of coefficients 
\param roots pointer to output buffer for roots

\details

\return
    the order of error polynomial

\b Assembly \b Assumptions
 - \a None

\b Cycle-Count
 - 2*T + (3*Nc + 5)*ceil(Nr/2) + 7 (worst case); 
   where Nc is the number of non-zero coefficients, Nr is the maximum of log(root[i])

*/

int chien_search(       UWord16 *logLambda, 
                        UWord8  *roots          );


/*! 
Calcuates error magnitudes by forney's algorithm

\param logLambda pointer to input buffer for logarithm of coefficients 
\param roots pointer to input buffer for roots
\param nerros number of errors
\param error_values pointer to output buffer for error magnitudes

\details

\return
    ( return 1 if any denominator is zero, else return 0)

\b Assembly \b Assumptions
 - \a None

\b Cycle-Count
 - (3*ceil(T/2) + 11)*ceil(T/2) + (4*deg_omega + 4*ceil(nerrors/2) + 15)*ceil(nerrors/2) + 8

*/

void forney(            UWord8  *s, 
                        UWord16 *logLambda, 
                        UWord8  *roots, 
                        Word32  nerrors, 
                        UWord8  *error_values   );

