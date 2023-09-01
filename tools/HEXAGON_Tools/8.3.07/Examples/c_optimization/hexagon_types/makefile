##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

CC	= hexagon-clang
CFLAGS	= -O2
Q6VERSION = v60
CFLAGS	+= -m${Q6VERSION} -Wno-cast-align -I../include

all: clean hexagon_types sim

OBJS = hexagon_types.o

hexagon_types: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o hexagon_types -lhexagon

clean:
	rm -f hexagon_types $(OBJS) pmu_statsfile.txt stats.txt

sim:
	hexagon-sim --timing hexagon_types
