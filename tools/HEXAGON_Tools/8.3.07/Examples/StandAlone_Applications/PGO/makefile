##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

CXX	= hexagon-clang++
AS	= hexagon-llvm-mc -filetype=obj
SIM	= hexagon-sim
CFLAGS	= -O2 -g
Q6VERSION	:= v60

CXXFLAGS += $(CXXF) -m${Q6VERSION} -std=c++03
ASFLAGS	+= -m${Q6VERSION}
SIMFLAGS+= -m${Q6VERSION}

OBJS = particle.o

all: clean build_std sim_std build_instr sim_instr convert build_pgo sim_pgo

# This is a standard run for comparison
build_std: particle.cpp
	$(CXX) $(CXXFLAGS) particle.cpp -o particle_std -lhexagon

sim_std:
	$(SIM) $(SIMFLAGS) particle_std

# This is the instrumented run
build_instr: particle.cpp
	$(CXX) $(CXXFLAGS) -fprofile-instr-generate particle.cpp -o particle_instr  -lhexagon

sim_instr:
	$(SIM) $(SIMFLAGS) particle_instr > instr.txt 2>&1

# Convert to llvm format
convert:
	hexagon-llvm-profdata merge default.profraw -o code.profdata

# This is the pgo result
build_pgo:
	$(CXX) $(CXXFLAGS) -fprofile-instr-use=code.profdata -mllvm -disable-block-placement=false -mllvm -whole-function-placement=true particle.cpp -o particle_pgo  -lhexagon

sim_pgo:
	$(SIM) $(SIMFLAGS) particle_pgo

clean:
	rm -f particle $(OBJS) *.txt *.map particle_* code.profdata default.profraw

