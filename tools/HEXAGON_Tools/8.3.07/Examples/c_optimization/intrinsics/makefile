##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

CC	= hexagon-clang
CFLAGS	= -O2
Q6VERSION = v60
CFLAGS	+= -m${Q6VERSION} -I../include

all: clean intrinsics sim

OBJS = intrinsics.o test_intrinsics.o

intrinsics: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o intrinsics -lhexagon

clean:
	rm -f intrinsics $(OBJS) pmu_statsfile.txt stats.txt
	
sim:
	hexagon-sim --timing intrinsics
