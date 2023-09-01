#include<stdio.h>

int foo_gc();

void fun_a(void){
  printf("%s\n", __func__);
  return ;
}

void fun_b(int b){
  printf("%s\n", __func__);
  return ;
}

void fun_c(int c, int d){
  printf("%s\n", __func__);
  printf("foo_gc from shlib %d\n", foo_gc());
  return ;
}
