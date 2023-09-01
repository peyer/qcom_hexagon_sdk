# Qualcomm Hexagon Libraries

Set of libraries for Qualcomm Hexagon DSP.

## About the project

QHL is a set of libraries that are designed to work on Qualcomm Hexagon DSP.  They provide a set of functions performing various mathematical computations.
Since performance is, alongside with accuracy, crucial for each mathematical operation, most of the QHL implementation is written in assembly language and using C intrinsics.

QHL consist of following libraries:
* Qualcomm Hexagon Math (QHMATH)
  - common mathematical computations on real numbers
* Qualcomm Hexagon Complex (QHCOMPLEX)
  - common mathematical computations on complex numbers
* Qualcomm Hexagon Basic Linear Algebra Subprograms (QHBLAS)
  - basic linear algebra operations
* Qualcomm Hexagon Digital Signal Processing (QHDSP)
  - digital signal processing operations

## Build instructions

In order to build all the libraries, run the following command:

```
make all
```

In order to remove all binaries, run the following command:

```
make clean
```

In order to see an example of how to invoke the libraries, please see examples/common/qhl.

## Further reading

For more detailed explanation of QHL project, please see the doxygen documentation in the **docs** folder.
