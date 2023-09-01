Hexagon LLVM Linker Script Examples

*******************************************************************************
Introduction
*******************************************************************************
This set of examples demonstrates the use of hexagon linker control scripts
which is based on the GNU linker scripting language.  There are two types
of library concepts demonstrated: static and dynamic.
Additional examples demonstrate the use of program headers when building a
static library and doing partial linking.

The objective of these examples is not to teach scripting language
syntax but rather to demonstrate how to use linker script commands to generate
static and dynamic libraries.

The linker control script is written as a text file and produces an elf format
in two views of the elf file, ie, that which is used for linking and that which
is used for execution.  Sections are used for linking object files while
segments which contain one or more sections are used for loading the
executable.

You can view the section to segment mapping using the command:

  hexagon-readelf --segments bin/main.elf.

Each example demonstrates how the example program can be compiled and linked
with and without a linker script.  In each example do the following:

  make - compile and simulate without a linker script
  make script - compile and simulate with a linker script

It should be noted that in each case where the example is built with and without
a script, the resulting image is near the same, thus, showing that the LLVM
linker can create different kinds of libraries and an executable very quickly
either way.

*******************************************************************************
Static Link Example (.a)
*******************************************************************************
This example creates a static library using the hexagon-ar utility.  The
resultant libmylib1.a is then linked with smain.o to create the smain.elf.

This example demonstrates that when not using a linker script the
special sections defined in mylib1.c are not bundled as they are when linking
with the the slink linker script.  The importance of where you place the
__bss_start in the .bss section will affect program behavior.  When placed at
the beginning of the .bss section the results of the example program will be
incorrect.  Be sure to review the linker script to learn how to correct the
results.

The following is a view of the section to segment mapping of the static link
using a linker script.

>hexagon-readelf --segments bin/main.elf

Elf file type is EXEC (Executable file)
Entry point 0xdf0000
There are 3 program headers, starting at offset 52

Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x001000 0x00df0000 0x00df0000 0x03f54 0x03f54 RWE 0x1000
  LOAD           0x005000 0x00df4000 0x00df4000 0x085e4 0x085e4 R E 0x1000
  LOAD           0x00e000 0x00dfd000 0x00dfd000 0x01bb4 0x01bb4 RW  0x1000

 Section to Segment mapping:
  Segment Sections...
   00     .start
   01     .init .custom_sections .text .fini .rodata .eh_frame
   02     .ctors .dtors .data .bss

*******************************************************************************
Dynamic Link Example (.so)
*******************************************************************************
Dynamic linking is accomplished by placing the name of a sharable library in
the executable image. Actual linking with the library routines does not occur
until the image is run, when both the executable and the library are placed
in memory.  An advantage of dynamic linking is that multiple programs can share
a single copy of the library.

The dynamic link example creates a dynamic library with suffix .so.  The
example then demonstrates how to dynamically link library functions
at execution time using functions from libdl.  When building an executable file
that uses dynamic linking, the linker adds a program header element of type
INTERP to an executable file, telling the system to invoke the dynamic linker
as the program interpreter.

You will notice in this example that there is a .dynamic section, a global
offset table, procedure linkage table, a hash table and initialization and
termination functions for the shared objects.

The following is a view of the section to segment mapping of the dynamic link
using a linker script.

>hexagon-readelf --segments bin/main.elf

Elf file type is EXEC (Executable file)
Entry point 0xdf0000
There are 7 program headers, starting at offset 52

Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  PHDR           0x000000 0x00000000 0x00000000 0x000e0 0x000e0 R E 0x4
  INTERP         0x001000 0x00000000 0x00000000 0x00001 0x00001 R   0x1
      [Requesting program interpreter: ]
  LOAD           0x001000 0x00000000 0x00000000 0x000a9 0x000a9 R   0x1000
  LOAD           0x002000 0x00df0000 0x00df0000 0x03f54 0x03f54 RWE 0x1000
  LOAD           0x006000 0x00df4000 0x00df4000 0x095e4 0x095e4 R E 0x1000
  LOAD           0x010000 0x00dfe000 0x00dfe000 0x00780 0x01bb4 RW  0x1000
  DYNAMIC        0x010018 0x00dfe018 0x00dfe018 0x00050 0x00050 RW  0x4

 Section to Segment mapping:
  Segment Sections...
   00
   01     .interp
   02     .interp .hash .dynsym .dynstr
   03     .start
   04     .init .custom_sections .text .fini .rodata .eh_frame
   05     .ctors .dtors .dynamic .got.plt .data .bss
   06     .dynamic

*******************************************************************************
Program Headers Script Example
*******************************************************************************
The program header example is basically a static link script that uses program
header commands to direct where sections should reside in a segment.  Program
headers are read by the system loader and describe how the program should be
loaded into memory.

The linker will create reasonable program headers by default.  However, in some
cases, it is desirable to specify the program headers more precisely.  An
example of its use is in the design of the modem primary bootloader ROM code.

The following is a view of the section to segment mapping of the static link
with program headers using a linker script.

>hexagon-readelf --segments bin/main.elf

Elf file type is EXEC (Executable file)
Entry point 0xdf0000
There are 5 program headers, starting at offset 52

Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x001000 0x00df0000 0x00df0000 0x03f54 0x03f54 RWE 0x1000
  LOAD           0x005000 0x00df4000 0x00df4000 0x070d4 0x070d4 R E 0x1000
  LOAD           0x00d000 0x00dfc000 0x00dfc000 0x01018 0x01018 RW  0x1000
  LOAD           0x00e040 0x00dfd040 0x00dfd040 0x00700 0x00700 RW  0x1000
  LOAD           0x00f000 0x00dfe000 0x00dfe000 0x00bb4 0x00bb4 RW  0x1000

 Section to Segment mapping:
  Segment Sections...
   00     .start
   01     .init .custom_sections .text .fini
   02     .rodata .eh_frame .ctors .dtors
   03     .data
   04     .bss

*******************************************************************************
Partial Link Script Example (.o)
*******************************************************************************
The partial link example demonstrates how a group of object files can be merged
into one object file.  This is also called relocatable output where it can then
be input into the linker.

When linking C++ programs, this option will not resolve references to
constructors; to do that, use `-Ur'.

When an input file does not have the same format as the output file, partial
linking is only supported if that input file does not contain any relocations.
Different output formats can have further restrictions; for example some
a.out-based formats do not support partial linking with input files in other
formats at all.

This example will combine ./bin/mylib1.o ./bin/mylib2.o into .bin/mylib3.o.
It will then link ./bin/ptmain.o ./bin/mylib3.o and call a shared library
./liba.so
*******************************************************************************
References
*******************************************************************************
80-N2040-15 Hexagon Utilities
80-N2040-16 Hexagon Linker Control Scripts Application Note

