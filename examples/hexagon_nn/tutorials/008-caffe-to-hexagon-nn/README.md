# What you will learn

In this tutorial you will learn how to use caffe_to_hexagon_nn, a python script that transforms a publicly available caffe graph into a hexagon nn implementation.  

# A word of caution

This script was created recently and has only been tested successfully with VGG 16. However, the script is written so as to be easily expanded to support additional caffe layers as required for other networks.

# Prerequisite

We provide instructions to run assuming a Linux environment but Windows is supported as well.

Please follow the instructions in the README.md in the root folder of this project (i.e. `$HEXAGON_SDK_ROOT/examples/hexagon_nn`) to setup the proper environment variables and virtual environment.

You should also make sure to have a valid input data file on device to execute this tutorial. If you haven't one already, you can do so with:
```
$ adb push animals_vgg_b/panda_224x224_vgg_b.dat /vendor/etc/
```

Further down in this tutorial, we explain how this directory was generated from jpg test images.

Furthermore, make sure to have the valid human-readable labels for this tutorial pushed to your testing device. If you haven't done so already, you can do so with:
```
$ adb push ${HEXAGON_NN}/test/vgg_labels.txt /vendor/etc/
```

# Usage overview

Invoking the script with option -h will display its usage example:
```
$ python caffe_to_hexagon_nn.py -h
```
The usage description indicates that the following parameters are expected:
  proto                 Path to prototxt
  model                 Path to caffemodel
  root                  Root name for generated implementation

For example,
```
$ python caffe_to_hexagon_nn.py proto.txt model.bin vgg
```
tells the script to turn the caffe protobuf file, proto.txt, and its model, model.bin, containing the coefficients, into two C files, vgg.c and vgg_data.c. These two files, when compiled, will create a fixed-point nn graph implementation of the Caffe graph running on the Hexagon DSP.

Note: 32-bit bias coefficients are generated in a third file, vgg_data_32b.c.

A number of options can be specified in the command line or in a config file:

`--debug_level DEBUG_LEVEL` will set the debug level used in the nn graph implementation to DEBUG_LEVEL

`--perf_dump` will generate readable profiling output in graph_app

`--generate_float_imp` will generate a floating-point reference implementation for debugging and benchmarking purposes. All operations will execute using floating-point arithmetic. One inference may take a long time (tens of minutes to an hour) to complete

`--check_init` will make run-time checks for each new node created during the nn graph initialization.

`--leave_coefficients_unchanged` will not vgg_data\*.txt files, thus saving on compilation time. It is a useful option when iterating with the script on creating a new implementation for which the coefficients have not changed.

`--use_8b_biases` will use 8-bit bias adds instead of 32-bit bias adds.

`--use_float_input` will generate a graph which expects float-based input instead of uint8-based

`--max_input MAX_INPUT / --min_input MIN_INPUT` are required when using uint8-based input; these values represent the max/min of the float range that your input maps to

# Usage examples

## First example: Generating a fixed-point nn graph implementation with 32-bit bias adds from a caffe floating-point model.

1. Download the VGG ILSVRC 16 caffe model and prototxt file

The Caffe model and prototxt files for VGG 16 are publicly available. For example, you can download them as follows:
```
$ wget https://gist.githubusercontent.com/ksimonyan/211839e770f7b538e2d8/raw/ded9363bd93ec0c770134f4e387d8aaaaa2407ce/VGG_ILSVRC_16_layers_deploy.prototxt
$ wget  http://www.robots.ox.ac.uk/~vgg/software/very_deep/caffe/VGG_ILSVRC_16_layers.caffemodel
```

2. Generate a VGG implementation for hexagon nn that uses 32-bit bias adds

By default, the script will use 32-bit bias add for expressing the Caffe convolution layers. However, because the script expects uint8-based input by default, you must specify the float range that these values map to. Due to VGG preprocessing (using an average of the per-channel ILSVRC mean image values), the range for this tutorial should be [-114.2, 140.8]. So the script can be invoked with the following options:
```
$ python ../../scripts/caffe_to_hexagon_nn.py VGG_ILSVRC_16_layers_deploy.prototxt VGG_ILSVRC_16_layers.caffemodel vgg --min_input -114.2 --max_input 140.8
```
Note: The script can take a few minutes to execute because of the large arrays that need to be processed.

3. Compile the code

Copy the source files the script generated into the ${HEXAGON_NN} directory and use the Hexagon NN Makefile to build the dsp dynamic library and the android executable:
```
cp -u vgg.c vgg_data.c vgg_data_32b.c vgg.h vgg_data_float.c ${HEXAGON_NN}
make -C $HEXAGON_NN tree VERBOSE=1 V66=1 V=hexagon_Release_dynamic_toolv82_v66
make -C $HEXAGON_NN tree VERBOSE=1 V=android_Release CDSP_FLAG=1 GRAPHINIT="vgg.c vgg_data.c vgg_data_32b.c vgg_data_float.c"
```
or simply run the existing build script:
```
./v66_build.sh
```

*Note: This assumes that your DSP target is a V66 ISA. If you have a V65 ISA, the code needs to be rebuilt for V65 instead as reflected in the script below. Code compiled with the V65=1 flag will not run on a V66 device as the Hexagon NN library makes use of a special V65-only unpublished multiplier instruction. (Generally, V66 is backward-compatible with V65. This multiplier instruction used in Hexagon NN is an exception to that rule.)*
```
./v65_build.sh
```

*Note: In order to reduce the size of graph_app, one should exclude files that are not being used from the list, GRAPHINIT="vgg.c vgg_data.c vgg_data_32b.c vgg_data_float.c", above.*

4. Push the DSP dynamic library and the executable, and execute the code

```
adb push ${HEXAGON_NN}/android_Release/ship/graph_app /data
adb push ${HEXAGON_NN}/hexagon_Release_dynamic_toolv82_v66/ship/libhexagon_nn_skel.so /vendor/lib/rfsa/adsp
adb shell /data/graph_app --iters 1 /vendor/etc/panda_224x224_vgg_b.dat --labels_filename "/vendor/etc/vgg_labels.txt"
```  

Alternatively, run the script v66_run.sh

*Note: Your device may not have a /vendor partition large enough to accomodate graph_app.  This is why in the example above we push the executable to the /data partition even though the convention would call for the executable to be pushed to the /vendor partition instead. The /data partition is generally large enough If that's the case, consider pushing graph_app to /data and running the application from there instead.*

The --labels_filename parameter specifies the labels that should be used for this execution of graph_app. If this argument is not given, there is a default set of labels that will be used. However, the default set of labels will not result in the correct human-readable output for this tutorial.

If your execution was successful, you should see the following displayed at the end of your command line:
```
Rank,Softmax,index,string
0,0.999803,388,giant panda
1,0.000166,296,ice bear
2,0.000011,294,brown bear
3,0.000011,387,lesser panda
4,0.000007,295,American black bear
AppReported: <DSP cycles (not including FastRPC overhead)>
```

### Running Multiple Images on graph_app
To test a directory of .dat's during one run of graph_app, please see `../../scripts/run_graph.py`. The top of this script contains its usage information.

## Second example: Recompiling the code with a different debug level but the same coefficients.

If you want to make a change that only affects the implementation but preserves the same coefficients, you can use the flag --leave_coefficients_unchanged and cut down drastically your compilation time.  For example, if you no longer care about debugging messages, you can set the debug level to 0 but also add the --leave_coefficients_unchanged argument to not regenerate the data file.

```
$ python ../../scripts/caffe_to_hexagon_nn.py VGG_ILSVRC_16_layers_deploy.prototxt VGG_ILSVRC_16_layers.caffemodel vgg --debug_level 0 --leave_coefficients_unchanged --min_input -114.2 --max_input 140.8
```

## Third example: Using 8-bit bias add instead.

If you are trying to use less memory, you may want to experiment with an implementation that uses 8-bit bias adds. This approach may reduce accuracy. Note that since we never generated 8-bit coefficients before, we should no longer add the leave_coefficients_unchanged argument.

```
$ python ../../scripts/caffe_to_hexagon_nn.py VGG_ILSVRC_16_layers_deploy.prototxt VGG_ILSVRC_16_layers.caffemodel vgg --use_8b_biases --min_input -114.2 --max_input 140.8
```

## Fourth example: Going back to first implementation.

Since 32-bit coefficients are kept in a separate file and are named differently, it is possible to switch between an 8-bit bias and a 32-bit bias implementation without regenerating the coefficient files.  As a result, you can re-use the command line from the first example with leave_coefficients_unchanged set and be able to recompile the project quickly.

```
$ python ../../scripts/caffe_to_hexagon_nn.py VGG_ILSVRC_16_layers_deploy.prototxt VGG_ILSVRC_16_layers.caffemodel vgg --leave_coefficients_unchanged --min_input -114.2 --max_input 140.8
```

## Fifth example: Experimenting with different coefficients.

You may be in a situation where you want to override the coefficients generated by the script. If you want to do so and retain the ability to go back and forth between the script coefficients and your custom coefficients, you should maintain your custom coefficients in a separate file.

In order to override arrays, you need to indicate in dic_external_coefficients how to map the names of coefficient arrays generated by the script to the array names you want to use instead.  You can also redefine in dic_external_extrema the extrema values of these arrays if these changed as well.

In the provided config file, config_override_first_bias.yaml, we show how to specify the options from the third example using the config file format. In addition, we show how to override the weight array and its extrema for the first layer:
```
$ python ../../scripts/caffe_to_hexagon_nn.py VGG_ILSVRC_16_layers_deploy.prototxt VGG_ILSVRC_16_layers.caffemodel vgg --config config_override_first_bias.yaml
```

The generated vgg.c file has now modified values for the extrema of the biases used in the first convolution:
```C
float max_biases_conv1_1[] = {2.0};  // changed from 2.06403708458
float min_biases_conv1_1[] = {0.0};  // changed from -0.0163164986923
```

In order to re-build the project, we now need to add one more file to the project, custom_vgg_arrays.c, which contains the array(s) we want to use as override:
```
cp -u vgg.c vgg_data.c vgg_data_32b.c vgg.h custom_vgg_arrays.c ${HEXAGON_NN}
make -C $HEXAGON_NN tree VERBOSE=1 V=android_ReleaseG CDSP_FLAG=1 V65=1 GRAPHINIT="vgg.c vgg_data.c vgg_data_32b.c custom_vgg_arrays.c"
```

When you run the application again, the app will generate during the initialization of the graph some simple comparisons between the arrays and extrema generated by the script against those specified as override:

*********************************************************************
```
array                | size       | min %err  | max %err | avg(%err) | avg(abs(%err))
biases_conv1_1       | 64         | -0.395376 | 0.000000 | -0.042953 | 0.042953
```

This indicates that the first bias array, of size 64, was overridden. The max positive and average absolute error are small but the max negative (min %err) is large (40%).  In addition, the report indicates that one value in particular shows a large difference between the array generated by the script and the override array: 
```
  biases_conv1_1[0] override (float_override=-0.100000, fix_override=-128) != original (float_original=0.734242, fix_original=-36 from [-0.016316,2.064037])
```

This makes sense since in the bias array specified in custom_vgg_arrays.c, we replaced the first value, 92 (which as a signed number is 92-128=-36), with 0 (which as a signed number is -128).

The report also provides some statistics on the overridden extrema:

Range for const arrays: override vs. original [min:zero point:max]
```
  biases_conv1_1      : [-0.100000;12.142858;2.000000] vs. [-0.016316;2.000000;2.064037]
 ```

Do note how by selecting a min and max of -.1 and +2.0 and dividing that interval in 255 even segments, 0 cannot be represented as an int perfectly.  With weights, even minor errors made on that zero value can quickly accumulate and produce incorrect output. It is therefore important to monitor where that zero point falls. This is not as important a consideration for bias arrays.

The application still produces the expected output, indicative of the fact that some errors on the bias array will not have a noticeable impact on the overall accuracy of te graph. However, if you change the min override value from -.1 to -100., you will force the output of the first layer, after ReLU, to all be zero, and as a result the graph application will fail as expected.

## Sixth Exammple: Generating More Test Images for VGG16
Different neural network implementations require different preprocessing techniques and input formats. We have provided the script, ../../scripts/img_to_dat.py, to help users preprocess .jpg images into .dat files. Here, we will provide examples and explanations for how to generate two sets of test images suitable for input into different Hexagon NN implementations. The two graphs take their input in two different forms which require slightly different configurations in the first few nodes of the implementation. Otherwise, the implementations are the same.

### Generating uint8-based .dat Files
To generate uint8-based test images for a VGG16 network, we will use the aforementioned script, scripts/img_to_dat.py. We will convert the .jpg images located in `${HEXAGON_NN}/test/animals` to generate .dat files suitable for input into our network by using the following command:

```
python ../../scripts/img_to_dat.py --root vgg --input ${HEXAGON_NN}/test/animals --size 224 --bgr --byte
```

This script will create two directories inside of `${HEXAGON_NN}/test` named `animals_cropped` and `animals_vgg_b`. The directories contain the cropped images derived from the images in `animals` and the .dat files derived from the images located in `animals_cropped`, respectively. The former directory allows you to verify that your provided images were cropped and resized properly by our script. The .dat files in the latter directory are suitable for input into an Hexagon NN implementation that expects uint8-based input, denoted by the `_b`.

A Hexagon NN implementation that expects uint8-based input can be generated by `caffe_to_hexagon_nn.py` by running the following command:

```
python ../../scripts/caffe_to_hexagon_nn.py VGG_ILSVRC_16_layers_deploy.prototxt VGG_ILSVRC_16_layers.caffemodel vgg --min_input -114.2 --max_input 140.8
```

*Note: graph_app can then be built and ran on a target device using the same methods as in previous examples. However, be sure to push your new test images to the device and pass them to graph_app on the command line.*

### Float-based Input
To generate float-based test images for a VGG16 network, we will use the same script as above. Using the same directory, `${HEXAGON_NN}/test/animals`, we can generate .dat files for input into our graph using the following command:

```
python ../../scripts/img_to_dat.py --root vgg --input ${HEXAGON_NN}/test/animals --size 224 --mean_r 123.68 --mean_g 116.779 --mean_b 103.939 --bgr
```

Then, a Hexagon NN implementation that expects float-based input can be generated by running the following command:

```
python ../../scripts/caffe_to_hexagon_nn.py VGG_ILSVRC_16_layers_deploy.prototxt VGG_ILSVRC_16_layers.caffemodel vgg --use_float_input
```

Like in the previous example, be sure to push the new test images to your device and pass them on the command line to graph_app. However, for float-based input, the additional command line parameter, `--elementsize` must be specified on the command-line when running graph_app. The following command can be used to run graph_app with this graph and one of the test images generate above:

```
adb shell /vendor/bin/graph_app --iters 1 --elementsize 4 /vendor/etc/panda_224x224_vgg_f.dat --labels_filename "/vendor/etc/vgg_labels.txt"
```

We specify `--elementsize 4` because the input file, `/vendor/etc/panda_224x224_vgg_f.dat`, is float-based. This argument is 1 (to denote uint8-based input) by default, as in the previous example.

When using float-based input, the user does not specify min/max values to caffe_to_hexagon_nn.py because the generated Hexagon NN implementation will autoquantize the float-based input according to the detected float min/max. When the Hexagon NN implementation expects uint8-based input, the .dat files can be considered prequantized to the range [0,255] and therfore the float min/max that these values correspond to must be specified. For a uint8-based .dat preprocessed for VGG, the quantized range [0, 255] maps to the float range [-114.2, 140.8] because of the per-channel mean image subtraction that must occur in the preprocessing for VGG.

*Note: Please see the img_to_dat.py script for more details regarding its usage and how one might preprocess images for other types of networks.*

### Generating a Single .dat File from a Single .jpg Image
img_to_dat.py can process a single .jpg file or an entire directory of .jpg images. If you want to preprocess a single .jpg image, use the `--input` command-line option like so:

```
python img_to_dat.py --root vgg --input ${HEXAGON_NN}/test/animals/panda.jpg --size 224 --mean_r 123.68 --mean_g 116.779 --mean_b 103.939 --bgr
```

*Note: The above command will generate a cropped image and .dat file that is located in the directory `${HEXAGON_NN}/test/animals`. Thus, panda.jpg should be moved to another directory if you don't want this output to be placed in this directory.*

## Seventh Example: Generating a Floating-Point Reference Implementation
Generating a floating-point implementation, can be useful for debugging or benchmarking a quantized implementation. The following can be used to generate a floating-point reference implementation:

```
python ../../scripts/caffe_to_hexagon_nn.py VGG_ILSVRC_16_layers_deploy.prototxt VGG_ILSVRC_16_layers.caffemodel vgg --generate_float_imp
```

*Note: Generating a floating-point implementation takes a long time. Furthermore, running a floating-point implementation takes a long time. In total, expect to take approximately two hours to generate and run a floating-point graph with one image.*

## Eighth Example: Profiling a Hexagon NN Implementation
Profiling your graph is used to see which nodes are costly performance-wise. The following can be used to generate an implementation with readable profiling output when running with graph_app:

```
python ../../scripts/caffe_to_hexagon_nn.py VGG_ILSVRC_16_layers_deploy.prototxt VGG_ILSVRC_16_layers.caffemodel vgg --min_input -114.2 --max_input 140.8 --perf_dump
```

Furthermore, you must enable profiling on graph_app by specifying the `perfdump` option. The following is an example:

```
adb shell /vendor/bin/graph_app --iters 1 /vendor/etc/panda_224x224_vgg_b.dat --labels_filename "/vendor/etc/vgg_labels.txt" --perfdump 1
```

# Script implementation overview

Explaining the inner details of the script is beyond the scope of this tutorial.  However, the script is written so that the rules for parsing parameters from each caffe proto file and the rules for mapping caffe layers into nn graph tensors are localized in ```dic_layer_parsers``` and ```dic_layer_to_nodes```.

In theory, these are the only variables that need to be changed to add support for more caffe layers.  In practice, if the new layers that are added introduce new parameters or new actions not already supported, the script will require additional changes.
 
