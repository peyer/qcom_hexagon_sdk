# Intro

This collection of tutorials will introduce you to the Hex-NN API.

This C-language API allows you to construct a graph of tensor-operations
which will run on the Hexagon DSP.  Using fastRPC, we can use this API
from ARM binaries.  After completing these tutorials, you should be able
to build your own ARM binaries which construct graphs that run efficiently
on the DSP, utilizing the high throughput and low power of the Hexagon DSP
for your deep network inferences.


# Recommended Order

These tutorials build on each other, so advanced users with little time can
skip to Tutorial-004 which shows everything except the quantization nodes
presented only in Tutorial-003.  Users who desire more clarity should begin
with tutorial-001, which lays a solid foundation and proceed in order,
noticing the new content introduced in each subsequent tutorial.
"diff <file-A> <file-B>" may also be a useful tool for highlighting the
areas where new concepts are introduced.


# Overview of Tutorials

## Tutorial 001
* part of nn_graph's API for building and running graphs
* Minimal required header-files


## Tutorial 002
* nn_graph's API for building and running graphs
* The fastRPC glue required to get the ARM CPU to speak to the DSP
* Minimal required header-files
* Inputs and outputs
* Const nodes


## Tutorial 003
* nn_graph's API for building and running graphs
* The fastRPC glue required to get the ARM CPU to speak to the DSP
* Minimal required header-files
* Inputs and outputs
* Const nodes
* Quantization and dequantization
* Improved error checking using macros


## Tutorial 004
* nn_graph's API for building and running graphs
* The fastRPC glue required to get the ARM CPU to speak to the DSP
* Minimal required header-files
* Inputs and outputs
* Const nodes
* Improved error checking using macros
* Benchmarking
* Node debug
* Validation
* *NOTE: This tutorial covers everything from previous tutorials,
      except quantization, which is only covered in Tutorial-003.*

# Build instructions

## Building the dynamic Hexagon NN library

The Hexagon NN library may either be built directly from the $HEXAGON_NN folder or from this tutorial directory, which leverages the hexagon.min building rules specified for this tutorial.

For example, in order to build the dynamic library for a V66 device and from this current tutorial location, type the following command:
```
make tree VERBOSE=1 V=hexagon_Release_dynamic_toolv83_v66 V66=1
```

## Running on simulator

As part of building the dynamic library, the make utility also runs the same examples on the simulator.  As a result, the logs that result from executing the previous make command include a run of the first four tutorial examples on the simulator.

If you want to build and run only one example, specify the tutorial you want to build with the GRAPH command line argument:

```
$ make tree GRAPH=001-nop VERBOSE=1 V=hexagon_Release_dynamic_toolv83_v66 V66=1
```

*Note: If you want to re-run a tutorial, you need to touch its source first to force the executable to be re-built and executed again, or you can capture the simulation command (i.e. starting with hexagon-sim) emitted by the make and re-issue it.*


## Running on target

### Building the Android executables

* Build the Android executables for tutorials 001-004 that will offload Hexagon NN execution to the ADSP (for targets where HVX is on ADSP, such as SDM830 and SDM835):
```
$ make V=android_ReleaseG tree
```

* Build the Android executables for tutorials 001-004 that will offload Hexagon NN execution to the CDSP (for targets where HVX is on CDSP, such as SDM845, SDM670, and SM8150):
```
$ make V=android_ReleaseG CDSP_FLAG=1 tree
```

* Build only one tutorial
```
$ make GRAPH=001-nop V=android_ReleaseG tree
```

### Copying and executing tutorial executables 001-004 on device

```
$ adb root
$ adb push android_ReleaseG/001-nop /vendor/bin/
$ adb push android_ReleaseG/002-add /vendor/bin/
$ adb push android_ReleaseG/003-quantized-add /vendor/bin/
$ adb push android_ReleaseG/004-xor-graph /vendor/bin/
$ adb shell chmod a+x /vendor/bin/00?-*
```

* Try to run them!
```
$ adb shell /vendor/bin/001-nop
$ adb shell /vendor/bin/002-add 0 0 0 1.5
$ adb shell /vendor/bin/003-quantized-add 0 0 0 1.5
$ adb shell /vendor/bin/004-xor-graph
```
