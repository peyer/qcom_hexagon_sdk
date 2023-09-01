##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################
#
# Makefile for Partial Linker scripting example
# make        - build without a linker script
# make script - build with a dynamic linker script for shmain.c
# make shared - build with a dynamic
#

Q6VERSION := v60
MAIN = ./bin/ptmain.elf
CC  =  hexagon-clang
LD = hexagon-link
CFLAGS = -m$(Q6VERSION) -DLINUX=1 -O2 -Wl,--force-dynamic -Wl,-E -ldl -G0
SCRIPT = -Wl,-T,./src/ptlink.script
LINKFLAGS = -fPIC -shared -nostartfiles
SHAREDLIB = ./bin/liba.so

SRC = ./src/ptmain.c ./src/ptlib.c ../common/mylib1.c ../common/mylib2.c

#
# default is to build without linker scripts
#
all: clean build_common build_no_script read sim
#
# build with linker script only for main
#
script: clean build_common main_with_script read sim
#
# build with linker scripts for main and shared library
#
shared: clean build_common main_sharedlib_with_script read sim

build_common: $(SRC)
	mkdir --parents bin
	$(CC) -m$(Q6VERSION) $(SRC) -c
	mv *.o bin/.
	$(LD) -r bin/mylib1.o bin/mylib2.o -o bin/mylib3.o

build_no_script:
	$(CC) ./src/ptmain.c bin/mylib3.o -o $(MAIN) $(CFLAGS)
	$(CC) ./src/ptlib.c -o $(SHAREDLIB) $(LINKFLAGS)

main_with_script:
	$(CC) ./src/ptmain.c bin/mylib3.o -o $(MAIN) $(CFLAGS) $(SCRIPT)
	$(CC) ./src/ptlib.c -o $(SHAREDLIB) $(LINKFLAGS)

main_sharedlib_with_script:
	$(CC) ./src/ptmain.c bin/mylib3.o -o $(MAIN) $(CFLAGS) $(SCRIPT)
	$(CC) ./src/ptlib.c -o $(SHAREDLIB) $(LINKFLAGS) $(SCRIPT)

read:
	hexagon-readelf -e $(MAIN)

sim:
	hexagon-sim $(MAIN)

clean:
	rm -fr bin stats.txt link.map pmu_stats* *.so
