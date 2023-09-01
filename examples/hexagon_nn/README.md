# Hexagon NN examples

Examples on how to use the Hexagon NN library.

## Requirements

* Linux or Windows: *Instructions below show how to build with Linux. The very same flow is used in Windows.*
* Hexagon SDK: *Use version >= 3.4.2*

## Setup

### Hexagon SDK Environment
Ensure you are running from a bash shell and that your Hexagon SDK environment is properly setup. Assuming the SDK is installed under /local/mnt/worspace/, you can do this by typing:
```
> bash
$ source /local/mnt/workspace/Hexagon_SDK/3.5.0/setup_sdk_env.source 
```

### Device signing

If you haven't done so already, please sign your device before running any of the Hexagon NN examples.  You may do so by running the testsig.py script located under <HEXAGON_SDK_ROOT>/scripts/ and documented in <HEXAGON_SDK_ROOT>/docs/Tools_Scripts.html. Alternatively, you may run the walkthrough script tutorials_walkthrough.py from the tutorials folder as this script will sign your device by default.

### Hexagon NN Setup
The hexagon-nn examples project has a dependency on the hexagon-nn library that is tracked through the environment variable `HEXAGON_NN`.  The `setup_hexagon_nn.source` script will assist you in ensuring that this dependency is met.

#### Usage
```
$ source setup_hexagon_nn.source [<nn graph current or desired location> [<repo> [<patch>]]]
```

* If you do not have a preference on where hexagon-nn should be located, let the `setup_hexagon_nn` script set this variable for you.  

* If no `hexagon-nn` is found at the expected location, the setup_hexagon_nn.source will download `hexagon-nn` from its default repository.  Again, you can overwrite that default by providing your own location as the second input argument to the script:

* Finally, if you have been provided with a patch to hexagon-nn.git, you can specify it as a third argument.

In summary, here are some examples of supported commands:
```
$ source setup_hexagon_nn.source
$ source setup_hexagon_nn.source /some/alternate/folder/hexagon-nn
$ source setup_hexagon_nn.source hexagon-nn https://source.codeaurora.org/quic/hexagon_nn/nnlib
$ source setup_hexagon_nn.source hexagon-nn https://source.codeaurora.org/quic/hexagon_nn/nnlib /local/mnt/workspace/patch/1234.diff
```

#### Notes
* When you run the script, if no hexagon-nn library can be found at the expected location, you will prompted to confirm whether you want to download it. Answer `y` to accept.

If the installation is successful, you should now see the hexagon-nn library content by typing:
```
$ ls $HEXAGON_NN
```

## Using virtual environments
*Note: Using a virtual environment is optional.*

virtualenv is a tool to create isolated Python environments. virtualenv creates a folder which contains all the necessary executables to use the packages that a Python project would need. 
This approach will ensure that your python dependencies are identical to those with which the library has been tested internally and thus maximize your chances of running your code successfully from the beginning.

Further information on virtual environments and setup instructions can be found in ${HEXAGON_SDK_ROOT}/tools/python_venv/README

## Building the Hexagon NN library

Instructions in this section should be executed before running the tutorials.

### Defining the interface needed to build Android applications using Hexagon NN

```$ cd $HEXAGON_NN```

* For targets where HVX is on ADSP, such as SDM820 and SDM835: ```$ make V=android_ReleaseG tree```

* For targets where HVX is on CDSP, such as SDM845, SDM670, and SM8150: ```$ make V=android_ReleaseG CDSP_FLAG=1 tree```

### Building the Hexagon shared object

```$ cd $HEXAGON_NN```

* Build for v60/adsp, ```$ make VERBOSE=1 V=hexagon_ReleaseG_dynamic_toolv83_v60 tree```

* Build for v65/cdsp, using v60-compatible instructions, ```$ make VERBOSE=1 V=hexagon_ReleaseG_dynamic_toolv83_v65 tree```

* Build for v65/cdsp, ```$ make VERBOSE=1 V65=1 V=hexagon_ReleaseG_dynamic_toolv83_v65 tree```

* Build for v66/cdsp, ```$ make VERBOSE=1 V66=1 V=hexagon_ReleaseG_dynamic_toolv83_v66 tree```

*Note: Code compiled with the V65=1 flag will not run on a V66 device as the Hexagon NN library makes use of a special V65-only unpublished multiplier instruction. (Generally, V66 is backward-compatible with V65. This multiplier instruction used in Hexagon NN is an exception to that rule.)*

### Copying the shared-objects to device

*Note: Your device may need configuration involving the following commands for the write to work*
```
$ adb root
$ adb disable-verity
$ adb remount -o rw,remount /system
$ adb root
```

Push the shared-objects to the target device
```
$ adb push hexagon_ReleaseG_dynamic_toolv83_v60/libhexagon_nn_skel.so /system/vendor/lib/rfsa/adsp
```
*Note: Replace 'hexagon_ReleaseG_dynamic_toolv83_v66' with whichever version you compiled*

