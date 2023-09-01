##############################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED
# All Rights Reserved
# Modified by QUALCOMM INCORPORATED on $Date$
##############################################################################
    CLADE Example

This example demonstrates how to build and run a program which utilizes cache-
line level compression and decompression (CLADE).  CLADE is a memory compression
technology which is used to decompress memory as it is brought into the L2 cache.

This example uses the mandelbrot algorithm to demonstrate how the program is
defined to reside in CLADE compression area using a linker script.  The
initialization code of the CLADE registers for this standalone application
resides in the crt0_standalone.o start up code.

The v62d_1536 or v66d_1536 architectures support CLADE and may be used to simulate
the example.

There is a make.cmd for windows and a makefile for linux.  Type make to build
and simulate the example.  To step through the code using hexagon-lldb type
make lldb.
