########################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED 
# All Rights Reserved 
# Modified by QUALCOMM INCORPORATED on $Date$ 
########################################################################

mandelbrot example:
The mandelbrot.c application was chosen to demonstrate the capabilities of the hexagon
profilers gprof style output since it spawns multiple worker threads during its execution. 
Each "compute_fractal" worker thread creates a part of the mandelbrot fractal. After 
completion of the computation, a worker thread locks a mutex (Mx) and writes to the image 
buffer. The master thread (thread 0) then displays the contents of the image buffer and releases the mutex.

Running Example:
1) make			<-- Cleans/builds/runs the simulation with profiling enabled
2) make profile		<-- Generates gprof style output with Flat profile and Call graphs. This output is written
                        to multiple files with the following naming convention - gprof.t_{thread}.txt

Result:
make: This simulation will usually takes a bit of time to perform the required 
calculations for v4 or lower architectures. It is usually quick for v60. When the 
calculations are done, the output is displayed, followed by a printout of the mandelbrot 
fractal.

make profile: This will generate the gprof style files containing flat profile and the call graph of this simulation.
