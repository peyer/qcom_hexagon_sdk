ifndef SDK_SETUP_ENV
$(error Error! SDK Environment not set up -> please run setup_sdk_env script from SDK root directory.)
endif

SUPPORTED_VS = $(default_VS)
SUPPORTED_VS += hexagon_Debug_dynamic_toolv83_v55 \
                hexagon_Debug_toolv83_v55 \
                hexagon_Release_dynamic_toolv83_v55 \
			    hexagon_Release_toolv83_v55 \
				hexagon_ReleaseG_dynamic_toolv83_v55 \
				hexagon_ReleaseG_toolv83_v55
# include the variant specific .min file.
# V=hexagon --> hexagon.min
# V=WinNT --> WinNT.min
# V=android --> android.min

ifndef V
$(error Varaint must be provided, pass a variant by adding 'V=<desired varaint>' to your build command)
endif

V_TARGET = $(word 1,$(subst _, ,$(V)))
include $(HEXAGON_SDK_ROOT)/build/make.d/$(V_TARGET)_vs.min
include $(HEXAGON_SDK_ROOT)/build/defines.min

BUILD_COPIES = $V/atomic$(LIB_EXT) \
               inc/AEEatomic.h \
               $(SHIP_DIR)/ ;

INCDIRS += inc

BUILD_QEXES = atomic_q
atomic_q.C_SRCS = src/atomic_q.c
atomic_q_LIBS = atomic

BUILD_LIBS = atomic
atomic.C_SRCS = src/atomic.c
atomic.C_SRCS += src/cae.c
-include rel.min

BULLETPROOF.C_SRCS += src/atomic.c
atomic_GCOV_OBJ = $V/atomic$(OBJ_EXT)

ifeq (qnx,$(V_TARGET))
    ifeq (1,$(V_aarch64))
	     DEFINES += _QNX64_
    else
         DEFINES += __QNX32__
	endif
endif

include $(RULES_MIN)
