// *****************************************************************************
//  | | | |                | \ | || \ | |
//  | |_| | _____  ________|  \| ||  \| |
//  |  _  |/ _ \ \/ /______| . ` || . ` |
//  | | | |  __/>  <       | |\  || |\  |
//  \_| |_/\___/_/\_\      \_| \_/\_| \_/
//
//    ______      __             _       __         ____  ____  __ __
//   /_  __/_  __/ /_____  _____(_)___ _/ /        / __ \/ __ \/ // /
//    / / / / / / __/ __ \/ ___/ / __ `/ /  ______/ / / / / / / // /_
//   / / / /_/ / /_/ /_/ / /  / / /_/ / /  /_____/ /_/ / /_/ /__  __/
//  /_/  \__,_/\__/\____/_/  /_/\__,_/_/         \____/\____/  /_/
//
// *****************************************************************************
//
// This tutorial introduces:
//   * nn_graph's API for building and running graphs
//   * The fastRPC glue required to get the ARM CPU to speak to the DSP
//   * Minimal required header-files
//   * Inputs and outputs
//   * Const nodes
//   * Improved error checking using macros
//   * Benchmarking
//   * Node debug
//   * Validation
//
// To do this, we'll construct a simple neural-network that XORs two inputs.
// The skills used to build this simple network are the same ones we'll use to
//   build larger graphs.
// We'll also show how we add debug and validation nodes to this graph.
//
// We choose to implement XOR because it is one of the simplest
//   mathematically interesting graphs, demonstrating how non-linear layers and
//   linear layers work together to simulate arbitrary (nonlinear) functions.



// hexagon_nn.h includes most things you'll need to create and run graphs.
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


// The structure of our basic XOR network looks like this, and has five layers:
//
//                SIMPLIFIED GRAPH
//         [ 1,-1]                 [1]
//         [-1, 1]                 [1]
//         "layer1a"            "layer3a"
//          0x1001a              0x1003a
//               \                   \.
// INPUT (x2) --> MatMul --> Relu --> MatMul --> OUTPUT (x1)
//   "layer0"    "layer1"  "layer2"  "layer3"    "layer4"
//    0x1000      0x1001    0x1002    0x1003      0x1004
//
// None of the math is important, but you might be curious, so here's
//   how it performs an XOR on our floating-point [0,1] inputs:
//
// First MatMul: (inputs [A,B])
// [A,B] x [ 1,-1] = [ C=A-B, D=B-A ]
//         [-1, 1]
//
// The 2-element vector created by the first MatMul is rectified,
//   removing any negative numbers and passed into the second MatMul
//   as C' and D'
//
// Second MatMul: (output Y)
// [C',D'] x [1] = [ Y=C'+D' ]
//           [1]
//
// The truth table is:
//  A=0 B=0 --> C=0  D=0  --> C'=0 D'=0 --> Y=0
//  A=0 B=1 --> C=-1 D=1  --> C'=0 D'=1 --> Y=1
//  A=1 B=0 --> C=1  D=-1 --> C'=1 D'=0 --> Y=1
//  A=1 B=1 --> C=0  D=0  --> C'=0 D'=0 --> Y=0
//
//
// We'll also need some nodes for debug and validation.
//   Here's what the graph looks like with
//   pprint - Display the contents of a tensor
//   close - Compare two tensors for approximate equality.:
//
//                  ACTUAL GRAPH
//         [ 1,-1]                 [1]
//         [-1, 1]                 [1]
//         "layer1a"            "layer3a"
//          0x1001a              0x1003a
//               \                   \.
//   [1,0] -----> MatMul --> Relu --> MatMul ----.
//   Const       "layer1"  "layer2"  "layer3"     \.
//  0x1001b       0x1001    0x1002    0x1003       \.
//                     \         \         \        \.
//                      PPrint    PPrint    PPrint   Close
//                      0x2000    0x2001    0x2002   0x2003
//                                                   /
//                                                  /
//                                                [1]
//                                               Const
//                                              0x2003a
//
//
//
// Here's how we're adding debug and validation:
//   1) Add Validation
//     a) "Op_Close", 0x2003, compares the final computation
//        against reference values.
//     b) The reference values come from a const node, 0x2003a
//     c) Because the reference is Const, we remove "INPUT" 0x1000
//        and instead use Const as our inputs.
//     d) op_close compares the tensors element-wise and fails if any
//        single element is further from the expectation than allowed.
//   2) Add debug
//     a) Three "PPrint" nodes will print the output after computation.




// We need to create some data structures describing our layers,
//   which we'll pass to the DSP as we append nodes to the graph.
//
// These structures describe the inputs and outputs of each node
//   (how the nodes attach to each other, and how they are sized)
//
// And we need arrays of constants to feed our const-nodes
//   (the weights of our model)


//***********************
// LAYER 1:  MatMul
//***********************

// The 'MatMul' operator takes two matrices as inputs.
// Later, when we create actual nodes, we'll give them unique IDs.
// Here, we need to refer to the unique IDs of our inputs.
// These are just uint32_t which we've selected for legibility.
//   You can use any uint32_t which pleases you, as long as each node is unique.
static hexagon_nn_input layer1_input_list[] = {
          { .src_id = 0x1001b, .output_idx = 0, },
          { .src_id = 0x1001a, .output_idx = 0, },
};
static hexagon_nn_output layer1_output_list[] = {
        { .rank = 4, .max_sizes = {1,1,1,2}, .elementsize = 4, },
};



//***********************
// LAYER 2:  Relu
//***********************

// The relu operator takes a single input and produces a single output.
// The output will always be the same size as the input.
// Notice that we've specified the output to be much larger than it really is.
// This means we'll allocate (4*3*2*1*4=96 bytes) for the output,
//   but really return a 1x1x1x2 output tensor given a 1x1x1x2 input tensor.
// Notice also my mistake of setting the output's .max_sizes BHWD depth=1,
//   when it should be 2.  This mistake will work because there is enough total
//   allocated storage, but future versions of the library could break here.
static hexagon_nn_input layer2_input_list[] = {
        { .src_id = 0x1001, .output_idx = 0, }, // From Matmul-layer1
};
static hexagon_nn_output layer2_output_list[] = {
        { .rank = 4, .max_sizes = {4,3,2,1}, .elementsize = 4, },
};



//***********************
// LAYER 3:  MatMul (again)
//***********************


static hexagon_nn_input layer3_input_list[] = {
        { .src_id = 0x1002, .output_idx = 0, },  // From Relu
        { .src_id = 0x1003a, .output_idx = 0, }, // From const-layer3a
};
static hexagon_nn_output layer3_output_list[] = {
        { .rank = 4, .max_sizes = {1,1,1,100}, .elementsize = 4, },
};



//***********************
// Close Checking
//***********************


static hexagon_nn_input close_input_list[] = {
        { .src_id = 0x1003, .output_idx = 0, },  // From final MatMul
        { .src_id = 0x2003a, .output_idx = 0, }, // From const-expects
};
// Note: OP_Close has no output tensors.




//***********************
// PPrinting
//***********************


static hexagon_nn_input pprint_0_input_list[] = {
        { .src_id = 0x1001, .output_idx = 0, },  // From first MatMul
};
static hexagon_nn_input pprint_1_input_list[] = {
        { .src_id = 0x1002, .output_idx = 0, },  // From Relu
};
static hexagon_nn_input pprint_2_input_list[] = {
        { .src_id = 0x1003, .output_idx = 0, },  // From final MatMul
};
// Note: OP_PPrint has no output tensors.




//***********************
// CONST WEIGHTS
//***********************

// Weights are stored in BHWD format, with depth as most-major rank.
// You can store weights as float, int, uint8_t, or whatever is supported
//   by the operation that is using them as inputs.

// These two weight tensors create our XOR functionality
static float layer1a_consts[4] = {
        1, -1,
        -1, 1,
};
static float layer3a_consts[2] = {
        1, 1,
};

// This const is our sample input
static float layer1b_consts[2] = {
        1,0,
};

// This const is our expected output, given the sample input
static float layer2003a_consts[1] = {
        1,
};


// Every call to the hexagon_nn_* API interface should be checked.
//   This sample  macro allows us to easily add checks to our code which
//   pinpoint our errors down to the line number.
#define ERRCHECK if (err) { \
                printf("Whoops... Some error at line %d: %d\n", __LINE__, err); \
                goto TEARDOWN; \
        }

int main(int argc, char **argv) {
        int err;

    // if running on Hexagon Simulator, need to extract the command line arguments.
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

        // Select our debug level.  0=none, >4=max
        // When creating new graphs, it's nice to have max debug
        //   even if you don't think you need it.
        err |= hexagon_nn_set_debug_level(graph_id, 100);
        ERRCHECK

        // A const-node is different than an ordinary node because it
        //   doesn't do any work, and has no inputs.
        // To define it, we must give it a unique node-id,
        //   a size, and data.
        // Caveat: The same pointers-crossing-from-ARM-to-DSP warnings presented
        //   in hexagon_nn_append_node(), above, apply here, too.
        err |= hexagon_nn_append_const_node(
                graph_id,                   // Graph handle we're appending into
                0x1001a,                    // Node identifier (a unique uint32)
                1,                          // size: batches
                1,                          // size: height
                2,                          // size: width
                2,                          // size: depth
                (uint8_t *) layer1a_consts, // Pointer to data
                4*sizeof(float)             // Length of data to copy
                );
        ERRCHECK

        // Append as many nodes and const_nodes as necessary to build your graph.
        // Only Directed-Acyclic-Graphs (DAGs) are supported by nnlib today.

        err |= hexagon_nn_append_const_node(
                graph_id,
                0x1003a,
                1,
                1,
                2,
                1,
                (uint8_t *) layer3a_consts,
                2*sizeof(float)
                );
        ERRCHECK

        // Sample Inputs
        err |= hexagon_nn_append_const_node(
                graph_id,
                0x1001b,
                1,
                1,
                1,
                2,
                (uint8_t *) layer1b_consts,
                2*sizeof(float)
                );
        ERRCHECK

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
        //   But we'll use zero inputs and a NULL buffer because we're
        //   self-testing
        // The output list should be an array of type hexagon_nn_output
        //   But we'll have zero outputs and a NULL buffer because we're
        //   self-testing
        //   (The input/output lists can be empty.)
        // The inputs in the input-list simply refer to outputs from prior nodes.
        // The outputs in the output-list define max sizing for output tensors,
        //   so storage can be correctly allocated.
        // Caveat: The input and output lists are pointers, and will cross
        //   from ARM onto DSP.  So we must be sure they will memory-map
        //   correctly, and also not disappear from the stack, e.g. if we exit
        //   the calling function before teardown.
        //   Here, we have created them as static structures.  You could also
        //   use rpcmem_alloc(), shown below.

        err |= hexagon_nn_append_node(
                graph_id,
                0x1001,
                OP_MatMul_f,
                NN_PAD_NA,
                layer1_input_list,
                2,
                layer1_output_list,
                1
                );
        ERRCHECK

        err |= hexagon_nn_append_node(
                graph_id,
                0x1002,
                OP_Relu_f,
                NN_PAD_NA,
                layer2_input_list,
                1,
                layer2_output_list,
                1
                );
        ERRCHECK

        err |= hexagon_nn_append_node(
                graph_id,
                0x1003,
                OP_MatMul_f,
                NN_PAD_NA,
                layer3_input_list,
                2,
                layer3_output_list,
                1
                );
        ERRCHECK

        // PPrint comes in flavors for several data types:
        //   PPrint_8  - 8bit integers
        //   PPrint_32 - 32bit integers
        //   PPrint_f  - 32bit floats
        err |= hexagon_nn_append_node(
                graph_id,
                0x2000,
                OP_PPrint_f,
                NN_PAD_NA,
                pprint_0_input_list,
                1,
                NULL,
                0
                );
        ERRCHECK
        err |= hexagon_nn_append_node(
                graph_id,
                0x2001,
                OP_PPrint_f,
                NN_PAD_NA,
                pprint_1_input_list,
                1,
                NULL,
                0
                );
        ERRCHECK
        err |= hexagon_nn_append_node(
                graph_id,
                0x2002,
                OP_PPrint_f,
                NN_PAD_NA,
                pprint_2_input_list,
                1,
                NULL,
                0
                );
        ERRCHECK

        // Expected Outputs
        err |= hexagon_nn_append_const_node(
                graph_id,
                0x2003a,
                1,
                1,
                1,
                1,
                (uint8_t *) layer2003a_consts,
                1*sizeof(float)
                );
        ERRCHECK

        // Close checking comes in multiple flavors:
        //   Close_f        - Float
        //   Close_int32    - Int32
        //   Close_qint32   - TODO - Explain
        //   Close_qint8    - TODO - Explain
        //   Close_q_quint8 - TODO - Explain
        err |= hexagon_nn_append_node(
                graph_id,
                0x2003,
                OP_Close_f,
                NN_PAD_NA,
                close_input_list,
                2,
                NULL,
                0
                );
        ERRCHECK

        // Prepare the graph for execution by optimizing it, allocating storage,
        //   connecting all the input/output pointers between nodes, and
        //   doing some basic checks, like number of input/output tensors and
        //   sizing for each node.
        if ((err = hexagon_nn_prepare(graph_id)) != 0) {
                printf("Whoops... Cannot prepare: %d\n", err);
                goto TEARDOWN;
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
                     0,                      // No inputs provided to
                     0,                      //   this self-checking graph
                     0,
                     0,
                     NULL,                   // Pointer to input buffer
                     0,                      // How many total bytes of input?
                     (unsigned int*) &out_batches,
                     (unsigned int*) &out_height,
                     (unsigned int*) &out_width,
                     (unsigned int*) &out_depth,
                     NULL,                   // Pointer to output buffer
                     0,                      // Max size of output buffer
                     (unsigned int*) &out_data_size)         // Actual size used for output
                    ) != 0) {

                printf("Whoops... run failed: %d\n",err);
                goto TEARDOWN;
        } else {
                // For benchmarking, we want to know how long the DSP spent
                //   in calculations.  This is measured in the DSP's
                //   do_execute() function, inside hexagon/src/execute.c
                // We'll get a big number here because our pprint nodes are
                //   included in the measurement.
                unsigned int cycleslo=0, cycleshi=0;
                hexagon_nn_last_execution_cycles(graph_id,&cycleslo,&cycleshi);
                printf("INFO: Run took %du %du cycles on DSP\n",
                       cycleshi, cycleslo);
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

