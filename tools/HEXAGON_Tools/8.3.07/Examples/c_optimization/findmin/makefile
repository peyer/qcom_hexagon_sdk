##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

CC	= hexagon-clang
CFLAGS	= -O2
Q6VERSION = v60
CFLAGS	+= -m${Q6VERSION}

all: clean findmin sim

OBJS = findmin.o test_findmin.o

findmin: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o findmin -lhexagon

clean:
	rm -f findmin $(OBJS) pmu_statsfile.txt stats.txt
	
sim:
	hexagon-sim --timing findmin
