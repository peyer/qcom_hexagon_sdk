#################################################################
# Copyright (c) \$Date\$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on \$Date\$
##################################################################

# 
# Top level makefile
# 

target: all
INSTALLPREFIX := /usr/local/hexagon/qc
Q6VERSION=v60

CC = hexagon-clang
COSIMNAME = Bus
RUN = hexagon-sim
RELEASE_DIR:=$(shell which ${RUN})
RELEASE_DIR:=$(shell dirname ${RELEASE_DIR})/../..
INCLUDE_DIR := $(RELEASE_DIR)/Tools/include/iss/


SYSTEM := $(shell uname)
ARCH := $(shell uname -m)
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
CFLAGS_COSIM += -fPIC -Wno-write-strings -fomit-frame-pointer -ffast-math
PLATDIR=lnx64

TEST_DIR := $(SRC_TOP)/test/
OBJDIR := $(SRC_TOP)/build/$(PLATDIR)
xOBJDIR := $(OBJDIR)
BM_DIR := $(SRC_TOP)/src

CFLAGS_COSIM += -I./ -Wall
MDCFLAGS := $(CFLAGS_COSIM)
DBG_CFLAGS := -g -DDEBUG
NDBG_CFLAGS := -O4
BINDIR := $(SRC_TOP)/bin/$(PLATDIR)
LIBSUFF := so

ifneq (Linux, $(findstring Linux,$(SYSTEM)))
	ARCH=
	DLLIB = 
	O_EXT := obj
	O_OUTPUT := /Fo
	PLATDIR=win

	CURR_DIR := $(shell pwd)
	TMP_CURR_DIR := $(shell cygpath -w ${CURR_DIR})
	
	SRC_DIR := $(shell echo "$(TMP_CURR_DIR)" | sed 's/\\/\//g')
	TMP_SRC_DIR := $(shell echo $(SRC_DIR) | sed 's,^/cygdrive/\(.\),\1:,g')
	SRC_TOP := $(TMP_SRC_DIR)/..
	EXEDIR := $(SRC_TOP)/bin
	BUILDDIR := $(SRC_TOP)/build
	# Compiler to use
	CC_COSIM := cl
	CFLAGS_COSIM := /nologo /Zm1000 /EHsc /D "WIN32" -DVCPP -DLITTLE_ENDIAN  /TP /I.
	LINKFLAGS := /nologo /dll libwrapper.lib
	LIBPATH := $(RELEASE_DIR)/Tools/lib/iss/
	DBG_CFLAGS := /MTd /Od /Gm /ZI
	NDBG_CFLAGS := /MT /O2
	MDCFLAGS := -DWIN32 -DVCPP -DLITTLE_ENDIAN -I.
	TEST_DIR := $(SRC_TOP)/test/
	OBJDIR := $(SRC_TOP)/build/$(PLATDIR)
	BM_DIR := $(SRC_TOP)/src
	AR := ar
	BINDIR := $(SRC_TOP)/bin/$(PLATDIR)
	LIBSUFF := dll
endif

TEST := hello

LIBBM = $(BINDIR)/$(COSIMNAME).$(LIBSUFF)


ifeq (Linux, $(findstring Linux,$(SYSTEM)))
	CFLAGS_COSIM += -I$(INCLUDE_DIR) -I$(BM_DIR)
	MDCFLAGS += -I$(INCLUDE_DIR) -I$(BM_DIR)
else
	CFLAGS_COSIM += /I`cygpath -w $(INCLUDE_DIR)` /I$(BM_DIR)
	MDCFLAGS += -I$(INCLUDE_DIR) -I$(BM_DIR) 
endif


ALLCFILES = $(wildcard *c)
DFILES = $(patsubst %,$(OBJDIR)/%,$(ALLCFILES:.c=.d))
-include $(DFILES)
OFILES = $(patsubst %,$(OBJDIR)/%,$(ALLCFILES:.c=.$(O_EXT)))

COSIM_FILE = newcosimCfg

all: clean mini_all
	#make -C . OPT_CFLAGS="$(NDBG_CFLAGS)" mini_all
ifeq (Linux, $(findstring Linux,$(SYSTEM)))
	${RUN} --cosim_file newcosimCfg --timing --m$(Q6VERSION) ${BINDIR}/${TEST}
else
	${RUN} --cosim_file newCygcosimCfg --timing --m$(Q6VERSION) ${BINDIR}/${TEST}
endif

mini_all: makedirs $(LIBBM) app cosimFile

debug:
	make -C . OPT_CFLAGS="$(DBG_CFLAGS)" BINDIR=$(BINDIR)/debug OBJDIR=$(OBJDIR)/debug mini_all

cosimFile:
	sed i${BINDIR}/ ${TEST_DIR}/cosimCfg > newCfg;\
	sed '$!N;s/\n//' newCfg > newcosimCfg;\
	rm -f newCfg;
ifneq (Linux, $(findstring Linux,$(SYSTEM)))
	sed 's/\.so/\.dll/' newcosimCfg > newCygcosimCfg;\
	rm -f newcosimCfg;
endif

app:
	$(CC) -m${Q6VERSION} -o ${BINDIR}/${TEST} ${TEST_DIR}/${TEST}.c

makedirs:
	mkdir -p $(BINDIR) $(OBJDIR);

$(OBJDIR)/%.$(O_EXT): %.c
ifeq (Linux, $(findstring Linux,$(SYSTEM)))
	$(CC_COSIM) $(OPT_CFLAGS) $(CFLAGS_COSIM) -g -c $< $(O_OUTPUT)$@
else
	"`cygpath -u "$(VCINSTALLDIR)\\bin\\$(CC_COSIM).exe"`" $(OPT_CFLAGS) $(CFLAGS_COSIM) -c $< $(O_OUTPUT)$@
endif

ifeq (Linux, $(findstring Linux,$(SYSTEM)))
$(LIBBM): $(OFILES)
	$(CC_COSIM) -shared  -o $@ $+
else
$(LIBBM): $(OFILES)
	"`cygpath -u "$(VCINSTALLDIR)\\Bin\\link.exe"`" `cygpath -w $(OFILES)` $(LINKFLAGS) /libpath:"`cygpath -w $(LIBPATH)`"  /out:$(LIBBM)
endif

install:  $(LIBBM)
	mkdir -p $(INSTALLPREFIX)/lib/iss/$(Q6VERSION)

clean:
	rm -rf $(EXEDIR) $(BUILDDIR) $(DFILES) newCfg newcosimCfg newCygcosimCfg pmu_statsfile.txt stats.txt
