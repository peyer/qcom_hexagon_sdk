##################################################################
# Copyright (c) $Date: 2012/03/07 01:00:01 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2012/03/07 01:00:01 $
##################################################################
TEST = scatter_gather
all: clean build sim

CC  = hexagon-clang
CXX = hexagon-clang++


Q6VERSION := v65
OPT=-O2

ifdef HVX_DOUBLE
CFLAGS = -DREGRESS=1 -DHVX_DOUBLE=1 -m${Q6VERSION} $(OPT) -g -G0 -Wall -mhvx -mhvx-length=128B -Wno-unused-variable
SIMF=$(Q6VERSION)a_512 --hvx_length 128
else
CFLAGS = -DREGRESS=0 -m${Q6VERSION} $(OPT) -g -G0 -Wall -mhvx -mhvx-length=64B -Wno-unused-variable
SIMF=-m$(Q6VERSION)a_512 --hvx_length 64
endif

all: clean build sim

OBJS =  $(TEST).o

build: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TEST) -lhexagon

*.o:*.c
	$(CC) $(CFLAGS) -o $(TEST).o -c $(TEST).c

sim:
	hexagon-sim $(SIMF) --bypass_idle --timing --packet_analyze profile.json --pmu_statsfile pmu_statsfile.txt $(TEST)

profile:
	hexagon-profiler --packet_analyze --json=profile.json --elf=$(TEST) -o $(TEST).html

clean:
	rm -fr $(TEST).o $(TEST) stats.txt pmu_statsfile.txt *.map *.json *.html

lldb:
	hexagon-lldb $(TEST)
