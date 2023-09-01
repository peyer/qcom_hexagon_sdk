###########################################################################
# Copyright (c) Date: Mon Nov 24 16:25:55 CST 2008 QUALCOMM INCORPORATED
# All Rights Reserved
# Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:55 CST 2008
###########################################################################


PROGRAM = fft
EXEC = $(PROGRAM).elf
ASM_SRC_DIR = ../asm_src
SRC_DIR = ../src
OBJ_DIR = ./build
INCLUDES = -I../../../include -I../include
Q6VERSION = v66

#---------------------------------------------------------#
#  Usage "make SRC=C" if use C-based code                 #
#---------------------------------------------------------#

ifeq ($(SRC),C)
_C_SRCS   = test_$(PROGRAM).c \
            genFFTtwiddle.c   \
            $(PROGRAM).c

_ASM_SRCS =
else
_C_SRCS   = test_$(PROGRAM).c \
            genFFTtwiddle.c

_ASM_SRCS = $(PROGRAM).S

endif

SRC_C_OBJS   = $(_C_SRCS:%.c=$(OBJ_DIR)/%.o)
SRC_ASM_OBJS = $(_ASM_SRCS:%.S=$(OBJ_DIR)/%.obj)

CC = hexagon-clang
CFLAGS = -Wall -O2 -m$(Q6VERSION) $(INCLUDES)

LD = hexagon-clang
LDFLAGS = -m$(Q6VERSION)
LLIBS = -lhexagon

RM = rm -Rf

ifneq ($(SRC),C)
CFLAGS += -DQDSP6_ASM
endif

all:
ifeq ($(shell expr $(Q6VERSION) \< v62), 1)
	@echo Q6VERSION must be v62 or greater to run this test
else
all: clean $(EXEC) sim
endif

$(EXEC): $(OBJ_DIR) $(SRC_C_OBJS) $(SRC_ASM_OBJS)
	$(LD) $(LDFLAGS) -o $@ $(SRC_C_OBJS) $(SRC_ASM_OBJS) $(LLIBS)

clean:
	$(RM) $(OBJ_DIR) $(EXEC) stats.txt pmu_statsfile.txt

sim: $(EXEC)
	hexagon-sim -m$(Q6VERSION) -- $(EXEC)

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: %.c
ifeq ($(shell expr $(Q6VERSION) \< v61), 1)
	@echo Q6VERSION must be v61 or greater to run this test
	exit
endif
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.obj: $(ASM_SRC_DIR)/%.S
	$(CC) $(CFLAGS) -c -o $@ $<


