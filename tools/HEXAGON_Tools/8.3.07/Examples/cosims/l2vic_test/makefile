##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

CC		= hexagon-clang
AS		= hexagon-as
SIM		= hexagon-sim
CFLAGS	= -O2 -g
Q6VERSION = v60

SIM_PATH        = $(shell dirname `which hexagon-sim`)
HEX_INCLUDE		= $(SIM_PATH)/../include/iss
HEX_LIB_PATH	= $(SIM_PATH)/../lib/iss
COSIM_NAME		= ./Bin/l2vic_test_cosim.so
MYCXX			= g++

CFLAGS		+= -m${Q6VERSION}
ASFLAGS		+= -m${Q6VERSION}
SIMFLAGS	+= -m${Q6VERSION} --timing
SYSTEM 		:= $(shell uname)

##################################################################
# This is for pre-release regression testing
##################################################################
ifneq (,$(Q6VERSION))
    INTERRUPT       = 31
ifeq ($(shell expr $(Q6VERSION) \>= v65), 1)
	INTERRUPT = 2
else
	INTERRUPT = 31
endif
else # Q6VERSION not defined
##################################################################
Q6VERSION = v60
INTERRUPT = 31
endif

all: clean build sim

build:
ifeq (Linux, $(findstring Linux,$(SYSTEM)))
	mkdir Bin
	$(MYCXX) -fpic -O3 -I$(HEX_INCLUDE) -L$(HEX_LIB_PATH) -shared -o $(COSIM_NAME) ./Source/l2vic_test_cosim.cpp
	$(CC) $(CFLAGS) -DINTERRUPT=$(INTERRUPT) ./Source/mandelbrot.c -o ./Bin/mandelbrot -lhexagon -DJUST_WAIT
else
	./make.cmd build
endif

clean:
ifeq (Linux, $(findstring Linux,$(SYSTEM)))
	rm -rf Bin stats.txt pmu_statsfile.txt
else
	./make.cmd clean
endif

sim:
ifeq (Linux, $(findstring Linux,$(SYSTEM)))
	$(SIM) $(SIMFLAGS) ./Bin/mandelbrot --cosim_file ./Source/cosims_lnx.txt
else
	./make.cmd sim
endif
