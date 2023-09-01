COSIM_NAME	= SHFcosim
TESTFILE	= shf

Q6VERSION= v60

SIM_PATH 	= $(shell dirname `which hexagon-sim`)
HEX_INCLUDE = $(SIM_PATH)/../include/iss
HEX_LIB_PATH= $(SIM_PATH)/../lib/iss
BIN_DIR		= ./bin/linux
TEST_DIR	= ./Test_program
COMMON_DIR	= ./common
COSIM_DIR	= ./cosim

CC              = hexagon-clang

all: clean makedirs $(COSIM_NAME) $(TESTFILE) cosim.cfg run

.PHONY:
run: $(COSIM_NAME) $(TESTFILE) cosim.cfg
	hexagon-sim --timing --cosim_file $(BIN_DIR)/cosim.cfg $(BIN_DIR)/$(TESTFILE)_test

cosim.cfg:
	@echo $(BIN_DIR)/$(COSIM_NAME).so > $(BIN_DIR)/$@

$(COSIM_NAME):
	$(CXX) -fpic -O3 -I$(HEX_INCLUDE) -I$(COMMON_DIR) -L$(HEX_LIB_PATH) -shared -o $(BIN_DIR)/$@.so $(COSIM_DIR)/$@.cpp

$(TESTFILE): 
	$(CC) -m$(Q6VERSION) -I$(COMMON_DIR) $(TEST_DIR)/$(TESTFILE).c -o $(BIN_DIR)/$@_test

makedirs:
	mkdir -p $(BIN_DIR) 

clean:
	rm -rf $(BIN_DIR) pmu_statsfile.txt stats.txt 
