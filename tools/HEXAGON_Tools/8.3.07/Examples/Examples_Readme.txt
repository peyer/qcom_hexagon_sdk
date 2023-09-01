

[*======================= COPYRIGHT NOTICE ======================*]
[* Copyright 2015 QUALCOMM Incorporated.                         *]
[* All rights reserved.                                          *]
[*                                                               *]
[* All data and information contained in or disclosed by this    *]
[* document is confidental and proprietary information of        *]
[* QUALCOMM Incorporated and all rights therein are expressly    *]
[* reserved. By accepting this material the recipient agrees     *]
[* that this material and the information contained therein is   *]
[* held in confidence and in trust and will not be used, copied, *]
[* reproduced in whole or in part, nor its contents revealed in  *]
[* any manner to others without the express written permission   *]
[* of QUALCOMM.                                                  *]
[*===============================================================*]


Hexagon Example Programs

----------------------------------------------------------------------


#### Overview ####

Every Hexagon tools release includes a large number of example
programs.

This document lists the example programs according to their
assigned category/subdirectory. A one-sentence description
is provided for each example listed.



#### Example categories ####

The example programs belong to the following categories (which
double as subdirectory names in the Examples directory):

  binutils
  c_optimization
  cosims
  Debugging
  libdl
  libnative
  Porting
  Profiling
  StandAlone_Applications
  Static_Analysis
  TRACE32
  libcore
  HVX

The example programs in all but two of these categories can be
used with any Hexagon processor version:

* The "libcore" category contains programs that are optimized
for processor versions up to V5 (though some are optimized to
work with only up to V3 or V4).

* The "HVX" category contains programs that are optimized for
the processor versions that support HVX.



#### binutils/* ####

- common
Shows how common symbols are handled by the Hexagon assembler
and linker

- falign
Shows the limitations of the Hexagon assembler directive .falign



#### c_optimization/* ####

- bitcount
Emphasizes need for eliminating unnecessary control flow from
programs (note - example is small and unrealistic)

- bkfir
Case study which examines a reference version of a standard FIR
filter.

- cleanup_loop
Shows how to vectorize code when the data length is not a multiple
of the vector length.

- findmin
Emphasizes an important concept for VLIW (very long instruction
word) architectures such as Hexagon: namely, eliminate control flow.

- hexagon_types
Brief introduction to the various Hexagon-specific data types that
are defined in <hexagon_types.h>.

- intrinsics
Compares two functions, one with intrinsics and one without, to
illustrate the reduction of cycles when using intrinsics in C code.

- restrict
Shows the benefits of using the __restrict keyword for pointers.

- timers
Shows how to use the simulator cycle timers to gather performance
statistics for a particular region of code.

- uniform_misalignment
Shows how to handle the case where data alignment is unknown at
compile time.

- valignb
Shows how to use the valignb intrinsic to support misaligned
array accesses.

- vectoradd
Demonstrates one of the most important C-optimization techniques
available for the HEXAGON: vectorization.

- vmux
Shows how to eliminate control flow with VMUX, the vector version
of MUX.



#### cosims/* ####

- badMem
Presents a cosim which registers valid memory regions, and flags
a failure when an application accesses memory outside the region.

- bus
Presents a cosim which models latency incurred by a bus access.

- FunctionProfiler
Presents a cosim model for profiling the functions of a Hexagon
application.

- L2timerCfg
Shows how to create a l2vic cosim configuration file to be used
with an RTOS such as QURT.

- l2vic_test
Shows how to create, build and simulate a cosim shared object
for linux and windows."

- PCRangeProfiler
Shows how to create a cosim for profiling sections of a Hexagon
application.

- qtimer_test
Shows how to use the l2vic and qtimer in a Hexagon stand-alone
application.

- qtimerCfg
Shows how to create a qtimer cosim configuration file to be used
with an RTOS such as QuRT.

- SemiHostedFunction
Shows how to use a semi-hosted callback function to perform an
aalgorithm in a native host (x86).

- streamer
Shows how to create, build and simulate a camera streamer
interface cosim."

- timerCfg
Shows how to create a timer cosim configuration file to be used
with an RTOS such as QuRT.



#### Debugging/* ####

- Example1
Shows the difference in alignment for a packed and unpacked
structure.

- Example2
Shows how mixing unpacked with packed structures can cause a
simulation crash.

- Example3
Shows how to troubleshoot the Example2 simulation crash using
the TRACE32 debugger.



#### libdl/* ####

- dlopen
Demonstrates how to create and open a dynamic library.



#### libnative/* ####

- libnative
Demonstrates how to build and execute instruction instrinsics.



#### Porting/* ####

- Hexify
Utility which converts all instances of "q6dsp" to "hexagon"
throughout a directory.



#### Profiling/* ####

- gprof
Demonstrates how to use the Hexagon gprof profiler.

- graphical
Demonstrates how to use the Hexagon graphical profiler.

- pmu_ctrs_test
Demonstrates how the hexagon PMU counters can be used to
monitor events in real-time.

- pmustats_spreadsheet
Demonstrates how to gather profiling statistics with a
macro-driven spreadsheet.

- proftool
Demonstrates how to gather stall trace data from the Hexagon
simulator and identify stalls that can be avoided.

- region
Demonstrates how the gprof profiler can selectively choose
regions in code to gather statistics on that region.

- codecoverage
Demonstrates the steps to build, simulate, create codecoverage
html report on Mandelbrot example.


#### StandAlone_Applications/* ####

- mandelbrot
Demonstrates how to spawn multiple threads that simultaneously
operate on an image buffer.

- tcm
Demonstrates how to create a program that will run from the
Hexagon V60 tightly-coupled memory (TCM).

- tlb
Demonstrates how the Hexagon memory management unit (MMU) is
programmed when using the stand-alone library.

- Linker
    Static_LinkerScript
	PHDR_LinkerScript
	Dynamic_LinkerScript
	Partial_LinkerScript
	
The Linker folder has four examples of how to generate static
and dynamic libraries, partial libraries with multiple objects
and static libraries with Program Headers.


#### Static_Analysis/* ####

- static_analyzer
Demonstrates how to use the LLVM clang static analyzer to find
potential bugs in a C/C++ program.



#### TRACE32/* ####

- Linux_Example
Demonstrates how to use the Lauterbach TRACE32 debugger with
the Hexagon simulator in a Linux environment.

- Windows_Example
Demonstrates how to use the Lauterbach TRACE32 debugger with
the Hexagon simulator in a Windows environment.



#### libcore/* ####

- AudioProc
Presents an audio processor upsampler.

- include
Folder of common header files used by libcore function examples.

- Math
Tests the Hexagon-optimized DSP-oriented math functions in dsp_libs.

- sfpFFT
Demonstrates a single-precision floating point FFT function.

- Vdelta_Helper
Demonstrates how to generate control inputs for the vrdelta and
vdelta instructions in V60 HVX (note - example is HVX-specific).



#### libcore/ImagProc/* ####

- colorcnvt/rgb2ycc
Converts RGB color values to YCbCr color space.

- colorcnvt/ycc2rgb
Converts YCbCr H1V1 to 24-bit RGB

- colorcnvt/yuv2rgb
Converts color from 24-bit YUV-plane to 24-bit RGB-plane.

- colorcorrect
Performs color correction on RGB-components of image.

- conv3x3
Calculates convolution of image with 3x3 mask.

- epsilonfilt
Does epsilon filtering of luma component.

- fdct8x8
Performs forward DCT transform on 8x8 blocks.

- gammacorrect
Performs gamma correction with lookup table.

- idct8x8
Performs inverse DCT transform on 8x8 blocks.

- median3x3
Performs 3x3 median filtering on image.

- scale
Downsamples an image by half.

- sobel
Performs 3x3 Sobel filtering on image.



#### libcore/sigProc/* ####

- bkfir
Signal processing function which performs FIR filtering on a bulk
of data.

- cholesky
Performs Cholesky matrix decomposition algorithm.

- correlation
Computes correlations between two real number sequences.

- cxFFT_IFFT
Demonstrates a fixed-point implementation of radix-4 scaled
FFT and IFFT.

- cxFFT32x16
Demonstrates the Hexagon-optimized FFT 32x16.

- cxFHT
Demonstrates the fast Hadamard transform on a complex array.

- cxFir
Demonstrates FIR filtering on a complex array.

- findmax
Demonstrates how to find maximum value and its index from an
array.

- iir
Demonstrates a cascade of biquads IIR filter.

- lms
Demonstrates an adaptive filter with least mean square
algorithm.

- pfFFT
Demonstrates the Hexagon-optimized 320- and 360-point FFT
routines.

- rFFT
Demonstrates the Hexagon-optimized real-valued FFT routines.



#### libcore/TeleComm/* ####

- aes
Demonstrates the Advance Encryption Standard (AES) cipher
algorithm.

- cdma1x_longPN
Demonstrates the Hexagon-optimized long PN sequence generator
routines.

- crc
Demonstrates the Hexagon-optimized Cyclic Redundancy Check (CRC)
routines.

- freqcorrect
Demonstrates the Hexagon-optimized frequency offset correction
routines.

- kasumi
Demonstrates the Hexagon-optimized 3GPP confidentiality/integrity
routines.

- pn
Demonstrates the Hexagon-optimized PN sequence generator routines.

- ReedSolomon
Demonstrates the Hexagon-optimized Reed-Solomon decoding routines.

- Turbo
Demonstrates the Hexagon-optimized Turbo decoding routines.

- viterbi/vit3g
Demonstrates the Hexagon-optimized Viterbi 3G decoding routines.

- viterbi/vitgsm
Demonstrates the Hexagon-optimized Viterbi GSM decoding routines.



#### HVX/* ####

- bilateral
Applies a 9x9 bilateral filter to a image.

- common
Common header files used by all HVX function examples.

- HVX/conv3x3a16
Applies a 3x3 kernel to filter a image.

- conv3x3a32
Applies a 3x3 kernel to filter a image.

- dilate3x3
Performs morphological dilation of an image using 3x3 mask.

- epsilon
Performs 9x9 filtering on an image.

- fast9
Performs FAST feature detection.

- gaussian
Performs Gaussian blur on an image with a 3x3 kernel or
5x5 kernel or 7x7 kernel.

- harriscorner
Performs Harris corner detection.

- histogram
Takes an 8-bit image block and returns the histogram of 256
32-bit bins.

- integrate
Calculates a 2D integration of an image.

- invsqrt
Computes 1 / squareroot(x) using interpolation.

- median
Performs 3x3 median filter operation on a image.

- ncc
Performs ncc using 8x8 template and 18x18 ROI in an image.

- nvl2torgb8888
Converts image from NV12 format to RGB8888 format.

- sigma3x3
Performs 3x3 sigma filtering on an image block.

- Sobel
Performs 9x9 Sobel filtering on an image block.

- testvectors
Performs a Sobel operator on an image.

- wiener9x9
Performs 9x9 Wiener filtering on an image block.
