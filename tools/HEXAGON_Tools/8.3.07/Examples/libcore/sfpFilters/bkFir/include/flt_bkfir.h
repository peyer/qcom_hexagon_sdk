/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008 
****************************************************************************/ 


/*  file - flt_bkfir.h
    Brief description of file
*/

/* 
Peforms floating-point FIR filtering on block of data

 - xin pointer to input buffer
 - coefs pointer to coefficents array
 - taps number of taps of FIR filter
 - length number of data to be filtered
 - yout pointer to output buffer


 Assembly  Assumptions
 -  taps is a multiple of 4 and no less than 4
 -  length is a multiple of 4 and no less than 4
 -  xin,  yout and  coefs arrays are aligned by 8-bytes

 - Cycle-Count gives the number of cycles to execute filter

 Notes
 - multi-sample implementation to reduce load bandwidth.

*/

void flt_bkfir(float *xin, float *coefs, int taps,  \
               int length, float *yout);
