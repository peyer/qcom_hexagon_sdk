#include <stdio.h>

// force the alignment of this int
// this will also force the alignment of the .data section
__attribute__((aligned(32)))
int var1 = 32;

__attribute__((aligned(64)))
int var2 = 64;

// assign this variable to a different section other than .data
__attribute__((section(".xbss_progbits")))
int var3 = 128;

// force this function to use a custom section name, it is called by the main image
__attribute__((section(".custom_section1")))
int function1(int arg)
{
	printf("in function1 - received arg=0x%x, var1=0x%x, arg*var1=0x%x\n",arg, var1, (arg*var1));
	return arg*var1;
}

// force this function to use a custom section name and is also called by the main image
__attribute__((section(".custom_section2")))
int function2(int arg)
{
	printf("in function2 - received arg=0x%x var2=0x%x, arg*var2=0x%x\n",arg, var2, (arg*var2));
	return arg*var2;
}

// prevent custom section3 from being garbage collected by using
// the KEEP directive on this section in the linker script
__attribute__((section(".custom_section3")))
int function3(int arg)
{
	printf("in function3 - received arg=0x%x, var3=0x%x, arg*var3=0x%x\n",arg,var3, (arg*var3));
	return arg*var3;
}

// force this function to use a custom section name, but it will be garbage collected
// becuase it is not used by the image
__attribute__((section(".custom_section4")))
int function4(int arg)
{
	printf("in function4 - received arg=0x%x var2=0x%x, arg*var2=0x%x\n",arg, var2, (arg*var2));
	return arg*var2;
}