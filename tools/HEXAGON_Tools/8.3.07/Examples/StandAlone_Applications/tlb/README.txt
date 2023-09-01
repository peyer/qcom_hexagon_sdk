                           README.TXT File for TLB Example

This example demonstrates how the Hexagon memory management unit is programmed when using
the standalone library.  Several things may be observed in this example:

    	1. The initialization of the MMU and its TLB entries are shown.
	2. The programming of new TLB entries are shown.
	3. Accessing the TLB entries using assembly language is shown.
	4. A TLB read/write exception occurrance when accessing an address not in the TLB is shown.

To build and execute the program:  make
To bring up the program in trace32 debugger: make t32


MMU Overview

The Hexagon processor includes a full-featured Memory Management Unit (MMU) with
address translation and protection capability. A flat virtual address space is translated to
physical addresses via a Translation Localized Buffer (TLB) that supports both instruction
and data accesses. The MMU is capable of checking all user and guest mode memory
accesses for proper access permissions. The TLB is software managed and thus can
support many different OS and multi-threading models.

The three threads in the Hexagon processor share a common physical address space. Each
thread contains a private 7-bit ID (the Address Space Identifier, or ASID) that is
prepended to a 32-bit virtual address1 to form a 39-bit tag-extended virtual address.
Through MMU programming, this virtual address can be mapped to any physical address.
The physical address space is a 64 gigabyte, 36-bit space that is largely defined at the
MSM level. The following figure illustrates the Hexagon Address Maps.

Address translation allows a virtual address to be mapped to any physical address. This is
achieved with a Translation Lookaside Buffer (TLB). When the MMU is enabled, each
address produced by a fetch, load, or store instruction is referred to as a virtual address.
This address is compared in parallel to all programmed entries in the TLB. A match
happens when the Virtual Page Number (VPN) of the fetch, load or store address matches
an entry in the TLB, and either the Global bit is set for that entry, or the ASID for that
entry matches the ASID of the current thread.

The TLB supports pages sizes of 4 KB, 16 KB, 64 KB, 256 KB, 1 MB, 4 MB, and 16 MB.
The virtual page number is bits [31:12] after masking by the appropriate page size. For
example, if the TLB entry indicates a 64 KB page, then the low 16 bits of the virtual
address are the offset within the page, and the upper 16 bits are used for comparison.
These upper 16 bits are compared in parallel against the upper 16 bits of the VPN entries
in the TLB.

Further discussion and description of the TLB table is in the Hexagon V60 Architecture
System-Level Specification Chapter 5 (80-V9418-16).  Also you can study the Hexagon
standalone library calls in the Hexagon Stand-alone Application User Guide (80-N2040-22)
which is in the documentation folder in the Hexagon Tools Release.
