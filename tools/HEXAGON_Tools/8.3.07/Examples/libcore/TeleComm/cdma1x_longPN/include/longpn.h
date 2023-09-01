/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:00 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:00 CST 2008 
****************************************************************************/ 


/*! \file pnseq.h
    \brief Brief description of file
*/

/*! 
Generate long PN sequence (cdma1x)

\param state0 pointer to initial decimated state
\param state1 pointer to initial decimated state  
\param long_code pointer to output buffer
\param num_64chips number of 64-chips

\details

\b Assembly \b Assumptions
 - None

\b Cycle-Count
 - 34*num_64chips/64 + 5

\b Notes
 - The most current bit corresponds to MSB 

*/


void long_code_gen_f_asm( UWord64 *state0, UWord64 *state1, UWord64 *long_code, int num_64chips);

/*! 
Generate modified seeds for long PN sequence (cdma1x)

*/
void mask_gen_asm(UWord64 *fib_1, UWord64 *fib0, UWord64 state, UWord64 mask);
UWord64  calc_dec_asm(UWord64, UWord64*); 
void split_states_asm(UWord64 * states, int n);

