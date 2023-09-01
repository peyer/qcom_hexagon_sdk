

PREFIX    = hexagon
Q6VERSION = v60
SYSTEM := $(shell uname)

ifeq ($(OS),Windows_NT)
        TOOL_PATH = $(shell dirname `where hexagon-clang`)/../..
else
        TOOL_PATH = $(shell dirname `which hexagon-clang`)/../..
endif


CC  = hexagon-clang -m$(Q6VERSION) $(CFLAGS) 
LD  = $(PREFIX)-link
SIM = $(PREFIX)-sim

CFLAGS += -g -O0 -G0

ARCHDIR=$(Q6VERSION)

#
# Librarys from the toolchain
#
CRT0_STANDALONE=$(shell $(CC) -G0 -print-file-name=crt0_standalone.o)
CRT0           =$(shell $(CC) -G0 -print-file-name=crt0.o)
INIT           =$(shell $(CC) -G0 -print-file-name=init.o)
LIB_STANDALONE =$(shell $(CC) -G0 -print-file-name=libstandalone.a)
LIB_C          =$(shell $(CC) -G0 -print-file-name=libc.a)
LIB_GCC        =$(shell $(CC) -G0 -print-file-name=libgcc.a)
FINI           =$(shell $(CC) -G0 -print-file-name=fini.o)
LIBDL          =$(TOOL_PATH)/Tools/target/hexagon/lib/$(ARCHDIR)/G0/libdl.a


DSO = liba.so libb.so

all: run

dlopen: $(DSO) dlopen.c
	$(CC) -c dlopen.c
	$(LD) -o $@ $(CRT0_STANDALONE) $(CRT0) $(INIT) dlopen.o $(LIBDL) --start-group  $(LIB_STANDALONE) --whole-archive $(LIB_C) --no-whole-archive $(LIB_GCC) --end-group $(FINI) --dynamic-linker= -E --force-dynamic


run: dlopen 
	hexagon-sim  $<

liba.so: liba.c
	$(CC) -fPIC -shared -nostartfiles  $< -o $@
libb.so: libb.c
	$(CC) -fPIC -shared -nostartfiles  $< -o $@


clean:
	rm -f $(DSO) *.o dlopen pmu_statsfile.txt stats.txt
