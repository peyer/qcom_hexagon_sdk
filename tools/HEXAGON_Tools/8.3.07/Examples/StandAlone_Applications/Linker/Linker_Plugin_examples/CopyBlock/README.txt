Hexagon LLVM Linker Plugin Example

The CopyBlock example demonstrates the use of the ControlMemorySizePlugin.

An output section (Block) is copied to a different output section.
The block may be copied directly, or modified by the plugin.

An example of when this plugin would be used is when the user wants to 
compress a block in a NULL segment and copy it to a LOAD segment.
CLADE is an example of why this plugin might be used.  The original block 
is retained but it is not loaded because it is in a PT_NULL segment.

This plugin is built to run in one of two ways:
  - If file-compression is available, the contents of section .uncompresswith are
  compressed and saved to section .compressedwith
  - If no standard file-compression is available (on Windows, and on Linux systems
  without zlib installed), a short character string is put into section .compressedwith.

The compress-vs-character-string decision is made by how HAVE_ZLIB is defined, near the
top of CopyBlock.cpp.
	#define HAVE_ZLIB 0 --> No file compression used (use characteter string, instead)
	#define HAVE_ZLIB 1 --> File compression enabled

When this example is run on Linux it will use zlib.so on the uncompressed block 
(section .uncompresswith) to compress it and then copy it to the compressed block 
(.compressedwith) in gzip format.
	If zlib.so is not installed on your machine, set HAVE_ZLIB to zero.
	With HAVE_ZLIB set to zero, the uncompressed string is not actually used, the copied
	block just contains a smaller string.


To build and execute the example:  make


The Hexagon linker on Linux (with zlib package installed) will display status messages 
shown below.
 (your file path will be different)

	Linking the hexagon executable and invoking the plugin ...
	hexagon-clang -mv60 -G0 target/functions.o target/main.o -o target/main.elf -Wl,-T,target/linker.script,--trace=plugin,-Map,target/CopyBlock.map
	Note: Using the absolute path /home/myaccount/Qualcomm/HEXAGON_Tools/<version>/Examples/StandAlone_Applications/Linker/Linker_Plugin_examples/CopyBlock/libCopyBlock.so for library CopyBlock
	Note: Loaded Library /home/myaccount/Qualcomm/HEXAGON_Tools/<version>/Examples/StandAlone_Applications/Linker/Linker_Plugin_examples/CopyBlock/libCopyBlock.so requested
	Note: Registration function found RegisterAll in Library libCopyBlock.so
	Note: Plugin handler getPlugin found in Library libCopyBlock.so
	Note: Registering all plugin handlers for plugin types
	Note: Found plugin handler for plugin type COPYBLOCKS in Library libCopyBlock.so
	Note: Creating MemoryBlock sized 1116 for output section .uncompress
	Note: Applying relocations for section .uncompress
	Note: Syncing relocations for section .uncompress
	Note: Adding memory blocks for section .uncompress

	Collecting Block .uncompresswith a size of 1116 bytes.

	Note: Running handler for section .uncompress
	Note: Plugin COPYBLOCKS requests memory 1116 bytes and returned 4096 bytes

	Copying Block.compressedwith a size of 633 bytes.

	Note: Plugin returned 1 blocks when processing section .uncompress
	Note: Plugin COPYBLOCKS Destroyed


[After the Hexagon linker is done, the makefile runs hexagon-readelf to show
the relative sizes of the .uncompress and .compress sections.]

	hexagon-readelf -S target/main.elf | grep -E "Address|compress"
	  [Nr] Name              Type            Address  Off    Size   ES Flg Lk Inf Al
	  [ 7] .uncompress       PROGBITS        000070e8 0080e8 00045c 00   A  0   0  8
	  [12] .compress         PROGBITS        00009e00 009e00 000279 00  WA  0   0  1

  
[Then the makefile runs hexagon-elfcopy to copy out just the .compress section to the
binary file compressed.gzip...]

	hexagon-elfcopy -O binary --only-section .compress target/main.elf compressed.gzip

[...then runs gzip to decompress the file compressed.gzip to show the contents are
the same as the .uncompress section:]

	gzip -c -d compressed.gzip
	Call me Ishmael. Some years ago - never mind how long precisely - having little or 
	no money in my purse, and nothing particular to interest me on shore, I thought I 
	would sail about a little and see the watery part of the world. It is a way I have 
	of driving off the spleen, and regulating the circulation. Whenever I find myself 
	growing grim about the mouth; whenever it is a damp, drizzly November in my soul; 
	whenever I find myself involuntarily pausing before coffin warehouses, and bringing 
	up the rear of every funeral I meet; and especially whenever my hypos get such an 
	upper hand of me, that it requires a strong moral principle to prevent me from 
	deliberately stepping into the street, and methodically knocking people's hats off 
	- then, I account it high time to get to sea as soon as I can. This is my substitute 
	for pistol and ball. With a philosophical flourish Cato throws himself upon his sword; 
	I quietly take to the ship. There is nothing surprising in this. If they but knew it, 
	almost all men in their degree, some time or other, cherish very nearly the same 
	feelings towards the ocean with me.


[The Hexagon linker on Windows will display status messages shown below.
 (your file path will be different)]

	Note: Using the absolute path C:\Qualcomm\HEXAGON_Tools\<version>\Examples\StandAlone_Applications\Linker\Linker_Plugin_examples\CopyBlock\CopyBlock.dll for library CopyBlock
	Note: Loaded Library C:\Qualcomm\HEXAGON_Tools\<version>\Examples\StandAlone_Applications\Linker\Linker_Plugin_examples\CopyBlock\CopyBlock.dll requested
	Note: Registration function found RegisterAll in Library CopyBlock.dll
	Note: Plugin handler getPlugin found in Library CopyBlock.dll
	Note: Registering all plugin handlers for plugin types
	Note: Found plugin handler for plugin type COPYBLOCKS in Library CopyBlock.dll
	Note: Creating MemoryBlock sized 1116 for output section .uncompress
	Note: Applying relocations for section .uncompress
	Note: Syncing relocations for section .uncompress
	Note: Adding memory blocks for section .uncompress

	Collecting Block .uncompresswith a size of 1116 bytes.

	Note: Running handler for section .uncompress

	Copying Block.compressedwith a size of 40 bytes.

	Note: Plugin returned 1 blocks when processing section .uncompress
	Note: Plugin COPYBLOCKS Destroyed


[After the Hexagon linker is done, the makefile runs hexagon-readelf to show
the relative sizes of the .uncompress and .compress sections.]

	hexagon-readelf -S .\target\main.elf | findstr "compress"

	  [ 7] .uncompress       PROGBITS        000070e8 0080e8 00045c 00   A  0   0  8
	  [12] .compress         PROGBITS        00009e00 009e00 000028 00  WA  0   0  1

  
[Then the makefile runs hexagon-llvm-objdump to display the contents of the .compress section,
to show the short file string that substituted for the larger .uncompress section:]
  
	hexagon-llvm-objdump -s -j .compress .\target\main.elf

	.\target\main.elf:      file format ELF32-hexagon

	Contents of section .compress:
	 9e00 4f70656e 696e6720 70617261 67726170  Opening paragrap
	 9e10 68206f66 20746865 20626f6f 6b204d6f  h of the book Mo
	 9e20 62792044 69636b2e                    by Dick.

	Done