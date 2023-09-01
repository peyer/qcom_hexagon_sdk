########################################################################
# Copyright (c) Date: Tue Aug 26 16:38:42 CDT 2008 QUALCOMM INCORPORATED 
# All Rights Reserved 
# Modified by QUALCOMM INCORPORATED on Tue Aug 26 16:38:42 CDT 2008 
########################################################################

Q6VERSION=v60
AS=hexagon-llvm-mc -filetype=obj
ASFLAGS=-m$(Q6VERSION)
CC=hexagon-clang
CFLAGS=-m$(Q6VERSION)
LD=hexagon-link

.PHONY: all clean

all: falign.o falign_limitations.o
	hexagon-llvm-objdump -d falign.o
	hexagon-llvm-objdump -d falign_limitations.o

clean:
	${RM} *.o
