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

// place this data in tcm using section attributes and the linker script
__attribute__ ((section(".tcm_data")))
int tcm_data_var = 3;

// place this function in tcm using section attributes and the linker script
__attribute__ ((section(".tcm_text")))
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

	// get the addres of the next available virtual page with HEXAGON_DEFAULT_PAGE_SIZE alignment
	unsigned int nextPage = (stackBase + HEXAGON_DEFAULT_PAGE_SIZE) & -HEXAGON_DEFAULT_PAGE_SIZE;

	// use add_translation_fixed to get a 1:1 mapping for tlb 1
	add_translation_fixed(1, (void *)nextPage, (void *)nextPage, 7, 7);

	// use add_translation_fixed to get a mapping to PA 0x00 uncached
	nextPage += HEXAGON_DEFAULT_PAGE_SIZE;
	add_translation_fixed(2, (void *)nextPage, (void *)0x00, 6, 7);

	// use add_translation_extended to get a 1:1 mapping of 16k
	nextPage += HEXAGON_DEFAULT_PAGE_SIZE;
	add_translation_extended(3, (void *)nextPage,  (unsigned long long)nextPage,  2, 0xf, 0x7, 0x7f, 0, 3);

	// use add_translation_extended to map the next page to a 36 bit PA mapping of 64k
	nextPage += HEXAGON_DEFAULT_PAGE_SIZE;
	add_translation_extended(4, (void *)nextPage,  0xf30000000,  4, 0xf, 0x7, 0x3f, 0, 3);

	printf("\nThe tlb mapping after adding tlb's 1 - 4:");
	print_tlbs();
	printf("\n");


	// add_translation only changes the mapping, the tlb won't be created
	// until it is accessed

	// get the address of the data_var variable
	unsigned int data_addr = (unsigned int)&data_var;

	// modify the default 1:1 tlb mapping to change the PA to 0x00
	nextPage += HEXAGON_DEFAULT_PAGE_SIZE;
	add_translation((void*)nextPage, (void*)0x00, 7);

	// setup a pointer that will use the next page VA to access the data_var
	unsigned int *probe = (unsigned int *)(nextPage + data_addr);

	// write the data_var using the nextPage tlb
	*probe = 0xbabebabe;

	printf("\nWrote to PA 0x%08x, using VA 0x%08x, data_var is now 0x%08x\n", (unsigned int)&data_var, (unsigned int)probe, data_var);

	// this will create tcm traffic
	printf("\nCalling the tcm function will cause the code to set up a tlb entry for it\n");
	printf("tcm_fuction returns = %d\n", tcm_function(argc));

	printf("\nThe tlb mapping after using add_translation and calling the tcm function");
	print_tlbs();
	printf("\n");


	return (0);
}

