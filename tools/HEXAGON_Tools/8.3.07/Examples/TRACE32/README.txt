[*======================= COPYRIGHT NOTICE ======================*]
[* Copyright 2009-2012 QUALCOMM Incorporated.                    *]  
[* All rights reserved.                                          *] 
[*                                                               *]   
[* All data and information contained in or disclosed by this    *]
[* document is confidential and proprietary information of       *]
[* QUALCOMM Incorporated and all rights therein are expressly    *]
[* reserved. By accepting this material the recipient agrees     *]
[* that this material and the information contained therein is   *]
[* held in confidence and in trust and will not be used, copied, *]
[* reproduced in whole or in part, nor its contents revealed in  *]
[* any manner to others without the express written permission   *]
[* of QUALCOMM.                                                  *]
[*===============================================================*]

TRACE32 Multi-Core Debug (MCD) Extension

--------
Overview
--------

Lauterbach TRACE32(R) is an in-circuit debugging system for programs that
execute on the Hexagon(TM) processor.

TRACE32 is normally used to debug programs on a hardware debug platform.
However, when used with the MCD extension, TRACE32 can be used to debug
programs on the Hexagon processor instruction set simulator (hexagon-sim).

This document describes how to use TRACE32 with the MCD extension to
debug programs on the simulator. It is not intended as a user guide for
either TRACE32 or the simulator. It is assumed users are experienced with
both these tools. 

---
MCD
---

The Multi-Core Debug (MCD) extension is a software back-end which is
dynamically loaded by TRACE32.

It replaces the Generic Debug Instrument Interface (GDI) dynamic library
used in earlier versions of the Hexagon development tools. MCD supports
full multi-core debugging capabilities, and eliminates the restrictions
and issues that affected GDI. 

MCD is available for both the Windows and Linux versions of TRACE32.

NOTE - MCD eliminates the need for the Python scripting that was used
       with GDI.

------------
Installation
------------

MCD is included with the Hexagon tools beginning with tools release
version 1.1.

The MCD dynamic library file is located at <install root>\qc\lib\iss:
 - hexagon-mcd64.so  (Linux version)
 - hexagon-mcd64.dll (Windows version)

The library file is loaded by TRACE32. Its location is specified in the
TRACE32 config file, examples of which are located at
<install root>\examples\TRACE32:

 - linux.cfg   (Linux version) 
 - win.cfg     (Windows version)

IMPORTANT - the *.cfg file is read by Trace32. This file tells Trace32 to
            load the MCD library. The shipped examples contain a line
            that looks like:

PBI=MCD hexagon-mcd64.dll   <Windows>
PBI=MCD hexagon-mcd64.so    <Linux>

To use these as shipped, one will need to make sure your PATH (on
Windows) contains the PATH to the T32_MCD.dll file. For Linux,
LD_LIBRARY_PATH must be set to include the PATH to T32_MCD.so.
Alternatively, if the users wishes, these config files can be modified to
include the complete path to the T32_MCD.dll/so. An example (Windows)
might look like:

PBI=MCD C:\Qualcomm\HEXAGON_Tools\8.0\qc\lib\iss\hexagon-mcd64.dll

NOTE - Example TRACE32 startup scripts are located at
       <install root>\examples\TRACE32. 

NOTE - some users may wish to simply copy the MCD libraries and scripts
       to their Trace32 installation and use them from there.  However,
       be aware that the MCD libraries need to match the installed
       simulator, so these files will need to be updated manually
       whenever a new version of the Hexagon tools is installed.

-----------------
Quick Start Guide
-----------------

MCD can either launch a new instance of the Hexagon processor simulator
or connect to an existing simulator instance.

The MCD extension is configured in TRACE32 with the SYSTEM.MCDCONFIG
command, which is supported in all versions of the Hexagon TRACE32
debugger released since March 2009.

SYSTEM.MCDCONFIG must be performed *before* attempting the
SYSTEM.MODE.ATTACH command.

NOTE - If SYSTEM.MCDCONFIG is not defined in your copy of TRACE32,
       upgrade to a newer TRACE32 version. 

+++ Launching TRACE32 with MCD Using the Example TRACE32 Config File

Windows:
  t32mqdsp6.exe -c <install root>\examples\TRACE32\win.cfg -s <install root>\examples\TRACE32\win.cmm
   
Linux:
  t32mqdsp6 -c <install root>\examples\TRACE32\linux.cfg -s <install root>\examples\TRACE32\linux.cmm
   
+++ Configuring MCD with the SYSTEM.MCDCONFIG Command

SYSTEM.MCDCONFIG arch=<v2|v3|v3l|v3m|v3c|v4|v4m|v4c|v4l|v5|v5h|v5a|v5c|v60> [hostname=<hostname>] [port=<port number>] [debug=<mask>] [simargs=<valid hexagon-sim arguments>]

Command restrictions:

1. The "arch" parameter *must* be specified, and must match the processor
   architecture of the executable file being debugged.

2. The optional "hostname" parameter specifies the host name of the
   machine a simulator instance is already running on. Any valid hostname
   can be specified (including "localhost").

   "hostname" is used to connect TRACE32 to an existing (i.e., already
   running) instance of the Hexagon processor simulator. The instance may
   be running on the current machine or on another machine. 

   NOTE - If "hostname" is specified, "port" must also be specified.

3. The optional "port" parameter specifies the TCP/IP port that will be
   used to communicate with a simulator instance. The port number value
   can be any number that is acceptable to strtoul() and does not violate
   the standard rules for TCP/IP port numbers (in particular, the port
   number must be greater than 1024).

   "port" is used to connect TRACE32 to an existing instance of the
   simulator.  It must be specified if the simulator is launched
   independently of TRACE32.
 
   "port" is also used to launch a simulator instance on the same machine
   TRACE32 is running on, but using a port number other than the default
   value (808368). In this case the "port" parameter is specified, but
   "hostname" is not.

   Alternate port numbers are specified in cases where it is necessary to
   avoid port number collisions (for example, when running multiple
   MCD-extended debuggers on the same host machine, with each
   communicating through a different port).

4. The optional "debug" parameter selectively controls the printing of
   diagnostic information by the MCD extension itself. 

   "debug" specifies a mask value which determines the type of
   information printed. The mask value bits are defined as follows:
   
   /* DEBUG control, settable with SYS.MCDCONFIG debug=xx */
   #define REGISTER_OPS   1   // register read/write messages
   #define MEMORY_OPS     2   // memory read/write messages
   #define STATUS_REPLY   4   // status reply messages from simulator
   #define FLOW_MSGS      8   // program flow messages
   #define PACKET_FLOW 0x10   // low-level packet diagnostics
   #define ERROR_MSGS  0x20   // error states
   #define TXLIST_MSGS 0x40   // transmit list diags
   #define TRIGGER_OPS 0x80   // breakpoint diags
   #define ALL_MSGS    0xFF   // all the above

   The diagnostic information is written to "stderr", which can be
   captured by redirecting it to a file when launching TRACE32.
   For example (on Windows):
   
      t32mqdsp6.exe -c win.cfg -s win.cmm 2>my_debug_output.txt
   
   The resulting file is ASCII text. 

   "debug" is used to debug the MCD extension itself (which can be useful
   if it is the suspected source of a system bug). The generated
   diagnostic file may provide insight into the cause of a particular
   problem; however, it is primarily intended for use by the MCD
   maintainers.

   NOTE - The diagnostic file generated by "debug" can grow rapidly and
          become extremely large.
   
   NOTE - When using "debug" it is a good idea to minimize the number of
          open TRACE32 windows, as each open window generates constant
          data traffic to the simulator.

   NOTE - DO NOT rely on the format or content of what is in this debug
          log -- it will change !!
   
5. The optional "simargs" parameter specifies simulator command arguments
   which will be passed to the simulator instance launched by MCD.  If
   used, it must appear as the last parameter in the SYSTEM.MCDCONFIG
   command.

   The specified simulator command arguments are not checked by TRACE32.
   The user is responsible for verifying that they are valid. 

   NOTE - The best way to do this is to manually launch the simulator
          with the specified command arguments and see if they work as
          expected.

   NOTE - "simargs" is enabled only when MCD is configured to launch a
          new simulator instance. If it is configured to connect to an
          existing instance, "simargs" will collect the specified
          simulator command arguments but then do nothing with them.

+++ For more information on MCD see the sample scripts located at
<install root>\examples\TRACE32.
   linux.cfg	- sample TRACE32 config file for MCD used with a linux
                TRACE32 debugger (t32mqdsp6).
   win.cfg		- sample TRACE32 config file for MCD used with a Windows
                TRACE32 debugger (t32mqdsp6.exe).
   hexagon.cmm	- sample TRACE32 PRACTICE script to initialize a Hexagon
                TRACE32 debugger

----------------------------------------
Debugging Test Application using TRACE32
----------------------------------------
The test application (mandelbrot) uses a standalone environment that takes
advantage of the multiple hardware threads available in the Hexagon processor.
This application can be debugged using TRACE32 in both the Linux and Windows
operating systems. To clean\build\debug this application, just type "make" in
the appropriate subfolders:
Linux_Example:		make (invokes Linux make on Makefile)
Windows_Example:	make (invokes make.bat)

----------
REFERENCES
----------
- 80-N2040-17 Hexagon Simulator User Guide
- 80-N2040-28 Hexagon TRACE32 User Guide
- Lauterbach TRACE32 documentation (available in PDF subdirectory of
  TRACE32 Installation)
