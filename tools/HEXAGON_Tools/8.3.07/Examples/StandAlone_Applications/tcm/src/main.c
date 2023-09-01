
#include <stdio.h>
#include <stddef.h>
#include <hexagon_standalone.h>

// these are defined in crt0.S
extern int heapBase, heapLimit;
extern int stackBase, stackLimit;

// helper function
void print_tlbs(void);

// this variable will go to the .data section
unsigned int data_var = 0xdfdfdfdf;

int tcm_data_var = 3;

int tcm_function(int a)
{
	return a*tcm_data_var;
}

// main will execute out of tlb 0
int main(int argc, char **argv)
{
	// show the data_var, heap and stack addresses
	printf("\nThe heap address range is  0x%08x - 0x%08x\n",  heapBase, heapLimit);
	printf("The stack address range is 0x%08x - 0x%08x\n",  stackLimit, stackBase);
	printf("The data_var is at address 0x%08x and has a value of 0x%08x\n", (unsigned int)&data_var, data_var);

	// show the mapping needed just to execute main (includes .start, main, heap and stack)
	printf("\nThe default tlb mapping is:");
	print_tlbs();
	printf("\n");

	// this will create tcm traffic
	printf("tcm_fuction returns = %d\n", tcm_function(argc));

	return (0);
}

