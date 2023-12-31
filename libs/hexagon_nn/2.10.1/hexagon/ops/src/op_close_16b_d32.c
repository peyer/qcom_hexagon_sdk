
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
#include "errstats.h"
/*
 * This operator checks to see if a d32 tensor (with min and max)
 * is 'close enough' to a supplied reference (float) tensor.
 * it is intended to be used in situations like this, where a single
 * quantized op is being tested; and the reference op is given the same
 * input (dequantized to match the quantized input), so  the results
 * can be expected to be very close to the reference result:
 *
 *   (Here ||| and == represent qu8_d32,min,max):
 *
 *
 *     (test constant)                        (test_constant)
 *           |                                     |
 *           |                                     |
 *      [QuantizeForTest_d32]                  [QuantizeForTest_d32]
 *        |||           |                        |||            |
 *        |||           |                        |||            |
 *        |||           |        +---------------|||------------+
 *        |||           |        |               |||
 *        |||        (reference op )             |||
 *        |||             |                      |||
 *        |||             +---------+            |||
 *        |||                       |            |||
 *        |||     +++============================+++
 *        |||     |||               |
 *       ( op. being tested)        |
 *        |||                       |
 *        |||     +-----------------+
 *        |||     |
 *       (Close_d32)
 */

//
// This node will now fail if the DUT's range is more than about 5x larger
// than it needs to be, based on the range of the reference data.
// This can be suppressed, by setting padding to NN_PAD_VALID.

#define MAX_TO_SHOW 100



////////////////////////////////////////////////////////


static int close_16b_d32_execute(struct nn_node *self, struct nn_graph *nn)
{
#ifdef TIMING_MODE
	return 0;
#endif
	if( nn_option_get(nn,debug_skip_check))  return 0;

	int i;
	const struct tensor *tensor_in = self->inputs[0];
	const struct tensor *tensor_in_min = self->inputs[1];
	const struct tensor *tensor_in_max = self->inputs[2];
	const struct tensor *tensor_ref = self->inputs[3];
	logmsg(nn,2,"close_d32 execute. self=%p ",self);


	float max_exc_err = 0.2f;		// the maximum excess error allowed
	float max_exc_err_frac = 0.05;	// max fraction of points with nonzero excess error.

	if( self->n_inputs >= 5 ){	// optional parms?
		if( self->inputs[4] != NULL){
			max_exc_err = tensor_get_float(self->inputs[4],0);
		}
		if( self->n_inputs >= 6 &&  self->inputs[5] != NULL ){
			max_exc_err_frac = tensor_get_float(self->inputs[5],0);
		}
	}

	for(i = 0; i < 4; i++){
		if(tensor_in->shape.dimension[i] != tensor_ref->shape.dimension[i] ){
			return errlog(nn,
				"close_d32: shape %d:%d:%d:%d does not match reference %d:%d:%d:%d",
				  tensor_in->shape.batches, tensor_in->shape.height,
				  tensor_in->shape.width, tensor_in->shape.depth,
				  tensor_ref->shape.batches, tensor_ref->shape.height,
				  tensor_ref->shape.width, tensor_ref->shape.depth);

		}
	}
	float dut_min_float = tensor_get_float(tensor_in_min,0);
	float dut_max_float = tensor_get_float(tensor_in_max,0);

	int ib, ih,iw,id;
	int batches = tensor_in->shape.batches;
	int height = tensor_in->shape.height;
	int width = tensor_in->shape.width;
	int depth = tensor_in->shape.depth;

	// check range is sane...
	int is_u16 = self->node_type == OP_Close_u16b_d32;
	float qscale = 32768.0f/fmaxf(dut_max_float, -dut_min_float);
	if (is_u16) qscale = 65536.0f / (dut_max_float - dut_min_float);
	float offset = 0;
	if (is_u16) offset = dut_min_float;
	// check constraints (also catch NaN and inf)
	if ( !( (dut_max_float >= 0.0f)		 // catches max <0  and NaN
	      && (dut_min_float <= 0.0f)	// catches min > 0 and NaN
	      && qscale > 0.0f 				// to catch inf
	      && qscale <= 65536e+6f ) ) {	// range must be >= 1e-6
		return errlog(nn,"invalid input range: %f .. %f", dut_min_float, dut_max_float);
	}

	/// ---------------------
	/// measure the stats
	// Use 3 levels to reduce precision loss in the accumulation
	struct err_stats errstatA, errstatB, errstatC;
	errstats_clear( & errstatA );
	errstats_clear( & errstatB );
	errstats_clear( & errstatC );
	float const * refp = (float const *)tensor_ref->data;

	float actual_min = -1e-6f;
	float actual_max = 1e-6f;


	for( ib = 0; ib < batches; ib++ ){
		for (ih = 0; ih < height; ih++ ){
			int ix_bh = (ib*height + ih) * (width*depth);
			for( iw = 0; iw < width; iw++ ){
				for( id = 0; id < depth ; id ++ ){
					int idx = ix_bh + iw*depth + id;

					int32_t tdata = *tensor_location_16b_d32( tensor_in, ib, ih, iw, id );
					if (is_u16) tdata = (uint16_t)tdata;
					float refx = refp[idx];
					float refdata = (refx- offset)*qscale;
					actual_min = fminf( actual_min, refx);
					actual_max = fmaxf( actual_max, refx);
					int is_clip = tdata == -32768 || tdata == 32767;
					if (is_u16) is_clip = tdata == 0 || tdata == 65535;
					errstats_add_point( &errstatA, tdata, refdata, idx, is_clip );
					if( errstatA.ncount >= 100){
						errstats_dump_and_clear( &errstatB, &errstatA );
						if( errstatB.ncount >= 10000 )
							errstats_dump_and_clear( &errstatC, &errstatB );
					}
				}
			}
		}
	}
	float over_range = 0.0;	// frac by which range is too large
	{
		float range_dut  = fmax(dut_max_float, actual_max ) - fmin( dut_min_float, actual_min);
		float range_ref =  actual_max - actual_min;
		if( range_ref >1e-3f){
			over_range = (range_dut-range_ref)/range_ref;
		}
	}

	// collect all the errors to errstatC.

	errstats_dump_and_clear( &errstatB, &errstatA );
	errstats_dump_and_clear( &errstatC, &errstatB );

	float mean_error,rms_error;
	errstats_find_mean_rms( &errstatC, &mean_error,&rms_error);

	int force_report = max_exc_err < 0.0f;
	max_exc_err = fabsf(max_exc_err);

	int acceptable =
			( errstatC.max_excerr <= max_exc_err )
		&& ( errstatC.ncount_excerr  <=  max_exc_err_frac * errstatC.ncount );
	if(self->padding != NN_PAD_VALID && over_range > 4.0f){
		logmsg(nn,0,"Computed range is too large for the reference data");
		acceptable = 0;
	}

	int loglev = (acceptable && !force_report)? 2: 0;

	logmsg(nn,loglev,"Ref data range = %f .. %f; dut range = %f .. %f; oversize by %.2f %%\n",
			actual_min, actual_max, dut_min_float, dut_max_float, over_range *100.0f);

	logmsg(nn,loglev,"Out of %d points: mean err = %.3f, rms err = %.3f; largest excess err = %.3f (%d are nonzero)",
			errstatC.ncount, mean_error, rms_error, errstatC.max_excerr, errstatC.ncount_excerr );
	if( errstatC.ncount_notsat >= 16){
		errstats_find_correlation( nn , loglev, &errstatC, -32767, 0, 32767);
	}
	if( ! acceptable ){
		int count = batches * height * width * depth;
		int ipos;
		// only log points with excess err > error_disp_lev; only log up to MAX_TO_SHOW
		// (but always log the worst one)
		float error_disp_lev = 0.0f;
		if( errstatC.max_excerr > 8.0f) error_disp_lev = errstatC.max_excerr * (float)(1./16.);

		logmsg(nn,0,"Exceeds error limits. Values below are in quantized units; only shown if exc err > %f", error_disp_lev);
		logmsg(nn,0,"\t\tActual\t\tExpected\tDiff\tExcErr");
		int nlogged = 0;

		for (ipos = 0; ipos < count; ipos++) {
			int b,h,w,d,k;
			d  = ipos % depth; k = ipos/ depth;
			w = k % width;	k= k/width;
			h = k % height;
			b = k / height;
			// find excess error
			int32_t tdata = *tensor_location_16b_d32( tensor_in, b, h, w, d );
			if (is_u16) tdata = (uint16_t)tdata;
			float refdat = refp[ipos];
			float refdatq = (refdat-offset)*qscale;
			float excerr = fabsf( tdata-refdatq) - fabsf( roundf(refdatq)-refdatq);
			if( excerr > error_disp_lev){
				char const * flag = ( ipos == errstatC.pos_largerr)? " <====": "";

				if( nlogged < MAX_TO_SHOW || ipos == errstatC.pos_largerr){
					logmsg(nn,0,"%d[%d,%d,%d,%d])\t%d\t%f\t%f\t%f%s",ipos,b,h,w,d, tdata,refdatq,tdata-refdatq,excerr, flag);
					nlogged ++;
				}else if( nlogged < MAX_TO_SHOW+10){
					logmsg(nn,0, "[Stopped after %d errors]", nlogged);
					nlogged = MAX_TO_SHOW +10;
				}
				if( ipos >= errstatC.pos_largerr && nlogged >= MAX_TO_SHOW+10) break;
			}
		}
		return errlog(nn, "test failed");
	}

	logmsg(nn,2,"close_d32 node %p OK",self);
	return 0;
}


//
//  input0:   test tensor, d32 quantized
//  input1:   float scalar: min of range
//  input2:   float scalar: max of range
//  input3:   reference tensor, float
// OPTIONAL (may be missing or null):
//   input4:  max acceptable 'excess error' - default 0.2
//   input5:  max accetable frac of outputs which have nonzero excess error; default = 0.05
//
// If the max acceptable error is negative, its abs value will be used, and stats will
// be reported even if the test passes.
//
//  No outputs


struct nn_node_ops nn_ops_for_Close_16b_d32 = {
	.execute = close_16b_d32_execute,
	.check = NULL,
	.ctor = node_alloc_common,
	.dtor = node_free_common,
	.n_inputs = NN_IOCOUNT_RANGE(4,6),
	.n_outputs = NN_IOCOUNT(0),
	.flags = NN_NODE_FLAG_D32_INPUT,
};

struct nn_node_ops nn_ops_for_Close_u16b_d32 = {
	.execute = close_16b_d32_execute,
	.check = NULL,
	.ctor = node_alloc_common,
	.dtor = node_free_common,
	.n_inputs = NN_IOCOUNT_RANGE(4,6),
	.n_outputs = NN_IOCOUNT(0),
	.flags = NN_NODE_FLAG_D32_INPUT,
};
