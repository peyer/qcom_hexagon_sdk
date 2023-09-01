##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################
#
# Makefile for Program Headers Linker scripting example
# make        - build without a linker script
# make script - build with a linker script
#

Q6VERSION := v60
MAIN = ./bin/pmain.elf
CC  =  hexagon-clang
CFLAGS = -m$(Q6VERSION) -O2 -G0 -ffunction-sections -fdata-sections
SCRIPT = ./src/plink.script
LINKFLAGS = -Wl,--section-start,.start=0xdf0000,--gc-sections

SRC = ./src/pmain.c ../common/mylib1.c

all: clean build read sim

script: clean linkScriptFile read sim

build: $(SRC)
	mkdir --parents bin
	$(CC) $(CFLAGS) $(SRC) -o $(MAIN) $(LINKFLAGS)

linkScriptFile:
	$(eval LINKFLAGS += -T$(SCRIPT))
	mkdir --parents bin
	$(CC) $(CFLAGS) $(SRC) -o $(MAIN) $(LINKFLAGS)

read:
	hexagon-readelf -e $(MAIN)

sim:
	hexagon-sim $(MAIN)

clean:
	rm -fr bin stats.txt link.map pmu_stats*