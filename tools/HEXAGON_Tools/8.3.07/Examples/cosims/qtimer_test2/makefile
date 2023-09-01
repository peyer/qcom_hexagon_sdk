##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

PROGRAM = 2qtimer_test

CC	= hexagon-clang
AS	= hexagon-as
SIM	= hexagon-sim
SYSTEM := $(shell uname)

Q6_CFG = q6ss.cfg
ifeq (Linux, $(findstring Linux,$(SYSTEM)))
T32_CFG = cmm/t32.cfg hexagon-mcd64.so
T32_EXE = bin/pc_linux64/t32mqdsp6
Q6_TIMER = qtimer.so
Q6_L2VIC = l2vic.so
else
T32_CFG = cmm/t32.cfg hexagon-mcd64.dll
T32_EXE = bin/windows64/t32mqdsp6.exe
Q6_TIMER = qtimer.dll
Q6_L2VIC = l2vic.dll
endif

CFLAGS	= -O2 -g
TIMER_FREQ = 19200000
L2_INT1	= 36
L2_INT2	= 58
L2_INT3	= 80
L2_INT4	= 112
CSR_BASE1 = 0xfab00000
CSR_BASE2 = 0xfab40000

##################################################################
# This is for pre-release regression testing
##################################################################
ifneq (,$(Q6VERSION))
    INTERRUPT       = 31
ifeq (v66, $(Q6VERSION))
    INTERRUPT       = 2
else
    INTERRUPT       = 31
endif
else # Q6VERSION not defined
Q6VERSION	= v60
INTERRUPT	= 31
##################################################################
	Q6VERSION	= v66
	INTERRUPT	= 2
endif


CFLAGS	= -m${Q6VERSION} -g -DQTMR_FREQ=$(TIMER_FREQ)
CFLAGS += -DL2_INT1=$(L2_INT1) -DL2_INT2=$(L2_INT2) -DL2_INT3=$(L2_INT3) -DL2_INT4=$(L2_INT4)
CFLAGS += -DINTERRUPT=$(INTERRUPT) -DQ6VERSION=$(subst v,,$(Q6VERSION))
SIMFLAGS = -m${Q6VERSION}$(ARCH) --bypass_idle --timing --pmu_statsfile mystats.txt --cosim_file $(Q6_CFG) --l2tcm_base 0xef5a 
#--fastl2vic_base 0xef78

ELF = $(PROGRAM).elf

all: clean build sim

build:
	echo $(Q6_TIMER) --csr_base=$(CSR_BASE1) --irq_p=$(L2_INT1),$(L2_INT2) --freq=19200000 --cnttid=0x11 > $(Q6_CFG)
	echo $(Q6_TIMER) --csr_base=$(CSR_BASE2) --irq_p=$(L2_INT3),$(L2_INT4) --freq=19200000 --cnttid=0x11 >> $(Q6_CFG)
	echo $(Q6_L2VIC) 4 0xfab10000 >> $(Q6_CFG)
	$(CC) $(CFLAGS) $(PROGRAM).c -o $(ELF) -lhexagon

clean:
	rm -f *stat*.txt *.elf *.out *.o *.cfg

sim:
	$(SIM) $(SIMFLAGS) $(ELF)

lldb:
	hexagon-lldb $(PROGRAM).elf -- $(SIMFLAGS)

t32:
	T32SYS=$(T32SYS)
	$(T32SYS)/$(T32_EXE) -c $(T32_CFG) -s cmm/hexagon.cmm $(ELF) $(Q6VERSION) $(Q6_CFG)
