##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

CC	= hexagon-clang
CFLAGS	= -O0
Q6VERSION = v60
CFLAGS	+= -m${Q6VERSION} -I../include

all: clean vmux sim

OBJS = findmin.o test_findmin.o

vmux: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o vmux -lhexagon

clean:
	rm -f vmux $(OBJS) pmu_statsfile.txt stats.txt

sim:
	hexagon-sim --timing vmux
