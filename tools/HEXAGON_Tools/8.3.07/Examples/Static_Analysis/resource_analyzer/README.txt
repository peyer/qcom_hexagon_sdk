*******************************************************************************
   Resource Analyzer Example README file

   Please refer to the Hexagon Resource Analyzer User Guide 80-N2040-21 Rev. B
*******************************************************************************

Overview

The resource analyzer is used after a program has been built. When it is
started, the resource analyzer first prompts for the name of a program file or
folder, and then analyzes the specified program and displays its resource usage.

The analyzer can input programs in either of the following forms:
	A single binary ELF file
	Multiple files stored in a program’s source code folder

The analyzer operates as a stand-alone graphical application. It provides a
tab-panel user interface which you can use to display various types of program
resource data.

Using the Example

The example will do a static analysis of the mandelbrot.c program.  Chapter 3
of the User Guide describes the analysis methodology.  This example focuses on
the command line usage of the resource analyzer and not the graphical front end.

There are four files generated in the RA folder as described in A.5 of the
user guide.

To start the example analysis type make on the command line, then cd to the
RA folder to study the output files.
