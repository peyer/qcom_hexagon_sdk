The L2VIC_TEST example shows three important capabilities:
  1. how to create a dll/shared object which uses libwrapper.lib
  2. how to build a cosim - example shows an l2vic emulator
  3. how to build these shared objects in windows and linux

Windows Environment Requirements:
This example uses Microsoft Visual Studio 15 to build the cosim.

Files:
    Makefile - builds and simulates your program
    Source/cosims_lnx.txt       - linux cosim file
    Source/cosims_win.txt       - windows cosim file
    Source/l2vic_registers.h    - l2vic register addresses used by mandelbrot.c
    Source/l2vic_test_cosim.cpp - L2VIC cosim emulator
    Source/l2vic_test_cosim.h   - l2vic cosim emulator resources
    Source/mandelbrot.c         - hexagon example program

Usage:
    make            - clean, build with JUST_WAIT option, simulate example
    make clean      - remove previous binary/statistics files
    make mandelbrot - clean and build cosim and mandelbrot without JUST_WAIT
    make sim        - simulate example

Notes:
    The linux and windows cosim files use relative addresses to access the
    l2vic cosim with the assumption that they reside in the Hexagon Tools Examples
    release folder.  If you copy this example to a different folder you will
    need to change the relative addresses.

    When you compile with -DJUST_WAIT option, the mandelbrot program will stay in a
    loop and wait for 3 interrupts and terminate.
