#/* ======================================================================= */
#/*  QUALCOMM TECHNOLOGIES, INC.                                            */
#/*                                                                         */
#/*  Halide Benchmarks/Examples in Device-standalone mode                   */
#/*                                                                         */
#/* ----------------------------------------------------------------------- */
#/*          Copyright (c) 2016 QUALCOMM TECHNOLOGIES Incorporated.         */
#/*                           All Rights Reserved.                          */
#/*                  QUALCOMM Confidential and Proprietary                  */
#/* ======================================================================= */

include ../Makefile.common

HALIDE_TEST_TOP_DIR ?= $(shell pwd)/../../..
DEVICE_STANDALONE_TEST_TOP_DIR ?= $(HALIDE_TEST_TOP_DIR)/standalone/device
DEVICE_STANDALONE_UTILS_DIR = $(DEVICE_STANDALONE_TEST_TOP_DIR)/utils
BLD_TOP ?= $(DEVICE_STANDALONE_TEST_TOP_DIR)/build
UTILS_BLD_TOP ?= $(BLD_TOP)/standalone/device/utils
UTILS_BIN ?= $(UTILS_BLD_TOP)/bin

ARMCC=$(ANDROID_ARM64_TOOLCHAIN)/bin/aarch64-linux-android21-clang
ARMCXX=$(ANDROID_ARM64_TOOLCHAIN)/bin/aarch64-linux-android21-clang++

ARM_INCLUDES = $(COMMON_INCLUDES) -I$(HEXAGON_SDK_ROOT)/libs/common/rpcmem/inc -I$(HEXAGON_SDK_ROOT)/libs/common/remote/ship/android_Release
ARM_CFLAGS= --sysroot $(ANDROID_NDK_ROOT)/platforms/android-21/arch-arm64 $(ARM_INCLUDES) -fsigned-char
ARM_LDFLAGS = -llog -fPIE -pie -l$(DSPTYPE)rpc -L$(HEXAGON_SDK_ROOT)/libs/common/remote/ship/android_Release_aarch64

.PHONY : all clean

all: $(UTILS_BIN)/libion_allocation.a

$(UTILS_BIN)/ion_allocation.o: $(DEVICE_STANDALONE_UTILS_DIR)/ion/ion_allocation.cpp $(DEVICE_STANDALONE_UTILS_DIR)/ion/ion_allocation.h
	mkdir -p $(@D)
	$(ARMCXX) $(ARM_CFLAGS) -O3  -c $< -o $@

$(UTILS_BIN)/libion_allocation.a: $(UTILS_BIN)/ion_allocation.o
	ar q $@ $^

clean:
	rm -rf $(UTILS_BIN)
