##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

TEST = mandelbrot
CC	= hexagon-clang
AS	= hexagon-llvm-mc -filetype=obj
SIM	= hexagon-sim
CFLAGS	= -O2 -g
Q6VERSION = v62

CFLAGS	+= -m${Q6VERSION}
ASFLAGS	+= -m${Q6VERSION}

ifeq (v62,$(Q6VERSION))
SIMFLAGS = -mv62d_1536
endif

ifeq (v65,$(Q6VERSION))
SIMFLAGS = -mv65h_1536
endif

ifeq (v66,$(Q6VERSION))
SIMFLAGS = -mv66d_1536
endif

ifeq (v67,$(Q6VERSION))
# check if valid version
ifeq ( ,$(shell hexagon-sim --help | grep v67))
override Q6VERSION=v62
SIMFLAGS = -mv62d_1536
else
SIMFLAGS = -mv67h_3072
endif
endif

CLADEFLAGS = -Wl,-T,clade.lcs,-Map,clade.map

OBJS = $(TEST).o

all: clean build sim

build: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TEST) -lhexagon $(CLADEFLAGS)

clean:
	rm -f $(TEST) $(OBJS) pmu* *.map

sim:
	$(SIM) --bypass_idle $(SIMFLAGS) $(TEST)

lldb:
	hexagon-lldb $(TEST) -- --bypass_idle -mv62d_1536
