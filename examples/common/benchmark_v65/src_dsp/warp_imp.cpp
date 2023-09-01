/**=============================================================================

@file
   warp_imp.cpp

@brief
   implementation file for warp RPC interface.

Copyright (c) 2018-2019 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

//==============================================================================
// Include Files
//==============================================================================

// enable message outputs for profiling by defining _DEBUG and including HAP_farf.h
#ifndef _DEBUG
#define _DEBUG
#endif
#include "HAP_farf.h"
#include "qurt_thread.h"

#include "q6cache.h"

// profile DSP execution time (without RPC overhead) via HAP_perf api's.
#include "HAP_perf.h"

#if (__HEXAGON_ARCH__ >= 65)
#include "HAP_vtcm_mgr.h"
#else
static void* HAP_request_VTCM(int a, int b) {return 0;}
static void HAP_release_VTCM(void *a)   {}
#endif

#include "AEEStdErr.h"

// includes
#include "benchmark.h"
#include "benchmark_asm.h"

#include "dspCV_worker.h"

#include "hexagon_types.h"
#include "hexagon_protos.h"

#include <stdint.h>
#include <algorithm>
#include <stddef.h>

#include "hvx_util.h"

#include "HAP_compute_res.h"

/*===========================================================================
    DEFINITIONS
===========================================================================*/
#define PROFILING_ON

// (128-byte is only mode supported in this example)
#define VLEN (128)
#define HVX_WARP_ASSERT(x)

#define MIN_VTCM_SZ            (64*1024)						// the minimum required VTCM size. Note that for VTCM size less than 256KiB,
																// the input size also dimenishes since the algorithm uses the VTCM as the 
																// intermediate buffer.
#define MAX_WORKER_NUM         (4)                              // correspond to the maximum of the number of HVX context
#define VTCM_VGATHER_SZ        (2048)                           // 2KiB for each worker thread
#define VTCM_VGATHER_TOTAL_SZ  (VTCM_VGATHER_SZ*MAX_WORKER_NUM)       
#define MAX_TILE_PROC_SZ       (512)                            // calculate using Chroma: 248KiB/(8*8*8)=496


// the info of each tile group
struct tilesInfo_t{
    int x;      // start pos of the starting(left most) tile, in tile unit
    int y;
    int sz;     // the size of tiles, in tile unit    
};

// the info of each tile proc within a intermediate image buffer
struct intmed_para_t{
    int intmed_orig_x; // origin of the intermediate image wrt the full input image.
    int intmed_orig_y;   
    int intmed_width;  // width of intermediate image in pixels
    int intmed_height; // height in rows

    // information for the processTile routines
    int intmed_offs_x;  // offset of the intermediate image wrt grid
    int intmed_offs_y;   

    // it stores the tile proc within the intermediate image buffer
    uint8_t *intmed_image;
};


//
// This struct carries information
// for an in-process operation
// Note:
//   - output row pitch must be vector-aligned
//   - no alignment constraint on output address
//   - row pitch must be a multiple of 4
//
struct warpop_context {
    // output buffer
    // When processing Chroma, out_width and out_height are assumed to be even, and in Luma pixels.
    // (so the actual height will be half of what is indicated, for 4:2:0).
    int out_width, out_height;  // in pixels
    uint8_t * output_img_Y;     // Luma output image
    unsigned out_rowpitch_Y;    // Luma image pitch
    uint8_t * output_img_C;     // Chroma output image
    unsigned out_rowpitch_C;    // Chroma image pitch

    //
    // grid must be int32_t aligned; things run a bit faster
    // if it's 8-byte aligned (and grid_rowpitch is even).
    //
    int grid_width, grid_height;    // size of grid array
    int32_t const * grid_array;     // points to grid of (x,y) pairs with 6 fractional bits
    int grid_rowpitch;              // row pitch (in int32 units)

    // input image dims
    // both must be even
    // (these are always Luma dims).
    //
    int input_width, input_height;
    uint32_t border_widths;         // normally 0

    unsigned intmed_rowpitch;       // row pitch in bytes (multiple of 4) of the intermediate image buffer  
};

// multi-threading context structure
typedef struct
{
    dspCV_synctoken_t    *token;             // worker pool token
    unsigned int          workerCount;       // atomic counter, shared by all workers    
    unsigned int          jobCount;          // atomic counter of the finished job, shared by all workers
    unsigned int          jobNum;            // total number of jobs, each tile processing unit is a job

    // the following is per tile group parameters
    warpop_context        *pCtx;
    uint8                 *vtcmVgatherBuf[MAX_WORKER_NUM]; // buffer used for vgather, per worker thread

    // the following is per tile proc parameters
    struct tilesInfo_t    *pTileProcInfo;
    struct intmed_para_t  *pIntmedPara;    
} warp_callback_t;

/*===========================================================================
    DECLARATIONS
===========================================================================*/
#ifdef __cplusplus
    extern "C" {
#endif

AEEResult benchmark_warp(
                            remote_handle64 handle,
                            const uint8* src, 
                            int inpLen, 
                            uint8* dst, 
                            int outpLen, 
                            int32 width,            
                            int32 height,           
                            int32 srcStride,
                            int32 dstStride,
                            const uint8* gridarr,   
                            int   gridarrLen, 
                            int32 gridStride,
                            int32 LOOPS,
                            int32 wakeupOnly,
                            int32 useComputRes,
                            int32* dspUsec,
                            int32* dspCyc
                            );                          
    
#ifdef __cplusplus
}
#endif

//
//
// API for warping:
// (1) set up the output image and grid array parameters.
//   It should be the case that out_width = 16*(grid_width-1)
//                              out_height = 16*(grid_height-1)
//   output_img_C and out_rowpitch_C are only needed if you are processing
//   Chroma.  Output row pitches must be multiple of 128, but the pointers
//   do not need to be aligned.
//
//   Set up input width, height
//   Set border width to 0.
//   Also, point warp_tmp at a 1K byte region within the VTCM.
//
//
//
// (2) divide the output into tile groups (rectangular subregions of tiles).
//      it is recommended that groups are at least 4 rows of 8; but any size is allowed.
//      It's most efficient when the width of a group is a multiple of 8.
//
//      For each tile group,
//          (a)  call findBBoxForTileGroup, this will set
//                      tilegroup_x,  tilegroup_y,
//                      tilegroup_width,  tilegroup_height,
///             (to select a region in the output) and will also set the following, which identify a source
//              region of the image:
//                      intmed_orig_x, intmed_orig_y,
//                      intmed_width, intmed_height
//              it also sets intmed_offs_x, intmned_offs_y
//                (which are usually the same as intmed_orig_x, intmed_orig_y, but not always).
//          (b) you must now copy the indicated rectangle from the input image to VTCM, setting
//                intmed_rowpitch and intmed_image. It is acceptable to copy a larger area (expanded on
//                right and/or bottom) and (optionally) expand intmed_width and intmed_height to indicate this.
//                See note below regarding Chroma.
//              If the region requested is too large, you can go back to step (2a) and specify a smaller
//              group of tiles.
//          (c) call processTileGroupLuma or processTileGroupChroma420 according to whether you are doing Chroma or Luma.
//
//  Notes:
//    intmed_orig_x is always a multiple of 4
//    intmed_orig_y is always a multiple of 2
//    intmed_width is always >= 4
//    intmed_height is always >= 4 and even.
//     (note that the source region can be become small when the source pixels are off the edge of the
//     input image. The above mimina are enforced, however).
//    The fields "intmed_offs_x, intmed_offs_y" are a message from findBBoxForTileGroup to
//     the processTile functions; do not rely on how they are set currently.
//
//  intmed_rowpitch must be set to a multiple of 4
//  intmed_image pointer must be a multiple of 4.
//
// Chroma (4:2:0) is processed by the same bounding box supplied for Luma; you need to load a region of the
// same width as the corresponding Chroma (with alternating U,V pixels) but of half the height; thus  intmed_orig_y
// and  intmed_height (which are both always even) should be divided by 2 when processing Chroma.
//
// To do Luma and Chroma, you may either
//   (A) do the whole process for Luma, and the whole process for Chroma (possibly using different group
//      structures for each)
//  or
//   (B) Within each tile group, repeat steps (2b,2c) for Luma and then Chroma.
// (or any mix).
//
//  To process the operation in multiple threads:
//      - each thread should have its own context; the upper fields (down to border_with) would be set up the same
//       way for all of them
//      - each thread needs its own 'warp_tmp' region in VTCM
//      - each thread can now do step 2 repeatedly in its own context, such that the set of groups forming the
//        whole output image is split between the threads. Each thread needs to have its own region in VTCM
//        for loading the intermediate image.
//
//
// INPUT BORDERS
//  Edge clipping is not very satisfactory in the default setup; for Luma, the sample point x is effectively
// clipped to the range 1.0 .. w-(2+1/64); so the edge issue is avoided, but you can't really get to the edge.
//  And likewise for y.
//
// A workaround is to add a border to the input image, and then compensate the grid, so that the 'clipped'
// coords will always fall within the border. You can also set 'border_widths' variable to "virtually" add a border:
//     - border_width has 4 uint8 values, from lsb to msb: left_border, right_border, top_border, bottom_border.
// All must be even, and the first two should be a multiple of 4 (0x04040404 is recommended).
// The effect of this on the warp code is simple:
//      - when findBBoxForTileGroup is called apparent, input size is increased: width is considered to be
//       left_border + right_border greater than specified,
//       and height is considered to be top_border + bottom_border greater than specified.
//     - all of the grid points are considered to be increased by (left_border, top_border)*64, so they are compensated
//       for the border (this effect on the actual warp is done by adjusting intmed_offs_x, intmed_offs_y).
//
// To support this, you need to compensate the findBBoxForTileGroup results and check to see if they include the border.
//  Normally, the input region identified by findBBoxForTileGroup falls strictly within the image size. When a border is defined,
//  it falls within the area with border added. So, you can copy pixels from the source image, and add border pixels as needed
//  if it includes the border:
//
//  Example: suppose border_widths = 0x04040404 and input_width, input_height = 1280,720.
//   Thus the padded input is 1288 x 728.
//
//     if findBBoxForTileGroup says to copy a 240x24 area starting at (x,y) = (116,24), then you should
//      copy  240x24 area starting at (x,y) = (112,20); no border is involved.
//
//     if findBBoxForTileGroup says to copy a 300x28 area starting at (x,y) = (988,42), that extends
//     to include column 1287 (all of the right border), so you should
//      - copy  296x28 area starting at (x,y) = (984,42)
//      - add to that, on the right, a 4x28 area consisting of 'right  margin' (whatever you want that to be); this
//        completes the 300x28 intermediate buffer.
//
//     if findBBoxForTileGroup says to copy a 264x30 area starting at (x,y) = (104,0): it extends into the
//     upper border, so you should
//      - create a 264x4 row "top border" in the intermediate buffer
//       - copy a 264x26 row area, from (x,y) = (104,0) in the input, below that top border in the intermediate buffer.
//
//   It is possible that a request will overlap the border on more than one edge, or that it may only partly enter
//   the border (e.g. it may start at row 2 of a 4-row upper border).
//
// For Chroma:
//   - all vertical extents are in Luma rows (so will need to be halved when considering the copy operation)
//   - x boundaries of copies will be even; padding will be U/V interleaved.
//
// It may be that the requested region falls entirely within the border area (this happens if the source pixels
// for the group are all entirely outside the actual image). In this case, you can create an intermediate image
//  filled with border pixels and run as usual; but if your border is all one color, you can also skip the warp
// operation and just fill the output tiles area with the desired pixel value. There are two functions
// fillTileGroupLuma and fillTileGroupChroma420, for this purpose.
//
//

void findBBoxForTileGroup(warpop_context & ctx,       int tilex, int tiley, int tileSz, int &minY, int &maxY);
void findBBoxForTileGroup(warpop_context & ctx,       int tilex, int tiley, int tile_wid, int tile_ht, intmed_para_t *pIntmedPara);

/*===========================================================================
    TYPEDEF
===========================================================================*/


/*===========================================================================
    LOCAL FUNCTION
===========================================================================*/
//
// Equivalent to
// (1) load n x (int32,int32) from ptr[0]..ptr[2*n-1]
//    where n is in range 1..8
//    These are returned in the lo part of the result.
// (2) load n x (int32,int32) from ptr[2]..ptr[2*n+1]
//    this  is returned in the hi vector.
// Both results thus contain n x(i32,i32) and followed by (VECN/8)-n invalid
// slots.
// Alignment: no alignment constraints on ptr.
//
// Function uses vector-aligned reads, and will not read a vector that doesn't
// contain any of p[0] .. p[2*n-1].
//
//
static inline HVX_VectorPair load_xy_and_delta( int32_t const * ptr, int n)
{
    static const int NVEC = hvxUtil::NVEC;
    // Find the aligned address of the first vector (the one containing
    // the first byte)
    //
    size_t p0 = reinterpret_cast<size_t>(ptr) & ~size_t(NVEC-1);
    // Find aligned the address of the vector containing the last byte
    //    (which is either the same, or one vector more)
    size_t p1 = (reinterpret_cast<size_t>(ptr) + (8*n+7)) & ~size_t(NVEC-1);
    //
    //  get the two vectors (or 1)
    HVX_Vector v0 = * reinterpret_cast< HVX_Vector const*>( p0);
    HVX_Vector v1 = * reinterpret_cast< HVX_Vector const*>( p1);
    //
    // extract the 'first part' with valign
    //
    int aloff = (int)reinterpret_cast<size_t>(ptr);
    HVX_Vector vals0 = Q6_V_valign_VVR(v1,v0, aloff);
    // for the second part, we can't use another align since the required
    // bytes could start within v1. So just vror vals0.
    // ptr+2 is aligned; we need to use a vlalign.
    HVX_Vector vals1 = Q6_V_vror_VR( vals0, 8);
    // and done.
    return Q6_W_vcombine_VV( vals1, vals0 );
}

//
// Assumptions about the (n+1 x 2) grid of points examined:
//  - if the difference between any two of the points is found, it must fit in 16 bit signed;
//  - if the difference between (xoff,yoff) and any of the points is found, it must fit
//    in 16 bit signed.
// Typically (xoff,yoff) is based on finding a bounding box for the whole set of points, but
// if some of the points are off left of the input image, or off the top, you would
//  clamp xoff ot yoff so that those points appear negative when (xoff,yoff) are subtracted,
// and thus will be clipped to the edge.
//
// With these assumptions, we can ignore all but the lower 16 bits of the grid points, and
// the (xoff,yoff) values, and do the offseting modulo 16 bits.



// load_point_interp_coeffs_8tile
//////////////////////////////////
// This function computes 4 vectors vbase, vdx, vdyL, vdyR; these are used to find 32
// pixel coords in parallel, as follows (for i in 0..7, j in 0..7 we generate all 2048
// pixels in the 16 tiles:
//
//    vleft = vdyL {*}  (4096*i + 2048)  +  vdx
//    vright = vdyR {*}  (4096*i + 2048)
//  pos = vbase +  average(   vleft {*} (15*2048-4096*j),    vright*(4096*j+2048)
//
// .. in which a {*} b means the fractional multiply a*b/32768, rounded.
//
// For the first of four pixels in each cell, we want
//     vbase = UL
//     vdx = UR-UL
//     vdyL = DL-UL
//     vdyR = ((DR-UR) + (DL-UR))/2
//
//  .. and (given the 8 sets of {UL,UR,LL,LR)) this applies to the first 8 even slots
// in the result: 0,2 ... 14  of 32 lanes
//
// for the second half of the vector, we can displace all the corners as follows:
// (equiv to adding  8 to i):
//     UL' = UL + (DL-UL)/2             UR' = UR + (DR-UR)/2
//     DL' = DL + (DL-UL)/2             DR' = DR + (DR-UR)/2
// and using those values, and the formulae above, get lanes 16, 18 .. 30
//
// for the odd lanes, likewise displace by 1/2 to the right (equiv to adding 8 to j )
//
//     UL" = UL + (UR-UL)/2             UR" = UR + (UR-UL)/2
//     DL" = DL + (DR-DL)/2             DR" = DR + (DR-DL)/2
// Note that vdx will be the same in even lanes as odd, since UR"-UL" = UR-UL
// Likewise vdyL and vdyR are the same in lanes 16..31 as in lanes 0..15
//
// If the <TWOROW> parameter is true, then ntiles must be in range 1..4
// and we process 2 rows of ntile:
//   the second row of tiles take the place of tiles 4..7 in the result.
//
//
//
struct point_interp_coeffs {
    HVX_Vector vbase;
    HVX_Vector vdx;
    HVX_Vector vdyL;
    HVX_Vector vdyR;

    // i = 0..7; j = 0..7
    inline HVX_Vector do_interp_for_luma( int i, int j )
    {
        int imul = 4096*i + 2048;
        int jmul = 4096*j + 2048;
        int ii = Q6_R_combine_RlRl(imul,imul);
        int jj = Q6_R_combine_RlRl(jmul,jmul);
        int jj1 = Q6_R_vsubh_RR( 0x80008000, jj);	// 32k-jmul, 32k-jmul)

        HVX_Vector vleft = Q6_Vh_vmpy_VhRh_s1_rnd_sat(vdyL,ii);
        HVX_Vector vright = Q6_Vh_vadd_VhVh( Q6_Vh_vmpy_VhRh_s1_rnd_sat(vdyR,ii), vdx);
        return Q6_Vh_vadd_VhVh(  vbase,
                Q6_Vh_vavg_VhVh_rnd(
                    Q6_Vh_vmpy_VhRh_s1_rnd_sat( vleft, jj1),
                    Q6_Vh_vmpy_VhRh_s1_rnd_sat( vright, jj)
                ));

    }
    // for the Chroma case, we want things to be 1/2 of what they
    // are for Luma (so that after removing 6 fractional bits, the integer
    // part of the result is 1/2 of what it was for Luma).
    // So the final calc is done as
    //    average(    vbase, vleft*jj1 + vright*jj )
    // and we arrange for jj1 and jj2 to be 1/2 of what they were
    // for the Luma case.
    //      Luma i calc uses imul = 4096*i + 2048   (i = 0.. 7)
    //        change to      imul = 8192*i+ 4096    to spread out over 0..3
    //      Luma j calc uses jmul = 4096*i + 2048   (j = 0.. 7)
    //        change to      jmul = 8192*i + 2048   to spread out over 0..3
    //                                            (with 4:2:0 positioning)
    //        change to      jmul = 4096*i + 1024   // to get 1/2
    //
    // i = 0..3; j = 0..3
    inline HVX_Vector do_interp_for_chroma( int i, int j )
    {
        int imul = 8192*i + 4096;
        int jmul = 4096*j + 1024;
        int ii = Q6_R_combine_RlRl(imul,imul);
        int jj = Q6_R_combine_RlRl(jmul,jmul);
        int jj1 = Q6_R_vsubh_RR( 0x40004000, jj);   // 16k-jmul, 16k-jmul)

        HVX_Vector vleft = Q6_Vh_vmpy_VhRh_s1_rnd_sat(vdyL,ii);
        HVX_Vector vright = Q6_Vh_vadd_VhVh( Q6_Vh_vmpy_VhRh_s1_rnd_sat(vdyR,ii), vdx);
        return Q6_Vh_vavg_VhVh_rnd(  vbase,
            Q6_Vh_vadd_VhVh(
                Q6_Vh_vmpy_VhRh_s1_rnd_sat( vleft, jj1),
                Q6_Vh_vmpy_VhRh_s1_rnd_sat( vright, jj)
            ));

    }
};

template< bool TWOROW>
static inline point_interp_coeffs load_point_interp_coeffs_8tile( int32_t const * xygrid,   // first coord in grid
                                                                           int gridpitch,            // grid pitch in int32's
                                                                           int ntiles,               // # of tiles wide 1..8
                                                                           int32_t xoff,             // subtract this from all x coords in grid
                                                                           int32_t yoff              // subtract this from all y coords in grid
                                                                         )
{
    static const int NVECH = hvxUtil::NVEC/2;
    // get sample points, up and down
    HVX_VectorPair sample_up = load_xy_and_delta( xygrid, ntiles);
    HVX_VectorPair sample_dn = load_xy_and_delta( xygrid + gridpitch, ntiles);

    HVX_Vector vUL = Q6_V_lo_W( sample_up);
    HVX_Vector vUR = Q6_V_hi_W( sample_up);
    HVX_Vector vDL = Q6_V_lo_W( sample_dn);
    HVX_Vector vDR = Q6_V_hi_W( sample_dn);
    
    if( TWOROW)
    {
        static const int NVECQ = NVECH/2;
        HVX_VectorPred lower_quad = Q6_Q_vsetq_R(NVECQ);
        HVX_VectorPair sample_3rd = load_xy_and_delta( xygrid + 2*gridpitch, ntiles);
        HVX_Vector vDDL = Q6_V_lo_W( sample_3rd);
        HVX_Vector vDDR = Q6_V_hi_W( sample_3rd);
        // now:
        // replace lanes 8..15 of vUL with lanes 0..7 of vDL
        // replace lanes 8..15 of vUR with lanes 0..7 of vDR
        // replace lanes 8..15 of vDL with lanes 0..7 of vDDL
        // replace lanes 8..15 of vDR with lanes 0..7 of vDDR
        vUL = Q6_V_vmux_QVV( lower_quad, vUL, Q6_V_vror_VR( vDL, 3*NVECQ ));
        vUR = Q6_V_vmux_QVV( lower_quad, vUR, Q6_V_vror_VR( vDR, 3*NVECQ ));
        vDL = Q6_V_vmux_QVV( lower_quad, vDL, Q6_V_vror_VR( vDDL, 3*NVECQ ));
        vDR = Q6_V_vmux_QVV( lower_quad, vDR, Q6_V_vror_VR( vDDR, 3*NVECQ ));
    }

    // find (DL-UL)/2 and (DR-UR)/2
    //
    HVX_Vector dyL_half = Q6_Vw_vnavg_VwVw( vDL, vUL);
    HVX_Vector dyR_half = Q6_Vw_vnavg_VwVw( vDR, vUR);
    //
    // add that to the lower results, and place into the upper half of results.
    HVX_VectorPred lower_half = Q6_Q_vsetq_R(NVECH);
    vUL = Q6_V_vmux_QVV( lower_half, vUL, Q6_V_vror_VR( Q6_Vw_vadd_VwVw(vUL, dyL_half),NVECH));// replace half of the values in vUL
    vUR = Q6_V_vmux_QVV( lower_half, vUR, Q6_V_vror_VR( Q6_Vw_vadd_VwVw(vUR, dyR_half),NVECH));
    vDL = Q6_V_vmux_QVV( lower_half, vDL, Q6_V_vror_VR( Q6_Vw_vadd_VwVw(vDL, dyL_half),NVECH));
    vDR = Q6_V_vmux_QVV( lower_half, vDR, Q6_V_vror_VR( Q6_Vw_vadd_VwVw(vDR, dyR_half),NVECH));
    // those are all the 'even' lane corners; find the odd-lane corners...

    HVX_Vector dxU_half = Q6_Vw_vnavg_VwVw( vUR, vUL);
    HVX_Vector dxD_half = Q6_Vw_vnavg_VwVw( vDR, vDL);

    HVX_Vector vUL1 =  Q6_Vw_vadd_VwVw(vUL, dxU_half);
    HVX_Vector vUR1 =  Q6_Vw_vadd_VwVw(vUR, dxU_half);
    HVX_Vector vDL1 =  Q6_Vw_vadd_VwVw(vDL, dxD_half);
    HVX_Vector vDR1 =  Q6_Vw_vadd_VwVw(vDR, dxD_half);

    // now we drop the upper 16 bits of each value, and pack the rest.
    // The vdeal will, within each 64-bit lane, convert
    //    vUL: {  xlo   xhi    ylo   yhi }
    //    vUL1:{  xol1, xhi1, ylo1, yhi1 }
    // to
    //         { xlo, ylo, xlo1, ylo1 }
    // (and the upper 16 bits in the upper vector are discarded)
    //
    vUL = Q6_V_lo_W( Q6_W_vdeal_VVR(vUL1, vUL, 6));
    vUR = Q6_V_lo_W( Q6_W_vdeal_VVR(vUR1, vUR, 6));
    vDL = Q6_V_lo_W( Q6_W_vdeal_VVR(vDL1, vDL, 6));
    vDR = Q6_V_lo_W( Q6_W_vdeal_VVR(vDR1, vDR, 6));
    //
    // ok now everything is 16 bits. All subtractions below must be done *without* saturation.

    HVX_Vector vdyL = Q6_Vh_vsub_VhVh(  vDL, vUL);

    point_interp_coeffs result;

    result.vbase = Q6_Vh_vsub_VhVh(  vUL, Q6_V_vsplat_R( Q6_R_combine_RlRl( yoff,xoff)));
    result.vdx = Q6_Vh_vsub_VhVh(  vUR, vUL);
    result.vdyL = vdyL;
    result.vdyR = Q6_Vh_vavg_VhVh( Q6_Vh_vsub_VhVh(  vDR, vUR), vdyL );
    return result;
}
//
// load cubic interp coefficients.
//
// The input contains 64 x and 64 y phase indices, each in range 0..63, ordered as
//  x0 x32 y0 y32 x1 x33 y1 y33 ....      x31 x63 y31 y63
//
// The result has a 'pack' of 4xu8 for each input index.
//
// Result 0:
//      xvals0 xvals1 xvals2            .. xvals31
// Result 1:
//      yvals0 yvals1 yvals2            .. yvals31
//
// Result 2
//      xvals32 xvals33 xvals34         .. xvals63
// Result 3:
//      yvals32 yvals33 yvals34         .. yvals63
//
// each xvals or yvals is a group of 4 coefficients obtained from one lookup index.
// We use two tables, each is 64xu16 (and thus fits in a vector); one table provides the first two coeffs
// and one provides the second. The whole operation takes a total of 8 vlut16 operations.
//

//
static inline HVX_Vector_x4 find_cubic_coeffs( HVX_Vector fracs)
{
    static const HVX_Vector_union_i8  tbl[2] = {
            {{   0,-128,  8,-72,  1,-128,  8,-69,  2,-128,  7,-66,  3,-127,  7,-64,  // [c0:c1]  @ (0,32,1,33 ... 31,63)
                 4,-127,  7,-61,  4,-126,  7,-59,  5,-125,  6,-55,  6,-124,  6,-52,
                 6,-123,  6,-50,  7,-122,  5,-46,  7,-121,  5,-44,  8,-120,  5,-42,
                 8,-118,  4,-38,  8,-116,  4,-36,  9,-115,  4,-34,  9,-113,  3,-31,
                 9,-111,  3,-29,  9,-109,  3,-27,  9,-107,  2,-24,  9,-105,  2,-22,
                 9,-103,  2,-20,  9,-100,  2,-18,  9,-98,  1,-15,  9,-96,  1,-14,
                 9,-93,  1,-12,  9,-91,  1,-11,  9,-88,  1, -9,  9,-85,  0, -6,
                 9,-83,  0, -5,  9,-80,  0, -4,  8,-77,  0, -2,  8,-75,  0, -1}},
            {{   0,  0,-72,  8, -1,  0,-75,  8, -2,  0,-77,  8, -4,  0,-80,  9,   // [c2:c3]  @ (0,32,1,33 ... 31,63)
                -5,  0,-83,  9, -6,  0,-85,  9, -9,  1,-88,  9,-11,  1,-91,  9,
               -12,  1,-93,  9,-14,  1,-96,  9,-15,  1,-98,  9,-18,  2,-100,  9,
               -20,  2,-103,  9,-22,  2,-105,  9,-24,  2,-107,  9,-27,  3,-109,  9,
               -29,  3,-111,  9,-31,  3,-113,  9,-34,  4,-115,  9,-36,  4,-116,  8,
               -38,  4,-118,  8,-42,  5,-120,  8,-44,  5,-121,  7,-46,  5,-122,  7,
               -50,  6,-123,  6,-52,  6,-124,  6,-55,  6,-125,  5,-59,  7,-126,  4,
               -61,  7,-127,  4,-64,  7,-127,  3,-66,  7,-128,  2,-69,  8,-128,  1}}
    };

    HVX_Vector table0 = tbl[0].as_v;        // table of first 2 coeffs
    HVX_Vector table1 = tbl[1].as_v;        // table of second 2 coeffs
    // reshuffle so that x are in even lanes, y in odd.
    fracs = Q6_V_vdelta_VV( fracs, Q6_V_vsplat_R(0x03010200));
    //input is now:
    //  x0 y32 x32 y0 x1 y33 x33 y1 ....      x31 y63 x63 y31
    // (which is weird for the y, but we have x & y in even/odd lanes);
    // the y is easily sorted out at the end.
    // And {x0,y0,x32,y32} -> {x0,x32,y0,y32} is not possible in a vdelta.


    HVX_VectorPair result0 = Q6_Wh_vlut16_VbVhI(fracs,table0,0);
    HVX_VectorPair result1 = Q6_Wh_vlut16_VbVhI(fracs,table1,0);
    result0 =  Q6_Wh_vlut16or_WhVbVhI(result0,fracs, table0, 1 );
    result1 =  Q6_Wh_vlut16or_WhVbVhI(result1,fracs, table1, 1 );
    result0 =  Q6_Wh_vlut16or_WhVbVhI(result0,fracs, table0, 2 );
    result1 =  Q6_Wh_vlut16or_WhVbVhI(result1,fracs, table1, 2 );
    result0 =  Q6_Wh_vlut16or_WhVbVhI(result0,fracs, table0, 3 );
    result1 =  Q6_Wh_vlut16or_WhVbVhI(result1,fracs, table1, 3 );

    // now:
    // result0.v0:   { xvals0.low  xvals32.low  xvals1.low ...   xvals31.low xvals63.low}
    // result0.v1:   { yvals32.low  yvals0.low  yvals33.low ...   yals63.low yvals31.low}
    // result1 is the same but has the upper words.
    // So, deal them out...
    HVX_VectorPair resultX = Q6_Wh_vshuffoe_VhVh( Q6_V_lo_W(result1), Q6_V_lo_W(result0) );
    // resultX.v0 is xvals0 xvals1 xvals2           .. xvals31
    // resultX.v1 is xvals32 xvals33 xvals34        .. xvals63
    HVX_VectorPair resultY = Q6_Wh_vshuffoe_VhVh( Q6_V_hi_W(result1), Q6_V_hi_W(result0) );
    // resultY.v0 is yvals32 yvals33 yvals34        .. yvals63
    // resultY.v1 is yvals0 yvals1 xvals2           .. yvals31
    HVX_Vector_x4 result = {
            Q6_V_lo_W(resultX), Q6_V_hi_W( resultY ),   // x0..31, y0..31
            Q6_V_hi_W(resultX), Q6_V_lo_W( resultY )    // x32..63, y32..63
    };
    return result;
}

// given two vectors containing interp weights (one of the x,y pairs from find_cubic_coeffs)
// and a pointer to four vectors containing source pixels, find 32 results, each interpolated
// as bicubic.
// Each 32-bit lane in the result is one output; each of the input vectors also has data for
// one output in each 32-bit lane (though, in the inputs, it's always four separate bytes).
static inline HVX_Vector cubic_interp_partial( HVX_Vector xcoeff, HVX_Vector ycoeff, HVX_Vector const *inpixels)
{
    HVX_Vector vbias = Q6_V_vsplat_R( 0x7fc0);
    HVX_Vector sum0 = Q6_Vw_vrmpyacc_VwVubVb( vbias, inpixels[0], xcoeff);
    HVX_Vector sum1 = Q6_Vw_vrmpyacc_VwVubVb( vbias, inpixels[1], xcoeff);
    HVX_Vector sum2 = Q6_Vw_vrmpyacc_VwVubVb( vbias, inpixels[2], xcoeff);
    HVX_Vector sum3 = Q6_Vw_vrmpyacc_VwVubVb( vbias, inpixels[3], xcoeff);

    // the sop range from about -36K .. 4K (due to negative weights)
    // with the bias, they range from -4K ..36K
    // After >>7 when converting to 16 bits, they are
    //    -32 .. 288
    // this needs saturation to convert to 8 bit.
    // The 'bias' includes:
    //   255*128 (which offsets the inverted input range -255..0  to  0..255)
    //    64      (rounding bias for the >>7).
    //

    HVX_Vector sum02 = Q6_Vh_vasr_VwVwR( sum2, sum0,7); // combine in pairs...
    HVX_Vector sum13= Q6_Vh_vasr_VwVwR( sum3, sum1,7);  // combine in pairs...
    HVX_Vector sumH = Q6_Vub_vsat_VhVh( sum13, sum02);  // result
    /// now the result has, in each lane, 4 horizontally resampled pixels;
    // we just do the same trick to get a 32-bit sum-of-products.
    // it will need >>7 and saturate to u8.
    //
    return Q6_Vw_vrmpyacc_VwVubVb( vbias, sumH, ycoeff);
}

template <bool TWOROW>
static inline HVX_VectorPred
lanemask_for_gather(int ntiles)
{
    HVX_VectorPred gather_lanemask = q6op_Q_vsetq2_R( 8*ntiles + 64); // write (masked) to 1st vector using store_mask, after vror( v, store_ror)
    if(!TWOROW)
    {
        if( ntiles < 8)
        {
            gather_lanemask = Q6_Q_and_QQn( gather_lanemask, Q6_Q_vsetq_R(64));
            gather_lanemask = Q6_Q_or_QQ( gather_lanemask, Q6_Q_vsetq_R(8*ntiles));
        }
    }
    else
    {
        HVX_Vector cyc4 = Q6_V_vand_VV( q6op_Vb_vindices(), q6op_Vb_vsplat_R(31));  // (0..31, four times)
        gather_lanemask = Q6_Q_vcmp_gt_VbVb( q6op_Vb_vsplat_R(ntiles*8), cyc4);     // 8 byte lanes/tile
    }
    
    return gather_lanemask;
}

template<int NUMOPS>        //NUMOPSN = 2 or 4 only
static inline void vgather_w(
        HVX_Vector vaddr,   // base addresses
        int row_pitch,
        HVX_VectorPred lanemask,
        void const * baseaddr,
        unsigned gather_limit,
        HVX_Vector results[NUMOPS] )
{
    HVX_Vector offs = Q6_V_vsplat_R(row_pitch);
    Q6_vgather_AQRMVw( &results[0], lanemask,(int32_t)baseaddr, gather_limit, vaddr );
    vaddr = Q6_Vw_vadd_VwVw( vaddr, offs );
    Q6_vgather_AQRMVw( &results[1], lanemask,(int32_t)baseaddr, gather_limit, vaddr );
    
    if( NUMOPS >= 3)
    {
        vaddr = Q6_Vw_vadd_VwVw( vaddr, offs );
        Q6_vgather_AQRMVw( &results[2], lanemask,(int32_t)baseaddr, gather_limit, vaddr );
    }
    
    if( NUMOPS >= 4 )
    {
        vaddr = Q6_Vw_vadd_VwVw( vaddr, offs );
        Q6_vgather_AQRMVw( &results[3], lanemask,(int32_t)baseaddr, gather_limit, vaddr );
    }

}

#define FAKEDEP_VM( vec, mem)	asm  ("/*%0 %1*/": "=v"(vec), "=m"(mem): "0"(vec))
#define FAKEDEP_VV( vec,vec2)	asm ("/*%0 %1*/": "=v"(vec), "=v"(vec2): "0"(vec), "1"(vec2))
#define FAKEDEP_VVM( vec,vec2,mem)	asm ("/*%0 %1 %2*/": "=v"(vec), "=v"(vec2), "=m"(mem): "0"(vec), "1"(vec2))

//////////////////////////////////////////////////////////
// Do (up to) 8 tiles of Luma, at out_x, out_y
// TWOROW = false: ntiles = 1..8, one row of tiles
// TWOROW = true: ntiles = 1..4, two rows of tiles
//////////////////////////////////////////////////////////
//
// Currently this has an issue in that the 'intmed_orig' must be within +/-500
// pixels of all of the grid coords, otherwise 16-bit relative pixel addresses
// (with 6 fractional bits) can overflow. The origin will normally be at the upper
// left of the bounding box of source pixels (but will be generally be constrained to
// be >= 0 - unless the entire area is negative).
// If tile groups are at most 16x16 tiles, this shouldn't be an issue (unless you are shrinking a fair bit).
//
// If this is a problem, it can be fixed using a 'local origin', per call to this function, as below:
//   (1) pick local origin from one of the grid points (e.g bottom right corner of first cell)
// and round it to pixels, with y even, x a multiple of 4.
//   (2) use that value to set up xoff,yoff to be passed to load_point_interp_coeffs_8tile
//      (in place of intmed_offs_{x,y})
//   (3) define delta_{x,y} = local_orig_{x,y} - intmed_offs_{x,y}  . The values returned
//      by do_interp_for_luma need to be increased by this amount (which is generally >0).
//      But this must be done *after* clipping, so:
//      (a) set up clipminx = -delta_x * 64;  clipmaxx = 64*(width-3)-1 - delta_x*64; and likewise
//           for y. Clip the results from do_interp_for_luma to this range. Be prepared for
//           these values being outside +/32K; the values should be saturated to i16 range before
//           use as a clip limit (no effect is needed when out of range).
//      (b) After clipping the x,y results, extract the integer & frac parts as currently, and
//         then add delta_x, delta_y to each integer (x,y) before doing Q6_Vw_vdmpy_VhRh_sat(xyint0, xymul);
//         or -- it's better to find a 32-bit value that can be added to the dmpy result to get the same effect.
//        (i.e. (inbuf_pitch*delta_y + delta_x)). The clipping will ensure that the final 32-bit offset
//       is within the allowable range.
//
// (for Chroma, the delta_x,y should be halved in step 3 so they are in units of Chroma pixels).
//
// An advantage of implementing the above, is that we would no longer need to have intmed_offs
// different from intmed_origin when the source area is fully outside, -  since this change solves
// that problem as well. So intmed_origin could be used when finding the deltas.
//
// do 4:2:0 Chroma warp on up to 8 adjacent tiles;
// the input and output arrays are both u/v interleaved
// So output area is 16*ntiles columns by 8 rows.
static void do_warp_bilinear_8tile_chroma_420(
                                                        warpop_context const &ctx,
                                                        int ntiles,
                                                        int out_x, 
                                                        int out_y, // out_x,out_y are both in tile unit                                                        
                                                        struct intmed_para_t &intmed_para, //per tile proc
                                                        uint8_t *pVtcmVgatherBuf           //per worker thread
                                                        )
{
    static const int NVEC = hvxUtil::NVEC;
    static const bool TWOROW = false;

    // convert from the convert from the frame coordinate to pixel coordinate, when actually used it is -32/-64
    int xoff = 64* intmed_para.intmed_offs_x - (-32);
    int yoff = 64* intmed_para.intmed_offs_y - (-64);

    int32_t const * grid_origin = ctx.grid_array + ctx.grid_rowpitch * out_y + 2*out_x;// points to the origin of current tile
    point_interp_coeffs intco = load_point_interp_coeffs_8tile<TWOROW>( grid_origin, ctx.grid_rowpitch, ntiles, xoff, yoff);

    // xsize/ysize are used for only for clipping. The actual generated points(i.e. offsets) uses the aboved 1*ntile 8x8 coords
    int xsize = intmed_para.intmed_width; 
    int ysize = intmed_para.intmed_height;    
    uint8_t const * inbuf_base = intmed_para.intmed_image;
    
    unsigned inbuf_pitch = ctx.intmed_rowpitch;//the row pitch is same to all buffer

    HVX_Vector *tbufv = (HVX_Vector *)pVtcmVgatherBuf;    
    
    unsigned outbuf_pitch = ctx.out_rowpitch_C;
    uint8_t *outbuf = ctx.output_img_C + 16*out_x + 8*out_y * outbuf_pitch;

    // it is assumed that xsize, ysize are in Luma pixels (and both even)
    //
    int xysize = Q6_R_combine_RlRl(ysize, xsize);
    // find the upper limit of the fractional value, so that none
    // of the 2x2 pixels will be at location >= xsize or ysize.
    // this is 64*(size/2-2)+63   = 32*size - 65

    int xylimit_max = Q6_R_vsubh_RR(xysize*32, 0x00410041);
    // xymul does 2*x + input_pitch*y
    // so that the 4 bytes read by the gather are:
    //   (u0,v0,u1,v1)   and then h interpolate between u0..u1  and v0..v1
    int xymul = Q6_R_combine_RlRl( inbuf_pitch, 2);

    int gather_limit = ((ysize>>1)-1)*inbuf_pitch + (xsize-1);// ysize>>1 is due to the coord is Luma's coord, for Chroma it should be half

    //
    // we need a mask for the vgathers, according to 'ntiles'
    // the mask is 0..ntiles*8-1 and  64..64 + ntiles*8-1
    // for ntiles = 8, it is all lanes.// maximum is 128byte
    // for TWOROW, we want 0..ntiles*8-1, and repeated 4 times
    HVX_VectorPred gather_lanemask = lanemask_for_gather<TWOROW>( ntiles);

    // decide on how to store the results. Each time it stores a 8x8 grid, i.e. 64*2(U+V)=128 pixels. Depends on whether outbuf is 128 byte aligned,
    // whether one vmem can store all 128 pixels, there exists 3 cases:
    //    store_is_aligned = true:
    //          full aligned writes of 128 pixels
    //    store_is_aligned = false, second_vec_store = 0
    //          write (masked) to single vector using ~store_mask_inv, after vror( v, store_ror)
    //    store_is_aligned = false, second_vec_store > 0
    //          write (masked) to 1st vector using ~store_mask_inv, after vror( v, store_ror)
    //          write (masked) to end vector using vsetq(second_vec_store), after vror( v, store_ror)
    //
    bool store_is_aligned = false;
    HVX_VectorPred store_mask_inv;
    int store_ror;
    int second_vec_store=0;
    {
        size_t addr_lo = (size_t)outbuf;        // address of 1st pixel
        size_t addr_hi  = addr_lo + ntiles*16;  // address of last pixel + 1
        store_ror = (NVEC-1) & - (int)addr_lo;
        store_mask_inv = Q6_Q_vsetq_R( (int)addr_lo );  // 'under' mask
        if( ((unsigned)((addr_hi-1)^addr_lo) & NVEC)==0)
        {   // does not cross a vector boundary
            // for ntiles=8, if we are here, it means the operation is full aligned vector
            // and we only need to set 'store_is_aligned'.
            store_is_aligned = (ntiles ==8);
            // (in general) add the 'upper' mask bits
            store_mask_inv = Q6_Q_or_QQn( store_mask_inv, q6op_Q_vsetq2_R(addr_hi));
        }
        else
        {
            // it needs two vec writes; find # of bytes stored in the second.
            // (will be turned into a mask later).
            second_vec_store = addr_hi & ( NVEC-1);
        }
    }

    for( int irow = 0; irow < 4; irow ++ )
    {
        //HVX_Vector out0, out1;

        //
        // we want icol to be bit reversed: 0,2,1,3
        //HVX_Vector interp_res64, interp_res64_prev;
        HVX_Vector interp_res128, interp_res128_prev;

        // each iteration generates 128 pixels:
        //   32 u, 32v in one row
        //   32 u, 32v in the other row
        //
        for( int icol = 0; icol < 2; icol++)
        {
            // get 32 x,y pos
            HVX_Vector xypos0 = intco.do_interp_for_chroma(irow, icol);         // 0,1
            HVX_Vector xypos2 = intco.do_interp_for_chroma(irow, icol+2);       // 2,3

            // clip all to range
            xypos0 = Q6_Vh_vmax_VhVh( xypos0, Q6_V_vzero());
            xypos0 = Q6_Vh_vmin_VhVh( xypos0, Q6_V_vsplat_R(xylimit_max));
            xypos2 = Q6_Vh_vmax_VhVh( xypos2, Q6_V_vzero());
            xypos2 = Q6_Vh_vmin_VhVh( xypos2, Q6_V_vsplat_R(xylimit_max));

            // right shift, to make it integer
            HVX_Vector xyint0 = Q6_Vh_vasr_VhR( xypos0, 6);
            HVX_Vector xyint2 = Q6_Vh_vasr_VhR( xypos2, 6);

            // convert to buffer address
            HVX_Vector gather_base_0 = Q6_Vw_vdmpy_VhRh_sat(xyint0, xymul);
            HVX_Vector gather_base_2 = Q6_Vw_vdmpy_VhRh_sat(xyint2, xymul);
            // issue two gather operation for each
            vgather_w<2>( gather_base_0, inbuf_pitch,  gather_lanemask, inbuf_base, gather_limit, tbufv);
            vgather_w<2>( gather_base_2, inbuf_pitch,  gather_lanemask, inbuf_base, gather_limit, tbufv+2);

            // now, find the 'corner' weights
            // first, find 64-xyfrac
            // combine the low bytes first
            HVX_Vector xyfrac1 = Q6_V_vand_VV( Q6_Vb_vshuffe_VbVb( xypos2, xypos0),	Q6_V_vsplat_R(0x3f3f3f3f));
            // fake dep: don't do the 'vsub' until after all the vgather are done.
            // Compiler doesn't seem to understand a single dependency over 4*128 bytes; so it's done
            // as 4. Unfortunately this requires all four addresses to be put in regs; but the compiler's
            // usage of 'vgather' already forces that.

            /*FAKEDEP_VM( xyfrac1, tbufv[0]);  // with 8.1.03, putting these in gives incorrect results!
            FAKEDEP_VM( xyfrac1, tbufv[1]);
            FAKEDEP_VM( xyfrac1, tbufv[2]);
            FAKEDEP_VM( xyfrac1, tbufv[3]);*/

            HVX_Vector xyfrac0 = Q6_Vb_vsub_VbVb( q6op_Vb_vsplat_R(0x40),xyfrac1 );

            // xyfrac0 is  {x0,x32,y0,y32, x1,x33, y1, y33 ...}
            // xyfrac1 is in the same order {x0', x32' ...
            HVX_Vector xwts01 = Q6_Vh_vshuffe_VhVh(  xyfrac1, xyfrac0  );       // x0, x32', x0', x32' ..
            HVX_Vector ywts1 = Q6_Vh_vshuffo_VhVh(  xyfrac1, xyfrac1  );        // y0', y32', y0', y32' ..
            // now we can do all the 'cross products'; they will range 0..128
            // Do the lower left & lower right, then subtract these from 2*xwts01
            // to get the upper left and upper right. This guarantees that they add up to 128
            // regardless of rounding.

            HVX_VectorPair Dprodh = Q6_Wuh_vmpy_VubVub( xwts01, ywts1); // {DL0, DR0, DL1, DR1...}. { DL32, DR32.. }

            HVX_Vector Dprod = q6op_Vub_vasr_WhR_rnd_sat( Dprodh, 5);   // {DL0, DL32, DR0, DR32... }
            HVX_Vector Uprod = Q6_Vb_vsub_VbVb(  Q6_Vb_vadd_VbVb(xwts01,xwts01), Dprod );  // {UL0, UL32, UR0, UR32... }

            HVX_VectorPair UDprod = Q6_Wb_vshuffoe_VbVb(Dprod,Uprod);   // {UL0, DL0, UR0, DR0 ... } {UL32, DL32, UR32, DR32 ...
            HVX_Vector UDprod0 = Q6_V_lo_W(UDprod); // {UL0, DL0, UR0, DR0 ... //
            HVX_Vector UDprod2 = Q6_V_hi_W(UDprod); // {UL32, DL32, UR32, DR32 ... //

            // read each set of pixels, rearrange bytes into the same order:  {ul, dl, ur, dr }
            // with U in the low vec and V in the high
            // volatile pointer to force compiler to read these in order shown.
            //  // with 8.1.03, putting 'volatile' in gives incorrect results!
            HVX_Vector /*volatile*/ const * voltbuf = tbufv;
            HVX_Vector tpix0 = voltbuf[0];
            HVX_Vector tpix1= voltbuf[1];
            HVX_VectorPair pixels0 = Q6_Wb_vshuffoe_VbVb( tpix1, tpix0);
            HVX_VectorPair pixels2 = Q6_Wb_vshuffoe_VbVb( voltbuf[3], voltbuf[2]);

            // now we can just use Q6_Vuw_vrmpyacc_VuwVubVb to get the SOPs; adding four products within
            // each lane. The bias converts back to 0..255 range.
            //
            HVX_Vector sopU_0 = Q6_Vuw_vrmpy_VubVub( UDprod0, Q6_V_lo_W(pixels0));
            HVX_Vector sopV_0 = Q6_Vuw_vrmpy_VubVub( UDprod0, Q6_V_hi_W(pixels0));
            HVX_Vector sopU_2 = Q6_Vuw_vrmpy_VubVub( UDprod2, Q6_V_lo_W(pixels2));
            HVX_Vector sopV_2 = Q6_Vuw_vrmpy_VubVub( UDprod2, Q6_V_hi_W(pixels2));

            // that's 128 results; pack down to a vector
            interp_res128_prev = interp_res128;
            interp_res128 = Q6_Vub_vsat_VhVh(
                    Q6_Vh_vasr_VwVwR_rnd_sat( sopV_2, sopV_0, 7),
                    Q6_Vh_vasr_VwVwR_rnd_sat( sopU_2, sopU_0, 7));
            // we have U0, V0, U2, V2 U4,V4 ...
        }
        // now reorder into two vectors for output.
        HVX_VectorPair out_row_pair = Q6_W_vshuff_VVR(interp_res128, interp_res128_prev ,-2);


        uint8_t * outbuf_plus_4 = outbuf + 4*outbuf_pitch;
        // write output pixelsq
        if( ! TWOROW)
        {
            if( store_is_aligned)
            {
                *(HVX_Vector *)outbuf = Q6_V_lo_W(out_row_pair);
                *(HVX_Vector *)outbuf_plus_4 = Q6_V_hi_W(out_row_pair);
            }
            else
            {
                HVX_Vector out0 = Q6_V_vror_VR( Q6_V_lo_W(out_row_pair), store_ror );
                HVX_Vector out1 = Q6_V_vror_VR( Q6_V_hi_W(out_row_pair), store_ror );
                q6op_vstcc_QnAV( store_mask_inv, (HVX_Vector *)outbuf , out0 );
                q6op_vstcc_QnAV( store_mask_inv, (HVX_Vector *)outbuf_plus_4 , out1 );
                
                if( second_vec_store > 0 )
                {
                    HVX_VectorPred s2mask = Q6_Q_vsetq_R(second_vec_store);
                    q6op_vstcc_QAV( s2mask, (HVX_Vector *)outbuf  + 1, out0 );
                    q6op_vstcc_QAV( s2mask, (HVX_Vector *)outbuf_plus_4 + 1 , out1 );
                }
            }
        }
        else
        {   // two row output.... write four rows, up to 64 pixels on each.
            HVX_Vector out0 = Q6_V_vror_VR( Q6_V_lo_W(out_row_pair), store_ror );
            HVX_Vector out1 = Q6_V_vror_VR( Q6_V_hi_W(out_row_pair), store_ror );
            HVX_Vector out0x = Q6_V_vror_VR( Q6_V_lo_W(out_row_pair), store_ror + NVEC/2 );
            HVX_Vector out1x = Q6_V_vror_VR( Q6_V_hi_W(out_row_pair), store_ror + NVEC/2 );
            q6op_vstcc_QnAV( store_mask_inv, (HVX_Vector *)outbuf , out0 );
            q6op_vstcc_QnAV( store_mask_inv, (HVX_Vector *)outbuf_plus_4 , out1 );
            q6op_vstcc_QnAV( store_mask_inv, (HVX_Vector *)(outbuf + outbuf_pitch*8) , out0x );
            q6op_vstcc_QnAV( store_mask_inv, (HVX_Vector *)(outbuf_plus_4+ outbuf_pitch*8) , out1x );

            if( second_vec_store > 0 )
            {
                HVX_VectorPred s2mask = Q6_Q_vsetq_R(second_vec_store);
                q6op_vstcc_QAV( s2mask, (HVX_Vector *)outbuf  + 1, out0 );
                q6op_vstcc_QAV( s2mask, (HVX_Vector *)outbuf_plus_4 + 1 , out1 );
                q6op_vstcc_QAV( s2mask, (HVX_Vector *)(outbuf  + outbuf_pitch*8) + 1, out0x );
                q6op_vstcc_QAV( s2mask, (HVX_Vector *)(outbuf_plus_4 + outbuf_pitch*8) + 1 , out1x );
            }
        }
        
        outbuf +=outbuf_pitch;
    }
}

template <bool TWOROW> static void do_warp_bicubic_8tile(
                                                                warpop_context const & ctx,
                                                                int ntiles,
                                                                int out_x, int out_y,
                                                                struct intmed_para_t &intmed_para, //per tile proc
                                                                uint8_t *pVtcmVgatherBuf           //per worker thread
                                                                )
{
    static const int NVEC = hvxUtil::NVEC;

    int xoff = 64* intmed_para.intmed_offs_x - (-96);
    int yoff = 64* intmed_para.intmed_offs_y - (-96);

    int32_t const * grid_origin = ctx.grid_array + ctx.grid_rowpitch * out_y + 2*out_x;
    point_interp_coeffs intco = load_point_interp_coeffs_8tile<TWOROW>(grid_origin, ctx.grid_rowpitch, ntiles, xoff, yoff);

    int xsize = intmed_para.intmed_width;
    int ysize = intmed_para.intmed_height;
    uint8_t const * inbuf_base = intmed_para.intmed_image;
    
    unsigned inbuf_pitch = ctx.intmed_rowpitch;             //the row pitch is same to all buffer
    
    HVX_Vector *tbufv = (HVX_Vector *)pVtcmVgatherBuf;        
    
    unsigned outbuf_pitch = ctx.out_rowpitch_Y;
    uint8_t *outbuf = ctx.output_img_Y + 16*(out_x + out_y*outbuf_pitch);

    int xysize = Q6_R_combine_RlRl(ysize, xsize);
    // find the upper limit of the fractional value, so that none
    // of the 4x4 pixels will be at location >= xsize or ysize.
    // this is 64*(size-3)-1

    int xylimit_max = Q6_R_vsubh_RR(xysize*64, 0x00c100c1);
    int xymul = Q6_R_combine_RlRl( inbuf_pitch, 1);

    int gather_limit = (ysize-1)*inbuf_pitch + (xsize-1);

    //
    // we need a mask for the vgathers, according to 'ntiles'
    // the mask is 0..ntiles*8-1 and  64..64 + ntiles*8-1
    // for ntiles = 8, it is all lanes.
    // for TWOROW, we want 0..ntiles*8-1, and repeated 4 times
    HVX_VectorPred gather_lanemask = lanemask_for_gather<TWOROW>( ntiles);

    // decide on how to store the results.
    // 3 cases:
    //    store_is_aligned = true:
    //          full aligned writes of 128 pixels
    //    store_is_aligned = false, second_vec_store = 0
    //          write (masked) to single vector using ~store_mask_inv, after vror( v, store_ror)
    //    store_is_aligned = false, second_vec_store > 0
    //          write (masked) to 1st vector using ~store_mask_inv, after vror( v, store_ror)
    //          write (masked) to end vector using vsetq(second_vec_store), after vror( v, store_ror)
    //
    bool store_is_aligned = false;
    HVX_VectorPred store_mask_inv;
    int store_ror;
    int second_vec_store=0;
    {
        size_t addr_lo = (size_t)outbuf;        // address of 1st pixel
        size_t addr_hi  = addr_lo + ntiles*16;  // address of last pixel + 1
        store_ror = (NVEC-1) & - (int)addr_lo;
        store_mask_inv = Q6_Q_vsetq_R( (int)addr_lo );  // 'under' mask
        if( ((unsigned)((addr_hi-1)^addr_lo) & NVEC)==0)
        {   // does not cross a vector boundary
            // for ntiles=8, if we are here, it means the operation is full aligned vector
            // and we only need to set 'store_is_aligned'.
            store_is_aligned = (ntiles ==8);
            // (in general) add the 'upper' mask bits
            store_mask_inv = Q6_Q_or_QQn( store_mask_inv, q6op_Q_vsetq2_R(addr_hi));
        }
        else
        {
            // it needs two vec writes; find # of bytes stored in the second.
            // (will be turned into a mask later).
            second_vec_store = addr_hi & ( NVEC-1);
        }
    }

    for( int irow = 0; irow < 8; irow ++ )
    {
        //HVX_Vector out0, out1;

        //
        // we want icol to be bit reversed: 0,4,2,6,1,5,3,7
        // but we want them in pairs, so icol  =0,2,1,3  and we also do icol + 4
        HVX_Vector interp_res64, interp_res64_prev;
        HVX_Vector interp_res128, interp_res128_prev;

        for( int icolv = 0; icolv < 4; icolv++ )
        {
            int icol = Q6_R_brev_R( icolv<<30);                             // 0,2,1,3

            // get 32 x,y pos
            HVX_Vector xypos0 = intco.do_interp_for_luma(irow, icol);       // 0,2,1,3
            HVX_Vector xypos4 = intco.do_interp_for_luma(irow, icol+4);     // 4,6,5,7

            // clip all to range
            xypos0 = Q6_Vh_vmax_VhVh( xypos0, Q6_V_vzero());
            xypos0 = Q6_Vh_vmin_VhVh( xypos0, Q6_V_vsplat_R(xylimit_max));
            xypos4 = Q6_Vh_vmax_VhVh( xypos4, Q6_V_vzero());
            xypos4 = Q6_Vh_vmin_VhVh( xypos4, Q6_V_vsplat_R(xylimit_max));

            // right shift, to make it integer
            HVX_Vector xyint0 = Q6_Vh_vasr_VhR( xypos0, 6);
            HVX_Vector xyint4 = Q6_Vh_vasr_VhR( xypos4, 6);
            // extract fractional part
            // combine the low bytes first
            HVX_Vector xyfrac = Q6_V_vand_VV( Q6_Vb_vshuffe_VbVb( xypos4, xypos0), Q6_V_vsplat_R(0x3f3f3f3f));

            // a series of table lookups will give us 4 sets of 32 x { 4 x u8 } coefficients,
            //   there are four sets because of (x,y) and because of col+0 and col+4
            //

            // convert to buffer address
            HVX_Vector gather_base_0 = Q6_Vw_vdmpy_VhRh_sat(xyint0, xymul);
            HVX_Vector gather_base_4 = Q6_Vw_vdmpy_VhRh_sat(xyint4, xymul);

            // issue four gather operations, for each of the groups...
            vgather_w<4>( gather_base_0, inbuf_pitch,  gather_lanemask, inbuf_base, gather_limit, tbufv);
            vgather_w<4>( gather_base_4, inbuf_pitch,  gather_lanemask, inbuf_base, gather_limit, tbufv+4);

            // fake constraints: ensure compiler can't start calculating coeffs until at least the first set
            // of four vgathers are issued.
             // with 8.1.03, putting these in gives incorrect results!
            //      FAKEDEP_VM( xyfrac, tbufv[0]);
            //      FAKEDEP_VM( xyfrac, tbufv[1]);
            //      FAKEDEP_VM( xyfrac, tbufv[2]);
            //      FAKEDEP_VM( xyfrac, tbufv[3]);

            HVX_Vector_x4 coeffs = find_cubic_coeffs( xyfrac);

            // find partial result for the first set (one result per 32-bit lane)
            HVX_Vector interp_res0 = cubic_interp_partial( coeffs.val[0], coeffs.val[1], &tbufv[0]);

            // second set
            HVX_Vector interp_res4 = cubic_interp_partial( coeffs.val[2], coeffs.val[3], &tbufv[4]);

            // combine them to 64 pixels, in h lanes of one vector
            interp_res64_prev= interp_res64;
            interp_res64 = Q6_Vh_vasr_VwVwR( interp_res4, interp_res0, 7 );
            if( icol==2)
            {
                interp_res128 = Q6_Vub_vsat_VhVh( interp_res64, interp_res64_prev);
                // we now have 128 final pixels, which are the pixels for every even( icol==2) or odd (icol==3)
                // column, on two different output rows. One more 'combine' to make the two rows.
            }
        }
        interp_res128_prev = interp_res128;
        interp_res128 = Q6_Vub_vsat_VhVh( interp_res64, interp_res64_prev);
        // based on ordering from load_point_interp_coeffs:
        //   interp_res128_prev has 128 pixels, (A0 A2 A4 ... A126 B0 B2 .. B126)
        //   interp_res128      has 128 pixels, (A1 A3 A5 ... A127 B1 B3 .. B127)
        // A full 'shuffle' gives us (A0 .. A127) and (B0..B127)
        //
        HVX_VectorPair out_row_pair = Q6_W_vshuff_VVR(interp_res128, interp_res128_prev ,-1);

        uint8_t * outbuf_plus_8 = outbuf + 8*outbuf_pitch;
        // write output pixels
        if( ! TWOROW)
        {
            if( store_is_aligned)
            {
                *(HVX_Vector *)outbuf = Q6_V_lo_W(out_row_pair);
                *(HVX_Vector *)outbuf_plus_8 = Q6_V_hi_W(out_row_pair);
            }
            else
            {
                HVX_Vector out0 = Q6_V_vror_VR( Q6_V_lo_W(out_row_pair), store_ror );
                HVX_Vector out1 = Q6_V_vror_VR( Q6_V_hi_W(out_row_pair), store_ror );
                q6op_vstcc_QnAV( store_mask_inv, (HVX_Vector *)outbuf , out0 );
                q6op_vstcc_QnAV( store_mask_inv, (HVX_Vector *)outbuf_plus_8 , out1 );
                if( second_vec_store > 0 )
                {
                    HVX_VectorPred s2mask = Q6_Q_vsetq_R(second_vec_store);
                    q6op_vstcc_QAV( s2mask, (HVX_Vector *)outbuf  + 1, out0 );
                    q6op_vstcc_QAV( s2mask, (HVX_Vector *)outbuf_plus_8 + 1 , out1 );
                }
            }
        }
        else
        {   // two row output.... write four rows, up to 64 pixels on each.
            HVX_Vector out0 = Q6_V_vror_VR( Q6_V_lo_W(out_row_pair), store_ror );
            HVX_Vector out1 = Q6_V_vror_VR( Q6_V_hi_W(out_row_pair), store_ror );
            HVX_Vector out0x = Q6_V_vror_VR( Q6_V_lo_W(out_row_pair), store_ror + NVEC/2 );
            HVX_Vector out1x = Q6_V_vror_VR( Q6_V_hi_W(out_row_pair), store_ror + NVEC/2 );
            q6op_vstcc_QnAV( store_mask_inv, (HVX_Vector *)outbuf , out0 );
            q6op_vstcc_QnAV( store_mask_inv, (HVX_Vector *)outbuf_plus_8 , out1 );
            q6op_vstcc_QnAV( store_mask_inv, (HVX_Vector *)(outbuf + outbuf_pitch*16) , out0x );
            q6op_vstcc_QnAV( store_mask_inv, (HVX_Vector *)(outbuf_plus_8+ outbuf_pitch*16) , out1x );
            
            if( second_vec_store > 0 )
            {
                HVX_VectorPred s2mask = Q6_Q_vsetq_R(second_vec_store);
                q6op_vstcc_QAV( s2mask, (HVX_Vector *)outbuf  + 1, out0 );
                q6op_vstcc_QAV( s2mask, (HVX_Vector *)outbuf_plus_8 + 1 , out1 );
                q6op_vstcc_QAV( s2mask, (HVX_Vector *)(outbuf  + outbuf_pitch*16) + 1, out0x );
                q6op_vstcc_QAV( s2mask, (HVX_Vector *)(outbuf_plus_8 + outbuf_pitch*16) + 1 , out1x );
            }
        }
        outbuf +=outbuf_pitch;
    }
}

template <bool ISLUMA> static void warp_callback(void* data)
{
    warp_callback_t    *dptr = (warp_callback_t*)data;
    
    unsigned id = dspCV_atomic_inc_return(&(dptr->workerCount)) - 1; // current worker thread ID
    unsigned int jobNum=dptr->jobNum;   

    // the common parameters for all worker threads are stored in pCtx    
    warpop_context *pCtx=dptr->pCtx;

    // the followings are the per worker thread parameters
    uint8 *pVtcmVgatherBuf=dptr->vtcmVgatherBuf[id];    
    struct tilesInfo_t *pTileProcInfo=dptr->pTileProcInfo; 
    struct intmed_para_t *pIntmedPara=dptr->pIntmedPara;
       
    int i;    
    while((i = dspCV_atomic_inc_return(&(dptr->jobCount)) - 1) < jobNum)
    {           
        // the parameters associated with index i are per tile proc, so need to be indicated by i
        if(ISLUMA)
        {
            do_warp_bicubic_8tile<false>(*pCtx, pTileProcInfo[i].sz, pTileProcInfo[i].x, pTileProcInfo[i].y, pIntmedPara[i], pVtcmVgatherBuf);
        }
        else
        {
            do_warp_bilinear_8tile_chroma_420(*pCtx, pTileProcInfo[i].sz, pTileProcInfo[i].x, pTileProcInfo[i].y, pIntmedPara[i], pVtcmVgatherBuf);
        }
    }    

    // release multi-threading job token
    dspCV_worker_pool_synctoken_jobdone(dptr->token);    
}


// given a range of cells in the grid:
//  (1) find a bounding box, of the fractional grid points;
//  (2) round these out to integer pixel coords meeting alignment criteria
//     (cols multiple of 4; rows even). 
//  (3) if this extends beyond the extent of the input image(only in width direction), then trim the ranges
//      as needed to make it fit.
//  (4) if the resulting area is at least 4 pixels in width and height, we are done; otherwise
//     we enlarge it as needed (on the side which is not clipped) to  accomplish this
//
//  The result is a bounding box on the input, and a pixel 'origin' which is normally the
//  same as the upper left corner of the bounding box. If the source area is too far outside the
//  image (as detected in step (4), then the origin may be outside the image. This is done to make sure
//  the origin does not stray too far from the original bounding box (as found in (2)), so that when the
//  origin is subtracted from the pixel coords, the result will fit in 16 bits. 
//  Offset is abstracted and used as e.g [400:517] to 400+[0:127], reduce the number of offset bits required
//  for vgather, currently it is -512~512.
//  For instance, suppose you have an input 720 pixels wide, and the X bounding box from (2) is calculated
//    to range from [-600 .. -440). In step (3), the range will be corrected to  [0..-440), and then
//    in step (4) this is corrected to [0..4), but the X origin will be left at  something  close to -440,
//    this produces the same effect (everything clipped off the left edge) but the differences between
//    this value and the corners of the mesh bounding box are limited to about -160 pixels, so there will
//    be no overflow when these are converted to 16 bits (with range +/-512 pixes).
// The result produced by this calculation is usable for Luma or Chroma; if your Chroma is 4:2:0, you
// need to divide the y coords by  2  to get actual Chroma extent (and the numbers will always be even).
//
// returned values:
//    {minY, maxY}
//       - the minimum and maximum of the y coordinate of the current tile group
void findBBoxForTileGroup(warpop_context & ctx,
                                   int tilex, 
                                   int tiley,
                                   int tileSz,
                                   int &minY,
                                   int &maxY
                                  )                                     
{
    HVX_WARP_ASSERT( tilex >=0 && tiley >=0 && tileSz>0);

    // find bbox from grid
    int minx, maxx, miny, maxy;

    {   
        int rowpit = ctx.grid_rowpitch;
        int32_t const * srcp = ctx.grid_array + 2*tilex + rowpit * tiley;        
        int tile_cols = ctx.grid_width-1; // the max of actual number of tile rows in the grid        
        
        {
            int32_t const * srcp0;
            int32_t const * srcp1;
            minx = maxx=  srcp[0];
            miny = maxy  = srcp[1];

            // part 1: the first fractional row if any                                        
            if(tilex!=0)
            {                   
                int tile_wid_1st=tile_cols-tilex;
                tile_wid_1st=std::min(tile_wid_1st, tileSz);
                
                srcp0 = srcp;
                srcp1 = srcp+rowpit;

                for(int j=0; j<=tile_wid_1st; j++)
                {
                    int32_t x0 = srcp0[2*j];
                    int32_t y0 = srcp0[2*j+1];       
                    int32_t x1 = srcp1[2*j];
                    int32_t y1 = srcp1[2*j+1];
                    
                    minx = std::min( minx, (int)std::min( x0,x1));
                    miny = std::min( miny, (int)std::min( y0,y1));
                    maxx = std::max( maxx, (int)std::max( x0,x1));
                    maxy = std::max( maxy, (int)std::max( y0,y1));                    
                }

                // the next code block will use the updated tilex, tiley, tileSz
                tilex = 0;
                tiley++;
                tileSz -=tile_wid_1st;
            }
            
            // part 2: multiple rows if any, not including the last fractional row
            int tile_ht;
            if(tileSz>=tile_cols)
            {
                tile_ht=tileSz/tile_cols;

                srcp0 = ctx.grid_array+2*tilex + rowpit * tiley;
                srcp1 = ctx.grid_array+2*tilex + rowpit * tiley+rowpit*tile_ht;
                
                for( int j = 0; j <= tile_cols; j++)// check both the upper and lower edges
                {
                    int32_t x0 = srcp0[2*j];
                    int32_t y0 = srcp0[2*j+1];
                    int32_t x1 = srcp1[2*j];
                    int32_t y1 = srcp1[2*j+1];
                    minx = std::min( minx, (int)std::min( x0,x1));
                    miny = std::min( miny, (int)std::min( y0,y1));
                    maxx = std::max( maxx, (int)std::max( x0,x1));
                    maxy = std::max( maxy, (int)std::max( y0,y1));
                }
                
                // now do the left, right, if any // only need to be done when tile_ht>1
                srcp1 = srcp0 + 2*tile_cols;
                for( int i =1; i < tile_ht; i++)
                {
                    srcp0 += rowpit;
                    srcp1 += rowpit;
                    int32_t x0 = srcp0[0];
                    int32_t y0 = srcp0[1];
                    int32_t x1 = srcp1[0];
                    int32_t y1 = srcp1[1];
                    minx = std::min( minx, (int)std::min( x0,x1));
                    miny = std::min( miny, (int)std::min( y0,y1));
                    maxx = std::max( maxx, (int)std::max( x0,x1));
                    maxy = std::max( maxy, (int)std::max( y0,y1));
                }         

                tilex = 0;
                tiley+=tile_ht;
                tileSz -=tile_ht*tile_cols;   
            }

            // part3: the last fractional row if any
            if(tileSz>0)
            {   
                srcp0 = ctx.grid_array+2*tilex + rowpit * tiley;
                srcp1 = ctx.grid_array+2*tilex + rowpit * (tiley+1);
                
                for(int j=0; j<=tileSz; j++)
                {                         
                    int32_t x0 = srcp0[2*j];
                    int32_t y0 = srcp0[2*j+1];
                    int32_t x1 = srcp1[2*j];
                    int32_t y1 = srcp1[2*j+1];
                    minx = std::min( minx, (int)std::min( x0,x1));
                    miny = std::min( miny, (int)std::min( y0,y1));
                    maxx = std::max( maxx, (int)std::max( x0,x1));
                    maxy = std::max( maxy, (int)std::max( y0,y1));
                }                
            }                                  
        }
    }

    // convert them all to pixels, rounding down & up
    // (include enough to cover Luma and Chroma)
    minx =  ((minx-96)>>8)*4;           // minx must be multiple of 4
    maxx =  ((maxx+32+4*64)>>7)*2;      // maxx must be even

    miny = ((miny-96)>>7)*2;            // miny must be a multiple of 2
    maxy = ((maxy+4*64)>>7)*2;          // maxy must be even.

    // get input dims; if there is a border,
    // increase input dims to include border.
    // some calculations are compensated for left & top border.
    int input_wid = ctx.input_width;
    int input_ht = ctx.input_height;
    uint32_t border = ctx.border_widths;
    int border_left = 0;
    int border_top = 0;

    if( border != 0)
    {
        int border_right, border_bottom;
        border_left = border & 0xFF;
        border_right = (border>>8) & 0xFF;
        border_top = (border>>16) & 0xFF;
        border_bottom = (border>>24) & 0xFF;
        HVX_WARP_ASSERT( ((border_left|border_right)&3)==0);
        HVX_WARP_ASSERT( ((border_top|border_bottom)&1)==0);
        // increase input size to include border
        input_wid += border_left + border_right;
        input_ht += border_top + border_bottom;

        minx += border_left;    // adjust bounding box
        maxx += border_left;
        miny += border_top;
        maxy += border_top;
    }     // temp storage (e.g. for vgather) in TCM. 1024 bytes, vec aligned.
    
    miny = std::max( miny, 0);
    maxy = std::min( maxy, input_ht );

    minY=miny;
    maxY=maxy;
       
    return;              
}

/*
    calcuate the parameters for each tile proc
*/
void findBBoxForTileGroup(warpop_context & ctx,
                                    int tilex, int tiley,               // origin in grid tile units
                                    int tile_wid, int tile_ht,          // size of the region in grid tiles
                                    intmed_para_t *pIntmedPara)
{
    HVX_WARP_ASSERT( tilex >=0 && tile_wid >= 1 && tilex + tile_wid < ctx.grid_width);
    HVX_WARP_ASSERT( tiley >=0 && tile_ht >= 1 && tiley + tile_ht < ctx.grid_height);

    // find bbox from grid
    int minx, maxx, miny, maxy;

    {
        int32_t const * srcp = ctx.grid_array + 2*tilex + ctx.grid_rowpitch * tiley;
        int rowpit = ctx.grid_rowpitch;

        if(  (((unsigned)(size_t)srcp | (unsigned)rowpit*sizeof(int32_t)) & 7 )== 0 )
        {
            // can do with int64 ops
            int64_t const * srcp0 = (int64_t const *)srcp;
            int64_t const * srcp1 = (int64_t const *)(srcp + rowpit*tile_ht);
            int64_t minxy = *srcp0;
            int64_t maxxy = minxy;
            int rp64 = rowpit >> 1; // rp64=rowpit for int64, originally rowpit is for int32

            for( int j = 0; j <= tile_wid; j++)
            {
                int64_t xy0 = srcp0[j];
                int64_t xy1 = srcp1[j];
                minxy = Q6_P_vminw_PP( minxy, Q6_P_vminw_PP(xy0,xy1));
                maxxy = Q6_P_vmaxw_PP( maxxy, Q6_P_vmaxw_PP(xy0,xy1));
            }
            
            // now do the left, right, if any
            if( tile_ht >1)
            {
                srcp1 = (int64_t const *)(srcp + 2*tile_wid);
                for( int i =1; i < tile_ht; i++)
                {
                    srcp0 += rp64;
                    srcp1 += rp64;
                    int64_t xy0 = srcp0[0];
                    int64_t xy1 = srcp1[0];
                    minxy = Q6_P_vminw_PP( minxy, Q6_P_vminw_PP(xy0,xy1));
                    maxxy = Q6_P_vmaxw_PP( maxxy, Q6_P_vmaxw_PP(xy0,xy1));
                }
            }
            
            minx = (int32_t) minxy;
            miny = (int32_t) (minxy>>32);
            maxx = (int32_t) maxxy;
            maxy = (int32_t) (maxxy>>32);
        }
        else
        {
            int32_t const * srcp0 = srcp;
            int32_t const * srcp1 = srcp + rowpit*tile_ht;
            minx = maxx=  srcp[0];
            miny = maxy  = srcp[1];

            for( int j = 0; j <= tile_wid; j++)
            {
                int32_t x0 = srcp0[2*j];
                int32_t y0 = srcp0[2*j+1];
                int32_t x1 = srcp1[2*j];
                int32_t y1 = srcp1[2*j+1];
                minx = std::min( minx, (int)std::min( x0,x1));
                miny = std::min( miny, (int)std::min( y0,y1));
                maxx = std::max( maxx, (int)std::max( x0,x1));
                maxy = std::max( maxy, (int)std::max( y0,y1));
            }
            
            // now do the left, right, if any
            srcp1 = srcp + 2*tile_wid;
            for( int i =1; i < tile_ht; i++)
            {
                srcp0 += rowpit;
                srcp1 += rowpit;
                int32_t x0 = srcp0[0];
                int32_t y0 = srcp0[1];
                int32_t x1 = srcp1[0];
                int32_t y1 = srcp1[1];
                minx = std::min( minx, (int)std::min( x0,x1));
                miny = std::min( miny, (int)std::min( y0,y1));
                maxx = std::max( maxx, (int)std::max( x0,x1));
                maxy = std::max( maxy, (int)std::max( y0,y1));
            }
        }
    }

    // convert them all to pixels, rounding down & up
    // (include enough to cover Luma and Chroma)
    minx =  ((minx-96)>>8)*4;           // minx must be multiple of 4
    maxx =  ((maxx+32+4*64)>>7)*2;      // maxx must be even

    miny = ((miny-96)>>7)*2;            // miny must be a multiple of 2
    maxy = ((maxy+4*64)>>7)*2;          // maxy must be even.

    // get input dims; if there is a border,
    // increase input dims to include border.
    // some calculations are compensated for left & top border.
    int input_wid = ctx.input_width;
    int input_ht = ctx.input_height;
    uint32_t border = ctx.border_widths;
    int border_left = 0;
    int border_top = 0;

    if( border != 0)
    {
        int border_right, border_bottom;
        border_left = border & 0xFF;
        border_right = (border>>8) & 0xFF;
        border_top = (border>>16) & 0xFF;
        border_bottom = (border>>24) & 0xFF;
        HVX_WARP_ASSERT( ((border_left|border_right)&3)==0);
        HVX_WARP_ASSERT( ((border_top|border_bottom)&1)==0);
        // increase input size to include border
        input_wid += border_left + border_right;
        input_ht += border_top + border_bottom;

        minx += border_left;    // adjust bounding box
        maxx += border_left;
        miny += border_top;
        maxy += border_top;
    }     // temp storage (e.g. for vgather) in TCM. 1024 bytes, vec aligned.

    // clip region to image boundaries

    int lo_x = std::max( minx, 0);
    int hi_x = std::min( maxx, input_wid);
    int lo_y = std::max( miny, 0);
    int hi_y = std::min( maxy, input_ht );

    int offs_x = lo_x;
    int offs_y = lo_y;

    // resulting region must be >= 4 in both directions;
    // compensate if not.
    if( hi_x < lo_x+4 )
    {                           // degenerate in x
        if(  lo_x == 0)
        {                       // was trimmed on the left...
            hi_x = 4;
            if( maxx < -4) 
            {
                offs_x = maxx+4; // else let it stand at 0
            }
        }
        else
        {
            lo_x = (hi_x-4)&~3; // use multiple of 4 with at least 4 to right
            if( offs_x <= input_wid )
            {
                offs_x = lo_x;
            }
        }
    }

    if( hi_y < lo_y+4 )
    {                           // degenerate in y
        if(  lo_y == 0)
        {   
                                // was trimmed on the top...
            hi_y = 4;
            if( maxy < -4)
            {
                offs_y = maxy+4; // else let it stand at 0
             }
        }
        else
        {
            lo_y = (hi_y-4)&~1;     // use multiple of 2 with at least 4 below
            if( offs_y <= input_ht)
            {
                offs_y = lo_y;
            }
        }
    }

    // the tile group info refers to the output domain, the four fields are only used in processTileGroupLuma/processTileGroupChroma420
    // the four fields: tilegroup_x, tilegroup_y, tilegroup_width, tilegroup_height are not necessary    
    // the intermediate image info, refers to the src image domain
    pIntmedPara->intmed_orig_x = lo_x;
    pIntmedPara->intmed_orig_y = lo_y;
    pIntmedPara->intmed_width = hi_x - lo_x;
    pIntmedPara->intmed_height = hi_y - lo_y;
    pIntmedPara->intmed_offs_x = offs_x - border_left;
    pIntmedPara->intmed_offs_y = offs_y - border_top;
}

// Divide a tile group into multiple tile proc units, which are of 1*8 size.
// When it comes to the boundary, tile proc might be cut to prevent cross the boundary.
int tileGrp2tileProc(warpop_context & ctx,
                          struct tilesInfo_t &tileGrp,       
                          int tile_cols,                     
                          struct tilesInfo_t *pTileProc,     
                          intmed_para_t *pIntmedPara,
                          int &tileProcNum
                        )
{
    int tileSzAcc=0;
    const int stride=tile_cols;
    int tileProcNumAcc=0;   
    const int numTilePerProc=8;
   
    int x=tileGrp.x;
    int y=tileGrp.y;
    
    while(1)
    {       
        int tileSz=std::min(stride-x,numTilePerProc);
        tileSz=std::min(tileSz, tileGrp.sz-tileSzAcc);
        
        pTileProc->x=x;
        pTileProc->y=y;
        pTileProc->sz=tileSz;            
        pTileProc++;

        if(tileProcNumAcc>MAX_TILE_PROC_SZ-1)
        {   
            // The maximum number of tile proc is currently estimated by MAX_TILE_PROC_SZ.
            // Generally it can only be traversed to get the precise number.
            return AEE_ENOMEMORY;
        }

        const int tile_ht=1;
        findBBoxForTileGroup(ctx,
                             x, y,              
                             tileSz,                    // size of the region in grid tiles
                             tile_ht,          
                             pIntmedPara+tileProcNumAcc);           
        
        tileProcNumAcc++;
                    
        // update for the next round of loop use
        tileSzAcc +=tileSz;
        x +=tileSz;

        if(tileSzAcc==tileGrp.sz)
        {
            break;
        }
         
        if(x==stride)// then move to the next row
        {
            x=0;
            y++;
        }                                               
    }           
    
    tileProcNum=tileProcNumAcc;
    
    return AEE_SUCCESS;
}

/*===========================================================================
    GLOBAL FUNCTION
===========================================================================*/
AEEResult benchmark_warp(
                            remote_handle64 handle,
                            const uint8* src, 
                            int inpLen, 
                            uint8* dst, 
                            int outpLen, 
                            int32 width,            
                            int32 height,           
                            int32 srcStride,
                            int32 dstStride,
                            const uint8* gridarr,   
                            int   gridarrLen, 
                            int32 gridStride,
                            int32 LOOPS,
                            int32 wakeupOnly,
                            int32 useComputRes,
                            int32* dspUsec,
                            int32* dspCyc
                            )                          
{
    *dspUsec = 0, *dspCyc = 0;
    if (wakeupOnly)
    {
        return AEE_SUCCESS;
    }

// only supporting HVX version in this example.
#if (__HEXAGON_ARCH__ < 60)
    return AEE_EUNSUPPORTED;
#endif

    // record start time (in both microseconds and pcycles) for profiling
#ifdef PROFILING_ON
    uint64 startTime = HAP_perf_get_time_us();
    uint64 startCycles = HAP_perf_get_pcycles();
#endif    

    if(!(src && dst && (width>=32)&&(height>=32)&&(srcStride%4==0)
             && (width%2==0&&height%2==0)
         )
      )
    {
        return AEE_EBADPARM;
    }
    
    const int meshW=int((width+15)/16)+1;
    const int meshH=int((height+15)/16)+1;

    const int outW = width;
    const int outH = height;// for Luma, it is outH. for Chroma, it is outH/2.

    const int tile_rows = meshH-1;
    const int tile_cols = meshW-1;		       
			   
    for (int loops = 0; loops < LOOPS; loops++)
    {
        // VTCM layout: first 2KiB*4 is allocate for warp_tmp, which is used as the results of vgather.
        //              Then the sequent 248KiB is allocate for the intermediate buffer.
		unsigned int avail_block_size;
        unsigned int max_page_size;
        unsigned int num_pages;
        int vtcmSz;
        if(0==HAP_query_avail_VTCM(&avail_block_size, &max_page_size, &num_pages))
        {
            if(max_page_size<MIN_VTCM_SZ)
            {
                FARF(ERROR,"Available VTCM size less than %d KiB, aborting...", MIN_VTCM_SZ/1024);
                return AEE_ENOMEMORY;                
            }
            
            vtcmSz=max_page_size;
        }
        else
        {
            FARF(ERROR, "Fail to call HAP_query_avail_VTCM,  aborting...");
            return AEE_ENOMEMORY;
        }

        compute_res_attr_t compute_res;
        unsigned int context_id = 0;
        uint8_t *pVtcm=NULL;
        
        if(useComputRes&&compute_resource_attr_init)
        {
            compute_resource_attr_init(&compute_res);
            compute_resource_attr_set_serialize(&compute_res, 1);
            compute_resource_attr_set_vtcm_param(&compute_res, vtcmSz, 0);
            
            context_id=compute_resource_acquire(&compute_res, 100000); // wait till 100ms
            
            if(context_id==0)
            {
                return AEE_ERESOURCENOTFOUND;
            }            
            
            pVtcm=(uint8_t *)compute_resource_attr_get_vtcm_ptr(&compute_res);
        }
        else
        {        
               if(useComputRes&&(!compute_resource_attr_init))  
               {
                    FARF(HIGH, "Compute resource APIs not supported. Use legacy methods instead.");
               }
               
               pVtcm=(uint8_t *)HAP_request_VTCM(vtcmSz, 1);
                  
               if (!pVtcm)
               {
                   FARF(ERROR,"Could not allocate VTCM, aborting...");
                   return AEE_ENOMEMORY;
               }  
         }

        char *pTileProcBuf=(char *)malloc(MAX_TILE_PROC_SZ*(sizeof(struct tilesInfo_t)+sizeof(struct intmed_para_t))); // 20KiB buffer        
                
        if(pTileProcBuf==NULL)
        {
            FARF(ERROR,"Could not allocate pTileProcBuf, aborting...");
            HAP_release_VTCM((void*)pVtcm);
            return AEE_ENOMEMORY;            
        }
        
        struct tilesInfo_t *pTileProcInfo=(struct tilesInfo_t *)pTileProcBuf;
        struct intmed_para_t *pIntmedPara=(struct intmed_para_t *)(pTileProcBuf+MAX_TILE_PROC_SZ*sizeof(struct tilesInfo_t));
    
         warpop_context wctx;
                      
         wctx.out_width = outW;             
         wctx.grid_width = meshW;
         wctx.grid_height = meshH;
         wctx.grid_array = (int32_t const *)gridarr;
         wctx.grid_rowpitch = gridStride*2;         // in int32's unit   
         wctx.input_width = width;
         wctx.border_widths = 0;    
         wctx.input_height = height; // For both Luma and Chroma, always use the height of Luma
         wctx.intmed_rowpitch=srcStride;   

         wctx.output_img_Y = dst;
         wctx.out_rowpitch_Y = dstStride;
         
         wctx.output_img_C = dst+dstStride*outH; 
         wctx.out_rowpitch_C = dstStride;
         
         // If i=0, then process Luma. If i=1, then process Chroma. Also use this to assign different parameters.         
         for(int i=0; i<2; i++)
         {  
            // For Luma, vertRatio=1. For Chroma, vertRatio=2. Since the height of the image is half of the Luma's
            int vertRatio; 
            
            if(i==0)
            {                 
                wctx.out_height = outH;
                vertRatio=1;              
            }
            else
            {
                wctx.out_height = outH/2;    
                vertRatio=2;                
            }

             const int vtcmBufWid=srcStride;     // assumed to be equal to the width of src image for simplicity reason.   
             const int vtcmBufHgtMax=(vtcmSz-VTCM_VGATHER_TOTAL_SZ)/vtcmBufWid;
             const int numTileTotal=tile_rows*tile_cols;
             
             // initial value, iterate in the while loop
             int tileGrpStartIdx=0; // value 0~(numTileTotal-1), use this to traverse all the tiles                                
             int tileGrpSz=numTileTotal;
             
             // the following four variables are used to record quantities when last successfully fit into the VTCM buffer
             int tileGrpSzLast;
             int minVtcmYLast;
             int maxVtcmYLast;
             int vtcmBufHgtLast;
             
             // set in the while loop
             int minVtcmY;
             int maxVtcmY;
                                           
             while(tileGrpStartIdx<numTileTotal) // traverse all tiles until there is no tiles left
             {                             
                 // use the bisection approach, starting from the maximum size
                 int tileGrpSrchLeft=tileGrpStartIdx;              
                 int tileGrpEndIdx=tileGrpStartIdx+tileGrpSz-1;
                 int tileGrpSrchRght=tileGrpEndIdx;              
             
                 // derived from tileGrpStartIdx, used outside of while(1) loop
                 const int tilex=tileGrpStartIdx%tile_cols;
                 const int tiley=tileGrpStartIdx/tile_cols;
             
                 int vtcmBufHgt; // the actual VTCM buffer height, <=vtcmBufHgtMax
             
                 /*
                  For simplicity reason, the tiles to be searched are restricted to first/last fractional rows+rectangular. 
                  input: the (x,y) coordinate of the starting tile, the total number of available tiles, tile grid info.
                  output: the minimum Y and maximum Y coordinate required by the tile group. The judgement is performed outside
                          of the function.
                 */                                                               
                 while(1)
                 {                    
                     findBBoxForTileGroup(wctx,
                                          tilex,
                                          tiley,
                                          tileGrpSz,
                                          minVtcmY, 
                                          maxVtcmY);
                                          
                     vtcmBufHgt=maxVtcmY/vertRatio-minVtcmY/vertRatio+1;                                                                          
                     
                     if(vtcmBufHgt<=vtcmBufHgtMax)
                     {
                         // if the current tile group can fit into the VTCM buffer, then try to increase the tile group size.
                         int step=tileGrpSrchRght-tileGrpEndIdx;  
             
                         tileGrpSzLast = tileGrpSz;
                         minVtcmYLast=minVtcmY;
                         maxVtcmYLast=maxVtcmY;
                         vtcmBufHgtLast=vtcmBufHgt;                                         
                         
                         if(step==0)
                         {
                             break;
                         }
                         else if(step==1)
                         {
                             tileGrpSrchLeft=tileGrpSrchRght;
                             tileGrpEndIdx=tileGrpSrchRght;
                         }
                         else
                         {
                             step/=2;
                             tileGrpSrchLeft=tileGrpEndIdx+1;
                             tileGrpEndIdx+=step;
                         }                                                                                 
                     }
                     else
                     {
                         int step=tileGrpEndIdx-tileGrpSrchLeft;                    
             
                         if(step==0)
                         {
                             break;
                         }
                         else if(step==1)
                         {
                             tileGrpSrchRght=tileGrpSrchLeft;
                             tileGrpEndIdx=tileGrpSrchLeft;                        
                         }
                         else
                         {
                             step/=2;
                             tileGrpSrchRght=tileGrpEndIdx-1;
                             tileGrpEndIdx-=step; 
                         }
                     }
             
                     // update for the next round of loop use
                     tileGrpSz=tileGrpEndIdx-tileGrpStartIdx+1;
                 }
                 
                 if(tileGrpEndIdx<=tileGrpStartIdx)
                 {
                     // if run here, means even the smallest tile size(like 1x1) cannot be fitted into the VTCM, then fail
                     FARF(ERROR,"Error found in tile group search: the smallest tile group i.e. 1x1 cannot be fitted into the VTCM, aborting...");
                     
                     HAP_release_VTCM((void*)pVtcm);
                     
                     if(pTileProcBuf)
                     {
                        free(pTileProcBuf);
                     }                     
                     
                     return AEE_EBADPARM;               
                 }
                 else
                 {
                     tileGrpSz=tileGrpSzLast;
                     minVtcmY=minVtcmYLast;
                     maxVtcmY=maxVtcmYLast;      

                     if(maxVtcmY>=height)
                     {
                        maxVtcmY -=vertRatio;
                     }
                     vtcmBufHgt=maxVtcmY/vertRatio-minVtcmY/vertRatio+1; 
                 }
                 
                 // if run here, it means successfully find one tile group, assign its parameters            
                 struct tilesInfo_t tileGrpInfo;
                 tileGrpInfo.x=tilex;
                 tileGrpInfo.y=tiley;
                 tileGrpInfo.sz=tileGrpSz;                 

                 // Copy the related src image into the VTCM. Since the region is assumed to be of the same width of src image, multiple rows need
                 // to be copied. Part 1: L2 Prefetch.
                 const int srcOffset=(0==i)?:srcStride*height; // For Chroma, need to skip the Luma part.
                 HVX_Vector *vSrc=(HVX_Vector*)(src+srcOffset+vtcmBufWid*minVtcmY/vertRatio); 
                 uint64_t L2FETCH_REGISTER = (1ULL <<48) | ((uint64_t)vtcmBufWid<<32) | ((uint64_t)width<<16) | vtcmBufHgt;
                 L2fetch((uint32_t)vSrc, L2FETCH_REGISTER);
                                  
                 // divide the tile group into multiple tile process unit, each call of do_warp_bilinear_8tile_chroma_420/do_warp_bicubic_8tile  
                 // processes one tile process unit.
                 int tileProcNum;
                 int retVal=tileGrp2tileProc(wctx, tileGrpInfo, tile_cols, pTileProcInfo, pIntmedPara, tileProcNum);
                          
                 if(retVal!=AEE_SUCCESS)
                 {   
                     FARF(ERROR,"Error found in tileGrp2tileProc(): the number of tile proc exceeds the pre-allocated buffer, aborting...");
                     
                     HAP_release_VTCM((void*)pVtcm);
                     
                     if(pTileProcBuf)
                     {
                        free(pTileProcBuf);
                     }                     
                     
                     return retVal;                                                               
                 }
                 
                 // adjust the last tile proc to align with the reference C++ codes.
                 // Need to eliminate the last tile proc if it is not the last one in the row and not 8. Leave it in the next round of processing.
                 int lastTileProcSz=pTileProcInfo[tileProcNum-1].sz;
                 if( (tileProcNum>1)&&(lastTileProcSz!=8)&&((pTileProcInfo[tileProcNum-1].x+lastTileProcSz)!=tile_cols))
                 {
                     tileGrpSz -=lastTileProcSz;
                     tileGrpInfo.sz=tileGrpSz;
                     tileProcNum--;
                 }
                            
                 int vtcmBufSz=vtcmBufWid*vtcmBufHgt;                        
             

                 // Copy the related src image into the VTCM. Part 2: vmem.                 
                 HVX_Vector *vDst = (HVX_Vector*)(pVtcm+VTCM_VGATHER_TOTAL_SZ);// pointer to the start address of the buffer within VTCM     
                 for(int j = 0; j < vtcmBufSz/VLEN; j++)       
                 {
                     *vDst++ = *vSrc++;
                 }
                 
                 int vtcmBufSzMod128=vtcmBufSz%VLEN;
                 if(vtcmBufSzMod128)
                 {
                     uint8 *pSrc=(uint8 *)vSrc;
                     uint8 *pDst=(uint8 *)vDst;
                     
                     for(int j=0; j<vtcmBufSzMod128; j++)
                     {
                         *pDst++=*pSrc++;
                     }                
                 }
                 
                 // init the parameters, can be divided into three classes: 1) multithread specific: worker pool, sync token
                 //                                                         2) global parameters for the current tile group             
                 //                                                         3) parameters for each tile are set within the callback function            
                 dspCV_worker_job_t   job;
                 dspCV_synctoken_t    token;
                 
                 int numWorkers = dspCV_num_hvx128_contexts;                                   
                 
                 dspCV_worker_pool_synctoken_init(&token, numWorkers);    // init the synchronization token

                 job.fptr=(0==i)?warp_callback<true>:warp_callback<false>;
                 
                 warp_callback_t dptr;
                 dptr.token = &token;
                 dptr.workerCount = 0;
                 dptr.jobNum=tileProcNum;
                 dptr.jobCount=0;
                 dptr.pCtx=&wctx;
                 dptr.pTileProcInfo=pTileProcInfo;
                 dptr.pIntmedPara=pIntmedPara;
                 
                 job.dptr = (void *)&dptr;
             
                 // each worker thread has a independent 2KiB buffer for vgather use
                 for(int j=0; j<numWorkers; j++)
                 {
                     dptr.vtcmVgatherBuf[j]=pVtcm+j*VTCM_VGATHER_SZ; 
                 }
             
                 for(int j=0; j<tileProcNum; j++)
                 {                                
                     pIntmedPara[j].intmed_image=pVtcm+VTCM_VGATHER_TOTAL_SZ+(pIntmedPara[j].intmed_orig_y-minVtcmY)/vertRatio*srcStride+pIntmedPara[j].intmed_orig_x;                
                 }            
                 
                 for(int j = 0; j < numWorkers; j++)
                 {
                     // for multi-threaded impl, use this line.
                     (void) dspCV_worker_pool_submit(job);
             
                     // This line can be used instead of the above to directly invoke the
                     // callback function without dispatching to the worker pool.
                     //job.fptr(job.dptr);                         
                 }
                 
                 dspCV_worker_pool_synctoken_wait(&token);     
                                         
                 // update for next round of while loop use
                 tileGrpStartIdx +=tileGrpSz; 
                 tileGrpSz = numTileTotal-tileGrpStartIdx;            
             }                        
         }

         if(useComputRes&&compute_resource_attr_init)
         {
            compute_resource_release(context_id);               
         }
         else
         { 
            HAP_release_VTCM((void*)pVtcm);
         }
         
         if(pTileProcBuf)
         {
            free(pTileProcBuf);
         }
    }
        
    // record end time (in both microseconds and pcycles) for profiling
#ifdef PROFILING_ON
    uint64 endCycles = HAP_perf_get_pcycles();
    uint64 endTime = HAP_perf_get_time_us();
    *dspUsec = (int)(endTime - startTime);
    *dspCyc = (int32)(endCycles - startCycles);
    FARF(HIGH,"warp profiling over %d iterations: %d PCycles, %d microseconds. Observed clock rate %d MHz",
                LOOPS, (int)(endCycles - startCycles), (int)(endTime - startTime), 
                (int)((endCycles - startCycles) / (endTime - startTime)));
#endif
    
    return AEE_SUCCESS;
}
