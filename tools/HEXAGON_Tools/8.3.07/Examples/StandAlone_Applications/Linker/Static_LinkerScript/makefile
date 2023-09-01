##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################
#
# Makefile for Static Linker scripting example
# make        - build without a linker script
# make script - build with a linker script
#

Q6VERSION := v60
MAIN = ./bin/smain.elf
CC  =  hexagon-clang
AR = hexagon-ar
CFLAGS = -m$(Q6VERSION) -O2 -G0 -ffunction-sections -fdata-sections
SCRIPT = ./src/slink.script
LINKFLAGS = -Wl,--section-start,.start=0xdf0000,--gc-sections

SRC = ./src/smain.c ../common/mylib1.c

all: clean build linkNoScriptFile read sim

script: clean build linkScriptFile read sim

build: $(SRC)
	mkdir --parents bin
	$(CC) $(CFLAGS) ./src/smain.c -c -o ./bin/smain.o
	$(CC) $(CFLAGS) -c ../common/mylib1.c -o ./bin/mylib1.o
	$(AR) -cq ./bin/libmylib1.a ./bin/mylib1.o
	$(AR) -t ./bin/libmylib1.a

linkNoScriptFile:
	$(CC) $(CFLAGS) ./bin/smain.o -o $(MAIN) ./bin/libmylib1.a $(LINKFLAGS)

linkScriptFile:
	$(CC) $(CFLAGS) ./bin/smain.o -o $(MAIN) ./bin/libmylib1.a $(LINKFLAGS) -T$(SCRIPT)

read:
	hexagon-readelf -e $(MAIN)

sim:
	hexagon-sim $(MAIN)

clean:
	rm -fr bin stats.txt link.map pmu_stats*
