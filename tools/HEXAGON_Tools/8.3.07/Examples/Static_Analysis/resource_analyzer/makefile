##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################
PROGRAM = mandelbrot
CC	= hexagon-clang
AS	= hexagon-llvm-mc -filetype=obj
SIM	= hexagon-sim
CFLAGS	= -O2
Q6VERSION := v60

CFLAGS	+= -m${Q6VERSION} -g
SIMFLAGS+= -m${Q6VERSION} --timing

OBJ = ./bin/$(PROGRAM).elf

all: clean build analyze sim

build:
	mkdir ./bin
	$(CC) $(CFLAGS) ./src/$(PROGRAM).c -o $(OBJ) -lhexagon

analyze:
	mkdir ./RA
	hexagon-analyzer-backend --dsp $(Q6VERSION) -o RA --elffile $(OBJ)

clean:
	rm -rf Whole* pmu_stats* $(OBJ) RA_* stats* ./RA ./bin

sim:
	$(SIM) $(SIMFLAGS) $(OBJ)
