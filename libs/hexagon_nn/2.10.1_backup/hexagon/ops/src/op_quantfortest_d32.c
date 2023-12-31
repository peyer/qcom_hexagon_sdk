
/*
 * Copyright (c) 2017-2019, The Linux Foundation. All rights reserved.
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
#include <string.h>
#include <math.h>
#include <quantize.h>
#if defined(__hexagon__)
#include <hexagon_types.h>
#endif
#include <hvx_hexagon_protos.h>
#include <stdio.h>

//
// tensor_in is a float tensor.
// find the min & max values in it,
// and return these via minmax[0], minmax[1]
//
// which:  0 = do all
//         1 = do first half only
//         2 = do second half only
// 'half' is not clearly defined; but it is guaranteed that if you
// call with which=1 and which=2 in two threads, the result will be
// valid for the whole thing.

void
tensor_find_range_float(
	struct tensor const * tensor_in,
	int which,
	float *minmax_p )
{
	float const * dp = (float const *)tensor_in->data;
	int n = tensor_in->shape.batches * tensor_in->shape.height
			 * tensor_in->shape.width * tensor_in->shape.depth;

	float minval = dp[0];
	float maxval = minval;

	if( which > 1 ){
		int nh = (n<8)? n: ((n >> 1)&~1);	// first half (and #even #)
		if( (which & 1) == 0 ){
			n -= nh;	// might be 0
			dp += nh;	// skip first half
		}else{
			n = nh;
		}
	}
	int i,np;
	np =  n >> 1;
	for( i = 0; i < np; i++ ){
		float x0 = dp[0];
		float x1 = dp[1];
		minval = fminf(  minval, fminf( x0, x1 ));
		maxval = fmaxf(  maxval, fmaxf( x0, x1 ));
		dp += 2;
	}
	if(n & 1){
		float x0 = dp[0];
		minval = fminf(  minval, x0);
		maxval = fmaxf(  maxval, x0);
	}
	minmax_p[0] = minval;
	minmax_p[1] = maxval;
}

//
// tensor_in is a float tensor
// out_min, out_max are its range (assumed to have been corrected)
// tensor_outqu8 is a qu8, d32 tensor of the same shape (already allocated)
// tensor_outf  is a float tensor of the same shape, already allocated
//
// The operation is to quantize tensor_in -> tensor_outqu8 and also
//   to dequantize to tensor_outf at the same time.
//
// note: tensor_outf may be null; no second output will be generated.

void
quantize_with_dequant(
	struct tensor const * tensor_in,
	struct tensor const * tensor_outqu8,
	struct tensor const * tensor_outf,
	float out_min,
	float out_max)
{
	int ib, iht,  iwid, id32, idep;

	int batches = tensor_in->shape.batches;
	int height = tensor_in->shape.height;
	int width = tensor_in->shape.width;
	int depth = tensor_in->shape.depth;

	int h_begin = 0;
	int h_end = height;	// height range (for threading)

	int n_per_row = width * depth;
	int n_per_batch = height * n_per_row;

	int u8_row_stride =  tensor_row_stride_d32( tensor_outqu8 );
	int u8_d32_stride =  tensor_d32_stride_d32( tensor_outqu8 );
	int dpad0 = tensor_outqu8->format.depth_pad[0];
	int nd32 = (unsigned)( dpad0 + depth + 31 )/32;

	float qscale = 255.0f / ( out_max - out_min );
	float deqscale = (out_max -out_min)/255.0f;

	float *poutf_0 = NULL;
	// batch loop
	//

	for( ib = 0; ib < batches; ib++ ){
		float const *pin_0 = (float const *)tensor_in->data + n_per_batch *ib;
		uint8_t *poutu8_0 = tensor_location_bhw_d32( tensor_outqu8, ib,0,0);
		if( tensor_outf != NULL)
			poutf_0 = (float *)tensor_outf->data + n_per_batch * ib;

		for( iht = h_begin; iht < h_end; iht ++ ){
			float const *pin = pin_0 + iht * n_per_row;

			if( tensor_outf != NULL ){
				float *poutf = poutf_0  + iht * n_per_row;

				// depth chunk loop
				int d0 = dpad0;
				int dremain = depth;
				for( id32 = 0; id32 < nd32; id32 ++ ){
					int dn = min_i32( 32-d0, dremain );
					uint8_t *poutu8 = poutu8_0 + iht*u8_row_stride+ id32*u8_d32_stride + d0;
					// width and inner depth loops.
					for( iwid = 0; iwid < width; iwid++ ){
						for( idep = 0; idep < dn; idep++ ){
							float x = pin[iwid*depth + idep];
							int xq = saturate_u8( roundf_i32( (x-out_min)*qscale ));
							poutu8[ iwid*32 + idep] = xq;
							poutf[ iwid * depth + idep] = (float)xq * deqscale + out_min;
						}
					}
					// next depth slice
					dremain -= dn;
					d0 = 0;
					pin += dn;
					poutf += dn;
				} // d32 loop
			}else{
				// depth chunk loop (no dequant)
				int d0 = dpad0;
				int dremain = depth;
				for( id32 = 0; id32 < nd32; id32 ++ ){
					int dn = min_i32( 32-d0, dremain );
					uint8_t *poutu8 = poutu8_0 + iht*u8_row_stride+ id32*u8_d32_stride + d0;
					// width and inner depth loops.
					for( iwid = 0; iwid < width; iwid++ ){
						for( idep = 0; idep < dn; idep++ ){
							float x = pin[iwid*depth + idep];
							int xq = saturate_u8( roundf_i32( (x-out_min)*qscale ));
							poutu8[ iwid*32 + idep] = xq;
						}
					}
					// next depth slice
					dremain -= dn;
					d0 = 0;
					pin += 32;
				} // d32 loop
			}
		} // height loop
	} // batch loop
}
static int get_option( struct nn_graph * nn, const struct tensor * tens_in, int default_val, char const *option_desc, int maxval)
{
	if( tens_in == NULL) return default_val;
	int newval = tensor_get_int32( tens_in,  0 );
	if( newval < 0 || newval > maxval){
		logmsg(nn,0,"quantfortest: value %d out of range (0..%d) for %s; using default of %d",
				newval, maxval, option_desc, default_val );
		return default_val;
	}else{
		return newval;
	}
}

static int quantfortest_execute(struct nn_node *self, struct nn_graph *nn)
{
	const struct tensor *tensor_in = self->inputs[0];
	struct tensor *tensor_outqu8 = self->outputs[0];
	struct tensor *tensor_outmin = self->outputs[1];
	struct tensor *tensor_outmax = self->outputs[2];
	struct tensor *tensor_outf = (self->n_outputs < 4)? NULL : self->outputs[3];

	// @@ check types

	float out_minmax[2];
	tensor_find_range_float( tensor_in, 0/*all of it*/, out_minmax );

	// establish that 0.0 is in the range and that max > min
	out_minmax[0]= fminf(0.0f, out_minmax[0]);
	out_minmax[1] = fmaxf( fmaxf(0.0f, out_minmax[1]), out_minmax[0] + 0.001f );
	if( out_minmax[0] < 0.0f ){
		// correct range so that 'zero point' is an integer
		adjust_minmax_for_zero( & out_minmax[0], & out_minmax[1] );
	}

	int batches = tensor_in->shape.batches;
	int height = tensor_in->shape.height;
	int width = tensor_in->shape.width;
	int depth = tensor_in->shape.depth;

	// process optional padding
	int dpad = 0;
	int wpad = 4;
	int wpad_r_min = 0;
	int hpad = 4;
	if( self->n_inputs >=2){
		dpad = get_option( nn, self->inputs[1], dpad, "depth padding", MAX_PADDING_DEPTH );
		if( self->n_inputs >=3 )
			wpad = get_option( nn, self->inputs[2], wpad, "width padding(left)", MAX_PADDING_WIDTH );
		if( self->n_inputs >=4 )
			wpad_r_min = get_option( nn, self->inputs[3], wpad_r_min, "width padding (min right)", MAX_PADDING_WIDTH );
		if( self->n_inputs >=5 )
			hpad = get_option( nn, self->inputs[4], hpad, "height", MAX_PADDING_HEIGHT );
	}


	int wall = (wpad + width + wpad_r_min + 3)&~3;
	int wpad_r = wall - (wpad + width);
	if( wpad_r > MAX_PADDING_WIDTH) wpad_r -= 4;

	int dall = (dpad + depth + 31) & ~31;
	int dpad_r = dall - (dpad + depth);


	// allocate the quantized tensor
	if (tensor_out_prepare_padded_d32(
		tensor_outqu8,  batches,
		height, hpad, hpad,
		width,  wpad, wpad_r,
		depth, dpad, dpad_r,
		NN_TYPE_QUINT8) != 0) {
		return errlog(nn,"failure preparing output, max_size=%d bhwd=%d,%d,%d,%d",
			tensor_outqu8->max_size,batches,height,width,depth);
	}

	// allocate the 'float' output tensor
	if( tensor_outf != NULL ){
		if( tensor_out_prepare_normal(tensor_outf,batches,height,width,depth, NN_TYPE_FLOAT )!= 0){
			return errlog(nn,"out too small: max_size=%d bhwd=%d,%d,%d,%d",
							tensor_outf->max_size,batches,height,width,depth);
		}
	}

	quantize_with_dequant( tensor_in, tensor_outqu8, tensor_outf, out_minmax[0], out_minmax[1] );

	tensor_set_single_float(tensor_outmin, out_minmax[0]);
	tensor_set_single_float(tensor_outmax, out_minmax[1]);
	return 0;
}


//
// QuantizeForTest:
// convert float to 8-bit quantized, using the actual min and max
// range the data.
// Output is d32 tensor, and min/max values.
// Optional 4th output is the same data requantized back to float.
// optional inputs/outputs may be NULL.

//  input 0:   float tensor
//  input 1:  (optional) scalar int: depth padding start - default 0  (0..31)
//  input 2:  (optional) scalar int: width padding start - default 4  (0..MAX_PADDING_WIDTH)
//  input 3:  (optional) scalar int: width padding end (min) - default 0  (0..MAX_PADDING_WIDTH)
//    The 'end' padding will be adjusted up so that the width total is a multiple of 4. If it exceeds
//    MAX_PADDING_WIDTH as a result, it will be adjusted down by 4.
//  input 4:  (optional): scalar int: top/bottom padding for height  default 4 (0..MAX_PADDING_HEIGHT)

//
//  output 0:   		qu8 d32 format tensor
//  output 1,2: 		 scalar float, output min & max
//  output 3 (optional): float tensor, requantized.

struct nn_node_ops nn_ops_for_QuantizeForTest_d32 = {
	.execute = quantfortest_execute,
	.check = NULL,
	.ctor = node_alloc_common,
	.dtor = node_free_common,
	.n_inputs = NN_IOCOUNT_RANGE(1,5),
	.n_outputs = NN_IOCOUNT_RANGE(3,4),
	.flags =  NN_NODE_FLAG_D32_OUTPUT,
};

// The QuantizeForTest_d32 serves as 'reference' for
// _AutoQuantize_d32, just without the fourth output.

struct nn_node_ops nn_ops_for_AutoQuantize_d32_ref = {
	.execute = quantfortest_execute,
	.check = NULL,
	.ctor = node_alloc_common,
	.dtor = node_free_common,
	.n_inputs = NN_IOCOUNT_RANGE(1,5),
	.n_outputs = NN_IOCOUNT(3),
	.flags =  NN_NODE_FLAG_D32_OUTPUT,
};
