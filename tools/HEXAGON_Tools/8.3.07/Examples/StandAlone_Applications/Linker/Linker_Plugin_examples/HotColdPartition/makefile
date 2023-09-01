#################################################################
## Copyright (c) \$Date\$ QUALCOMM INCORPORATED.
## All Rights Reserved.
## Modified by QUALCOMM INCORPORATED on 1/12/2018
##################################################################

all: clean pluginlib link show


PLUGIN_NAME = HotColdPartition

# Set the path to the apropriate clang++ here:
PLUGIN_CXX = /pkg/qct/software/llvm/build_tools/llvm501_171216/bin/clang++

# Find the path to the hexagon tools
RUN = hexagon-sim
RELEASE_DIR:=$(shell which ${RUN})
RELEASE_DIR:=$(shell dirname ${RELEASE_DIR})/../..

# Directories for the plugin
PLUGIN_INCLUDE_DIR := $(RELEASE_DIR)/Tools/include/QCLD/PluginAPI
PLUGIN_LIBS_DIR := $(RELEASE_DIR)/Tools/lib

# Folder containing the target files
TARGET_DIR := target

# Hexagon configurations
Q6VERSION = v66
TARGET_CC=hexagon-clang
CFLAGS = -m$(Q6VERSION) -O2 -G0 -fno-inline -ffunction-sections -fdata-sections 

# x86_64 Clang:
plugin:
	@echo
	@echo Compiling the Linker Plugin ...
	$(PLUGIN_CXX) -c -I $(PLUGIN_INCLUDE_DIR) $(PLUGIN_NAME).cpp -std=c++11 -fPIC -stdlib=libc++

pluginlib:plugin
	@echo
	@echo Linking the Linker Plugin ...
	$(PLUGIN_CXX) -shared ./$(PLUGIN_NAME).o  -L $(PLUGIN_LIBS_DIR) -lLW  -z defs -stdlib=libc++ -o lib$(PLUGIN_NAME).so
	@echo
	@echo Compiling the hexagon objects ...
	
# Hexagon Clang:
OBJS = $(TARGET_DIR)/text_sw.o $(TARGET_DIR)/compress.o $(TARGET_DIR)/main.o

%.o:%.c 
	$(TARGET_CC) $(CFLAGS) -c $< -o $@

link: $(OBJS)
	@echo
	@echo Linking the hexagon executable and invoking the plugin ...
	$(TARGET_CC) -m$(Q6VERSION) -G0 $(OBJS) -o $(TARGET_DIR)/main.elf -Wl,-T,$(TARGET_DIR)/linker.script,--trace=plugin,-Map,$(TARGET_DIR)/$(PLUGIN_NAME).map

show:
	@echo
	@echo The .text_sw section was partitioned into two sections
	hexagon-readelf -S $(TARGET_DIR)/main.elf | grep -E 'Address|text_sw'
	@echo
	@echo The .compress section was re-ordered to put the cold sections at the end
	hexagon-readelf -s ./target/main.elf | grep comp_
	@echo

sim:
	$(RUN) $(TARGET_DIR)/main.elf

clean:
	@echo
	@echo Cleaning Files ...
	rm -fr *.o $(TARGET_DIR)/*.o *.so $(TARGET_DIR)/*.map $(TARGET_DIR)/*.elf $(TARGET_DIR)/pmu_statsfile.txt
