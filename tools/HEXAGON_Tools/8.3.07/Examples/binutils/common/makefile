########################################################################
# Copyright (c) Date: Tue Aug 26 16:38:42 CDT 2008 QUALCOMM INCORPORATED 
# All Rights Reserved 
# Modified by QUALCOMM INCORPORATED on Tue Aug 26 16:38:42 CDT 2008 
########################################################################


AS=hexagon-llvm-mc -filetype=obj
CC=hexagon-clang
LD=hexagon-link

.PHONY: all clean

all: common0 common16

clean:
	${RM} common0 common16 *.o

common0: common.o
	${LD} -G0 -o $@ $^

common16: common.o
	${LD} -G16 -o $@ $^
