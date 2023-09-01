##############################################################################
     Profile Guided Optimization Example

This example demonstrates how a program can be optimized using simulation
profiling information.  The standard build of the example is simulated to
gather the total pcycles without optimization as a reference.  The example
is then built with instrument generation so the simulator will build instrument
text to guide the optimization pass.  The hexagon-llvm-profdata will convert
the raw profile data to llvm format so the hexagon-clang++ compiler will be
able to use the profile information to build an optimized hexagon binary.
A final simulation is run to demonstrate the reduction in pcycles as a result
of profile guided optimization.

To go through all the steps type: make
##############################################################################