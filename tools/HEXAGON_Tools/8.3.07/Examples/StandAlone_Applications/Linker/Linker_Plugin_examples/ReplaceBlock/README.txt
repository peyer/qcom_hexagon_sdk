Hexagon LLVM Linker Plugin Example

The ReplaceBlock example demonstrates the use of the ControlFileSizePlugin.

An output section (Block) is replaced by a different block of smaller or equal size.
The Virtual Addresses are not changed, only the payload of the block changes.

An example of when this plugin would be used is when the user wants to 
replace an uncompressed block with a compressed block.

This plugin is built to run in one of two ways:
  - If file-compression is available, the contents of section .compress are
  compressed and saved back to section .compress
  - If no standard file-compression is available (on Windows, and on Linux systems without zlib installed),
  a short character string is put into section .compress, replacing its original contents.

The compress-vs-character-string decision is made by how HAVE_ZLIB is defined, near the top of ReplaceBlock.cpp.
	#define HAVE_ZLIB 0 --> No file compression used (use characteter string, instead)
	#define HAVE_ZLIB 1 --> File compression enabled

When this example is run on Linux it will use zlib.so on the uncompressed block 
(section .compress) to compress it and then copy it back to the compressed block 
in gzip format.
	If zlib.so is not installed on your machine, set HAVE_ZLIB to zero.
	With HAVE_ZLIB set to zero, the uncompressed string is not actually used, the copied
	block just contains a smaller string.


To build and execute the example:  make


The Hexagon linker on Linux (with zlib package installed) will display status messages shown below.
 (your file path will be different)

	Note: Using the absolute path /home/myaccount/Qualcomm/HEXAGON_Tools/<version>/Examples/StandAlone_Applications/Linker/Linker_Plugin_examples/ReplaceBlock/libReplaceBlock.so for library ReplaceBlock
	Note: Loaded Library /home/myaccount/Qualcomm/HEXAGON_Tools/<version>/Examples/StandAlone_Applications/Linker/Linker_Plugin_examples/ReplaceBlock/libReplaceBlock.so requested
	Note: Registration function found RegisterAll in Library libReplaceBlock.so
	Note: Plugin handler getPlugin found in Library libReplaceBlock.so
	Note: Registering all plugin handlers for plugin types
	Note: Found plugin handler for plugin type REPLACEBLOCK in Library libReplaceBlock.so
	Note: Creating MemoryBlock sized 1116 for output section .compress
	Note: Applying relocations for section .compress
	Note: Syncing relocations for section .compress
	Note: Adding memory blocks for section .compress

	Collecting Block .compress with a size of 1116 bytes.

	Note: Running handler for section .compress
	Note: Plugin REPLACEBLOCK requests memory 1116 bytes and returned 4096 bytes

	Replacing Block .compress with a size of 633 bytes.

	Note: Plugin returned 1 blocks when processing section .compress
	Note: Plugin REPLACEBLOCK Destroyed


[After the Hexagon linker is done, the makefile runs hexagon-readelf to show
the new size of the .compress section, and the addresses for both .compress and 
.marker sections.  The address for .marker, which immediately follows .compress,
shows that the linker does not collapse the mapping of sections that come
after .compress just because .compress has been reduced in size.]

	hexagon-readelf -S target/main.elf | grep -E "Address|compress|marker"
	  [Nr] Name              Type            Address  Off    Size   ES Flg Lk Inf Al
	  [ 3] .compress         PROGBITS        00004058 005058 000279 00   A  0   0  8
	  [ 4] .marker           PROGBITS        000044c0 0054c0 000014 00  AX  0   0 16

  
[Then the makefile runs hexagon-elfcopy to copy out just the .compress section to the
binary file compressed.gzip...]

	hexagon-elfcopy -O binary --only-section .compress target/main.elf compressed.gzip

[...then runs gzip to decompress the file compressed.gzip to show the contents are
the same as the original uncompressed .compress section:]

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

	Note: Using the absolute path C:\Temp\Linker_Plugin_examples\ReplaceBlock\ReplaceBlock.dll for library ReplaceBlock
	Note: Loaded Library C:\Temp\Linker_Plugin_examples\ReplaceBlock\ReplaceBlock.dll requested
	Note: Registration function found RegisterAll in Library ReplaceBlock.dll
	Note: Plugin handler getPlugin found in Library ReplaceBlock.dll
	Note: Registering all plugin handlers for plugin types
	Note: Found plugin handler for plugin type REPLACEBLOCK in Library ReplaceBlock.dll
	Note: Creating MemoryBlock sized 1116 for output section .compress
	Note: Applying relocations for section .compress
	Note: Syncing relocations for section .compress
	Note: Adding memory blocks for section .compress

	Collecting Block .compress with a size of 1116 bytes.

	Note: Running handler for section .compress

	Replacing Block .compress with a size of 40 bytes.

	Note: Plugin returned 1 blocks when processing section .compress
	Note: Plugin REPLACEBLOCK Destroyed


[After the Hexagon linker is done, the makefile runs hexagon-readelf to show
the new size of the .compress section, and the addresses for both .compress and 
.marker sections.  The address for .marker, which immediately follows .compress,
shows that the linker does not collapse the mapping of sections that come
after .compress just because .compress has been reduced in size.]

	hexagon-readelf -S .\target\main.elf | findstr "compress marker"

	  [ 3] .compress         PROGBITS        00004058 005058 000028 00   A  0   0  8
	  [ 4] .marker           PROGBITS        000044c0 0054c0 000014 00  AX  0   0 16

  
[Then the makefile runs hexagon-llvm-objdump to display the contents of the .compress section,
to show the short file string that substituted for the larger .compress section:]

	hexagon-llvm-objdump -s -j .compress .\target\main.elf

	.\target\main.elf:      file format ELF32-hexagon

	Contents of section .compress:
	 4058 4f70656e 696e6720 70617261 67726170  Opening paragrap
	 4068 68206f66 20746865 20626f6f 6b204d6f  h of the book Mo
	 4078 62792044 69636b2e                    by Dick.

	Done