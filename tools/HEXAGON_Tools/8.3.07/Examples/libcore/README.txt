Qualcomm Proprietary
Copyright (c) 2012  Qualcomm Incorporated.  All rights reserved.
 
All data and information contained in or disclosed by this document are
confidential and proprietary information of Qualcomm Incorporated, and
all rights therein are expressly reserved. By accepting this material,
the recipient agrees that this material and the information contained
therein are held in confidence and in trust and will not be used,
copied, reproduced in whole or in part, nor its contents revealed in
any manner to others without the express written permission of Qualcomm
Incorporated.

This software may be subject to U.S. export, re-export, or transfer
("export") laws.  Diversion contrary to U.S. law prohibited.

Hexagon is a registered trademark of Qualcomm Incorporated in the 
United States and other countries.



                     Hexagon Core Library 
                     ====================

This library contains a comprehensive set of common functions across 
a wide variety of signal, image and Audio processing applications.
All functions are well-optimized in assembly for Hexagon. Please note
some restrictions, such as array alignment, etc., might be applied 
to the assembly implementations. These restrictions are detailed in 
the header (*.h) files along with each function.



Contents
=========

-- libcore
    |
    |__ docs
    |
    |__ include
    |
    |__ Math      ___ dsp_libs  ______ invert (approx)
    |              |                |_ invert (approx,fast)
    |              |                |_ invert (netowns) 
    |              |                |_ invert (16-bit FP)
    |              |                |_ divide
    |              |                |_ sqrt
    |              |                |_ inv_sqrt
    |              |                |_ log10
    |              |                |_ exp10
    |              |                |_ approx_uinv_linlut8
    |              |                |_ approx_uinv_cubelut8
    |              |                |_ approx_uinv_newt
    |              |                |_ approx_uinv_tylr
    |              |                |_ approx_dB_to_ulin_linlut8
    |              |                |_ approx_dB_to_ulin_cubefit
    |              |                |_ approx_dB_to_ulin_quadlut8
    |              |                |_ approx_ulin_to_dB_linlut8
    |              |                |_ approx_ulin_to_dB_cubelut8
    |              |_ dsplib_trignm __ sin
    |                               |_ cos
    |                               |_ arctan
    |                               |_ approx_atan2_poly7
    |
    |__ SigProc   ___ bkfir
    |              |_ cholesky
    |              |_ correlation
    |              |_ cxFFT32x16
    |              |_ cxFFT_IFFT
    |              |_ cxFHT
    |              |_ cxFir
    |              |_ findmax
    |              |_ iir
    |              |_ lms
    |              |_ pfFFT (192/320/360-point)
    |              |_ rFFT
    |
    |__ AudioProc ___ upsampler
    |              
    |
    |
    |__ ImagProc  ___ idct8x8
    |              |_ fdct8x8
    |              |_ conv3x3
    |              |_ epsilonfilt
    |              |_ gammacorrect
    |              |_ median3x3
    |              |_ scale
    |              |_ sobel
    |              |_ colorcorrect
    |              |_ colorcnvt __ yuv2rgb 
    |                           |_ ycc2rgb (H1V1/H2V2/H1V2/H2V1)
    |                           |_ rgb2ycc 
    |
    |__ TeleComm  ___ viterbi ____ vitgsm
                   |           |__ vit3g
                   |_ freqcorrect
                   |_ PN
                   |_ cdma1x_longPN
                   |_ CRC
                   |_ kasumi
                   |_ aes
                   |_ ReedSolomon
                   |_ Turbo
     


The working examples reside in the following directories:

AudioProc/up_sampler/test
ImagProc/colorcnvt/ycc2rgb/test
ImagProc/colorcnvt/yuv2rgb/test
ImagProc/colorcnvt/rgb2ycc/test
ImagProc/colorcorrect/test
ImagProc/conv3x3/test
ImagProc/epsilonfilt/test
ImagProc/fdct8x8/test
ImagProc/gammacorrect/test
ImagProc/idct8x8/test
ImagProc/median3x3/test
ImagProc/scale/test
ImagProc/sobel/test
Math/test
SigProc/bkFir/test
SigProc/cholesky/test
SigProc/correlation/test
SigProc/cxFFT32x16/test
SigProc/cxFFT_IFFT/test
SigProc/cxFHT/test
SigProc/cxFir/test
SigProc/findmax/test
SigProc/iir/test
SigProc/lms/test
SigProc/pfFFT/test
SigProc/rFFT/test
TeleComm/aes/test
TeleComm/cdma1x_longPN/test
TeleComm/crc/test
TeleComm/freqcorrect/test
TeleComm/kasumi/test
TeleComm/pn/test
TeleComm/ReedSolomon/test
TeleComm/Turbo/test
TeleComm/viterbi/vit3g/test
TeleComm/viterbi/vitgsm/test

To run all of them from the top level, you can run:

make all_clean
make all_tests

To run a specific example, you may change to that directory and 
invoke the specific makefile.  For example:

cd AudioProc/up_sampler/test
make clean
make sim

The makefiles use hexagon-clang and hexagon-sim as the compiler and 
simulators, respectively.  These can be overridden from the 
commandline via CC and SIM make variables.






