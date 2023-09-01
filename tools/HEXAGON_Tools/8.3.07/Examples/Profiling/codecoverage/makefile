##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

CC	= hexagon-clang
AS	= hexagon-llvm-mc -filetype=obj
SIM	= hexagon-sim
CFLAGS	= -O2 -g
Q6VERSION := v60

CFLAGS	+= -m${Q6VERSION}
ASFLAGS	 = -m${Q6VERSION}
SIMFLAGS = -m${Q6VERSION} --timing

OBJS = mandelbrot.o

all: clean build sim merge coverage

build: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o mandelbrot -lhexagon

# simulate with profiling enabled
sim:
	$(SIM) $(SIMFLAGS) --profile mandelbrot

# optionally merge gmon the files with the merge utility
merge:
	rm -fr gmon.merged
	hexagon-gmon-merge -o gmon.merged gmon.t*

# optionally create a viewable text file from a gmon file
dump:
	hexagon-gmon-dump gmon.merged > dumpgmon.txt

# run the code coverage tool
coverage:
	hexagon-coverage -i mandelbrot --html report gmon.merged

clean:
	rm -f mandelbrot $(OBJS) pmu_statsfile.txt stats.txt gmon.* dump*.txt
	rm -fr report


