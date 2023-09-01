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
ASFLAGS	 = -m${Q6VERSION}
SIMFLAGS = -m${Q6VERSION} --timing
OBJ_DIR  = ./bin
SRC_DIR  = ./src
OBJS = $(OBJ_DIR)/main.o $(OBJ_DIR)/tlb.o

# set tcm size to 128k
TCM_SIZE	= 0x20000
STACK_SIZE	= 0x4000
HEAP_SIZE	= 0x10000

TCM_BASE_ADDR = 0xd8000000

LINKFLAGS = -Wl,--section-start,.start=$(TCM_BASE_ADDR), \
			-Wl,--defsym,TCM_BASE_ADDR=$(TCM_BASE_ADDR), \
			-Wl,--defsym,L2_CACHE_SIZE=1, \
			-Wl,--defsym,HEAP_SIZE=$(HEAP_SIZE), \
			-Wl,--defsym,STACK_SIZE=$(STACK_SIZE)
ELF = ./bin/main

all: clean build sim

sim:
	$(SIM) $(SIMFLAGS) $(ELF)

clean:
	rm -rf ./bin stats.txt pmu_statsfile.txt

build: makedir $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(ELF) $(LINKFLAGS) -Wl,-Map=$(OBJ_DIR)/link.map

makedir:
	mkdir -p ./bin

t32:
	T32SYS=$(T32SYS)
	$(T32SYS)/t32mqdsp6.exe -c cmm/win.cfg -s cmm/hexagon.cmm $(ELF) ${Q6VERSION}

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
