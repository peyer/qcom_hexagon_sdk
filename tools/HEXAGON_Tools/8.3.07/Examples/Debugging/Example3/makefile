##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

CC	= hexagon-clang
CFLAGS	= -O2 -G0 -g
Q6VERSION	= v60
CFLAGS	+= -m${Q6VERSION}
SYSTEM := $(shell uname)
## Be sure to initialize T32SYS to point to T32 latest version
ifeq (Linux, $(findstring Linux, $(SYSTEM)))
T32SYSBIN = bin/pc_linux64/t32mqdsp6
CFG = linux.cfg
else
T32SYSBIN = bin/windows64/t32mqdsp6.exe
CFG = win.cfg
endif

OBJS = main.o

PROGRAM = main

all: clean $(PROGRAM) debug

$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(PROGRAM)

sim:
	hexagon-sim -m${Q6VERSION} $(PROGRAM)

debug:
	@echo system = $(SYSTEM)
	$(T32SYS)/$(T32SYSBIN) -c $(CFG) -s hexagon.cmm main v60

clean:
	rm -f $(PROGRAM) $(OBJS) core dump.txt pmu_statsfile.txt stats.txt

