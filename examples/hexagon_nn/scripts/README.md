All scripts provided in this folder are as is. They are tools intended to provide examples on how one could interface the Hexagon NN library.

* caffe_quant.proto
Caffe proto file with SNPE custom quantization fields.

* caffe_quant_pb2.py/pyc
Python file generated from caffe_quant.proto using protoc --python_out=. caffe_quant.proto

* caffe_to_hexagon_nn.py
Converts Caffe model to hexgon NN implementation. (Only tested on a handful of networks so far; will not support most of the common networks available today.)

* dat2img.py:
Converts raw image RGB data to PNG image format.

* dump_for_tensorboard.py:
Takes a Tensorflow model to generate a file that can be fed into tensorboard to display the corresponding graph.

# imagedump.py:
Dumps a .jpg to a .dat file format consumed by Hexagon NN

* img_to_dat.py
Converts and preprocesses .jpg and .bmp files to .dat files according to user-specified parameters for use by Hexagon NN.

* memdbg.py:
Reports memory allocation and deallocation to detect possible memory leaks.

* tensorflow_to_hexagon_nn.py:
Converts tensorflow model graph to hexagon NN implementation with optional test wrapper, and generate reference test data.
