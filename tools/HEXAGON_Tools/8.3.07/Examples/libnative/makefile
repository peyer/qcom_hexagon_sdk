#
# Makefile for libnative example
#
# gcc options- -Wall = enable all warnings
#            - -g add debug symbols
#            - -I include folder path
#            - -L library folder path
#            - -l link libnative.a and libmath
#            - -o output file name
#
# cl options - /MD = place MSVCRT.lib into .obj file,
#            - /IC = include following files,
#            - /link = invoke linker and add following file,
#            - /out = generate output file.

PROGRAM = intrinsics
EXE = $(PROGRAM).exe
INCLUDE_DIR := $(RELEASE_DIR)/include/iss/
SYSTEM := $(shell uname)

ifeq (Linux, $(findstring Linux,$(SYSTEM)))
	RELEASE_DIR := $(shell dirname `which hexagon-sim`)/..
	#RELEASE_DIR:=$(shell dirname ${RELEASE_DIR})/..
	ARCH := $(shell arch)
	NM = nm -A $(RELEASE_DIR)/libnative/lib/libnative.a > libnative-calls.txt
	CC_LIB := gcc
	ifeq ($(ARCH), x86_64)
		CFLAGS_LIB += -fomit-frame-pointer -ffast-math
		PLATDIR=lnx64
	else
		CFLAGS_LIB += -fomit-frame-pointer -ffast-math -march=pentium3
		PLATDIR=lnx32
	endif
	CFLAGS_LIB += -Wall -o intrinsics -g -I./ -I$(RELEASE_DIR)/libnative/include $(PROGRAM).c  -L$(RELEASE_DIR)/libnative/lib -lnative -lm
else
	NM = dumpbin /ALL /OUT:libnative-calls.txt $(RELEASE_DIR)/libnative/lib/libnative.a
endif

all: clean build run

clean:
ifeq (Linux, $(findstring Linux,$(SYSTEM)))
	rm -f $(EXE) $(PROGRAM).obj $(PROGRAM) libnative-calls.txt
else
	./make.cmd clean
endif

build:
ifeq (Linux, $(findstring Linux,$(SYSTEM)))
	$(CC_LIB) $(CFLAGS_LIB)
else
	./make.cmd
endif

run:
ifeq (Linux, $(findstring Linux,$(SYSTEM)))
	./intrinsics
endif

list:
	$(NM)