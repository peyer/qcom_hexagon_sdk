#include <stdio.h>


__attribute__((section(".text.unused")))
int unused_function(int a)
{
	return a*3;
}


int main(int argc, char *argv[])
{
	puts("Hello Qualcomm\n");

    return 0;
}