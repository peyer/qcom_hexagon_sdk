##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

CC	= hexagon-clang
SIM	= hexagon-sim

CFLAGS	= -O2 -g -G0
Q6VERSION := v60

CFLAGS	+= -m${Q6VERSION}
ASFLAGS	= -m${Q6VERSION}
SIMFLAGS= -m${Q6VERSION} --timing

MCD_LINUX = @echo PBI=MCD hexagon-mcd64.so > t32.cfg
MCD_WIN = @echo PBI=MCD hexagon-mcd64.dll > t32.cfg

ifeq (Linux, $(findstring Linux, $(shell uname)))
T32SYSBIN = bin/pc_linux64/t32mqdsp6
MCD = $(MCD_LINUX); cat ./cmm/t32.cfg >> t32.cfg
else
T32SYSBIN = bin/windows64/t32mqdsp6.exe
MCD = $(MCD_WIN) & type cmm/t32.cfg >> t32.cfg
endif

OBJS = main.o tlb_asm.o tlb.o

all: clean tlb sim

tlb: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o main -Wl,-Map=link.map -Wl,-T,linker.script

clean:
	rm -f main $(OBJS) stats.txt pmu_statsfile.txt link.map *.cfg

sim:
	$(SIM) $(SIMFLAGS) main

t32:
	@echo -e MAKE SURE TO SET T32SYS ENVIRONMENT VARIABLE
	$(MCD)
	$(T32SYS)/$(T32SYSBIN) -c t32.cfg -s cmm/hexagon.cmm main ${Q6VERSION}
