###############################################################################
#
#  Yaml Example for parsing linker map files
#
###############################################################################

This example uses python scripts to parse a map file which is produced by the
hexagon-link utility.  The objective of this example is to demonstrate how to
extract important information from a map file.

The makefile performs different tasks by invoking one of 12 command line
arguments.  There is also a test argument which will invoke all 12 arguments.

In order to analyze the mapfile the following steps are taken:

1. create the object file from the source
    hexagon-clang -mv60 -O2 -G0 -Wall -g -c -o main.o main.c

2. Link the object file with libraries, enable garbage collection, create an
   elf output and a map file in yaml format.
    hexagon-clang -mv60 -O2 -G0 -Wall -g main.o -o main -Wl,-gc-sections,-Map=map.yaml,-MapStyle=yaml

3. Parse the map file using the python script, MyParser.py.
    python .\\scripts\\MyParser.py map.yaml