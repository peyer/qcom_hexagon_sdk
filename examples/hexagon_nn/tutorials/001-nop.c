// ******************************************************************
//  | | | |                | \ | || \ | |
//  | |_| | _____  ________|  \| ||  \| |
//  |  _  |/ _ \ \/ /______| . ` || . ` |
//  | | | |  __/>  <       | |\  || |\  |
//  \_| |_/\___/_/\_\      \_| \_/\_| \_/
//
//    ______      __             _       __         ____  ____ ___
//   /_  __/_  __/ /_____  _____(_)___ _/ /        / __ \/ __ <  /
//    / / / / / / __/ __ \/ ___/ / __ `/ /  ______/ / / / / / / /
//   / / / /_/ / /_/ /_/ / /  / / /_/ / /  /_____/ /_/ / /_/ / /
//  /_/  \__,_/\__/\____/_/  /_/\__,_/_/         \____/\____/_/
//
// ******************************************************************
//
// This tutorial introduces:
//   * part of nn_graph's API for building and running graphs
//   * Minimal required header-files
//
// To do this, we'll construct a graph with just a single NOP
//


// hexagon_nn.h includes most of the things you'll need to create and run graphs.
// Its most important includes are nn_graph.h, which includes nn_graph_if.h.
// Together, these provide the data-types for input/output tensors,
//   and the API you'll use for initializing, building, preparing and running
//   your graphs.
// NOTE: hexagon_nn.h redefines malloc(), alloc(), etc. so
//   they become a compile-error "OOPS MALLOC".  This is because you should
//   use rpcmem_alloc() instead.
#include <hexagon_nn.h>

// hexagon_nn_ops.h defines the various graph operations (e.g. "MatMul", "NOP"
//   and "Relu") which you can do.  Internally, it just expands
//   interface/ops.def into a usable format.  ops.def contains the list of
//   all implemented ops.
#include "hexagon_nn_ops.h"

// For printf, etc.
#include <stdio.h>

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




// The structure of our NOP network looks like this.
//   It's really just a NOP floating in space, with no inputs or outputs.
//
//
//                   ==============
//    ?????????      ||    NOP   ||      ?????????
//   ??nothing??     || id=0x1000||     ??nothing??
//    ?????????      ==============      ?????????
//



int main(int argc, char **argv) {
        int err;

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
        //                                     (we won't need any for now)
        //   6) hexagon_nn_prepare           - Allocate memory, strategize,
        //                                     and optimize the graph for speed
        //   7) hexagon_nn_execute           - Run an inference
        //   8) hexagon_nn_teardown          - Destroy the graph, free resources

        // Ensures that nnlib is ready to start working.
        hexagon_nn_config();

        // Initialize a fresh, empty graph.  Return a graph-handle by reference.
        hexagon_nn_nn_id graph_id;
        if (hexagon_nn_init(&graph_id)) {
                printf("Whoops... Cannot init\n");
				return 2;
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
        // We need to provide a unique-id so other nodes can connect.
        // The operation can be any of the ops found in interface/ops.def,
        //   prefixed with "OP_" (e.g. OP_MatMul_f, OP_Relu_f, OP_MaxPool_f)
        // Our NOP node doesn't need any padding, because it won't do anything.
        // Our input/output lists will be NULL in this example,
        //   but for real graphs we'll need to connect nodes using these lists.
        hexagon_nn_append_node(
                graph_id,           // Graph handle we're appending into
                0x1000,             // Node identifier (any unique uint32)
                OP_Nop,             // Operation of this node (e.g. Concat, Relu)
                NN_PAD_NA,          // Padding type for this node
                NULL,               // The list of inputs to this node
                0,                  //   How many elements in input list?
                NULL,               // The list of outputs from this node
                0                   //   How many elements in output list?
                );

        // Prepare the graph for execution by optimizing it, allocating storage,
        //   connecting all the input/output pointers between nodes, and
        //   doing some basic checks, like number of input/output tensors and
        //   sizing for each node.
        if (hexagon_nn_prepare(graph_id)) {
                printf("Whoops... Cannot prepare\n");
        }


        // Execute an inference on our input data.
        // Real graphs require input and output buffers, but we'll
        //   just use zero-size NULL pointers for this NOP example.
        uint32_t out_batches, out_height, out_width, out_depth, out_data_size;
        if ((err = hexagon_nn_execute(
                     graph_id,
                     0, 0, 0, 0,             // Our input has 0-dimension
                     NULL,                   // Pointer to input data
                     0,                      // How many total bytes of input?
                     (unsigned int *) &out_batches,
                     (unsigned int *) &out_height,
                     (unsigned int *) &out_width,
                     (unsigned int *) &out_depth,
                     (uint8_t *)NULL,        // Pointer to output buffer
                     0,                      // Max size of output buffer
                     (unsigned int*) &out_data_size)         // Actual size used for output
                    ) != 0) {

                printf("Whoops... run failed: %d\n",err);
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



