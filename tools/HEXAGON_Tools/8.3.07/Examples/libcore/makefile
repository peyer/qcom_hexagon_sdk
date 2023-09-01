########################################################################
# Copyright (c) Date: Tue Aug 26 16:38:43 CDT 2008 QUALCOMM INCORPORATED 
# All Rights Reserved 
# Modified by QUALCOMM INCORPORATED on Tue Aug 26 16:38:43 CDT 2008 
########################################################################




CC=hexagon-clang
LD = $(CC) $(CCF)
SIM=hexagon-sim

#  if tooldir is set, add tool-specific paths due to the way we package them
ifneq ($(TOOLDIR),)
   CC:=$(TOOLDIR)/gnu/bin/$(CC)
   LD:=$(TOOLDIR)/gnu/bin/$(LD)
   SIM:=$(TOOLDIR)/qc/bin/$(SIM)
endif

debug:
	echo CC=$(CC)
	echo LD=$(LD)
	echo SIM=$(SIM)

results.txt: $(EXEC)
	$(SIM) $(EXEC) > $@

TESTDIRS =
TESTDIRS += AudioProc/up_sampler/test
TESTDIRS += ImagProc/colorcnvt/rgb2ycc/test
TESTDIRS += ImagProc/colorcnvt/ycc2rgb/test
TESTDIRS += ImagProc/colorcnvt/yuv2rgb/test
TESTDIRS += ImagProc/colorcorrect/test
TESTDIRS += ImagProc/conv3x3/test
TESTDIRS += ImagProc/epsilonfilt/test
TESTDIRS += ImagProc/fdct8x8/test
TESTDIRS += ImagProc/gammacorrect/test
TESTDIRS += ImagProc/idct8x8/test
TESTDIRS += ImagProc/median3x3/test
TESTDIRS += ImagProc/scale/test
TESTDIRS += ImagProc/sobel/test
TESTDIRS += Math/test
TESTDIRS += SigProc/bkFir/test
TESTDIRS += SigProc/cholesky/test
TESTDIRS += SigProc/correlation/test
TESTDIRS += SigProc/cxFFT32x16/test
TESTDIRS += SigProc/cxFFT_IFFT/test
TESTDIRS += SigProc/cxFHT/test
TESTDIRS += SigProc/cxFir/test
TESTDIRS += SigProc/findmax/test
TESTDIRS += SigProc/iir/test
TESTDIRS += SigProc/lms/test
TESTDIRS += SigProc/pfFFT/test
TESTDIRS += SigProc/rFFT/test
TESTDIRS += TeleComm/aes/test
TESTDIRS += TeleComm/cdma1x_longPN/test
TESTDIRS += TeleComm/crc/test
TESTDIRS += TeleComm/freqcorrect/test
TESTDIRS += TeleComm/kasumi/test
TESTDIRS += TeleComm/pn/test
TESTDIRS += TeleComm/ReedSolomon/test
TESTDIRS += TeleComm/Turbo/test
TESTDIRS += TeleComm/viterbi/vit3g/test
TESTDIRS += TeleComm/viterbi/vitgsm/test

%/results.txt:
	@cd `dirname "$@"`;\
	$(MAKE) results.txt;\
	grep "*** PASS!" results.txt;\
	if [ "$$?" -ne "0" ]; then\
	   echo Test failed!;\
	   rm $@;\
	   exit 1;\
	fi

#  false target for cleaning
%/clean.txt:
	@cd `dirname "$@"`;\
	$(MAKE) clean;\
	rm -f "results.txt"

all_tests:  ${TESTDIRS:test=test/results.txt}


all_clean:  ${TESTDIRS:test=test/clean.txt}




