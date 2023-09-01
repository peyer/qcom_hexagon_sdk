##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

CC	= hexagon-clang
CFLAGS	= -O2
Q6VERSION = v60
CFLAGS	+= -m${Q6VERSION} -I../include

all: clean uniform_misalignment sim

OBJS = misalignment.o test_misalignment.o

uniform_misalignment: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o uniform_misalignment -lhexagon

clean:
	rm -f uniform_misalignment $(OBJS) pmu_statsfile.txt stats.txt

sim:
	hexagon-sim --timing uniform_misalignment
