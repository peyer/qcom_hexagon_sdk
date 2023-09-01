This example tests the Hexagon optimized CRC routines 
found in "crc.h".  It runs against a sample set of 
data and compares the output against precomputed values.

To run c sim:

make clean
make SRC=C sim

To run asm:

make clean
make sim

To run with V4 new funcions:

make clean
make ARCH=V4 sim

To run with V2/V3 funcions:

make clean
make sim

can mix with C for example

make clean
make SRC=C ARCH=V4 sim

The simulation will print "*** PASS" if the test passed.


