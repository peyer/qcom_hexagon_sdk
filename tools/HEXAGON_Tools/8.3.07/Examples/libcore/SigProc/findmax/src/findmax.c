/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>
#include "typedef.h"
#include "findmax.h"

/*
 *---------------------------------------------------*
 *  Function:   findmax                              *
 *---------------------------------------------------*
 *  Description: find maximum value and its index    * 
 *                                                   *
 *  Note:                                            *
 *  In some case where multiple maximums exist,      *
 *  one-by-one search has to be conducted in order   *
 *  to assure bit-exactness                          *
 *  Otherwise, take a vector version for speed       *
 *---------------------------------------------------*
*/

Word32 findmax( Word32 *xin, int length, int *mindex)
{
  int index = 0;
  Word32 max_val = xin[0];
  int i;

  for (i = 1; i< length; i++) {

#ifdef FIRST_PREFERED
     if( xin[i] > max_val ) {
#else
     if( xin[i] >= max_val ) {
#endif
        max_val = xin[i];
        index = i;
     }
  }

  *mindex  = index;
  return max_val;
}


/*
 *---------------------------------------------------*
 *  Function:   VecMax                               *
 *---------------------------------------------------*
 *  Description: find maximum value and its index    *
 *                                                   *
 *  Assumption: length must be a multipe of 4        *
 *                                                   *
 *  Note:                                            *
 *  This is a fast version if not really cares about *
 *  whicn index should be taken when multiple        *
 *  maximums exist                                   *
 *---------------------------------------------------*
*/
Word32 VecMax( Word32 *xin, int length, int *mindex)
{
   int max0, max1, max2, max3; 
   int max02, max13, max_val;
   int i, gi02, gi13, gi; 
   int index = 0;
   
   max0 = 0x80000000;
   max1 = 0x80000000;
   max2 = 0x80000000;
   max3 = 0x80000000;

   for (i=0; i<length; i+=4)
   {
      max0 = ( xin[i+0] > max0 ) ? xin[i+0] : max0;
      max1 = ( xin[i+1] > max1 ) ? xin[i+1] : max1;
      max2 = ( xin[i+2] > max2 ) ? xin[i+2] : max2;
      max3 = ( xin[i+3] > max3 ) ? xin[i+3] : max3;
   }
  
   gi02  = ( max2 > max0 ) ? 2    : 0;
   max02 = ( max2 > max0 ) ? max2 : max0;
   gi13  = ( max3 > max1 ) ? 3    : 1;
   max13 = ( max3 > max1 ) ? max3 : max1;

   gi      = ( max13 > max02 ) ? gi13  : gi02;
   max_val = ( max13 > max02 ) ? max13 : max02;

   for (i=0; i<length; i+=4)
   {
      if ( xin[i+gi] == max_val )
      {
          index = i + gi;
          break;
      }
   }

   *mindex = index;
   return max_val;
}
