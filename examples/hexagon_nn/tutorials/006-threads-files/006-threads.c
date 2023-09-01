// ******************************************************************
//  | | | |                | \ | || \ | |
//  | |_| | _____  ________|  \| ||  \| |
//  |  _  |/ _ \ \/ /______| . ` || . ` |
//  | | | |  __/>  <       | |\  || |\  |
//  \_| |_/\___/_/\_\      \_| \_/\_| \_/
//
//    ______      __             _       __      ____  ____  _____
//   /_  __/_  __/ /_____  _____(_)___ _/ /     / __ \/ __ \/ ___/
//    / / / / / / __/ __ \/ ___/ / __ `/ /_____/ / / / / / / __ |
//   / / / /_/ / /_/ /_/ / /  / / /_/ / /_____/ /_/ / /_/ / /_/ /
//  /_/  \__,_/\__/\____/_/  /_/\__,_/_/      \____/\____/\____/
//
// ******************************************************************
//
// This tutorial introduces:
// * Using a newly created OP, "op_AddConst"
//


#include <hexagon_nn.h>
#include "../interface/hexagon_nn_ops.h"
#include <stdio.h>
#include "sdk_fastrpc.h"


#define OUT_BATCH 1
#define OUT_HEIGHT 1
#define OUT_WIDTH 2
#define OUT_DEPTH 2
#define OUT_ELEMENTSIZE (sizeof(char))
#define OUT_SIZE (OUT_BATCH*OUT_DEPTH*OUT_HEIGHT*OUT_WIDTH*OUT_ELEMENTSIZE)



// The structure of our ADD network looks like this, and has
//   three layers and four nodes:
//
//          [ 42 ] <------Constant
//          "layer1a"
//           0x1001a
//                \.
// INPUT (2x2) --> AddConst --> OUTPUT (2x2)
//   "layer0"    "layer1"       "layer2"
//    0x1000      0x1001         0x1002
//


//***********************
// LAYER 0:  INPUT
//***********************

#define IN_BATCH 1
#define IN_HEIGHT 1
#define IN_WIDTH 2
#define IN_DEPTH 2
#define IN_ELEMENTSIZE (sizeof(char))
#define IN_SIZE (IN_BATCH * IN_HEIGHT * IN_WIDTH * IN_DEPTH * IN_ELEMENTSIZE)
static hexagon_nn_output layer0_output_list[] = {
        { .rank = 4,
          .max_sizes = {IN_BATCH, IN_HEIGHT, IN_WIDTH, IN_DEPTH},
          .elementsize = IN_ELEMENTSIZE, },
};


//***********************
// LAYER 1:  AddConst
//***********************
static hexagon_nn_input layer1_input_list[] = {
        { .src_id = 0x1000, .output_idx = 0, },  // From INPUT-layer0
        { .src_id = 0x1001a, .output_idx = 0, }, // From const-layer1a
};
static hexagon_nn_output layer1_output_list[] = {
        { .rank = 4, .max_sizes = {1,1,2,2}, .elementsize = sizeof(char), },
};


//***********************
// LAYER 2:  OUTPUT
//***********************
static hexagon_nn_input layer2_input_list[] = {
        { .src_id = 0x1001, .output_idx = 0, }, // From Add-layer1
};


//***********************
// CONST WEIGHTS
//***********************
static char layer1a_consts[1] = { 42 };


#define ERRCHECK if (err) { \
                printf("Whoops... Some error at line %d: %d\n", __LINE__, err); \
                goto TEARDOWN; \
        }


int main(int argc, char **argv) {
        int err;

        if (fastrpc_setup() != 0) return 1;

        // Set power level (to max/turbo)
        if ((err = hexagon_nn_set_powersave_level(0)) != 0) {
                printf("Whoops... Cannot set power level: %d\n", err);
                goto TEARDOWN;
        }

        hexagon_nn_config();

        hexagon_nn_nn_id graph_id;
        if ((err = hexagon_nn_init(&graph_id)) != 0) {
                printf("Whoops... Cannot init: %d\n", err);
                goto TEARDOWN;
        }
        err |= hexagon_nn_set_debug_level(graph_id, 100);
	ERRCHECK

        err |= hexagon_nn_append_node(
                graph_id,
                0x1000,
                OP_INPUT,
                NN_PAD_NA,
                NULL,
                0,
                layer0_output_list,
                1
                );
	ERRCHECK

        err |= hexagon_nn_append_const_node(
                graph_id,
                0x1001a,
                1,
                1,
                1,
                1,
                (uint8_t *) layer1a_consts,
                1 * sizeof(char)
                );
	ERRCHECK

        err |= hexagon_nn_append_node(
                graph_id,
                0x1001,
                OP_AddConst_8,
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
                OP_OUTPUT,
                NN_PAD_NA,
                layer2_input_list,
                1,
                NULL,
                0
                );
	ERRCHECK

        if ((err = hexagon_nn_prepare(graph_id)) != 0) {
                printf("Whoops... Cannot prepare: %d\n", err);
                goto TEARDOWN;
        }

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
        char *in = (char *) input;
        int i;
        for (i=0; i<4; i++) {
                in[i] = 0;
                if (i+1<argc) {
			int user_in;
                        sscanf(argv[i+1],"%d",&user_in);
			*(in+i) = (char) user_in;
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
                     &out_batches,
                     &out_height,
                     &out_width,
                     &out_depth,
                     (uint8_t *)output,      // Pointer to output buffer
                     OUT_SIZE,               // Max size of output buffer
                     &out_data_size)         // Actual size used for output
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
                       out_batches, out_height, out_width, out_depth,
                       out_data_size
                        );
                goto TEARDOWN;
        }

        // Display the outputs
        char *out = (char *) output;
        printf("Got output: [ %d %d ]\n", out[0], out[1]);
        printf("            [ %d %d ]\n", out[2], out[3]);

        printf("Got input:  [ %d %d ]\n", in[0], in[1]);
        printf("            [ %d %d ]\n", in[2], in[3]);

TEARDOWN:
        // Free the memory, especially if we want to build subsequent graphs
        hexagon_nn_teardown(graph_id);

        // Stop fastRPC
        fastrpc_teardown();

        return err;
}


