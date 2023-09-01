Hexagon LLVM Linker Plugin Example

The IslandSections example demonstrates the use of the SectionIteratorPlugin.

The IslandPlugin example detects and reports elements of any .island section 
that access memory outside of .island space.

Read through the commented IslandSections.cpp plugin source file to understand 
how this plugin works.

The three 'Error:' messages at the end report instances where code in an .island section 
made reference to memory space outside of .island space.  Code in .island space should
constrain its memory reads/writes to .island space.  Identifying where these overreaches
occur is a useful diagnostic tool.

 
To build and execute the example:  make


The Hexagon linker on Linux will display status messages shown below.
 (your file path will be different)
 
On Windows, the linker will reference
C:\Qualcomm\HEXAGON_Tools\<version>\Examples\StandAlone_Applications\Linker\Linker_Plugin_examples\IslandSections\IslandSections.dll

	Note: Using the absolute path /home/myaccount/Qualcomm/HEXAGON_Tools/<version>/Examples/StandAlone_Applications/Linker/Linker_Plugin_examples/IslandSections/libIslandSections.so for library IslandSections
	Note: Loaded Library /home/myaccount/Qualcomm/HEXAGON_Tools/<version>/Examples/StandAlone_Applications/Linker/Linker_Plugin_examples/IslandSections/libIslandSections.so requested
	Note: Registration function found RegisterAll in Library libIslandSections.so
	Note: Plugin handler getPlugin found in Library libIslandSections.so
	Note: Registering all plugin handlers for plugin types
	Note: Found plugin handler for plugin type FINDUSES in Library libIslandSections.so

	Error: The input section .text.f1 in output section .text.island is accessing .rodata.var2_ro in output section .rodata.  The section use is .rodata.var2_ro from file target/vars.o

	Error: The input section .text.f3 in output section .text.island is accessing .data.var4_d in output section .data.  The section use is .data.var4_d from file target/vars.o

	Error: The input section .text.f4 in output section .text.island is accessing .text.f2 in output section .text_sw.  The section use is .text.f2 from file target/text_sw.o

	Note: Plugin FINDUSES Destroyed

