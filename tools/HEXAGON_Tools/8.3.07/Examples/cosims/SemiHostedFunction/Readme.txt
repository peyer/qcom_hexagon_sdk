Semi-Hosted Function cosimulation module
----------------------------------------

This cosim module demonstrates the use of a semi-hosted
function. It demonstrated how an algorithm (in this case squaring
integer values) can be performed in native host (x86) by
using a semi hosted callback function.

In this example, the Test program has the following dummy
function:
	void Cosim_based_func(pSIMPLE_STRUCT pStruct_A, pint32 p_i)

The cosim registers the following semi hosted call back function
with the simulator:
	void SemiHostCallback(void *HexWrapperPtr)

During simulation, when the program counter of execution reaches
the dummy function in the Test program, the simulator will
instead call the semi hosted function to run natively in the
host environment.

The following is the key sequence of calls that are made to
accomplish this:
1. At the start of simulation, the Simulator loads the cosim
2. Simulator calls RegisterCosim function within cosim
3. The AddBeforeSimulationStartsCallback API, registers a "Before
Simulation Starts" callback (BSSCallback) with the simulator. This
is done to ensure that the simulator is completely configured before
this callback is called.
4. when the BSSCallback is called, it looks up the function address
"funcAddr" of the dummy function Cosim_based_func from the simulator.
5. It then registers the SemiHostedCallback function to be called
when the PC reaches the "funcAddr"
6. Within the SemiHostedCallback, the api GetCurrentHWThreadNum provides
the active HW thread number. This is used in the api ReadThreadRegister
to obtain the R0 and R1 values. Based on the ABI, these register values
represent the function arguments to the dummy function.
7. Using the api ReadMemory the semi hosted callback function,
reads the values in simulator memory pointed to by the arguments
8. Then processing (squaring the integer values) is performed on
the local copies of these variables
9. Using the api WriteMemory the semi hosted callback function,
writes the result values back to simulator memory.
10. The result of the Test program shows that the values are
indeed changed appropriately in the memory of the simulator environment.


To build in Windows:
--------------------
The building on Windows requires MSVC 2015.

To build type: make. This will clean all binaries, build cosim,
build test program and then launch a simulation with the cosim attached.

To build in Linux:
------------------
Building on Linux requires a version of GCC compatible with
4.8.4 to be installed in your system path.

From this directory, run the following command in a shell:
> make

This will invoke the Makefile. The make will clean all binaries, build
the cosim, build test program and launch a simulation with the cosim attached.

Reference:
----------
Refer to the Hexagon Simulator System API User Guide (80-N2040-18) for
more details on creating cosimulation models.  You can find a copy at:
go/hexagonsupport
