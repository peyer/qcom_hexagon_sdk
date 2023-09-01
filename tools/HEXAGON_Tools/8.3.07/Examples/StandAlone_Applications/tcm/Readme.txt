###############################################################################
##
## Hexagon V60 Tightly-Coupled Memory (TCM) Interface Example
##
###############################################################################
This example demonstrates how to create a program that will run from the Hexagon
V60 TCM.

TCM Overview

The V60 Hexagon processor contains internal L2 memory which can be used as L2
cache or tightly-coupled memory (TCM).  The L2 memory can be partitioned between
the L2 cache and TCM.  The L2 memory size depends on the processor version
you select.

From the software perspective TCM is simply a type of physical memory which can
be used like any other memory. From the hardware perspective, however, TCM has
the key property of offering low-latency access from the core.

TCM memory can be accessed from either the Hexagon processor core or from the
external slave port. Read/write accesses from the slave port originate from
external hardware blocks, such as the DMA.

Uncached accesses to TCM

The Hexagon processor has a full-featured MMU that translates virtual addresses
to physical addresses. Each page contains a set of cache attribute bits. To
access TCM using the uncached method, the following must be true:

 - The virtual page must be set to uncached type. The operating system is
   responsible for configuring the MMU.

 - The physical page falls within the programmed range. The memory range is chip
   specific, and differs across chips.

Cached accesses to TCM

For performance reasons, it is often desirable to cache TCM data in the level 1
data or instruction caches. This allows for dual-access zero-overhead load and
store operations.  To access TCM using the cached method, the following conditions
should be true:

  - The virtual page should be set to cacheable write-through or write-back. The
   Operating System is responsible for configuring the MMU cacheability types.

  - The physical page falls within the programmed range. The memory range is
   chip specific, and differs across chips.

Example Details

The files associated with the example are as follows:

   main.c - prints information about stack, heap, calls print_tlb
   tlb.c  - calls get_tlb_entry and prints the tlb information
   makefile - linux/cygwin makefile compiles,links,simulates example
   make.cmd - windows cmd script compiles,links,simulates example
   cmm/hexagon.cmm - Trace32 initialization script
   cmm/mywin.cmm - Trace32 debug windows setup
   cmm/win.cfg - Trace32 start up configuration

To build and simulate the program type: make
To debug the program using trace32 type: make t32

The heap resides after the .bss section (uninitialized data section) and the
stack resides below the heap.  Since the TCM is of limited size, the heap
and stack are defined to a fixed size to insure the compiler does not allocate
too large a heap and stack area.

A linker script is not needed in this example as the program requirements are
specified in the makefile as command line arguments passed from the compiler to
the linker.
