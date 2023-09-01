*******************************************************************************
** Hexagon simulator API Example
**
** This example demonstrates how to use hexagon-sim system API calls to invoke
** the simulator in a system simulation mode.  The reference document on system
** simulation is "Hexagon Simulator System API" User Guide, 80-N2040-18 P.
**
** This example demonstrates the following:
** 1. how to configure the simulator engine
** 2. how to setup external device such as a bus or configure a cosim
** 3. how to pass parameters to the simulator engine
** 4. how to set it up for debugging.
** 5. how to control simulation.
** 6. how you can use this environment to design and debug a co-simulation module.
**
** The example currently passess 4 parameters to the program that is being simulated
** which are -mv<hexagon_version> -G <port_num> <program_to_simulate> [<cosim_cfg_file>].
** The -G entry is considered a parameter as it is counted in argc.
**
** Parameter 3 is a port number so it can wait to be remotely connected to
** a debugger.  If you wish to just run the simulation without hooking up to a debugger
** you will use Port 0 so it will not wait for a debugger.  This is automatically done in
** in the make.cmd and makefile for "make" and "make sim".
**
** For debugging, the make.cmd will default to Port 9912 and the makefile will generate a
** random port number.  If you wish to use a different port number in Windows
** you can set the environment variable: set PORT=<port_num> and this will be the default
** port used for debugging.
**
** If you would like to provide a cosim of your own, you will need to add a config file
** which points to a cosim .dll or .so. Then change the test for argc to add a new
** argument in hexagon-system.cpp.  Add the new argument to the makefile so
** when you invoke hexagon-system you pass the cosim config file name to the
** simulator.  Finally, uncomment line 7 of hexagon-system.cpp before you rebuild.
**
** Here is how you use the make.cmd or makefile:
** to clean,build,simulate code : make
** to build code                : make build
** to simulate                  : make sim
** to debug in t32              : make t32
** to debug in lldb             : make lldb
**
** The simulator example is currently built with Microsoft Visual Studio 14 for
** Windows and gnu compiler for Linux.  This allows the system simulator to run
** in native binary.  The hexagon test program is built using the hexagon
** tool suite and run by the system simulator after loading the hexagon binary.
**
** The following files are in this example:
** 1. src/main.cpp  - this is the code that configures and invokes the simulator engine.
** 2. src/hello.c   - this is the hexagon code that the instantiated simulator will run.
** 3. src/lldb-setup.txt - this is a hexagon-lldb command script to attach to the application.
** 4. make.cmd      - a windows cmd makefile
** 5. makefile      - a linux makefile
** 6. cmm folder    - This is the necessary files if you want to fire up Lauterbach t32
**
**
*******************************************************************************
