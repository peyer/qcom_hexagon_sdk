##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

CC = hexagon-clang
AS = hexagon-llvm-mc -filetype=obj
SIM =hexagon-sim
CFLAGS = -O2

Q6VERSION := v60

CFLAGS += -m${Q6VERSION} -I../include
ASFLAGS += -m${Q6VERSION}
SIMFLAGS += -m${Q6VERSION} --timing

OBJS = bkfir.o bkfir_asm.o test_bkfir.o

all: clean bkfir sim
	
bkfir: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o bkfir -lhexagon

sim:
	$(SIM) $(SIMFLAGS) bkfir

clean:
	rm -f bkfir $(OBJS) pmu_statsfile.txt stats.txt
