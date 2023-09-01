# Qualcomm Hexagon Libraries Example

This example illustrates how to use the Qualcomm Hexagon Libraries (QHL) located under $HEXAGON_SDK_ROOT/libs/common/qhl.

Currently, QHL is only supported for V66 targets. 

## Overview

The example calls a few functions from the qhmath and qhblas libraries and sends their output to the console.

This example uses a different build approach from the other example projects in the SDK: it uses a standalone Makefile and doesn't rely on either CMAKE or make.d.

## Instructions

All step-by-step instructions for building and running the test both on simulator and on target are captured in the run.sh script.

You may run the script directly.  To do so, simply run './run.sh' from a bash shell in Linux, or execute the same script from your Windows prompt by running 'bash run.sh'.

You can also execute manually each instruction present in the script to reproduce separately the build, run on simulator, and run on target steps.

