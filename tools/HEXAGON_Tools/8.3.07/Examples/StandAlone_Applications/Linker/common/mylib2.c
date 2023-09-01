#include <stdio.h>

// force the alignment of this int
// this will also force the alignment of the .data section
__attribute__((aligned(32)))
int var5 = 256;

__attribute__((aligned(64)))
int var6 = 512;

// assign this variable to a different section other than .data
__attribute__((section(".xbss_progbits")))
int var7 = 1024;

// force this function to use a custom section name, it is called by the main image
__attribute__((section(".custom_section5")))
int function5(int arg)
{
	printf("in function5 - received arg=0x%x, var5=0x%x, arg*var5=0x%x\n",arg, var5, (arg*var5));
	return arg*var5;
}

// force this function to use a custom section name and is also called by the main image
__attribute__((section(".custom_section6")))
int function6(int arg)
{
	printf("in function6 - received arg=0x%x var6=0x%x, arg*var6=0x%x\n",arg, var6, (arg*var6));
	return arg*var6;
}

// prevent custom section3 from being garbage collected by using
// the KEEP directive on this section in the linker script
__attribute__((section(".custom_section7")))
int function7(int arg)
{
	printf("in function7 - received arg=0x%x, var7=0x%x, arg*var7=0x%x\n",arg,var7, (arg*var7));
	return arg*var7;
}

// force this function to use a custom section name, but it will be garbage collected
// becuase it is not used by the image
__attribute__((section(".custom_section8")))
int function8(int arg)
{
	printf("in function8 - received arg=0x%x var5=0x%x, arg*var5=0x%x\n",arg, var5, (arg*var5));
	return arg*var5;
}