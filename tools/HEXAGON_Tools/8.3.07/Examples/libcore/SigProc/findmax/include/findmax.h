/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:00 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:00 CST 2008 
****************************************************************************/ 


/*! \file findmax.h
    \brief Brief description of file
*/

/*! 
Search maximum value and its index in an array

\param xin pointer to data array
\param length length of the data array to be searched
\param mindex pointer to the index value returned by function

\details

\return
maximum value of the array

\b Assembly \b Assumptions
 - None

\b Cycle-Count
 - length + 3

\b Notes
 - FIRST_PREFERED defines the preference of the search for the case with multiple maximum.

*/

Word32 findmax(Word32 *xin, int length, int *mindex);



/*! 
Search maximum value and its index in an array

\param xin pointer to data array
\param length length of the data array to be searched
\param mindex pointer to the index value returned by function

\details

\return
maximum value of the array

\b Assembly \b Assumptions
 - None

\b Cycle-Count
 - length/2 + 10

\b Notes
 - This is a fast version of findmax(). BUT they are not equivalent for the case with multiple maximums.

*/
Word32 VecMax(Word32 *xin, int length, int *mindex);

