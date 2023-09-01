/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008 
****************************************************************************/ 


/*! \file turbo.h
    \brief Brief description of file
*/

// Parameter Structure for Turbo Decoder
typedef struct {
    //PARAMETERS
    int num_iter;             // NUMBER OF ITERATIONS
    int nFrameSize;           // FRAME SIZE
    int nTails;               // NUMBER OF TAIL BITS
    // INPUTS
    short *xs;                // SYSTEMATIC DATA
    short *xp1;               // DATA FROM FIRST ENCODER
    short *xp2;               // DATA FROM SECOND ENCODER
    short *tail1;             // SYSTEMATIC TAIL DATA FROM FIRST ENCODER
    short *tail2;             // SYSTEMATIC TAIL DATA FROM SECOND ENCODER
    int *intrlve;             // INTERLEAVER MAPPING
    //OUTPUTS
    short *dec_bits;          // OUTPUTS
    // WORKING BUFFERS
    short *xLin;              // SYSTEMATIC DATA + EXTRINSIC
    short *L1;                // EXTRINSIC DATA PRODUCED BY FIRST DECODER
    short *L2;                // EXTRINSIC DATA PRODUCED BY SECOND DECODER
    short *alpha;             // ALPHAS
} TCDEC;


/*! 
LogMAP Turbo decoding 

\param tc pointer to the parameter structure

\details

\b Assembly \b Assumptions
 - \a working buffers and interleave table must be aligned by 8-bytes.

\b Cycle-Count
 - 100.7 cycles/bit/iteration

\b Notes
 - 22-entry LUT is used for max*()

*/
        
int turbo_dec( TCDEC *tc );

