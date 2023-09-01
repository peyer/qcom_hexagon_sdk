This application combines multiple computational DSP kernel examples into a 
single command-line application. The following kernels can be tested at configurable
clock settings, resolutions, and frame rates. (Note that these implementations are
hand-optimized, but not necessarily fully optimized up to the latest architecture. They
are intended as reasonable example workloads for measuring performance and power).

  epsilon - 9x9 epsilon filter
  bilateral - 9x9 bilateral filter
  fast9 - fast9 corner detection
  integrate - integral image
  dilate3x3 - 3x3 dilate filter
  dilate5x5 - 5x5 dilate filter
  conv3x3 - 3x3 convolution filter
  gaussian7x7 - 7x7 Gaussian filter
  sobel3x3 - 3x3 Sobel filter
  fft - 2D FFT (1024x1024 only)
  fast9 - fast algorithm for corner detection
  warp - Image warping
  
It should be located in the Hexagon SDK installation, at <SDK>/examples/common/benchmark

To build the project, do. 

    make tree VERBOSE=1 V=android_ReleaseG_aarch64
    make tree VERBOSE=1 V=hexagon_ReleaseG_dynamic_toolv83_v66

Note1: Substitute the supported build flavor as desired.
	In the above example, android_ReleaseG_aarch64 and hexagon_ReleaseG_dynamic_toolv83_v66 are 
	the build flavours for building the android executable/shared object and 
	hexagon shared objects respectively.
	
	
Note2: As part of building the hexagon variant of this project, test validation 
	is performed in the hexagon simulation environment. 

In order to select different options for the test run on the simulator as part of building 
the hexagon variant of the project, you can modify the following lines in hexagon.min:
QEXE_SIM_OPTIONS +=--dsp_clock 1000 --ahb:lowaddr 0xc0000000 --ahb:highaddr 0xc0ffffff
QEXE_CMD_OPTIONS +=-f epsilon -w 256 -h 64

To re-run the simulation, without re-building the project , perhaps with different options, 
copy/paste the simulation command emitted by the above make (hexagon variant) command, 
and alter the options as desired. It should look something like this, though your paths, 
SDK, and tools version may differ:

	D:/Hexagon_SDK/3.4.2/tools/HEXAGON_Tools/8.3/Tools/bin/hexagon-sim -mv66g_1024 --simulated_returnval --usefs hexagon_ReleaseG_dynamic_toolv83_v66 --pmu_statsfile hexagon_ReleaseG_dynamic_toolv83_v66/pmu_stats.txt --cosim_file hexagon_ReleaseG_dynamic_toolv83_v66/q6ss.cfg --l2tcm_base 0xd800 --rtos hexagon_ReleaseG_dynamic_toolv83_v66/osam.cfg     D:/Hexagon_SDK/3.4.2/libs/common/qurt//computev66/sdksim_bin/runelf.pbn --  D:/Hexagon_SDK/3.4.2/libs/common/run_main_on_hexagon/ship/hexagon_ReleaseG_dynamic_toolv83_v66/run_main_on_hexagon_sim  -- benchmark_q.so  -f epsilon -w 256 -h 64

    
To flash the project on target after building the project, do 
    adb root
    adb wait-for-device
    adb remount
    adb push android_ReleaseG_aarch64/ship/benchmark /vendor/bin
    adb shell chmod 755 /vendor/bin/benchmark
    adb push android_ReleaseG_aarch64/ship/libbenchmark.so /vendor/lib64/
	
    adb push hexagon_ReleaseG_dynamic_toolv83_v66/ship/libbenchmark_skel.so /vendor/lib/rfsa/adsp/	
	adb push hexagon_ReleaseG_dynamic_toolv83_v66/libdspCV_skel.so /vendor/lib/rfsa/adsp/
	adb push hexagon_ReleaseG_dynamic_toolv83_v66/libapps_mem_heap_stub.so /vendor/lib/rfsa/adsp/

To execute the project on target, do 
	adb shell /vendor/bin/benchmark [options]
	example: adb shell /vendor/bin/benchmark -f epsilon -P 255  -L 10 -l 10 -s

usage (or, run benchmark without options to display current usage):

  benchmark -f function_name [-cdsv] [-w width] [-h height] [-L rpc_loops]
            [-l dsp_loops] [-u usec] [-P power_level] [-y latency] [-m mode]
            [-o file_name]
options:

  -f function_name: Which function to benchmark. Options follow.
       conv3x3 - 3x3 convolution over source image.          
       dilate3x3 - 3x3 dilate filter over source image.
       dilate5x5 - 5x5 dilate filter over source image.
       gaussian7x7 - 7x7 Gaussian filter over source image.
       integrate - 32-bit integral image over source image.
       epsilon - 9x9 epsilon filter over source image.
       bilateral - 9x9 bilateral filter over source image.
       fast9 - fast9 corner detection over source image (stops at 5000 corners).
       sobel3x3 - sobel3x3 - 3x3 Sobel filter over source image.
       fft - 2D 1024x1024 FFT (8-bit real input, 32-bit fixed-point Q29.3 complex output 
         (each row is 1024 real followed by 1024 imaginary).
       gather - simple example usage of VGATHER instruction.
       scatter - simple example usage of VSCATTER instruction.
       crash10 - a dummy memcpy that triggers a DSP PD crash every 10th invocation.
       warp - warp transform with a homograph matrix.
       	
  -c: mark RPC buffers as uncached from the Apps processor.\n
       May reduce RPC overhead.
       
  -d: enable DSP DCVS algorithm for adjusting clocks to optimize power
       (OFF by default).

  -q: enable FastRPC QoS mode to disable CPU low-power states, improving
       performance at some power cost. (OFF by default).
       
  -z: run benchmark DSP library in an unsigned PD.

  -s: Skip bit-exactness checks (this saves test execution time, should not
       impact profiling results).
       
  -r: Participate in cooperative serialization. Concurrent participants will 
       each be gated on a shared virtual resource so that each participant 
       can make full use of cDSP resources in turn. This helps overall performance
       of some concurrencies.
       
  -v: verbose mode, displays extra profiling information for each RPC
       loop (OFF by default).

  -w width: Width of input image to use for benchmarking. Some functions.
       may require a multiple of 128.

  -h height: Height of input image to use for benchmarking.

  -L rpc_loops: Number of round-trip RPC invocations of the desired
       benchmarking function.

  -l dsp_loops: Number of iterations of function within the DSP per RPC
       invocation.

  -u usec: Number of micro-seconds between each RPC invocation (default
       33333, i.e. 30 fps).

  -p power_level: <Deprecated, instead use -P> Desired power level. 0 is highest-performance, i.e.
       Turbo. 1 is next-highest, etc. Specific values are target-dependent,
       e.g. it could be that 0 = Turbo, 1 = Nominal, 2 = SVS1, 3=SVS2, and
       4 or higher is VDD_min.

  -y latency: The sleep latency tolerance threshold (in micro-seconds) in
       the DSP. The higher the value, the deeper level of sleep will be
       allowed when idle. Usually this need not be modified (100 is
       default).

  -m mode: Specifies some options for establishing baseline benchmarks as
       follows (0 is default):
       0 - Run benchmarked function as specified by the command line.
       1 - Make RPC invocations for the function specified in command line,
           but skip the actual processing (to help measure baseline
           power/performance).
       2 - Wake up the CPU at the specified usec interval, but skip the RPC
           invocation and related processing (to help measure baseline
           power/performance).

  -o file_name: Specifies a path/file to append profiling results to. This
       saves to a comma-separated text file, containing info about the
       test case and profiling result. If file does not exist, it will be
       created.
	
  -P power_level: Desired power level. 0 is Disable Voltage corner, 1 is lowest-performance, i.e.
       SVS2. 2 is next higher performance than 1, etc. Specific values are target-dependent,
       Please refer to the definition of HAP_dcvs_voltage_corner_t @ SDK_ROOT\incs\HAP_power.h
       e.g. it could be that 1 = SVS2, 2 = SVS, 3 = SVSPLUS, 4=NOM, and
       5 = NOMPLUS 6 = TURBO, 7 = TURBOPLUS.
	   
To do build, flash and execute the project in SM8150 device with a single script
	python benchmark_v65_walkthrough.py -T sm8150

In order to exercise the auto-vectorization compilation option, please set USE_CONV3x3_C to 1 in file hexagon.min.
If additionally change the USE_CONV3x3_AUTOVEC_FLAG to 0 will turn off the auto-vectorization compilation flag to use the pure C version.
    
Simple profiling summary will be displayed to shell, and saved to a .csv file if -o option is used. 
DSP logs (i.e. QDXM or Mini-DM) will contain slightly more info. For full profiling, 
follow instructions at <SDK>/docs/Tools_Hexagon Profiler.html.
 