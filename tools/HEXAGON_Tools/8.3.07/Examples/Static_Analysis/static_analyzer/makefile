#################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
#################################################################

CXX = hexagon-clang++
POST_PROCESS = post-process
Q6VERSION = v60
REPORT_DIR = report-dir
CXXFLAGS = -c -m$(Q6VERSION) --compile-and-analyze $(REPORT_DIR)
OBJS = test.o
SRC = test.cpp

all: clean run

run:
	$(CXX) $(CXXFLAGS) -o $(OBJS) $(SRC);

clean:
	rm -rf *.o $(REPORT_DIR)
