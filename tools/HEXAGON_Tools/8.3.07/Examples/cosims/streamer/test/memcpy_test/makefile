###########################################################################
# Copyright (c) Date: Mon Nov 24 16:25:55 CST 2008 QUALCOMM INCORPORATED
# All Rights Reserved
# Modified by QUALCOMM INCORPORATED on Mon Nov 24 16:25:55 CST 2008
###########################################################################

TEST = memcpy_v60
Q6VERSION   = v60

IMG_BASE      = 0x40000000
TCM_BASE      = 0xd8000000
##################################################
STREAMER_BASE = 0xd0000000                      ##
## WARNING! Changing the streamer base requires ##
## a corresponding change in the cosim.cfg file.##
##################################################

EXEC = $(TEST)
ASM_SRC_DIR = ../asm_src
INC_DIR     = ../include
OBJ_DIR     = ../build
IN_DIR      = ../input
OUT_DIR     = ../output

INPUT   = $(IN_DIR)/bayer_3k_x_2k.in
OUTPUT  = $(OUT_DIR)/bayer.out
SIZE = 3072 2048

C_SRC   = test_memcpy.c
ASM_SRC = hvx_copy_line.S

CC  = hexagon-clang
LD  = $(CC)
AS  = $(CC)

ASM_FLAGS = -m$(Q6VERSION)
CFLAGS    = -Wall -mhvx -mhvx-length=128B -O3 -m$(Q6VERSION) -I$(INC_DIR) -DTCM_BASE=$(TCM_BASE) -DSTREAMER_BASE=$(STREAMER_BASE)
LDFLAGS   = -m$(Q6VERSION) -Wl,--section-start=.start=$(IMG_BASE),--defsym=HEAP_SIZE=0x3000,--defsym=STACK_SIZE=0x4000,--defsym=L2_CACHE_SIZE=1,--defsym=ISDB_SECURE_FLAG=2,--defsym=ISDB_TRUSTED_FLAG=2,--defsym=ISDB_DEBUG_FLAG=2

OBJS = $(C_SRC:%.c=$(OBJ_DIR)/%.o) $(ASM_SRC:%.S=$(OBJ_DIR)/%.obj)

$(EXEC): $(OBJS)
	$(LD) $(LDFLAGS) -o $(OBJ_DIR)/$@ $(OBJS) $(LLIBS)

$(OBJS): | $(OBJ_DIR)

clean:
	rm -Rf $(OBJ_DIR) $(OUT_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.obj: $(ASM_SRC_DIR)/%.S
	$(CC) -m$(Q6VERSION) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

$(OUT_DIR):
	mkdir $(OUT_DIR)
