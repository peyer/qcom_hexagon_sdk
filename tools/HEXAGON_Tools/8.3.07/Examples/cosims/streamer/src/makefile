#################################################################
# Copyright (c) \$Date\$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on \$Date\$
##################################################################

SYSTEM := $(shell uname)
ifeq (Linux, $(findstring Linux,$(SYSTEM)))
##################################################
# Top level makefile
##################################################

	Q6VERSION =v60
	COSIMNAME = Streamer
	TEST = memcpy_v60
	TCM_BASE = 0xd8000000
##################################################
STREAMER_BASE = 0xd0000000                      ##
## WARNING! Changing the streamer base requires ##
## a corresponding change in the cosim.cfg file.##
##################################################
	INPUT := bayer_3k_x_2k.in
	OUTPUT := bayer.out
	SIZE = 3072 2048

	SIM = hexagon-sim
	RELEASE_DIR := $(shell which ${SIM})
	RELEASE_DIR:=$(shell dirname ${RELEASE_DIR})/..
	INCLUDE_DIR := $(RELEASE_DIR)/include/iss/
	INPUT_FILE := bayer_3k_x_2k.in
	OUTPUT_FILE := bayer.out
	ARCH    := $(shell arch)
	SRC := $(shell pwd)
	SRC_TOP := $(SRC)/..
	EXEDIR := $(SRC_TOP)/bin
	BUILDDIR := $(SRC_TOP)/build
	# Compiler to use
	# g++ must be g++-4.1.2 or later. Please set LD_LIBRARY_PATH to point to
	# the g++ distribution lib dir (typically $(g++-4.1.2-or-later-install-dir)/lib64
	# for linux64 and $(g++-4.1.2-or-later-ins all-dir)/lib for linux32)
	CC_COSIM := g++
	AR := ar
	DLLIB = -ldl
	O_EXT := o
	O_OUTPUT := -o
	CFLAGS_COSIM += -fPIC
	ifeq ($(ARCH), x86_64)
		CFLAGS_COSIM += -fomit-frame-pointer -ffast-math
		PLATDIR=lnx64
	else
		CFLAGS_COSIM += -fomit-frame-pointer -ffast-math -march=pentium3
		PLATDIR=lnx32
	endif
	OBJDIR := $(SRC_TOP)/build/$(PLATDIR)
	xOBJDIR := $(OBJDIR)
	BM_DIR := $(SRC_TOP)/src
 
	CFLAGS_COSIM += -I./ -Wall
	DBG_CFLAGS := -g
	NDBG_CFLAGS := -O4
	OPT_CFLAGS=$(NDBG_CFLAGS)
	BINDIR := $(SRC_TOP)/bin/$(PLATDIR)
	LIBSUFF := so
	LIBBM = $(BINDIR)/$(COSIMNAME).$(LIBSUFF)
	TEST_DIR := $(SRC_TOP)/test/memcpy_test

	INPUT_DIR  = $(SRC_TOP)/input
	OUTPUT_DIR = $(SRC_TOP)/output
	SIMFLAGS  = --simulated_returnval --cosim_file cosim.cfg -m$(Q6VERSION) --timing --tcm:lowaddr $(TCM_BASE) $(BINDIR)/$(TEST) -- $(SIZE) $(INPUT_DIR)/$(INPUT) $(OUTPUT_DIR)/$(OUTPUT)

	CFLAGS_COSIM += -I$(INCLUDE_DIR) -I$(BM_DIR)

	ALLCFILES = $(wildcard *c)
	DFILES = $(patsubst %,$(OBJDIR)/%,$(ALLCFILES:.c=.d))
	-include $(DFILES)
	OFILES = $(patsubst %,$(OBJDIR)/%,$(ALLCFILES:.c=.$(O_EXT)))
endif
.PHONY: clean

ifeq (Linux, $(findstring Linux,$(SYSTEM)))
all: clean mini_all sim
else
all:
	./make.cmd
endif

sim:
ifeq (Linux, $(findstring Linux,$(SYSTEM)))
	@echo
	@echo NOTE: Simulation may take up to 2 minutes ...
	@echo
	$(SIM) $(SIMFLAGS)
else
	./make.cmd sim
endif

mini_all: makedirs $(LIBBM) build

debug:
ifeq (Linux, $(findstring Linux,$(SYSTEM)))
	$(MAKE) -C . OPT_CFLAGS="$(DBG_CFLAGS)" BINDIR=$(BINDIR) OBJDIR=$(OBJDIR) mini_all
	$(SIM) $(SIMFLAGS)
endif

build:
ifeq (Linux, $(findstring Linux,$(SYSTEM)))
	$(MAKE) -C $(TEST_DIR) $(MAKE_ARGS) TCM_BASE=$(TCM_BASE) STREAMER_BASE=$(STREAMER_BASE)
	cp $(TEST_DIR)/../build/$(TEST) $(BINDIR)/$(TEST)
	mkdir -p $(INPUT_DIR)
	mkdir -p $(OUTPUT_DIR)
	unzip $(TEST_DIR)/../input/bayer_3k_x_2k.zip -d $(INPUT_DIR)
	sed "s~Streamer~$(BINDIR)/Streamer~" $(TEST_DIR)/cosim.cfg > cosim.cfg;
else
	./make.cmd build
endif

makedirs:
ifeq (Linux, $(findstring Linux,$(SYSTEM)))
	mkdir -p $(BINDIR) $(OBJDIR);
endif

$(OBJDIR)/%.$(O_EXT): %.c
	$(CC_COSIM) $(OPT_CFLAGS) $(CFLAGS_COSIM) -g -c $< $(O_OUTPUT)$@

$(LIBBM): $(OFILES)
	$(CC_COSIM) -shared  -o $@ $+

clean:
ifeq (Linux, $(findstring Linux,$(SYSTEM)))
	rm -rf $(EXEDIR) $(BUILDDIR) $(DFILES) $(INPUT_DIR) $(OUTPUT_DIR) cosim.cfg *.txt *.out*
	$(MAKE) -C $(TEST_DIR) $(MAKE_ARGS) clean
else
	./make.cmd clean
endif

check:
	$(TEST_DIR)/interface_to_img.py --mask 0x3f $(OUTPUT_DIR)/bayer.out.0 $(OUTPUT_DIR)/bayer.out0.pix
	$(TEST_DIR)/interface_to_img.py --mask 0x3f $(OUTPUT_DIR)/bayer.out.1 $(OUTPUT_DIR)/bayer.out1.pix
	$(TEST_DIR)/interface_to_img.py --mask 0x3f $(INPUT_DIR)/bayer_3k_x_2k.in $(OUTPUT_DIR)/bayer_3k_x_2k.pix
	diff -b $(OUTPUT_DIR)/bayer_3k_x_2k.pix $(OUTPUT_DIR)/bayer.out0.pix
	diff -b $(OUTPUT_DIR)/bayer_3k_x_2k.pix $(OUTPUT_DIR)/bayer.out1.pix
	test $$? -eq 0
