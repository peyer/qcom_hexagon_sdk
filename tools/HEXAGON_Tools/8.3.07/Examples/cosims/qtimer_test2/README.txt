                           README.TXT File for QTimer Example

This example demonstrates how the Hexagon core can simulate an interrupt controller
and dual timer using the l2vic and qtimer cosim modules shipped with the Hexagon
tools release.  This example demonstrates the following:

    1. How to initialize the l2vic and qtimer modules
	2. How to enable and service interrupts
	3. How to define the interrupt number, frequency and timer register base addresses
	4. How to specify the cosim module parameters when simulating the qtimer_test
	5. How to assign an interrupt number to a thread (t0 uses irq2, t1 uses irq3)
	6. How to use fastl2vic interrupts

Note: Steps 5 and 6 are only applicable for architectures v66 and higher.

To build and execute the program in standalone mode:  make
To bring up the program in trace32 debugger: make t32

L2VIC/Qtimer Overview

Reference documents:
	1. Qualcomm Timer (Qtimer) V1.0 Core (80-N7698-1H)
	2. Second-Level Vector Interrupt Controller (L2VIC) TLM-2.0.1 Model
	3. QDSP6v5 Subsystem Hardware Programming Guide (80-N1733-3H)

It should be noted that the cosim models are meant to help in simulating a
QDSP6 subsystem and are not part of the Hexagon core.
