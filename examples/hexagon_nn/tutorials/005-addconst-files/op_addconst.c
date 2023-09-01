/*
 * Copyright (c) 2016-2017, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *    * Neither the name of The Linux Foundation nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include <nn_graph.h>
#if defined(__hexagon__)
#include "hexagon_types.h"
#endif


// .execute functions always take as inputs an nn_node and nn_graph.
// For most operations, the nn_node will be all you'll need.
// The nn_graph is most useful for print buffering.
static int addconst_execute(struct nn_node *self, struct nn_graph *nn)
{
	// Unpack the input and output tensors of this node.
	// Remember that the inputs must be treated as const.
	// See addconst_check(), below, for our validation of the number of
	//   in/out tensors.
	const struct tensor *a_tensor = self->inputs[0];
	const struct tensor *b_tensor = self->inputs[1];
	struct tensor *out_tensor = self->outputs[0];

	// Only during execute() do we know the actual tensor dimensions.
	// The dimensions can change on every call to execute() e.g. if the
	//   input image size changes.
	// Here, we validate our assumption that b_tensor is a scalar value.
	if ( (b_tensor->shape.batches != 1)
	     || (b_tensor->shape.height != 1)
	     || (b_tensor->shape.width != 1)
	     || (b_tensor->shape.depth != 1) ) {
		return errlog(nn,"AddConst requires second tensor 1x1x1x1 dimension. (You gave %dx%dx%dx%d)",
			      b_tensor->shape.batches,
			      b_tensor->shape.height,
			      b_tensor->shape.width,
			      b_tensor->shape.depth);
	}

	// Set the size of the output tensor.
	out_tensor->shape.batches = a_tensor->shape.batches;
	out_tensor->shape.height = a_tensor->shape.height;
	out_tensor->shape.width = a_tensor->shape.width;
	out_tensor->shape.depth = a_tensor->shape.depth;
	out_tensor->data_size = out_tensor->shape.batches *
		out_tensor->shape.height *
		out_tensor->shape.width*
		out_tensor->shape.depth;
	out_tensor->format.layout = NN_LAYOUT_PLAIN; //simple in-memory array format
	out_tensor->format.type = 1; // 1=chars, 4=int32/float
	// Note: This format field will change with new interface updates.
	//       e.g. format.type will soon distinguish int32 versus float types.


	// Read the constant from our scalar input
	unsigned char constant = ((char*)b_tensor->data)[0];

	// Add constant to tensor
	int i;
	char *in = (char *) a_tensor->data;
	char *out = (char *) out_tensor->data;
	for (i=0; i<out_tensor->data_size; i++) {
		*(out++) = *(in++) + constant;
	}

	// Return 0 for success, 1 for failure of node.
	return 0;
}

// .check functions validate the number of inputs and outputs for the op-type.
// Return 0=success, 1=failure
static int addconst_check(struct nn_node *self, struct nn_graph *nn)
{
	if (self->n_inputs != 2) return errlog(nn,"Wrong # inputs");
	if (self->n_outputs != 1) return errlog(nn,"Wrong # outputs");
	return 0;
}

// This struct says how the "OP_AddConst_8" will work.
struct nn_node_ops nn_ops_for_AddConst_8 = {
	.execute = addconst_execute,
	.check = addconst_check,
	.ctor = node_alloc_common,
	.dtor = node_free_common,
};

// This struct defines the reference model.
// For now, we're just declaring the reference model to match the non-reference version.
// This is not needed if you use OP_DEF instead of OP_DEF_WREF in ops.def
struct nn_node_ops nn_ops_for_AddConst_8_ref = {
	.execute = addconst_execute,
	.check = addconst_check,
	.ctor = node_alloc_common,
	.dtor = node_free_common,
};

