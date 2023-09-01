##############################################################################
     Link Time Optimization (LTO) Example

The LLVM Link Time Optimizer provides complete transparency, while doing
intermodular optimization, in the compiler tool chain. Its main goal is to let
the developer take advantage of intermodular optimizations without making any
significant changes to the developer's makefiles or build system. This is
achieved through tight integration with the linker. In this model, the linker
treates LLVM bitcode files like native object files and allows mixing and
matching among them. The linker uses libLTO, a shared object, to handle LLVM
bitcode files. This tight integration between the linker and LLVM optimizer
helps to do optimizations that are not possible in other models. The linker
input allows the optimizer to avoid relying on conservative escape analysis.

This example demonstrates how to use LTO in 'full' mode to optimize out
functions which are not used.

To go through all the steps type: make
##############################################################################
