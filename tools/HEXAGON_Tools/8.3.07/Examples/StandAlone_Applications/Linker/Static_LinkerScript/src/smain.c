#include <stdlib.h>
#include <stdio.h>

int function1(int arg);
int function2(int arg);
int function3(int arg);

static int foo(int (*myfcn_ptr)(), int arg)
{
	return myfcn_ptr(arg);
}

int main(int argc, char** argv)
{
	unsigned i;
	int(*fPtr[3])(int) = { function1, function2, function3};

	for(i=0; i<3; i++)
		printf("function%d return value = 0x%x\n", i, foo(fPtr[i], i+1));

	return 0;
}