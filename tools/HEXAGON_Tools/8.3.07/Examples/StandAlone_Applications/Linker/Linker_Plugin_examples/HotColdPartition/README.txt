Hexagon LLVM Linker Plugin Example

Linker scripts can invoke more than one linker plugin.  The HotColdPartition example demonstrates
the use of two linker plugins on the same image:
	ChunkIteratorPlugin		- move portions of .text_sw to .text_sw.cold
	SectionIteratorPlugin	- re-order the .compress section to put the .cold sections at the end

Read through the commented HotColdPartition.cpp plugin source file to understand how this plugin works.


To build and execute the example:  make


The Hexagon linker on Linux will display status messages shown below.
 (your file path will be different)

On Windows, the linker will reference
C:\Qualcomm\HEXAGON_Tools\<version>\Examples\StandAlone_Applications\Linker\Linker_Plugin_examples\HotColdPartition\HotColdPartition.dll

	Note: Using the absolute path /home/myaccount/Qualcomm/HEXAGON_Tools/<version>/Examples/StandAlone_Applications/Linker/Linker_Plugin_examples/HotColdPartition/libHotColdPartition.so for library HotColdPartition
	Note: Loaded Library /home/myaccount/Qualcomm/HEXAGON_Tools/<version>/Examples/StandAlone_Applications/Linker/Linker_Plugin_examples/HotColdPartition/libHotColdPartition.so requested
	Note: Registration function found RegisterAll in Library libHotColdPartition.so
	Note: Plugin handler getPlugin found in Library libHotColdPartition.so
	Note: Registering all plugin handlers for plugin types
	Note: Found plugin handler for plugin type CHANGESECTIONS in Library libHotColdPartition.so
	Note: Using the absolute path /home/myaccount/Qualcomm/HEXAGON_Tools/<version>/Examples/StandAlone_Applications/Linker/Linker_Plugin_examples/HotColdPartition/libHotColdPartition.so for library HotColdPartition
	Note: Loaded Library /home/myaccount/Qualcomm/HEXAGON_Tools/<version>/Examples/StandAlone_Applications/Linker/Linker_Plugin_examples/HotColdPartition/libHotColdPartition.so requested
	Note: Registration function found RegisterAll in Library libHotColdPartition.so
	Note: Plugin handler getPlugin found in Library libHotColdPartition.so
	Note: Registering all plugin handlers for plugin types
	Note: Found plugin handler for plugin type ORDERCHUNKS in Library libHotColdPartition.so

	The following output sections are being changed.

	Changing output section for .text.sw_func2.cold in file target/text_sw.o from: .text_sw to: .text_sw.cold
	Changing output section for .text.sw_func4.cold in file target/text_sw.o from: .text_sw to: .text_sw.cold

	Finished processing cold input sections.

	Note: Plugin CHANGESECTIONS Destroyed
	Note: Running handler for section .compress

        ****************************************************************************
        This plugin only operates on the section named: .compress
        ****************************************************************************
        This is the orignal order of this section:
        ****************************************************************************
        .text
        .text.comp_func1
        .text.comp_func2.cold
        .text.comp_func3
        .text.comp_func4.cold
        ****************************************************************************
        This is the changed order of this section:
        ****************************************************************************
        .text.comp_func3
        .text.comp_func1
        .text
        .text.comp_func2.cold
        .text.comp_func4.cold
        ****************************************************************************


	Note: Plugin ORDERCHUNKS Destroyed

[After the Hexagon linker is done, the makefile runs hexagon-readelf to show
.text_sw.cold was created, and that comp_func2_cold and comp_func4_cold were
located after comp_func3 and comp_func1:]

	The .text_sw section was partitioned into two sections
	hexagon-readelf -S target/main.elf | grep -E 'Address|text_sw'
	  [Nr] Name              Type            Address  Off    Size   ES Flg Lk Inf Al
	  [ 4] .text_sw          PROGBITS        00004120 005120 000054 00  AX  0   0 16
	  [ 5] .text_sw.cold     PROGBITS        00004180 005180 000054 00  AX  0   0 16

	The .compress section was re-ordered to put the cold sections at the end
	hexagon-readelf -s ./target/main.elf | grep comp_
	   336: 00004060    36 FUNC    GLOBAL DEFAULT    3 comp_func3
	   337: 00004090    36 FUNC    GLOBAL DEFAULT    3 comp_func1
	   338: 000040c0    36 FUNC    GLOBAL DEFAULT    3 comp_func2_cold
	   339: 000040f0    36 FUNC    GLOBAL DEFAULT    3 comp_func4_cold

	Done
