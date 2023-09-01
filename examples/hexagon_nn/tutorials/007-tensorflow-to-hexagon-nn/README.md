# What you will learn

In this tutorial you will learn how to use a python script to transform a publicly available graph into a form which can run on the Hexagon DSP.

# Overview

There are many popular formats for neural networks.  Depending on the network you wish to run, you may need to import a graph from one of them.  The basic task is always the same:
1) Convert the graph to an equivalent graph using only the available Hexagon NN operations.
1a) This may require replacing ops with equivalent ops or subgraphs of ops.
1b) This may require implementing missing ops. (See tutorials 005 and 006)
2) Store the graph and weights in a form conducive to the following runtime operations:
2a) Instantiate const nodes for all weights
2b) Instantiate op nodes for all operations, with required connectivity
It's worth noting that Hexagon NN does not support certain features, like "while" loops or other forms of cyclic graphs, though these features may eventually be supported.

In this tutorial, we'll use a sample python script to convert the popular inceptionv3 network into a form that will run on Hexagon NN.  Because we want the network to run fast and utilize HVX, we'll start from a network that uses 8-bit weights.


# Exercise

# Start from a frozen protobuf.  This means it's got both the graph and the weights.  (It should be moderate in size, e.g. 24-100MB for inceptionv3, because of all that data!)
There's an excellent tutorial at https://petewarden.com/2016/05/03/how-to-quantize-neural-networks-with-tensorflow/
Those instructions won't quite work for us, so use it only as a reference.  Also, it's got instructions for downloading an inceptionv3 model:
```
curl http://download.tensorflow.org/models/image/imagenet/inception-2015-12-05.tgz -o /tmp/inceptionv3.tgz
tar xzf /tmp/inceptionv3.tgz -C /tmp/
```

This drops a 96MB protobuf in /tmp/classify_image_graph_def.pb

We'll next quantize this protobuf, which converts the weights from 32-bit floating point into 8-bit fixed-point values.

First, make sure you've built the 'transform_graph' tool.

For example, install bazel, retrieve the tensorflow source, and build it.  This flow should lead you to execute the following command.

```
    bazel build tensorflow/tools/graph_transforms:transform_graph
```

Please note also that the internal script we use has a dependency on tensorflow as well.  The version of tensorflow that you use to build transform_graph tool and run the internal script should be the same.  For example, if your python environment specifies `tensorflow==1.14`, you should download and install the same version for tensorflow:

```
    git clone https://github.com/tensorflow/tensorflow.git
    git checkout v1.14.0
```

Once transform_graph is installed, use it to convert the protobuf (you will probably need to give the full path to your bazel-bin below):
```
    bazel-bin/tensorflow/tools/graph_transforms/transform_graph  --in_graph=/tmp/classify_image_graph_def.pb --out_graph=/tmp/inception_v3_quantized.pb  --inputs="Mul" --outputs='softmax'  --transforms='add_default_attributes strip_unused_nodes(type=float, shape="1,299,299,3") remove_nodes(op=Identity, op=CheckNumerics) fold_constants(ignore_errors=true) fold_batch_norms fold_old_batch_norms quantize_weights quantize_nodes fold_constants strip_unused_nodes sort_by_execution_order'
```

Great!  Now, you should have a quantized protobuf that's also been transformed to do things like
1) Start from the 'Mul' node, removing leading nodes like 'DecodeJPEG' which we don't want and don't implement on DSP
2) Fold unwanted nodes like 'batchNorm' that are only useful during training, so they're not consuming compute or space
3) Fold constant nodes, e.g. subgraphs that have all-constant inputs, to save space.
4) Sort by execution order, which is required for Hexagon NN to run the graph

# (optional) View the original or final graph in tensorboard.  This is a great way to debug what's in your graph when things aren't working.  In my own example, I was able to debug several problems here:
  1) My const-folding was happening before batch-norm reduction, so I wasn't folding a bunch of nodes inside the batch-norm subgraphs, resulting in nn_malloc() failures because my graph was exceedingly large
  2) My front-end was consuming float inputs, but I was trying to feed uint8 in nn_graph, resulting in lots of NaN inputs which propogated into supernodes and manifested as infinite loops.
First, you'll need to load your graph into a tensorflow instance and dump some info for tensorboard to use.  This can be done with a very simple script:
  python ./tb_load.py /tmp/inception_v3_quantized.pb
Now, open tensorboard to view it
  ```
  tensorboard --logdir . &
  firefox http://localhost:6006 &
  ```


3) Convert the quantized protobuf into a C file
3a) This might only work with python2, so we'll load a virtualenv for python2 with all our required dependencies:
```
$ cd ../../
$ virtualenv -p python2 env2
$ source env2/bin/activate
$ pip install -r environments/req2.txt
$ cd -
```

3b) Now run the tool that reads a protobuf into tensorflow, and iterates over the nodes, emitting C code that will use the Hexagon NN API to build our graph:
```
python ../../scripts/tensorflow_to_hexagon_nn.py /tmp/inception_v3_quantized.pb ./inceptionv3_v1.yaml > iv3.c
```

4) NOTE that your quantized graph still has a floating-point first-layer which quantizes the input.  This cost is paid once per iteration and would be cheaper to do off the DSP.  We'll leave it for now, but you can see the nodes in init_graph() as:

```
$  APPEND_NODE("Mul",0x1023e,OP_INPUT,NN_PAD_NA,inputs_for_1023e,0,outputs_for_1023e,1);
$  APPEND_NODE("conv/Conv2D_eightbit/Mul__port__0/reshape",0x1023f,OP_Flatten,NN_PAD_NA,inputs_for_1023f,2,outputs_for_1023f,1);
$  APPEND_NODE("conv/Conv2D_eightbit/Mul__port__0/max",0x10240,OP_Max_f,NN_PAD_VALID,inputs_for_10240,3,outputs_for_10240,1);
$  APPEND_NODE("conv/Conv2D_eightbit/Mul__port__0/min",0x10242,OP_Min_f,NN_PAD_VALID,inputs_for_10242,3,outputs_for_10242,1);
$  APPEND_NODE("conv/Conv2D_eightbit/Mul__port__0/quantize",0x10244,OP_Quantize,NN_PAD_NA,inputs_for_10244,3,outputs_for_10244,3);
```

5) Copy into an nn_graph repo and build  (e.g. for 845(v65)
```
$ yes | cp -u iv3.c ${HEXAGON_NN}
$ make -C $HEXAGON_NN tree VERBOSE=1 V65=1 V=hexagon_ReleaseG_dynamic_toolv83_v65
$ make -C $HEXAGON_NN tree VERBOSE=1 V=android_ReleaseG CDSP_FLAG=1 V65=1 GRAPHINIT="iv3.c"
```


6) Copy it onto your phone, and test
```
$ adb push $HEXAGON_NN/android_ReleaseG/graph_app /data/
$ adb push $HEXAGON_NN/hexagon_ReleaseG_dynamic_toolv83_v65/libhexagon_nn_skel.so /system/vendor/lib/rfsa/adsp/
$ adb push $HEXAGON_NN/test/panda_299x299.dat /data/
$ adb shell chmod a+x /data/graph_app
$ adb shell /data/graph_app --input_to_float 1 --iters 20 /data/panda_299x299.dat
```
You should expect output like the following:
```
$ Run!
$ output size=4032
$ Rank,Softmax,index,string
$ 0,0.940308,169,giant panda
$ 1,0.002943,7,lesser panda
$ 2,0.000613,61,brown bear
$ 3,0.000534,878,earthstar
$ 4,0.000424,374,lawn mower
$ AppReported: 41682408
```
Although this performance is good, we can do better.  The first layer of our graph which converts float to int cannot use the HVX vectorized instructions.  We can see how much this has cost us by measuring the performance per-node.
```
$ adb shell /data/graph_app --input_to_float 1 --iters 20 --perfdump 1 /data/panda_299x299.dat  | grep __port__0
$ node,0x10244,conv/Conv2D_eightbit/Mul__port__0/quantize,OP_Quantize,executions,20,cycles,4057344,0.436610 %,cum_cycles,105436038,11.345947
```
We see that this node consumed 4,057,344 of our total 41,682,408 cycles.  In fact, if we rework our front-end to consume uint8 instead of float (and drop the --input_to_float option), we can get performance numbers like these:
```
$ Run!
$ output size=4032
$ Rank,Softmax,index,string
$ 0,0.920437,169,giant panda
$ 1,0.002365,7,lesser panda
$ 2,0.001022,61,brown bear
$ 3,0.000687,163,American black bear
$ 4,0.000628,374,lawn mower
$ AppReported: 29649491
```

