##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################
#
# Makefile for Shared Library Linker scripting example
# make        - build without a linker script
# make script - build with a dynamic linker script for dmain.c
# make shared - build with a dynamic
#

Q6VERSION := v60
MAIN = ./src/dmain.c
LIB = ../common/mylib1.c
ELF = ./bin/dmain.elf
CC  =  hexagon-clang
CFLAGS = -m$(Q6VERSION) -DLINUX=1 -O2 -Wl,--force-dynamic -Wl,-E -ldl -G0
SCRIPT = -Wl,-T,./src/dlink.script
LINKFLAGS = -fPIC -shared -nostartfiles
SHAREDLIB = ./bin/mylib1.so

SRC = $(MAIN) $(LIB)

#
# default is to build without linker scripts
#
all: clean main_no_script read sim
#
# build with linker script only for main
#
script: clean main_with_script read sim
#
# build with linker scripts for main and shared library
#
shared: clean main_with_script_sharedlib read sim

main_no_script: $(SRC)
	mkdir --parents bin
	$(CC) $(MAIN) -o $(ELF) $(CFLAGS)
	$(CC) $(LIB) -o $(SHAREDLIB) $(LINKFLAGS)

main_with_script:
	mkdir --parents bin
	$(CC) $(MAIN) -o $(ELF) $(CFLAGS) $(SCRIPT)
	$(CC)$(LIB) -o $(SHAREDLIB) $(LINKFLAGS)

main_with_script_sharedlib:
	mkdir --parents bin
	$(CC) $(MAIN) -o $(ELF) $(CFLAGS) $(SCRIPT)
	$(CC) $(LIB) -o $(SHAREDLIB) $(LINKFLAGS) $(SCRIPT)

read:
	hexagon-readelf -e $(ELF)

sim:
	hexagon-sim $(ELF)

clean:
	rm -fr bin stats.txt link.map pmu_stats*
