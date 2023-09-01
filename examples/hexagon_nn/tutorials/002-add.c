// ******************************************************************
//  | | | |                | \ | || \ | |
//  | |_| | _____  ________|  \| ||  \| |
//  |  _  |/ _ \ \/ /______| . ` || . ` |
//  | | | |  __/>  <       | |\  || |\  |
//  \_| |_/\___/_/\_\      \_| \_/\_| \_/
//
//    ______      __             _       __         ____  ____ ___
//   /_  __/_  __/ /_____  _____(_)___ _/ /        / __ \/ __ \__ \.
//    / / / / / / __/ __ \/ ___/ / __ `/ /  ______/ / / / / / /_/ /
//   / / / /_/ / /_/ /_/ / /  / / /_/ / /  /_____/ /_/ / /_/ / __/
//  /_/  \__,_/\__/\____/_/  /_/\__,_/_/         \____/\____/____/
//
// ******************************************************************
//
// This tutorial introduces:
//   * nn_graph's API for building and running graphs
//   * The fastRPC glue required to get the ARM CPU to speak to the DSP
//   * Minimal required header-files
//   * Inputs and outputs
//   * Const nodes
//
// To do this, we'll construct a very simple neural-network that adds two
//   2x2 matrices.  We'll need to feed our input, which will be added to
//   a 2x2 constant, then output as a 2x2 matrix to the screen.



// hexagon_nn.h includes most of the things you'll need to create and run graphs.
// Its most important includes are nn_graph.h, which includes nn_graph_if.h.
// Together, these provide the data-types for input/output tensors,
//   and the API you'll use for initializing, building, preparing and running
//   your graphs.
// NOTE: hexagon_nn.h redefines malloc(), alloc(), etc. so
//   they become a compile-error "OOPS MALLOC".  This is because you should
//   use rpcmem_alloc() instead.
#include <hexagon_nn.h>

// hexagon_nn_ops.h defines the various graph operations (e.g. "MatMul"
//   and "Relu") which you can do.  Internally, it just expands
//   interface/ops.def into a usable format.  ops.def contains the list of
//   all implemented ops.
#include "hexagon_nn_ops.h"

// For printf, etc.
#include <stdio.h>

#ifdef __hexagon__
#include <stdlib.h>
#include <string.h>
int qtest_get_cmdline(char *, int);
#endif
// If you're already familiar with SDK programming for the DSP,
//   you've probably used fastRPC.  There's already lots of examples
//   documenting its use, and the purpose of this tutorial is to
//   expose the hexagon_nn_* API, so we'll ignore the fastRPC details.
// For these tutorials, we create a couple functions
//   fastrpc_setup() and fastrpc_teardown(), and some required includes.
// FastRPC allows our code running on the ARM to call functions located
//   on the DSP, quite seamlessly.
// To enable this ARM/DSP communication, we need to open a channel.
//   We'll also need to be careful later how we call functions that cross
//   the ARM/DSP partition, e.g. sending pointers, to ensure the ARM and
//   DSP see the same data.
#include "sdk_fastrpc.h"


// Data is passed into and out of nodes as tensors.
// Tensors are like matrices with more dimensions.
// The tensors we use typically have four dimensions, shown below.
// Here we define the size of our output tensor as a 2x2 array of floats.
#define OUT_BATCH 1
#define OUT_HEIGHT 1
#define OUT_WIDTH 2
#define OUT_DEPTH 2
#define OUT_ELEMENTSIZE (sizeof(float))
#define OUT_SIZE (OUT_BATCH*OUT_DEPTH*OUT_HEIGHT*OUT_WIDTH*OUT_ELEMENTSIZE)



// The structure of our ADD network looks like this, and has
//   three layers and four nodes:
//
//          [ 0.5, 3.14] <------Constant
//          [1000,  0.0]
//          "layer1a"
//           0x1001a
//                \.
// INPUT (2x2) --> Add --> OUTPUT (2x2)
//   "layer0"    "layer1"   "layer2"
//    0x1000      0x1001     0x1002
//


// We need to create some data structures describing our layers,
//   which we'll pass to the DSP as we append nodes to the graph.
//
// These structures describe the inputs and outputs of each node
//   (how the nodes attach to each other, and how they are sized)
//
// And we need arrays of constants to feed our const-node
//   (the weights of our model)

//***********************
// LAYER 0:  INPUT
//***********************

// The first layer of our graph is the "INPUT" node.
// It doesn't receive data from any other nodes, so its input list is empty.
static hexagon_nn_input empty_input_list[] = {
};

// The input node does produce outputs. (It's a "pure source")
// We'll store the data in "Batch,Height,Width,Depth" format, "BHWD" for short.
//   with depth as the most-major rank.
//   (Sequential depth items neighbor in memory)
// The 'rank' should always be 4.  'rank' is a placeholder for future
//   development within the nnlib kernel.
#define IN_BATCH 1
#define IN_HEIGHT 1
#define IN_WIDTH 2
#define IN_DEPTH 2
#define IN_ELEMENTSIZE (sizeof(float))
#define IN_SIZE (IN_BATCH * IN_HEIGHT * IN_WIDTH * IN_DEPTH * IN_ELEMENTSIZE)
static hexagon_nn_output layer0_output_list[] = {
        { .rank = 4,
          .max_sizes = {IN_BATCH, IN_HEIGHT, IN_WIDTH, IN_DEPTH},
          .elementsize = IN_ELEMENTSIZE, },
};


//***********************
// LAYER 1:  Add
//***********************

// The 'Add_f' operator takes two matrices of floats as inputs.
// Later, when we create actual nodes, we'll give them unique IDs.
// Here, we need to refer to the unique IDs of our inputs,
//   which are "0x1000"(layer0) and "0x1001a"(layer1a)
// These are just uint32_t which we've selected for legibility.
//   You can use any uint32_t which pleases you, as long as each node is unique.
// The output list is just specifying the max-size to reserve for this node.
static hexagon_nn_input layer1_input_list[] = {
        { .src_id = 0x1000, .output_idx = 0, },  // From INPUT-layer0
        { .src_id = 0x1001a, .output_idx = 0, }, // From const-layer1a
};
static hexagon_nn_output layer1_output_list[] = {
        { .rank = 4, .max_sizes = {1,1,2,2}, .elementsize = sizeof(float), },
};


//***********************
// LAYER 2:  OUTPUT
//***********************

// The output node has no outputs, so we will use a NULL list below.
static hexagon_nn_input layer2_input_list[] = {
        { .src_id = 0x1001, .output_idx = 0, }, // From Add-layer1
};


//***********************
// CONST WEIGHTS
//***********************

// Weights are stored in BHWD format, with depth as most-major rank.
// You can store weights as float, int, uint8_t, or whatever is supported
//   by the operation that is using them as inputs.
static float layer1a_consts[4] = {
        0.5,  3.14,
        1000, 0.0,
};




int main(int argc, char **argv) {
    int err;
#ifdef __hexagon__

    char *buf = malloc(1024);
    if (!buf) return -1;
    char *argvbuf = malloc(1024);
    if (!argvbuf)
    {
        free(buf);
        return -1;
    }
    // char buf[1024];  // for holding & parsing the command line
    // char argvbuf[1024];

    argv = (char**) argvbuf;
    argc = 0;

    // system call to retrieve the command line, supported by q6 simulator.
    qtest_get_cmdline(buf, 1024);

    // 1st argv is the program being run (i.e. "fastcv_test.ext") and its path
    argv[0] = strtok(buf, " ");

    // loop to pick up the rest of the command line args from the command line
    while (NULL != (argv[++argc] = strtok(NULL, " "))) {};
#endif

    // Start the ARM/DSP communications channel so we can call
    //   library functions that execute on the dsp.
    if (fastrpc_setup() != 0) return 1;

    // The nnlib API consists of functions that begin "hexagon_nn_*"
    // This prefix indicates that the function will actually run on the DSP.
    // To run a neural network we'll use this basic API:
    //   1) hexagon_nn_config            - Start nnlib, preparing globals
    //   2) hexagon_nn_init              - Initialize a new graph
    //   3) hexagon_nn_set_debug_level   - Enable debug
    //   4) hexagon_nn_append_node       - Add nodes to the graph
    //   5) hexagon_nn_append_const_node - Add constants (pure data, not ops)
    //   6) hexagon_nn_prepare           - Allocate memory, strategize,
    //                                     and optimize the graph for speed
    //   7) hexagon_nn_execute           - Run an inference
    //   8) hexagon_nn_teardown          - Destroy the graph, free resources

    // Ensures that nnlib is ready to start working.
    hexagon_nn_config();

    // Initialize a fresh, empty graph.  Return a graph-handle by reference.
    hexagon_nn_nn_id graph_id;
    if ((err = hexagon_nn_init(&graph_id)) != 0) {
            printf("Whoops... Cannot init: %d\n", err);
            goto TEARDOWN;
    }

    // Set power level (to max/turbo)
    if ((err = hexagon_nn_set_powersave_level(0)) != 0) {
            printf("Whoops... Cannot set power level: %d\n", err);
            goto TEARDOWN;
    }

    // Select our debug level.  0=none, >4=max
    // When creating new graphs, it's nice to have max debug
    //   even if you don't think you need it.
    hexagon_nn_set_debug_level(graph_id, 100);

    // Append a node to the graph.
    // We need to provide a unique-id so other nodes can connect,
    //   and special input/output lists as created above.
    // The operation can be any of the ops found in interface/ops.def,
    //   prefixed with "OP_" (e.g. OP_MatMul_f, OP_Relu_f, OP_MaxPool_f)
    // Padding-type is important for convolutional nodes, and defines what
    //   happens at the edges, where the convolutional kernel will reach
    //   beyond the boundary of the image it's being convolved with.
    //   For non-convolutional layers, NN_PAD_NA is usually correct.
    // The  input list should be an array of type hexagon_nn_input
    // The output list should be an array of type hexagon_nn_output
    //   The input/output lists can be empty.
    // The inputs in the input-list simply refer to outputs from prior nodes.
    // The outputs in the output-list define max sizing for output tensors,
    //   so storage can be correctly allocated.
    // Caveat: The input and output lists are pointers, and will cross
    //   from ARM onto DSP.  So we must be sure they will memory-map
    //   correctly, and also not disappear from the stack, e.g. if we exit
    //   the calling function before teardown.
    //   Here, we have created them as static structures.  You could also
    //   use rpcmem_alloc(), shown below.
    hexagon_nn_append_node(
            graph_id,           // Graph handle we're appending into
            0x1000,             // Node identifier (any unique uint32)
            OP_INPUT,           // Operation of this node (e.g. Concat, Relu)
            NN_PAD_NA,          // Padding type for this node
            empty_input_list,   // The list of inputs to this node
            0,                  //   How many elements in input list?
            layer0_output_list, // The list of outputs from this node
            1                   //   How many elements in output list?
            );

    // A const-node is different than an ordinary node because it
    //   doesn't do any work, and has no inputs.
    // To define it, we must give it a unique node-id,
    //   a size, and data.
    // Caveat: The same pointers-crossing-from-ARM-to-DSP warnings presented
    //   in hexagon_nn_append_node(), above, apply here, too.
    hexagon_nn_append_const_node(
            graph_id,                   // Graph handle we're appending into
            0x1001a,                    // Node identifier (a unique uint32)
            1,                          // size: batches
            1,                          // size: height
            2,                          // size: width
            2,                          // size: depth
            (uint8_t *) layer1a_consts, // Pointer to data
            4 * sizeof(float)             // Length of data to copy
            );

    // Append as many nodes and const_nodes as necessary to build your graph.
    // Only Directed-Acyclic-Graphs (DAGs) are supported by nnlib today.
    hexagon_nn_append_node(
            graph_id,
            0x1001,
            OP_Add_f,
            NN_PAD_NA,
            layer1_input_list,
            2,
            layer1_output_list,
            1
            );

    hexagon_nn_append_node(
            graph_id,
            0x1002,
            OP_OUTPUT,
            NN_PAD_NA,
            layer2_input_list,
            1,
            NULL,
            0
            );

    // Prepare the graph for execution by optimizing it, allocating storage,
    //   connecting all the input/output pointers between nodes, and
    //   doing some basic checks, like number of input/output tensors and
    //   sizing for each node.
    if ((err = hexagon_nn_prepare(graph_id)) != 0) {
            printf("Whoops... Cannot prepare: %d\n", err);
            goto TEARDOWN;
    }

    // The input and output tensors will be passed from here on the ARM
    //   to the function running on the DSP.  We need to ensure they
    //   can be efficiently mapped by fastRPC, so we place them in ION memory
    // You could also use this type of allocation for your const-nodes' data.
    uint8_t *output;
    if ((output = rpcmem_alloc(
                 ION_HEAP_ID_SYSTEM,
                 RPCMEM_DEFAULT_FLAGS,
                 OUT_SIZE)
                ) == NULL) {
            printf("Whoops... Cannot malloc for outputs\n");
            err=1;
            goto TEARDOWN;
    }

    uint8_t *input;
    if ((input = rpcmem_alloc(
                 ION_HEAP_ID_SYSTEM,
                 RPCMEM_DEFAULT_FLAGS,
                 IN_SIZE)
                ) == NULL) {
            printf("Whoops... Cannot malloc for inputs\n");
            err=1;
            goto TEARDOWN;
    }

    // Initialize our inputs, from the commandline values given.
    // The input and output data is in BHWD format, just like const nodes.
    //   (depth is most-major, i.e. neighboring in memory)
    float *in = (float *) input;
    int i;
    for (i=0; i<4; i++) {
            in[i] = 0;
            if (i+1<argc) {
                    sscanf(argv[i+1],"%f",in+i);
            }
    }

    // Execute an inference on our input data.
    // The actual number of batches/height/width/depth in the output
    //   is computed by the graph, based on the inputs and graph,
    //   and are returned by reference.
    // We provide a max-size for our output buffer (how much did we allocate)
    //   but the actual size consumed might be less, and is returned
    //   by reference.
    // If the execution fails, it will return non-zero.
    // FastRPC will return nonzero (39) for all subsequent calls following a
    //   non-zero return-code from any interface function, until reset.
    uint32_t out_batches, out_height, out_width, out_depth, out_data_size;
    if ((err = hexagon_nn_execute(
                 graph_id,
                 IN_BATCH,
                 IN_HEIGHT,
                 IN_WIDTH,
                 IN_DEPTH,
                 (const uint8_t *)input, // Pointer to input data
                 IN_SIZE,                // How many total bytes of input?
                 (unsigned int*) &out_batches,
                 (unsigned int*) &out_height,
                 (unsigned int*) &out_width,
                 (unsigned int*) &out_depth,
                 (uint8_t *)output,      // Pointer to output buffer
                 OUT_SIZE,               // Max size of output buffer
                 (unsigned int*) &out_data_size)         // Actual size used for output
                ) != 0) {

            printf("Whoops... run failed: %d\n",err);
            goto TEARDOWN;
    }

    // Sanity check that our output is sized as expected,
    //   else we might have built our graph wrong.
    if ( (out_batches != OUT_BATCH) ||
         (out_height != OUT_HEIGHT) ||
         (out_width != OUT_WIDTH) ||
         (out_depth != OUT_DEPTH) ||
         (out_data_size != OUT_SIZE) ) {
            printf("Whoops... Output sizing seems wrong: (%ux%ux%ux%u %u)\n",
                   (unsigned int) out_batches, (unsigned int) out_height,
                   (unsigned int) out_width, (unsigned int) out_depth,
                   (unsigned int) out_data_size
                    );
            err = 1;
            goto TEARDOWN;
    }

    // Display the outputs
    float *out = (float *) output;
    printf("Got output: [ %f %f ]\n", out[0], out[1]);
    printf("            [ %f %f ]\n", out[2], out[3]);

    // quick & dirty correctness check
    for (i = 0; i < 4; i++)
    {
        if (out[i] != in[i] + layer1a_consts[i])
        {
            printf("Expected: [ %f %f ]\n", in[0] + layer1a_consts[0], in[1] + layer1a_consts[1]);
            printf("          [ %f %f ]\n", in[2] + layer1a_consts[2], in[3] + layer1a_consts[3]);
            err = 1;
            goto TEARDOWN;
        }
    }

TEARDOWN:
    // Free the memory, especially if we want to build subsequent graphs
    hexagon_nn_teardown(graph_id);

    // Stop fastRPC
    fastrpc_teardown();
    
    if (!err) printf("Test Passed!\n");
    else printf ("Test Failed, err=%d\n", err);

    return err;
}
