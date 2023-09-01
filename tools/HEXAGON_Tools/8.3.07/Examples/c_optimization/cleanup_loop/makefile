##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

CC	= hexagon-clang
CFLAGS	= -O2
Q6VERSION = v60
CFLAGS	+= -m${Q6VERSION} -I../include

OBJS = cleanup_loop.o test_cleanup_loop.o

all: clean cleanup_loop sim

cleanup_loop: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o cleanup_loop -lhexagon

clean:
	rm -f cleanup_loop $(OBJS) pmu_statsfile.txt stats.txt

sim:
	hexagon-sim --timing cleanup_loop
