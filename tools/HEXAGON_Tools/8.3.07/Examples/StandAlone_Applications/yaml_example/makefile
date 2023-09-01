##################################################################
# Copyright (c) $Date: 2012/03/07 01:00:01 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2012/03/07 01:00:01 $
##################################################################

CC  = hexagon-clang
CXX = hexagon-clang++

Q6VERSION := v60
CFLAGS = -m${Q6VERSION} -O2 -G0 -Wall -g
LINKFLAGS = -Wl,-gc-sections,-Map=map.yaml,-MapStyle=yaml

SYSTEM := $(shell uname)
ifeq (Linux, $(findstring Linux,$(SYSTEM)))
	SCRIPTS = ./scripts/
else
	SCRIPTS = .\\scripts\\
endif

OBJS =  main.o

all: clean main myparser

main: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o main $(LINKFLAGS)

*.o:*.c
	$(CC) $(CFLAGS) -o main.o -c main.c

arch: main
	@echo
	@echo ------architecture output------
	@echo
	python $(SCRIPTS)YAMLMapParser.py --info architecture map.yaml

arch_sizes: main
	@echo
	@echo ------architecture and sizes output------
	@echo
	python $(SCRIPTS)YAMLMapParser.py --info architecture,sizes map.yaml

sizes: main
	@echo
	@echo ------sizes output------
	@echo
	python $(SCRIPTS)YAMLMapParser.py --info sizes map.yaml

sumsizes: main
	@echo
	@echo ------summary sizes output------
	@echo
	python $(SCRIPTS)YAMLMapParser.py --info summarysizes map.yaml

totals: main
	@echo
	@echo ------totals output------
	@echo
	python $(SCRIPTS)YAMLMapParser.py --info totals map.yaml

unused: main
	@echo
	@echo ------unused sections output------
	@echo
	python $(SCRIPTS)YAMLMapParser.py --info unused map.yaml

unusedsyms: main
	@echo
	@echo ------unused symbols output------
	@echo
	python $(SCRIPTS)YAMLMapParser.py --info unusedsymbols map.yaml

map: main
	@echo
	@echo ------map output------
	@echo
	python $(SCRIPTS)YAMLMapParser.py --map map.yaml

list: main
	@echo
	@echo ------create list file------
	@echo
	python $(SCRIPTS)YAMLMapParser.py --info sizes --list=list.txt map.yaml

xref: main
	@echo
	@echo ------create cross reference file------
	@echo
	python $(SCRIPTS)YAMLMapParser.py --xref --list=xref.txt map.yaml

help:
	@echo
	@echo ------makefile help options------
	@echo
	python $(SCRIPTS)YAMLMapParser.py --help

myparser: main
	@echo
	@echo ------parse map file and create output------
	@echo
	python $(SCRIPTS)MyParser.py map.yaml

clean:
	rm -fr *.o main stats.txt pmu_statsfile.txt map.txt list.txt xref.txt *.yaml

test: all arch arch_sizes sizes sumsizes totals unused unusedsyms map list xref help