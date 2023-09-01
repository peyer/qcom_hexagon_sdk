
V = 66

ifndef HEXAGON_TOOLS_ROOT
HEXAGON_TOOLS_ROOT = $(DEFAULT_HEXAGON_TOOLS_ROOT)
endif
 
CC = $(HEXAGON_TOOLS_ROOT)/Tools/bin/hexagon-clang
ARCH = $(HEXAGON_TOOLS_ROOT)/Tools/bin/hexagon-ar
CFLAGS = -mv$(V) -fpic -O2 -G0 -g -Wall -Werror -c -Wno-cast-align -Wpointer-arith -Wno-missing-braces -Wno-strict-aliasing -fno-exceptions -fno-strict-aliasing -fno-zero-initialized-in-bss -fdata-sections -D__V_DYNAMIC__ -Wstrict-prototypes -Wnested-externs
LDFLAGS = -shared -mv$(V) -G0 -Wl

MODULES = qhmath qhcomplex qhdsp/fft qhblas qhdsp/filters qhdsp/decimation qhdsp/interpolation

ifeq ($(OS),Windows_NT)
MK_DIR = mkdir
else
MK_DIR = mkdir -p
endif

# Sources related stuff
SRC_DIR := $(addprefix src/,$(MODULES))
INC_DIR := inc/qhmath inc/qhcomplex inc/qhdsp inc/qhblas inc/internal
OUTPUT_DIR = bin
STATIC_OBJ_DIR := $(addprefix $(OUTPUT_DIR)/static/,$(MODULES))
SHARED_OBJ_DIR := $(addprefix $(OUTPUT_DIR)/shared/,$(MODULES))

QHMATH_STATIC_OBJS := $(patsubst src/%.S,$(OUTPUT_DIR)/static/%.o,$(wildcard src/qhmath/*.S)) \
                      $(patsubst src/%.c,$(OUTPUT_DIR)/static/%.o,$(wildcard src/qhmath/*.c))
QHCOMPLEX_STATIC_OBJS := $(patsubst src/%.S,$(OUTPUT_DIR)/static/%.o,$(wildcard src/qhcomplex/*.S))
QHDSP_STATIC_OBJS := $(patsubst src/%.S,$(OUTPUT_DIR)/static/%.o,$(wildcard src/qhdsp/fft/*.S)) \
                     $(patsubst src/%.c,$(OUTPUT_DIR)/static/%.o,$(wildcard src/qhdsp/fft/*.c)) \
                     $(patsubst src/%.S,$(OUTPUT_DIR)/static/%.o,$(wildcard src/qhdsp/filters/*.S)) \
                     $(patsubst src/%.c,$(OUTPUT_DIR)/static/%.o,$(wildcard src/qhdsp/filters/*.c)) \
                     $(patsubst src/%.S,$(OUTPUT_DIR)/static/%.o,$(wildcard src/qhdsp/decimation/*.S)) \
                     $(patsubst src/%.c,$(OUTPUT_DIR)/static/%.o,$(wildcard src/qhdsp/decimation/*.c)) \
                     $(patsubst src/%.S,$(OUTPUT_DIR)/static/%.o,$(wildcard src/qhdsp/interpolation/*.S)) \
                     $(patsubst src/%.c,$(OUTPUT_DIR)/static/%.o,$(wildcard src/qhdsp/interpolation/*.c))
QHBLAS_STATIC_OBJS := $(patsubst src/%.S,$(OUTPUT_DIR)/static/%.o,$(wildcard src/qhblas/*.S)) \
                      $(patsubst src/%.c,$(OUTPUT_DIR)/static/%.o,$(wildcard src/qhblas/*.c))

QHMATH_SHARED_OBJS := $(patsubst src/%.S,$(OUTPUT_DIR)/shared/%.o,$(wildcard src/qhmath/*.S)) \
                      $(patsubst src/%.c,$(OUTPUT_DIR)/shared/%.o,$(wildcard src/qhmath/*.c))
QHCOMPLEX_SHARED_OBJS := $(patsubst src/%.S,$(OUTPUT_DIR)/shared/%.o,$(wildcard src/qhcomplex/*.S))
QHDSP_SHARED_OBJS := $(patsubst src/%.S,$(OUTPUT_DIR)/shared/%.o,$(wildcard src/qhdsp/fft/*.S)) \
                     $(patsubst src/%.c,$(OUTPUT_DIR)/shared/%.o,$(wildcard src/qhdsp/fft/*.c)) \
                     $(patsubst src/%.S,$(OUTPUT_DIR)/shared/%.o,$(wildcard src/qhdsp/filters/*.S)) \
                     $(patsubst src/%.c,$(OUTPUT_DIR)/shared/%.o,$(wildcard src/qhdsp/filters/*.c)) \
                     $(patsubst src/%.S,$(OUTPUT_DIR)/shared/%.o,$(wildcard src/qhdsp/decimation/*.S)) \
                     $(patsubst src/%.c,$(OUTPUT_DIR)/shared/%.o,$(wildcard src/qhdsp/decimation/*.c)) \
                     $(patsubst src/%.S,$(OUTPUT_DIR)/shared/%.o,$(wildcard src/qhdsp/interpolation/*.S)) \
                     $(patsubst src/%.c,$(OUTPUT_DIR)/shared/%.o,$(wildcard src/qhdsp/interpolation/*.c))
QHBLAS_SHARED_OBJS := $(patsubst src/%.S,$(OUTPUT_DIR)/shared/%.o,$(wildcard src/qhblas/*.S)) \
                      $(patsubst src/%.c,$(OUTPUT_DIR)/shared/%.o,$(wildcard src/qhblas/*.c))

INCLUDES  := $(addprefix -I,$(INC_DIR))

# Builds
BIN_DIR = bin
QHMATH_STATIC_LIB = $(BIN_DIR)/libqhmath.a
QHMATH_SHARED_LIB = $(BIN_DIR)/libqhmath.so
QHCOMPLEX_STATIC_LIB = $(BIN_DIR)/libqhcomplex.a
QHCOMPLEX_SHARED_LIB = $(BIN_DIR)/libqhcomplex.so
QHDSP_STATIC_LIB = $(BIN_DIR)/libqhdsp.a
QHDSP_SHARED_LIB = $(BIN_DIR)/libqhdsp.so
QHBLAS_STATIC_LIB = $(BIN_DIR)/libqhblas.a
QHBLAS_SHARED_LIB = $(BIN_DIR)/libqhblas.so
STATIC_LIBS = $(QHDSP_STATIC_LIB) $(QHCOMPLEX_STATIC_LIB) $(QHBLAS_STATIC_LIB) $(QHMATH_STATIC_LIB)
SHARED_LIBS = $(QHMATH_SHARED_LIB) $(QHCOMPLEX_SHARED_LIB) $(QHDSP_SHARED_LIB) $(QHBLAS_SHARED_LIB)

# Libs
LIBS_DIR = lib
OPENLIBM_LIB = $(LIBS_DIR)/openlibm/libopenlibm.a
MUSL_LIB = $(LIBS_DIR)/musl/libm.a


QHL_FLAGS = -DBENCH_QHL $(INCLUDES)
QHL_ADSP_FLAGS = -DRUN_ON_ADSP $(QHL_FLAGS)
OPENLIBM_FLAGS = -DBENCH_OPENLIBM $(INCLUDES)
MUSL_FLAGS = -DBENCH_MUSL $(INCLUDES)
HSDK_FLAGS = -DBENCH_HSDK $(INCLUDES)

# FFT C implementation used in FFT tests
FFT_INTERNAL_SRC_DIR = src/qhdsp/fft/internal
FFT_INTERNAL_CSRC := $(wildcard $(FFT_INTERNAL_SRC_DIR)/*.c)
FFT_INTERNAL_OUTPUT_DIR = src/output/static/qhdsp/fft/internal
FFT_INTERNAL_OBJ := $(patsubst $(FFT_INTERNAL_SRC_DIR)/%.c, $(FFT_INTERNAL_OUTPUT_DIR)/%.o,$(FFT_INTERNAL_CSRC))
FFT_INTERNAL_SHARED_OUTPUT_DIR = src/output/shared/qhdsp/fft/internal
FFT_INTERNAL_SHARED_OBJ := $(patsubst $(FFT_INTERNAL_SRC_DIR)/%.c, $(FFT_INTERNAL_SHARED_OUTPUT_DIR)/%.o,$(FFT_INTERNAL_CSRC))

# FILTERS C implementation used in filter (FIR/IIR) tests
FILTERS_INTERNAL_SRC_DIR = src/qhdsp/filters/internal
FILTERS_INTERNAL_CSRC := $(wildcard $(FILTERS_INTERNAL_SRC_DIR)/*.c)
FILTERS_INTERNAL_OUTPUT_DIR = src/output/static/qhdsp/filters/internal
FILTERS_INTERNAL_OBJ := $(patsubst $(FILTERS_INTERNAL_SRC_DIR)/%.c, $(FILTERS_INTERNAL_OUTPUT_DIR)/%.o,$(FILTERS_INTERNAL_CSRC))

TEST_BENCH_QHDSP_COMMON_SRC_DIR = tests/qhdsp/common
TEST_BENCH_FILTERS_INTERNAL_SRC_DIR = tests/qhdsp/filters
TEST_BENCH_FILTERS_INTERNAL_CSRC := $(wildcard $(TEST_BENCH_FILTERS_INTERNAL_SRC_DIR)/*.c)
TEST_BENCH_FILTERS_INTERNAL_OUTPUT_DIR = $(QHL_TOBJ_DIR)
TEST_BENCH_FILTERS_INTERNAL_OBJ := $(patsubst $(TEST_BENCH_FILTERS_INTERNAL_SRC_DIR)/%.c, $(TEST_BENCH_FILTERS_INTERNAL_OUTPUT_DIR)/%.o,$(TEST_BENCH_FILTERS_INTERNAL_CSRC))

TEST_BENCH_DECIMATION_INTERNAL_SRC_DIR = tests/qhdsp/decimation
TEST_BENCH_INTERPOLATION_INTERNAL_SRC_DIR = tests/qhdsp/interpolation

DIRS = $(STATIC_OBJ_DIR) $(SHARED_OBJ_DIR) \
       $(BIN_DIR) $(QHL_TOBJ_DIR) $(QHL_ADSP_TOBJ_DIR) $(OPENLIBM_TOBJ_DIR) \
       $(MUSL_TOBJ_DIR) $(HSDK_TOBJ_DIR) \
	   $(FFT_INTERNAL_OUTPUT_DIR) $(FFT_INTERNAL_SHARED_OUTPUT_DIR) \
       $(FILTERS_INTERNAL_OUTPUT_DIR)

vpath %.S $(SRC_DIR)
vpath %.c $(SRC_DIR)
vpath test_%.c $(TEST_DIR)
vpath bench_%.c $(TEST_DIR)

define build_static
$1/%.o: %.S
	$(CC) $(CFLAGS) $$< -o $$@

$1/%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $$< -o $$@
endef

define build_shared
$1/%.o: %.S
	$(CC) $(CFLAGS) $$< -o $$@

$1/%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $$< -o $$@
endef

.PHONY: all clean checkdirs test

all: checkdirs $(STATIC_LIBS) $(SHARED_LIBS) test

libqhmath: $(QHMATH_STATIC_LIB) $(QHMATH_SHARED_LIB)
libqhcomplex: $(QHCOMPLEX_STATIC_LIB) $(QHCOMPLEX_SHARED_LIB)
libqhdsp: $(QHDSP_STATIC_LIB) $(QHDSP_SHARED_LIB)
libqhblas: $(QHBLAS_STATIC_LIB) $(QHBLAS_SHARED_LIB)

$(QHMATH_STATIC_LIB): checkdirs $(QHMATH_STATIC_OBJS)
	$(ARCH) -rcs $@ $(QHMATH_STATIC_OBJS)

$(QHCOMPLEX_STATIC_LIB): checkdirs $(QHCOMPLEX_STATIC_OBJS)
	$(ARCH) -rcs $@ $(QHCOMPLEX_STATIC_OBJS)

$(QHDSP_STATIC_LIB): checkdirs $(QHDSP_STATIC_OBJS)
	$(ARCH) -rcs $@ $(QHDSP_STATIC_OBJS)

$(QHBLAS_STATIC_LIB): checkdirs $(QHBLAS_STATIC_OBJS)
	$(ARCH) -rcs $@ $(QHBLAS_STATIC_OBJS)

$(QHMATH_SHARED_LIB): checkdirs $(QHMATH_SHARED_OBJS)
	$(CC) -shared $(QHMATH_SHARED_OBJS) -o $@ $(LDFLAGS)

$(QHCOMPLEX_SHARED_LIB): checkdirs $(QHCOMPLEX_SHARED_OBJS)
	$(CC) -shared $(QHCOMPLEX_SHARED_OBJS) -o $@ $(LDFLAGS)

$(QHDSP_SHARED_LIB): checkdirs $(QHDSP_SHARED_OBJS)
	$(CC) -shared $(QHDSP_SHARED_OBJS) -o $@ $(LDFLAGS)

$(QHBLAS_SHARED_LIB): checkdirs $(QHBLAS_SHARED_OBJS)
	$(CC) -shared $(QHBLAS_SHARED_OBJS) -o $@ $(LDFLAGS)

test: $(STATIC_LIBS) $(TEST_OBJS) $(BENCH_OBJS)

checkdirs: $(DIRS)

$(DIRS):
	$(MK_DIR) "$@"

$(foreach static_dir,$(STATIC_OBJ_DIR),$(eval $(call build_static,$(static_dir))))
$(foreach shared_dir,$(SHARED_OBJ_DIR),$(eval $(call build_shared,$(shared_dir))))

fft_internal: checkdirs $(FFT_INTERNAL_OBJ)

fft_internal_shared: checkdirs $(FFT_INTERNAL_SHARED_OBJ)

# $($(eval $(call build_static,$(FFT_INTERNAL_OUTPUT_DIR))))
# $($(eval $(call build_shared,$(FFT_INTERNAL_SHARED_OUTPUT_DIR))))

filters_internal: $(FILTERS_INTERNAL_OBJ)

$($(eval $(call build_static,$(FILTERS_INTERNAL_OUTPUT_DIR))))


clean:
	rm -rf $(OUTPUT_DIR) $(TEST_OUTPUT_DIR) $(BIN_DIR)

# Makefile debugging trick:
# call print-VARIABLE to see the runtime value of any variable
print-%:
	@echo '$*=$($*)'

#=====================================================================================
# Generate output files for Smart Speeker Development Board with QCS403 (MP25-PF599-8)
#=====================================================================================

bench_target_cpu:
	@echo Generate output files for CPU target on Smart Speeker Development Board
	make -f cpu.mk clean
	make -f cpu.mk all
