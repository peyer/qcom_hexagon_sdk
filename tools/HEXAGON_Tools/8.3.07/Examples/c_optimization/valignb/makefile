##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

CC	= hexagon-clang
CFLAGS	= -O2
Q6VERSION = v60
CFLAGS	+= -m${Q6VERSION} -I../include

all: clean valignb sim

OBJS = pairadd.o test_pairadd.o

valignb: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o valignb -lhexagon

clean:
	rm -f valignb $(OBJS) pmu_statsfile.txt stats.txt

sim:
	hexagon-sim --timing valignb
