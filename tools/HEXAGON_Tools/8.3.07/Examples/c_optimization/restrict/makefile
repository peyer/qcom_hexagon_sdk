##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

CC	= hexagon-clang
CFLAGS	= -O2
Q6VERSION = v60
CFLAGS	+= -m${Q6VERSION}

all: clean restrict sim

OBJS = restrict.o test_restrict.o

restrict: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o restrict -lhexagon

clean:
	rm -f restrict $(OBJS) pmu_statsfile.txt stats.txt

sim:
	hexagon-sim --timing restrict
