#################################################################
# Copyright (c) \$Date\$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on \$Date\$
##################################################################

#
# Top level makefile
#
PORT = $(shell awk -v min=9500 -v max=15000 'BEGIN{srand(); print int(min+rand()*(max-min+1))}')
ARCH=60
Q6VERSION = v$(ARCH)
CC = hexagon-clang
SRC_TOP := $(shell pwd)

SYSTEM_NAME = hexagon-system
TARGET := hello
TEST_DIR := $(SRC_TOP)/src
BIN_DIR := $(SRC_TOP)/bin

RELEASE_DIR:=$(shell which hexagon-sim)
RELEASE_DIR:=$(shell dirname ${RELEASE_DIR})/..
INCLUDE_DIR := $(RELEASE_DIR)/include/iss
LIB_DIR := $(RELEASE_DIR)/lib

# Compiler to use
# g++ must be g++-4.1.2 or later. Please set LD_LIBRARY_PATH to point to
# the g++ distribution lib dir (typically $(g++-4.1.2-or-later-install-dir)/lib64
# for linux64 and $(g++-4.1.2-or-later-ins all-dir)/lib for linux32)
CC_SYSTEM := g++
CFLAGS_SYSTEM = -c -fPIC -Wno-write-strings -fomit-frame-pointer -ffast-math
CFLAGS_SYSTEM += -I./ -Wall -g -I$(INCLUDE_DIR) -o $(BIN_DIR)/$(SYSTEM_NAME).obj
LFLAGS_SYSTEM =  -L$(RELEASE_DIR)/lib/iss -l:$(RELEASE_DIR)/lib/iss/libhexagoniss$(Q6VERSION).so -l:$(RELEASE_DIR)/lib/iss/libwrapper.so -ldl -l:$(LIB_DIR)/libedit.so.0
SIMFLAGS =  -m$(Q6VERSION) -G 0 $(BIN_DIR)/$(TARGET).elf
DBGFLAGS = -m$(Q6VERSION) -G $(PORT) $(BIN_DIR)/$(TARGET).elf
CFLAGS = -m$(Q6VERSION) -g

all: clean build sim

clean:
	rm -rf $(BIN_DIR) pmu_statsfile.txt stats.txt

build:
	mkdir -p $(BIN_DIR)
	$(CC_SYSTEM) $(CFLAGS_SYSTEM) $(TEST_DIR)/$(SYSTEM_NAME).cpp
	$(CC_SYSTEM) -o $(BIN_DIR)/$(SYSTEM_NAME) $(BIN_DIR)/$(SYSTEM_NAME).obj $(LFLAGS_SYSTEM)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$(TARGET).elf $(TEST_DIR)/$(TARGET).c
	chmod +x $(BIN_DIR)/$(SYSTEM_NAME)

sim:
	$(BIN_DIR)/$(SYSTEM_NAME) $(SIMFLAGS)

lldb:
	sed -i 's/gdb-remote.*/gdb-remote $(PORT)/' ./src/lldb-setup.txt
	$(BIN_DIR)/$(SYSTEM_NAME) $(DBGFLAGS) &
	xterm -e "hexagon-lldb -s src/lldb-setup.txt" &

t32:
	$(BIN_DIR)/$(SYSTEM_NAME) $(DBGFLAGS) &
	$(T32SYS)/bin/pc_linux64/t32mqdsp6-qt -c cmm/linux.cfg -s cmm/hex.cmm $(PORT) $(ARCH) $(BIN_DIR)/$(TARGET).elf linux_layout
