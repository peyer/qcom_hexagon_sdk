#include<stdio.h>

void fun(void)
{
  printf("Inside function: %s\n", __func__);
  printf("File name:       %s\n", __FILE__);
  return ;
}
