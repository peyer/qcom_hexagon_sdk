/* ===================================================================
     Author : David J. Hoyle 

     Source:  Matrix Computations by Golub,
              Numerical Recipes in C
  ===================================================================== */


/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008 
****************************************************************************/ 


/*! \file cholesky.h
    \brief Brief description of file
*/

/*! 
Performs cholesky matrix decomposition algorithm

\param X pointer to the matrix
\param n demension of the matrix

\details

\b Assembly \b Assumptions
 - \a None

\b Cycle-Count
 - 0.5*N^3 + 2.5*N^2 + 24*N + 36

\b Notes
 - Reference: Matrix Computations by Golub, and Numerical Recipes in C.

*/
void cholesky(Word32 * X, int n);



/*! 
Computes inverse square root

\param inputin input integer
\param mantPtr pointer to the mantissa of the output 
\param expPtr pointer to the exponent of the output

\details

\b Assembly \b Assumptions
 - \a None

\b Cycle-Count
 - 

\b Notes
 - Reference: Numerical Recipes in C.

*/
void invSqrt(int inputin, int *mantPtr, int *expPtr);                     

