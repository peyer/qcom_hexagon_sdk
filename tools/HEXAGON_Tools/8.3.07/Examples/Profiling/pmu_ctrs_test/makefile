##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

PROGRAM = pmu_ctrs_test
CC 	= hexagon-clang
SIM	= hexagon-sim

CFLAGS	= -O2 -g

Q6VERSION = v60

CFLAGS	+= -m${Q6VERSION}
ASFLAGS	= -m${Q6VERSION}

SIMFLAGS = -m${Q6VERSION} --timing
SIMFLAGS += --ahb:lowaddr 0x28000000 --ahb:highaddr 0x29000000

EXEC = $(PROGRAM).elf

all: clean build sim

clean:
	rm -f -r *.elf pmu*.txt stats.txt

build:
	$(CC) $(CFLAGS) $(PROGRAM).c -o $(EXEC) -lhexagon

sim:
	$(SIM) $(SIMFLAGS) $(EXEC)
