#
#  Makefile for range_iterator c++ example
#  Please note that this example works for
#  hexagon tools versions 8.0 and higher.
#

Q6VERSION = v60
TEST = range_iterator

CXX = hexagon-clang++
SIM = hexagon-sim

CXXFLAGS = -m$(Q6VERSION) -g -std=c++11 -stdlib=libc++
SIM_FLAGS = --timing -m$(Q6VERSION)

all: clean build sim

build:
	$(CXX) $(CXXFLAGS) $(TEST).cpp -o $(TEST).elf

clean:
	rm -f pmu_stat* *.elf *.o *.out

sim:
	$(SIM) $(SIM_FLAGS) $(TEST).elf
