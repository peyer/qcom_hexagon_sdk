/***************************************************************************
* Copyright (c) Date: Mon Nov 24 16:26:03 CST 2008 QUALCOMM INCORPORATED 
* All Rights Reserved 
* Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:26:03 CST 2008 
****************************************************************************/ 


#include <stdio.h>
#include "typedef.h"
#include "findmax.h"

int main()
{
  Word32 x[16] = {
  25308,-27775,-12267,-12510,10657,9189,-23853,30557,11637,-29336, 24551, 12457, 3, 2, 1, 0 };
  Word32 xmax;
  int index;

  // Test findmax()
  xmax = findmax(x, 10, &index);

  if ( (xmax !=30557) || (index !=7) ) {
     printf("*** FAILED!!! \n");
     return 1;
  }

  // Test VecMax()
  xmax = VecMax(x, 16, &index);

  if ( (xmax !=30557) || (index !=7) ) {
     printf("*** FAILED!!! \n");
     return 1;
  }

  printf("*** PASS! \n");
  return 0;
}


