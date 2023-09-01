#include "hfiles.h"

void Hexagon_function(pSIMPLE_STRUCT pStruct_A, pint32 p_i)
{
	// handled by the Hexagon simulation environment
	printf("\nHexagon function\ts.a = %d;\ts.b = %d;\ti = %d\n", pStruct_A->a, pStruct_A->b, *p_i);
	return;
}

void Cosim_based_func(pSIMPLE_STRUCT pStruct_A, pint32 p_i)
{
	// handled by the semi-host cosim
}

int main()
{
	// Define local struct 
	SIMPLE_STRUCT struct_ONE;
	struct_ONE.a = 5;
	struct_ONE.b = 10;
	pSIMPLE_STRUCT p_struct_ONE;
	p_struct_ONE = &struct_ONE;

	// Define local int
	int32 i = -15;
	pint32 p_i = &i;

	printf("\nStarting main\n");

	// Call Hexagon function
	Hexagon_function(p_struct_ONE, p_i);

	// Call Cosim function
	Cosim_based_func(p_struct_ONE, p_i);

	// Call Hexagon function
	Hexagon_function(p_struct_ONE, p_i);

	return 0;
}
