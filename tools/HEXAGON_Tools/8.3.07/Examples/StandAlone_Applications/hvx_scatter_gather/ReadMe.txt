*******************************************************************************
*  Hexagon V65+ VTCM scatter/gather Example
*******************************************************************************
This example demonstrates how to use the Vector Tightly Coupled Memory (VTCM) with
the scatter/gather features of the HVX coprocessor.  Please note that this example
only works with Hexagon Tools Versions 8.1.x and higher.  Also, the architecture
must be v65 or higher. Note that this example defaults to 64B vector length.

Here's a summary of the V65+ HVX enhancements:

Dedicated HVX memory : VTCM

    Vector aligned loads and stores
    Vector unaligned loads and stores
    Coherent scalar access (load, store, dcfetch, dczero, syncht/barrier)

Scatter/gather using VTCM

    16-bit and 32-bit data elements
    16-bit and 32-bit addressing for 16-bit data elements
    Q predicated (per-byte) writes
    Scatters: 6 tags
    Scatter-accumulate: 3 tags
    Gather-copy: 6 tags
    Network store release: 3 tags

Vector TCM

The VTCM is a section of fast on-chip memory design to improve power consumption
and performance of HVX application. Currently it's size is defined as 256KB and
it consists of 128 (64 odd and 64 even) banks. HVX transactions to the VTCM do
not go through the L2 cache; therefore, usage of the VTCM can alleviate L2 cache
pressure. Due to the banking scheme, unaligned VMEM loads and stores are full
bandwidth.  Bank conflicts are not yet modeled in the simulator.

The scatter/gather operations will only operate on data present in the VTCM
(although this is not yet enforced in the simulator).

VMEM load and store operations take priority over scatter/gather operations when
accessing the VTCM.

Placing Data in VTCM

Data needs to be copied into the VTCM. The fastest way to copy data into VTCM
is to use HVX for the copy: L2Fetch into the cache and then move the data with
VMEM operations.

When VTCM is enabled, address checks will occur and VTCM bound transactions are
not sent to the L2.

Example

This example will demonstrate the key features of scatter/gather using VTCM.
Scalar vs Vector Scatter cycle counts will show the decrease in instruction cycles
when using the vector scatter/gather.

Type "make" to build, simulate and create a profile.json of the example
Type "make profile" to create an html profile summary of commits/stalls
Type "make lldb" to invoke the hexagon-lldb debugger to walk through the code.
Type "make HVX_DOUBLE=1" to test v65 with 128B width.
Type "make Q6VERSION=v66 HVX_DOUBLE=1 SIMF="-mv66a_1024 --hvx_length 128"
to test v66 with 128B width.  Alternatively, you can change the makefile to reflect
the changes that are on the command line and just type make.

Additional Reference:

You can also review the Hexagon SDK 3.3 document titled "VTCM Manager" which describes
SDK API calls to request/release/query the VTCM with examples.
