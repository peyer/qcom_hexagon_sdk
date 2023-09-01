Example: sfpFFT - Single precision floating point FFT

This example tests the Hexagon optimized FFT and inverse FFT routines
found in "fft.h".  It runs against a sample set of data and compares
the output against precomputed values.

To clean/build/simulate example enter:
	cd ./test
	make

The simulation runs 6 tests which takes approximately 2.5 minutes and will
produce results similar to the following display:

	Test #0...
	Test 128-point FFT: SNR= 133.35  of 122.37(max)
	Test 256-point FFT: SNR= 129.13  of 127.31(max)
	Test #1...
	Test 128-point FFT: SNR= 132.88  of 123.86(max)
	Test 256-point FFT: SNR= 130.29  of 127.13(max)
	Test #2...
	Test 128-point FFT: SNR= 132.55  of 122.74(max)
	Test 256-point FFT: SNR= 130.75  of 126.38(max)
	Test #3...
	Test 128-point FFT: SNR= 134.29  of 123.24(max)
	Test 256-point FFT: SNR= 130.53  of 124.65(max)
	Test #4...
	Test 128-point FFT: SNR= 133.03  of 122.80(max)
	Test 256-point FFT: SNR= 130.78  of 127.02(max)
	Test #5...
	Test 128-point FFT: SNR= 132.77  of 121.31(max)
	Test 256-point FFT: SNR= 130.38  of 124.52(max)
	 256-point Cycle-count: 9874
	 512-point Cycle-count: 23530
	1024-point Cycle-count: 50020
	2048-point Cycle-count: 115324

