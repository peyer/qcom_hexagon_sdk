##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

CC	= hexagon-clang
CFLAGS	= -O2
Q6VERSION = v60
CFLAGS	+= -m${Q6VERSION}

all: clean timers sim

OBJS = timers.o

timers: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o timers -lhexagon

clean:
	rm -f timers $(OBJS) pmu_statsfile.txt stats.txt

sim:
	hexagon-sim --timing timers
