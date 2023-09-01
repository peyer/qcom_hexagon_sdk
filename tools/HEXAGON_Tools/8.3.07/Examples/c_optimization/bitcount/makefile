##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

CC	= hexagon-clang
Q6VERSION = v60
CFLAGS	= -m${Q6VERSION}

OBJS = bitcount.o test_bitcount.o

all: clean bitcount sim

bitcount: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o bitcount -lhexagon

clean:
	rm -f bitcount $(OBJS) pmu_statsfile.txt stats.txt
	
sim:
	hexagon-sim --timing bitcount
