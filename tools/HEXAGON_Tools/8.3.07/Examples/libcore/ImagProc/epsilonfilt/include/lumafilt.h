/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:25:59 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:59 CST 2008 
****************************************************************************/ 


/*! \file lumafilt.h
    \brief Brief description of file
*/

typedef struct {
   Word16   hp_coeff[10];          // 8-bit 
   Word16   hp_epsilon;            // 8-bit 
   Word16   hp_shift;             
   Word16   lp_coeff[10];          // 8-bit 
   Word16   lp_epsilon;            // 8-bit 
   Word16   lp_shift;         
   Word16   identity_coef[10]; 
   Word16   mask_sel;                        
} LumaFiltParams  __attribute__((aligned(8)));


/*! 
Performs epsilon filtering for luma component

\param ch pointer to input buffer
\param width number of pixels per line
\param height number of lines
\param param_in pointer to structure \a LumaFiltParams
\param out pointer to output buffer

\details

\b Assembly \b Assumptions
 - None

\b Cycle-Count
 - (12*(width-1)+3)*(height-2) + 14

\b Notes
 - None.

*/

void luma_filt( UWord8 *ch, int width, int height,  
                LumaFiltParams *param_in, UWord8 *out);
