.EXPORT_ALL_VARIABLES:

CURR_DIR = $(shell pwd)
FEATURES ?= hvx_128
Q6_VERSION ?= 66

ifeq ($(shell expr $(Q6_VERSION) \< 66), 1)
  $(error Q6_VERSION should be at least 66)
endif 


CURR_DIR = $(shell pwd)
BLD_TOP ?= $(CURR_DIR)/../../../..
BLD_DIR ?= $(BLD_TOP)/standalone/simulator/apps/dmatest
BIN ?= $(BLD_DIR)/bin
SIMULATOR_STANDALONE_TOP_DIR = $(CURR_DIR)/../../
TEST_INCLUDE = $(SIMULATOR_STANDALONE_TOP_DIR)/include

.PHONY: all

all: clean run 

clean:
	rm -rf $(BIN)

run: 
	$(CURR_DIR)/test-dma

prerequisites: run

dmatest: prerequisites 
