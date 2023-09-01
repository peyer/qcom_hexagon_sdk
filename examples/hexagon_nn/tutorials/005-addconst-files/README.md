# What you will learn

In this tutorial you will learn how to:
1) Write a custom op which runs on the DSP
2) Link your new op into the libhexagon_nn_skel.so build
3) Create a simple android executable which uses your new op

This is the first tutorial that expects you to make some actual changes to the nn_graph implementation itself though all required code is provided.

# Overview:

* Ops are defined in `$HEXAGON_NN/hexagon/ops/src/` and must provide the following:
    * A struct of type `nn_node_ops` which provides the following functions:
        1) .execute - called by `hexagon_nn_execute()`, operates on tensors
        2) .check - called during setup, validates number of in/out tensors
        3) .ctor - constructor, typically just 'node_alloc_common'
        4) .dtor - destructor, typically just 'node_free_common'
    * Implementations of the `.execute` and `.check` functions
    * The name of the `nn_node_ops` struct should be of the form: `struct nn_node_ops nn_ops_for_<OP_NAME>`

* Files added in `$HEXAGON_NN/hexagon/ops/src/` should be referenced in `$HEXAGON_NN/hexagon/files.mak`

* Ops defined as 'struct nn_node_ops nn_ops_for_<OP_NAME>' should be declared as `DEF_OP(<OP_NAME>)` in `$HEXAGON_NN/interface/ops.def`

* Once declared and defined, you can instantiate your new ops as `OP_<OP_NAME>`. For example, like the following:
```
hexagon_nn_append_node(gid,nid,OP_AddConst_8,NN_PAD_NA,NULL,0,NULL,0);
```

# Detailed Exercise:
1) Examine the `op_addconst.c` file.  Notice that it creates an `nn_node_ops` struct, and defines the functions used for `.execute` and `.check`.
    * Now, copy it into the `$HEXAGON_NN` repo as:
    ```
    $ cd $HEXAGON_SDK_ROOT/examples/hexagon_nn/tutorials
    $ cp 005-addconst-files/op_addconst.c $HEXAGON_NN/hexagon/ops/src
    ```
2) Add the `op_addconst.c` to `$HEXAGON_NN/hexagon/files.mk` (within `HEXAGON_NN_C_SRCS`) so it can be found for compilation
    * You'll just add a line like the following: `hexagon/ops/src/op_addconst.c \`
    	* Shortcut: `$ sed -i -e 's/HEXAGON_NN_C_SRCS :=/HEXAGON_NN_C_SRCS := hexagon\/ops\/src\/op_addconst.c/' $HEXAGON_NN/hexagon/files.mak`
    * Similarly, you could add assembly files to HEXAGON_NN_ASM_SRCS.  Remember that C/ASM function names must not collide.

3) Add the new op-name to $HEXAGON_NN/interface/ops.def so it will be available
    * You'll just add a line like the following, near the end of the file: `DEF_OP_WREF(AddConst_8)`
        * Shortcut: `$ sed -i -e 's/\#ifdef __SELF_DEF_OP_WREF/DEF_OP_WREF(AddConst_8)\n\#ifdef __SELF_DEF_OP_WREF/' $HEXAGON_NN/interface/ops.def`
    * *Note: `DEF_OP_WREF` is just like `DEF_OP` but additionally defines an `OP_<OP_NAME>_ref` intended to be a reference implementation.*

4) Now you can compile libhexagon_nn_skel.so for your desired target flavor and your op will be available.
    ```
    $ cd $HEXAGON_NN && make VERBOSE=1 V=hexagon_ReleaseG_dynamic_toolv83_v65 tree
    ```
5) Build a testcase that uses your new op
    * Ensure that there is a C-file in a location the Makefile can build
    ```
    $ cd $HEXAGON_SDK_ROOT/examples/hexagon_nn/tutorials
    $ ln -s 005-addconst-files/005-addconst.c 005-addconst.c
    ```
    * Add tutorial 005 following 004
    ```
    $ sed -ie "s/004-xor-graph/004-xor-graph 005-addconst/" android.min
    ```
    * Build the testcase
    
    	* *Note: CDSP_FLAG=1 is necessary only for targets with a CDSP, such as SDM660, SDM845, SDM670, SM8150, and beyond.*
    
    ```
    $ make V=android_ReleaseG tree VERBOSE=1 CDSP_FLAG=1
    ```

6) Modify tutorials/Makefile to deploy and run for you.  E.g.:
    ```
    $ adb shell mkdir -p /vendor/bin/nn_tutorials
    $ adb push android_ReleaseG\ship\005-addconst /vendor/bin/nn_tutorials
    $ adb push $HEXAGON_NN/hexagon_ReleaseG_dynamic_toolv83_v65/ship/libhexagon_nn_skel.so /vendor/lib/rfsa/adsp
    $ adb shell chmod 755 /vendor/bin/nn_tutorials/005-addconst
    $ adb shell /vendor/bin/nn_tutorials/005-addconst
    ```
7) *(Recommended)* Add your new op to `$HEXAGON_NN/docs/ops.txt`, purely for documentation:
    ```
    AddConst_8:
    	input 0: tensor (uint8)
	input 1: scalar (uint8)
    	output 0: tensor (uint8)
    	Adds scalar value to input tensor, producing output tensor
    ```

# What to Expect:
The binary executable should accept four integer inputs (a 2x2 tensor).
It should add 42 to each of these values and present the output as a 2x2 tensor:
```
$ 005-addconst 0 1 2 3
    [ 42 43 ]
    [ 44 45 ]
```
