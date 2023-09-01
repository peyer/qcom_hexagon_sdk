#------------------------------------------------------------------------------
# Copyright (c) 2015-2016 Qualcomm Technologies, Inc.  All Rights Reserved
#------------------------------------------------------------------------------

ifeq ($(OS),Windows_NT)
  ARCH                  := win
  EXE                   := .exe
else # Linux
  ARCH                  := lnx
  EXE                   :=
endif

# Hexagon Tools
CC                      := hexagon-clang
SIM                     := hexagon-sim
PROF                    := hexagon-profiler


# Input Files
SRC                     := mandelbrot.c


# Build Options
Q6_COMPILER             ?= v62
Q6_TARGET               ?= v62h_1536
OPTIMIZE_STALL          ?= 0
OPT_FLAGS               := -O2 -g
CPPFLAGS                := -m$(Q6_COMPILER) -DOPTIMIZE_STALL=$(OPTIMIZE_STALL)
CFLAGS                  := $(OPT_FLAGS)


# Output Files
_ELF                    := $(SRC:.c=$(OPTIMIZE_STALL).elf)
_PKT_JSON               := pkt_profile$(OPTIMIZE_STALL).json
_PKT_HTML               := mandelbrot$(OPTIMIZE_STALL).html
_PMU_TXT                := pmu_statsfile$(OPTIMIZE_STALL).txt


# Output File Locations
ELF_DIR                 := $(Q6_COMPILER)/
RUN_DIR                 := $(Q6_TARGET)/
DIRS                    := $(ELF_DIR) $(RUN_DIR)

ELF                     := $(ELF_DIR)$(_ELF)
PKT_JSON                := $(RUN_DIR)$(_PKT_JSON)
PKT_HTML                := $(RUN_DIR)$(_PKT_HTML)
PMU_TXT                 := $(RUN_DIR)$(_PMU_TXT)


# Build Rules
all: cleanall html html_stallopt

$(ELF):: $(ELF_DIR)

$(ELF):: $(SRC)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< -lhexagon

$(PKT_JSON):: $(ELF)
	$(SIM) -m$(Q6_TARGET) --bypass_idle --timing --packet_analyze $@ --pmu_statsfile $(PMU_TXT) $<

$(PKT_HTML): $(PYTHON_SRC) $(PKT_JSON) $(ELF)
	$(PROF) --packet_analyze --json=$(PKT_JSON) --elf=$(ELF) -o $@


$(DIRS):
	mkdir -p $@


$(PKT_JSON):: $(RUN_DIR)

# Targets that cannot be built
Makefile makefile makefile.example $(SRC) $(PYTHON_SRC): ;


# Convenience targets
.PHONY: all clean cleanall html html_stallopt json vars

elf:  $(ELF_DIR) $(ELF)

html: $(RUN_DIR) $(PKT_HTML)

html_stallopt:
	OPTIMIZE_STALL=1 $(MAKE) -f $(firstword $(MAKEFILE_LIST)) html

json: $(RUN_DIR) $(PKT_JSON)

vars:
	@echo ELF_DIR=$(ELF_DIR)
	@echo RUN_DIR=$(RUN_DIR)
	@echo DIRS=$(DIRS)
	@echo ELF=$(ELF)
	@echo PKT_JSON=$(PKT_JSON)
	@echo PKT_HTML=$(PKT_HTML)
	@echo PMU_TXT=$(PMU_TXT)

clean:
	rm -f $(PMU_TXT)
	@rm -f pmu_stats.txt

cleanall: clean
	rm -f $(PKT_HTML) $(PKT_JSON) $(ELF)
	@if [ "$(strip $(DIRS))" != "" ]; \
	then \
	  for i in $(DIRS); \
	  do \
	    if [ -d $$i -a "$$i" != "." -a "$$i" != ".." ]; \
	    then \
	      echo "rmdir -p --ignore-fail-on-non-empty $$i"; \
	            rmdir -p --ignore-fail-on-non-empty $$i; \
	    fi; \
	    done; \
	fi;
