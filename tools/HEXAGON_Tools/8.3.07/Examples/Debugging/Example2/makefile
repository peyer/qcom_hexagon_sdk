##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

CC	= hexagon-clang
CFLAGS	= -O2 -G0 -g
Q6VERSION	= v60
CFLAGS	+= -m${Q6VERSION}

OBJS = main.o

PROGRAM = main

all: clean $(PROGRAM) sim

$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(PROGRAM)

sim:
	hexagon-sim -m${Q6VERSION} $(PROGRAM)

debug:
	hexagon-lldb main

clean:
	rm -f $(PROGRAM) $(OBJS) core dump.txt pmu_statsfile.txt stats.txt

