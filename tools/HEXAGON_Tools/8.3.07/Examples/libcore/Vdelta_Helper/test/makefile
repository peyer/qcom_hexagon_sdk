
###########################################################################
# Copyright (c) Date: Mon Nov 24 16:25:55 CST 2008 QUALCOMM INCORPORATED
# All Rights Reserved
# Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:55 CST 2008
###########################################################################

TEST = vdelta_helper

EXEC = $(TEST).elf
OBJ_DIR = ../build
SRC_DIR = ../src
Q6VERSION = v60
RUN = hexagon-sim
CC = hexagon-clang

CFLAGS = -Wall -O3 -m$(Q6VERSION)
REQ_LDFLAGS = -Wl,--section-start=.start=0x01000000
_C_SRCS   = vdelta_helper_c.c vdelta_helper_d.c
SRC_C_OBJS   = $(_C_SRCS:%.c=$(OBJ_DIR)/%.o)
SIMF = -m$(Q6VERSION) --timing --simulated_returnval

#default goal
all: clean build sim

build: $(EXEC)

$(EXEC): $(SRC_C_OBJS)
	$(CC) -m$(Q6VERSION) $(REQ_LDFLAGS) -o $@ $(SRC_C_OBJS) -lhexagon

$(SRC_C_OBJS): | $(OBJ_DIR)

$(SRC_ASM_OBJS): | $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(EXEC) pmu_statsfile.txt

sim:
	$(RUN) $(SIMF) $(EXEC)
	$(POST)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I../include -c -o $@ $<

$(OBJ_DIR):
	mkdir $(OBJ_DIR)
