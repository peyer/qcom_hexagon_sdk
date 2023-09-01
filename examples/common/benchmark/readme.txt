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

It should be located in the Hexagon SDK installation, at <SDK>/examples/common/benchmark

To build for simulation, do (substitute build flavor as desired)
    make tree VERBOSE=1 V=<static build flavor, e.g. hexagon_Release_toolv81_v62>

to run the simulation, do 
    hexagon-sim <static build flavor>/benchmark_q [options]
    
To build and install to target, do
    make tree VERBOSE=1 android_Release
    adb root
    adb wait-for-device
    adb remount
    adb push android_Release/ship/benchmark /vendor/bin
    adb shell chmod 755 /vendor/bin/benchmark
    make tree VERBOSE=1 V=<dynamic build flavor, e.g. hexagon_Release_dynamic_toolv81_v62>
    adb push <dynamic build flavor>/ship/libgemm_skel.so /vendor/lib/rfsa/adsp

To run on target, 
adb shell /vendor/bin/benchmark [options]

usage (or, run benchmark without options to display current usage):

  benchmark -f function_name [-cdsv] [-w width] [-h height] [-L rpc_loops]
            [-l dsp_loops] [-u usec] [-p power_level] [-y latency] [-m mode]
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
       crash10 - a dummy memcpy that triggers a DSP PD crash every 10th invocation.

  -d: enable DSP DCVS algorithm for adjusting clocks to optimize power
       (OFF by default).

  -c: mark RPC buffers as uncached from the Apps processor.\n
       May reduce RPC overhead.
       
  -s: Skip bit-exactness checks (this saves test execution time, should not
       impact profiling results).

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

  -p power_level: Desired power level. 0 is highest-performance, i.e.
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

Simple profiling summary will be displayed to shell, and saved to a .csv file if -o option is used. 
DSP logs (i.e. QDXM or Mini-DM) will contain slightly more info. For full profiling, 
follow instructions at <SDK>/docs/Tools_Hexagon Profiler.html.
 