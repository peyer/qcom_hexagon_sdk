Hexagon LLVM Linker Plugin Examples

This set of examples demonstrates the use of Hexagon linker plugins.

Linker plugins are supported only by Hexagon 8.2.x and newer tools.

Three examples are included:
	CopyBlock		- an output section (Block) is copied
        			  to a different output section
	IslandSections		- detect and report improper code design
	ReplaceBlock		- an output section is replaced by a
        			  different block of smaller or equal size

The objective of these examples is not to teach plugin details,
but to demonstrate how to use linker plugins to generate desired
outputs.  Each example highlights one of the linker plugin
features:
	- Section Iterator
	- Output Section Iterator
	- Control File Size
	- Control Memory Size
        - Working with Chunks.

A linker plugin is written as one or more C/C++ source files.  These
source files are compiled and linked into a dynamic shared library.
The shared library interacts with the Hexagon linker as the Hexagon code is built.


Linker plugin shared libraries are built to run on the same x86_64 host that runs the
Hexagon tools (Windows or Linux).

Requirements:
1. The Hexagon tools release directory must be available in your system path:
	export PATH=<Hexagon_Install>/Tools/bin:$PATH
2. Build Tools for linker plugins
	on Linux:
	For Hexagon 8.2.x tools, use clang version 3.8.0 for x86_64 Target to build the plugin shared object
	For Hexagon 8.3.x tools, use clang version 5.0.1 for x86_64 Target to build the plugin shared object

	on Windows:
	MSVC 2014 must be available in your system path to build on Windows.
	Please note that in order to preserve the integrity of the PATH environment
	variable, it is saved prior to calling the vcvarsall.bat program, then it
	is restored to its original value.  This allows you to execute the make.cmd
	file multiple times without growing the PATH enviroment variable.

Each linker plugin example is organized with the linker plugin README, source, and make
files immediately under the example folder name, and with the Hexagon source and linker
script files in each 'target' sub-folder.

Read the README file in each example's folder to learn more about each example.

When building the Hexagon source code (i.e., when the linker plugin runs), the plugin
must be able to read the Hexagon .o files.  We recommend building the Hexagon files in two stages:
	1) build the .o files
	2) link the .o files into the final binary

