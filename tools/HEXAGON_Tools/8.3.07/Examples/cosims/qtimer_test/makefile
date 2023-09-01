##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

PROGRAM = qtimer_test

CC	= hexagon-clang
AS	= hexagon-as
SIM	= hexagon-sim
SYSTEM := $(shell uname)
Q6_CFG = q6ss.cfg

T32_CFG = cmm/t32.cfg hexagon-mcd64.so
T32_EXE = bin/pc_linux64/t32mqdsp6

CFLAGS	= -O2 -g

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

CFLAGS	+= -m${Q6VERSION} -DQ6VERSION=$(Q6VERSION) -DCSR_BASE=0xfab00000 -DQTMR_FREQ=19200000 -DIRQ1=67 -DIRQ2=68 -DINTERRUPT=$(INTERRUPT)

SIMFLAGS += -m${Q6VERSION} --timing --bypass_idle --cosim_file $(Q6_CFG) $(ELF)

ELF = $(PROGRAM).elf

all: clean build sim

build:
	echo qtimer.so --csr_base=0xfab00000 --irq_p=67,68 --freq=19200000 --cnttid=0x11 > q6ss.cfg
	echo l2vic.so 4 0xfab10000 >> q6ss.cfg
	$(CC) $(CFLAGS) $(PROGRAM).c -o $(ELF) -lhexagon

clean:
	rm -f pmu_statsfile.txt stats.txt *.elf *.out *.cfg

sim:
	$(SIM) $(SIMFLAGS)

t32:
	T32SYS=$(T32SYS)
	$(T32SYS)/$(T32_EXE) -c $(T32_CFG) -s cmm/hexagon.cmm $(ELF) $(Q6VERSION) $(Q6_CFG)
