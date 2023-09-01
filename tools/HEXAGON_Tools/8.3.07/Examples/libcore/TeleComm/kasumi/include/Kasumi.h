/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:00 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:00 CST 2008 
****************************************************************************/ 


/*! \file Kasumi.h
    \brief Brief description of file
*/



/*! 
Implements key schedule algorithm used in Kasumi

\param key pointer to keys

\details

\b Assembly \b Assumptions
 - \a key array is aligned by 4-bytes

\b Cycle-Count
 - 103

\b Notes
 - None.

*/

void KeySchedule( UWord8 *key );



/*! 
Implements KASUMI algorithm, the core for the
3GPP Confidentiality and Integrity algorithms

\param data pointer to 64-bit data

\details

\b Assembly \b Assumptions
 - \a data is aligned by 8-bytes

\b Cycle-Count
 - 283

\b Notes
 - None.

*/

void Kasumi( UWord8 *data );



/*! 
Implements f8, the 3GPP Confidentiality algorithm.

\param key pointer to keys
\param count  count
\param bearer bearer
\param dir direction
\param data pointer to bit stream
\param length length of bit stream

\details

\b Assembly \b Assumptions
 - \a key array is aligned by 4-bytes

\b Cycle-Count
 -  289*ceil(N/64) + ceil(N/8) + 514

\b Notes
 - None.

*/

void f8( UWord8 *key, int count, int bearer, int dir, UWord8 *data, int length );

