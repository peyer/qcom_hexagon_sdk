QHL_DIR = $(HEXAGON_SDK_ROOT)/libs/common/qhl
QHL_DIR_BIN = $(QHL_DIR)/bin
QHL_INC_DIRS := $(QHL_DIR)/inc/qhmath $(QHL_DIR)/inc/qhcomplex $(QHL_DIR)/inc/qhdsp $(QHL_DIR)/inc/qhblas $(QHL_DIR)/inc/internal
QHL_LIBS = $(QHL_DIR_BIN)/libqhdsp.a $(QHL_DIR_BIN)/libqhcomplex.a $(QHL_DIR_BIN)/libqhmath.a $(QHL_DIR_BIN)/libqhblas.a

V = 66

ifndef HEXAGON_TOOLS_ROOT
HEXAGON_TOOLS_ROOT = $(DEFAULT_HEXAGON_TOOLS_ROOT)
endif
 
CC = $(HEXAGON_TOOLS_ROOT)/Tools/bin/hexagon-clang

CFLAGS = -mv$(V) -fpic -O2 -G0 -g -Wall -Werror -c -Wno-cast-align -Wpointer-arith -Wno-missing-braces -Wno-strict-aliasing -fno-exceptions -fno-strict-aliasing -fno-zero-initialized-in-bss -fdata-sections -D__V_DYNAMIC__ -Wstrict-prototypes -Wnested-externs
LDFLAGS = -shared -mv$(V) -G0 -Wl
INCLUDES = $(addprefix -I,$(QHL_INC_DIRS))


all: $(QHL_LIBS) qhl_example.so

$(QHL_DIR)/bin/%.a:
	$(MAKE) -C $(QHL_DIR)

%.o: %.c
	$(CC) -fPIC $(CFLAGS) $(INCLUDES) $<  -o $@

%.so: %.o
	$(CC) $(LDFLAGS) -o $@ $< $(QHL_LIBS)

clean:
	rm -rf *.o *.so
