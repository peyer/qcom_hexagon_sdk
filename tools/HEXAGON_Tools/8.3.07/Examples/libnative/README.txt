#*****************************************************************************#
#
#	Libnative Example Readme File
#
#*****************************************************************************#
The intrinsics emulation library emulates Hexagon processor instruction
intrinsics on non-Hexagon processor target platforms. The library
(named libnative) allows developers to compile a C or C++ program that contains
intrinsics into a native Linux or Windows executable. This enables code
portability, easier debugging, and a more efficient development workflow for
Hexagon processor applications.

The library is included in the tools release in directory Tools/libnative/lib.
The windows libnative library are a .dll and .lib files whereas the linux
libnative library is a .a file.

To use the windows library you need to compile with the Visual Studio 64 bit
compiler.  Also the PATH environment variable needs to have the dll path.
This is initialized in the make.cmd script.

This example demonstrates how to build and execute two intrinsics (add/multiply).
The makefile will build the example in either windows or linux.  To build and
run the example type: make

If you would like to see the symbols of the libnative.a or the windows
libnative.lib statically-linked library listed into a text file
type: make list
