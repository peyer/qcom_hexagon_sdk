########################################################################
# Copyright (c) Date: Tue Aug 26 16:38:42 CDT 2008 QUALCOMM INCORPORATED 
# All Rights Reserved 
# Modified by QUALCOMM INCORPORATED on Tue Aug 26 16:38:42 CDT 2008 
########################################################################


PROGRAM = test_cxfir
Q6VERSION = -mv60
EXEC = $(PROGRAM)

ASM_SRC_DIR = ../asm_src
SRC_DIR = ../src
INCLUDES = -I../../../include -I../include

include ../../../makefile

#---------------------------------------------------------#
#  Usage "make SRC=C" if use C-based code                 #
#---------------------------------------------------------#

ifeq ($(SRC),C)
  VPATH = $(SRC_DIR)
  _C_SRCS = $(PROGRAM).c \
            cxfir.c
  _ASM_SRCS =  
else
  ifeq ($(Q6VERSION),-mv2)
    $(error ERROR: Attempting to build v3 code for a v2 architecture)
  endif
  VPATH = $(ASM_SRC_DIR)
  _C_SRCS = $(PROGRAM).c
  _ASM_SRCS = cxfir.S
endif

OBJS   = $(_C_SRCS:%.c=%.o) $(_ASM_SRCS:%.S=%.o)

CC = hexagon-clang
CCF = -O3
CFLAGS = -Wall $(CCF) $(Q6VERSION) $(INCLUDES) 
ASFLAGS = -Wall $(CCF) $(Q6VERSION) $(INCLUDES) 

LD = $(CC) $(CCF)
LDFLAGS = $(Q6VERSION)
LLIBS = -lhexagon

RUN = hexagon-sim

RM = rm -rf


$(EXEC): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LLIBS) 


clean: 
	$(RM) *.o $(PROGRAM) pmu_statsfile.txt stats.txt


sim: $(EXEC)
	$(RUN) $(EXEC) 

