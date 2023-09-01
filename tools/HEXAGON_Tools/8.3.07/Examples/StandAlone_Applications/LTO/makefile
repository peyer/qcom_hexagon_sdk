##################################################################
# Copyright (c) $Date: 2012/03/07 01:00:01 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2012/03/07 01:00:01 $
##################################################################

all: clean main_lto newline main newline read_main newline read_main_lto

CC  = hexagon-clang

Q6VERSION := v60

CFLAGS = -m${Q6VERSION} -O2 -g -G0 -Wall

OBJS	=  file1.o main.o
BC_OBJS =  file1.bc main.bc

main_lto: $(BC_OBJS)
	$(CC) $(CFLAGS) $(BC_OBJS) -flto -o main_lto

main: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o main

%.bc : %.c
	$(CC) $(CFLAGS) -flto -c $< -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

newline:
	@echo

read_main:
	@echo
	@echo Without LTO, all the foo functions are kept
	hexagon-readelf -s main | grep -w "foo[0-9]\|main" | grep -v main.c
	@echo

read_main_lto:
	@echo
	@echo With LTO, all the foo functions are removed
	hexagon-readelf -s main_lto | grep -w "foo[0-9]\|main"
	@echo

clean:
	rm -fr *.o *.bc main pmu*.txt main_lto
	@echo

