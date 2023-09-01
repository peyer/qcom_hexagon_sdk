##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

PROGRAM = tls

CC	= hexagon-clang
AS	= hexagon-as
SIM	= hexagon-sim

Q6VERSION	:= v60
CFLAGS	= -O2 -g -m${Q6VERSION}
SIMFLAGS = --bypass_idle -m${Q6VERSION} --timing $(ELF)
ELF = $(PROGRAM).elf

all: clean build sim

build:
	$(CC) $(CFLAGS) $(PROGRAM).c -o $(ELF) -lhexagon

clean:
	rm -f pmu_statsfile.txt stats.txt *.elf *.out pmu* stat*

sim:
	$(SIM) $(SIMFLAGS)

lldb:
	hexagon-lldb $(ELF)
