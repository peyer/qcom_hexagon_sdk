##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

CC	= hexagon-clang
AS	= hexagon-llvm-mc -filetype=obj
CFLAGS	= -O2 -g
Q6VERSION := v60

CFLAGS	+= -m${Q6VERSION}
ASFLAGS	+= -m${Q6VERSION}
SYSTEM = $(shell uname)

## Be sure to initialize T32SYS to the latest version of T32
ifeq (Linux, $(findstring Linux, $(SYSTEM)))
T32SYSBIN = bin/pc_linux64
else
T32SYSBIN = bin/windows64
endif

OBJS = mandelbrot.o

all: clean mandelbrot t32

mandelbrot: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o mandelbrot

clean:
	rm -f mandelbrot $(OBJS) pmu_statsfile.txt stats.txt

t32:
	$(T32SYS)/$(T32SYSBIN)/t32mqdsp6 -c linux.cfg -s hexagon.cmm mandelbrot v60
