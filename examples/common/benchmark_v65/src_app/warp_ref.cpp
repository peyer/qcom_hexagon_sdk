/**=============================================================================
Copyright (c) 2018 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/
/* ======================================================================== */
/*  Reference C version of warp                                             */
/* ======================================================================== */


//==============================================================================
// Include Files
//==============================================================================
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <algorithm>
#include "benchmark_ref.h"

/*===========================================================================
    DEFINITIONS
===========================================================================*/

#define MAX(a,b)       ((a) > (b) ? (a) : (b))

template <class T>
struct ArrBufDesc_ {
    int width, height;
    T * data;
    unsigned rowpitch;  // in units of T
    
    T * rowptr(int i)
    { 
        return data + i*rowpitch;
    }

    T const * rowptr(int i) const
    { 
        return data + i*rowpitch;
    }

    void allocate( int h, int w)
    {
        assert( h >=1 && w >= 1);
        unsigned strd = w;
        void * mem = malloc( strd* h * sizeof(T));
        assert( mem != NULL);
        width = w;
        height = h;
        data = (T*) mem;
        rowpitch = strd;
    }
    
    void release() 
    { 
        if(data)
        { 
            free( (void*)data);
            data = NULL;
        }
    }
};

struct XYPair {
                    int32_t x, y;
};

/*===========================================================================
    DECLARATIONS
===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

void warp_ref(unsigned char *src, int srcStride, int srcWidth, int srcHeight, int32_t *gridarr,  unsigned char *dst, int dstStride);  

int constructMeshFromHomography(
            float const hMat[9],        // the matrix  (rows)
            float in_scale,             // overall scale (see above)
            int grid_pitch,             // grid pitch in output pixels (or  grid_pitchV*4096 + grid_pitchH)
            int gridW, int gridH,       // grid dimensions
            int32_t * gridArr,          // pointer to grid[0,0].x
            int gridStride,             // row pitch of gridArr in int32_t, taken as gridW*2 if zero
            int use_frame_origin);      // if !=0, the matrix is assumed to use frame origin
#ifdef __cplusplus
}
#endif

// bicubic interpolation is done with negative weights (so that -128 can be used
// in a signed byte).
//    (1) find sum-of-products with u8 pixels and signed weights
//    (2) add 128*255 to the result  (to bring result to u8 range)
//    (3) >>7 with rounding, saturate to u8 range
//
// This inverts the image, but after H interp and V interp it will be normal again.
//
static const int8_t BicubicTable[64][4] = {
           { 0,-128,  0,  0},  { 1,-128,  -1, 0},  { 2,-128,  -2, 0},  { 3,-127,  -4, 0},
           { 4,-127,  -5, 0},  { 4,-126,  -6, 0},  { 5,-125,  -9, 1},  { 6,-124, -11, 1},
           { 6,-123, -12, 1},  { 7,-122, -14, 1},  { 7,-121, -15, 1},  { 8,-120, -18, 2},
           { 8,-118, -20, 2},  { 8,-116, -22, 2},  { 9,-115, -24, 2},  { 9,-113, -27, 3},
           { 9,-111, -29, 3},  { 9,-109, -31, 3},  { 9,-107, -34, 4},  { 9,-105, -36, 4},
           { 9,-103, -38, 4},  { 9,-100, -42, 5},  { 9, -98, -44, 5},  { 9, -96, -46, 5},
           { 9, -93, -50, 6},  { 9, -91, -52, 6},  { 9, -88, -55, 6},  { 9, -85, -59, 7},
           { 9, -83, -61, 7},  { 9, -80, -64, 7},  { 8, -77, -66, 7},  { 8, -75, -69, 8},
           { 8, -72, -72, 8},  { 8, -69, -75, 8},  { 7, -66, -77, 8},  { 7, -64, -80, 9},
           { 7, -61, -83, 9},  { 7, -59, -85, 9},  { 6, -55, -88, 9},  { 6, -52, -91, 9},
           { 6, -50, -93, 9},  { 5, -46, -96, 9},  { 5, -44, -98, 9},  { 5, -42,-100, 9},
           { 4, -38,-103, 9},  { 4, -36,-105, 9},  { 4, -34,-107, 9},  { 3, -31,-109, 9},
           { 3, -29,-111, 9},  { 3, -27,-113, 9},  { 2, -24,-115, 9},  { 2, -22,-116, 8},
           { 2, -20,-118, 8},  { 2, -18,-120, 8},  { 1, -15,-121, 7},  { 1, -14,-122, 7},
           { 1, -12,-123, 6},  { 1, -11,-124, 6},  { 1,  -9,-125, 5},  { 0,  -6,-126, 4},
           { 0,  -5,-127, 4},  { 0,  -4,-127, 3},  { 0,  -2,-128, 2},  { 0,  -1,-128, 1}
};

static void interp_cubic_1( uint8_t * outp, uint8_t const *inarr, unsigned inarr_rowpitch, int sx, int sy, int /*nchan*/);
static void interp_cubic_N( uint8_t * outp, uint8_t const *inarr, unsigned inarr_rowpitch, int sx, int sy, int nchan);

/*===========================================================================
    TYPEDEF
===========================================================================*/
typedef void (*pixel_interp_fp) ( uint8_t * outp, uint8_t const *inarr, unsigned inarr_rowpitch, int sx, int sy, int nchan);
typedef ArrBufDesc_<uint8_t> ArrBufDesc_u8;
typedef ArrBufDesc_<XYPair> ArrBufDesc_XY;

/*===========================================================================
    LOCAL FUNCTION
===========================================================================*/
static inline int32_t round_to_int( float x )
{
	return (int32_t)floorf(0.5f + x);
}

static inline int rmode_set_tonearest(){ return 0;}

static inline void rmode_restore(int){}

// r is 1/z; return an estimate for 1/(z+d) given d
//  this is a 2nd order extrapolation
//   1/(z+d) ~=~   r + d * f'(z) + (1/2)d^2 * f''(z)
//
// where f(z) = 1/z so
//        f'(r) = -1/z^2    = -r^2
//       f''(r) = 2/z^3    =  2r^3
//
//    1/(z+d) ~=~   r - d *r^2 + d^2 * r^3
//            ~=~   r - (r*d)*r *( 1 - (r*d))
//
static inline float recip_offset( float r, float d)
{
    // approx of 1/zz ...  previous + dzz * d (1/z)/dz  + (1/2)dzz^2 * d2(1/z)dz2
    float dr = d * r;
    return r- dr * r * (1.0f-dr);
}

// r is an estimate of 1/x; return a better one
static inline float refine_recip( float r, float x)
{
    return r * (2.0f - r*x);
}

static inline int mul_rsh15_rnd( int a , int b)
{
    return (int64_t(a)* b  + 16384) >> 15;
}

// this emulates the new code which works on 8 blocks at once
// (calculating 4 pixels within each block)
template <bool IS_CHROMA> static void interpolate_tile_8_block_method (
        int32_t const upper[4],     // -> (x,y), (x,y)  upper corners
        int32_t const lower[4] ,    // -> (x,y), (x,y) lower corners
        int offsx, int offsy,       // offsets
        int32_t *outp,              // pointer to 1st row of 8x8 (or 4x4) output tile
        unsigned out_pitch )        // output row pitch (in int32's)
{

    int N = IS_CHROMA?4:8;          // size of output quadrant

    // outer loop : first pass does all 'x', second does all 'y'
    for( int ixy = 0; ixy < 2; ixy++)
    {
        // get sums and deltas at top and bottom
        int offs = (ixy==0)? offsx : offsy;
        int pt_UL_0 = upper[ixy];
        int pt_UR_0 = upper[ixy+2];
        int pt_DL_0 = lower[ixy];
        int pt_DR_0 = lower[ixy+2];

        for( int quadsel = 0; quadsel < 4; quadsel ++)
        {
            // 0 =  upper left
            // 1 =  upper right
            // 2 =  lower left
            // 3 =  lower right
            int pt_UL = pt_UL_0;
            int pt_UR = pt_UR_0;
            int pt_DL = pt_DL_0;
            int pt_DR = pt_DR_0;

            int32_t * quad_base = outp + ixy;

            if( quadsel >= 2)
            {
                // offset points down 1/2 a block
                int dyLh = (pt_DL - pt_UL)>>1;
                int dyRh = (pt_DR - pt_UR)>>1;
                pt_UL += dyLh;
                pt_UR += dyRh;
                pt_DL += dyLh;
                pt_DR += dyRh;
                quad_base += N * out_pitch;
            }
            
            if( (quadsel&1) == 0)
            {
                // offset points right 1/2 a block
                int dxUh = (pt_UR - pt_UL)>>1;
                int dxDh = (pt_DR - pt_DL)>>1;
                pt_UL += dxUh;
                pt_UR += dxUh;
                pt_DL += dxDh;
                pt_DR += dxDh;
                quad_base += N * 2;
            }
            // now we have 4 corners for the quadrant
            // change to base, dx, dyL, dyR

            int base = pt_UL + offs;
            int dx = pt_UR - pt_UL;
            int dyL = pt_DL - pt_UL;
            int dyR = (dyL + pt_DR-pt_UR)>>1;


            for(int i = 0; i < N; i++)
            {
                int ik = 4096*i + 2048;
                if( IS_CHROMA) ik <<= 1;
                int vleft = mul_rsh15_rnd( dyL,  ik);
                int vright = mul_rsh15_rnd( dyR,  ik) + dx;

                for( int j = 0; j < N; j++ )
                {
                    int val;
                    if(IS_CHROMA)
                    {
                        int jk = 4096*j + 1024;
                        int jk1 = 16384-jk;
                        val = (base + mul_rsh15_rnd( vleft, jk1) + mul_rsh15_rnd(vright, jk)+ 1) >>1;
                    }
                    else
                    {
                        int jk = 4096*j + 2048;
                        int jk1 = 32768-jk;
                        int tmp = (mul_rsh15_rnd( vleft, jk1) + mul_rsh15_rnd(vright, jk) + 1)>>1;
                        val = base + tmp;
                    }
                    quad_base[2*j + out_pitch*i] = val;
                } // for j
            } // for i
        } //for quadsel
    } //for ixy
}

static inline int single_h_interp( int frc, uint8_t p0, uint8_t p1, uint8_t p2, uint8_t p3)
{
    int8_t const * cp = BicubicTable[frc];
    int sop = cp[0]*p0 + cp[1]*p1 + cp[2] *p2 + cp[3] * p3;
    int res = ( sop + 255*128 + 64)>>7;
    
    return ((unsigned)res < 256) ? res : (res < 0) ? 0 : 255;
}

static inline int single_v_interp( int frc, uint8_t const px[4])
{
    return single_h_interp( frc, px[0],px[1], px[2], px[3]);
}

static void interp_cubic_1( uint8_t * outp, uint8_t const *inarr, unsigned inarr_rowpitch, int sx, int sy, int /*nchan*/)
{
    uint8_t const * srcp = &inarr[ (sy>>6) * inarr_rowpitch + (sx>>6) ];
    uint8_t hinterp[4];

    int fracx = sx & 0x3F;
    int fracy = sy & 0x3F;
    
    for(int i  =0; i < 4; i++)
    {
        hinterp[i] = single_h_interp( fracx, srcp[0], srcp[1], srcp[2], srcp[3]);
        srcp += inarr_rowpitch;
    }
    
    *outp = single_v_interp( fracy, &hinterp[0]);
}

static void interp_cubic_N( uint8_t * outp, uint8_t const *inarr, unsigned inarr_rowpitch, int sx, int sy, int nchan)
{
    uint8_t const * srcp0 = &inarr[ (sy>>6) * inarr_rowpitch + (sx>>6)*nchan ];
    uint8_t hinterp[4];

    int fracx = sx & 0x3F;
    int fracy = sy & 0x3F;
    
    for( int c= 0; c < nchan; c++)
    {
        uint8_t const * srcp = srcp0;
        
        for(int i  =0; i < 4; i++)
        {
            hinterp[i] = single_h_interp( fracx, srcp[0], srcp[nchan], srcp[2*nchan], srcp[3*nchan]);
            srcp += inarr_rowpitch;
        }
        
        outp[c] = single_v_interp( fracy, &hinterp[0]);
        srcp0++;
    }
}

static void interp_bilinear_1( uint8_t * outp, uint8_t const *inarr, unsigned inarr_rowpitch, int sx, int sy, int);

static void interp_bilinear_2( uint8_t * outp, uint8_t const *inarr, unsigned inarr_rowpitch, int sx, int sy, int );

static void interp_bilinear_func( uint8_t * outp, uint8_t const *inarr, unsigned inarr_rowpitch, int sx, int sy, int nchan);

static inline void interp_bilinear_inline( uint8_t * outp, uint8_t const *inarr, unsigned inarr_rowpitch, int sx, int sy, int nchan)
{
    uint8_t const * srcp = &inarr[ (sy>>6) * inarr_rowpitch + (sx>>6)*nchan ];
    int fracx = sx & 0x3F;
    int fracy = sy & 0x3F;
    int fracx0 = 64-fracx;

    // find the weights for each corner
    // make sure they add to 128

    int w10 = ( fracx0*fracy + 16 ) >> 5;
    int w11 = ( fracx*fracy + 16 )>> 5;
    int w00 = 2*fracx0 - w10;
    int w01 = 2*fracx - w11;

    // srcp[0], srcp[nchan], srcp[inarr_rowpitch],srcp[inarr_rowpitch+nchan] are four surrounding points around the target point
    for(int ic = 0; ic < nchan; ic++)
    {
        int sop = srcp[0]*w00 + srcp[nchan]*w01; 
        sop += srcp[inarr_rowpitch] * w10 + srcp[inarr_rowpitch+nchan]*w11;
        srcp ++;
        
        // this will be in range -256 .. 32894
        int res = (sop + 64)>>7;        // now -2 .. 257

        outp[ic] = ((unsigned)res < (unsigned)256)? res : (res < 0)? 0:255;
    }
}

static void interp_bilinear_1( uint8_t * outp, uint8_t const *inarr, unsigned inarr_rowpitch, int sx, int sy, int /*nchan*/)
{
    return interp_bilinear_inline( outp, inarr,inarr_rowpitch, sx, sy, 1 );
}
static void interp_bilinear_2( uint8_t * outp, uint8_t const *inarr, unsigned inarr_rowpitch, int sx, int sy, int /*nchan*/)
{
    return interp_bilinear_inline( outp, inarr,inarr_rowpitch, sx, sy, 2 );
}
static void interp_bilinear_func( uint8_t * outp, uint8_t const *inarr, unsigned inarr_rowpitch, int sx, int sy,int nchan)
{
    return interp_bilinear_inline( outp, inarr,inarr_rowpitch, sx, sy, nchan );
}


/*===========================================================================
    GLOBAL FUNCTION
===========================================================================*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// given
//   - a homography as [3,3] matrix, mapping output coords to input coords
//   - mesh parameters
// construct a mesh of (x,y) coords which represent input sample sizes
//
//
//  the mapping is given by
//    [xx]           [u]
//    [yy] = [ mat ] [v]
//    [zz]           [1]
//.. then
//   x = in_scale * xx/zz
//   y = in_scale * yy/zz
//   .. where u,v are output pixel coord and x,y are input pixel coords.
// in_scale is a supplied overall scale factor.
//
//  The matrix is assumed to work with 'pixel origin' where the upper left pixel
//   is at (0,0), in input and output.
//  The mesh is generated with respect to 'frame origin', in which the upper left
//   pixel is at (0.5,0.5). The matrix is converted (internally) to frame origin;
//   if the matrix is already composed using frame-origin, there is an option to
//   leave it that way.
//
// The grid has a pitch in pixels; normally this is the same in both directions
// but if grid_pitchH != grid_pitchV you can pass 'grid_pitchV*4096 + grid_pitchH' as the value.
//
// The grid is stored as an array of int32_t, with 6 fractional pixels. If you want
// a different number, adjust in_scale accordingly (e.g. in_scale = 0.5 gives you 5 fractional bits).
//

// The dimensions of the grid are gridH rows, gridW cols,  2 values per entry (x,y).
// Each row is gridW*2 int32_t; the spacing between rows is given as gridStride
// (if ==0, this is taken as gridW*2).
//
// row i, column j of the grid corresponds to (in  'frame' coords):
//     x = j*grid_pitchH
//     y = i*grid_pitchV
//
// The function works with 'float' values and performs a small number of actual
// 'divide' ops (one per row of the grid).
//
int constructMeshFromHomography(
        float const hMat[9],        // the matrix  (rows)
        float in_scale,             // overall scale (see above)
        int grid_pitch,             // grid pitch in output pixels (or  grid_pitchV*4096 + grid_pitchH)
        int gridW, int gridH,       // grid dimensions
        int32_t * gridArr,          // pointer to grid[0,0].x
        int gridStride,             // row pitch of gridArr in int32_t, taken as gridW*2 if zero
        int use_frame_origin )      // if !=0, the matrix is assumed to use frame origin
    {
    // copy in matrix, while adjusting for frame origin...
    // matrix we want is
    //   [ 1   0   0.5 ]  [  supplied ]  [ 1   0  -0.5 ]
    //   [ 0   1   0.5 ]  [  matrix   ]  [ 0   1  -0.5 ]
    //   [ 0    0   1  ]  [           ]  [ 0    0   1  ]
    float hMatrix[9];//hMatrix is the converted result of hMat from pixel origin to frame origin
    float xyoffs = 0.5f;
    if(use_frame_origin) xyoffs = 0.0f;


    // copy matrix in, adjusting as needed;
    // mul by left-hand offset matrix, and
    // mul 1st two rows by 64*scale.

    in_scale *= 64.0f;      // 6 fractional bits

    for( int i = 0; i < 3; i++)
    {
        float r0 = hMat[i];
        float r1 = hMat[i+3];
        float r2 = hMat[i+6];
        float tr2 = xyoffs * r2;
        hMatrix[i] = (r0 + tr2)*in_scale;
        hMatrix[i+3] = (r1 + tr2)*in_scale;;
        hMatrix[i+6] = r2;
    }
    
    if( use_frame_origin == 0)
    {
        // mul by matrix on the right:
        // subtract half the sum of 1st two cols from last column.
        for( int i=0; i < 9; i+=3)
        {
            hMatrix[i+2] -= 0.5f*(hMatrix[i]+ hMatrix[i+1]);
        }
    }
    
    //
    // get the grid pitch and check it
    //
    float grid_pitchH = float(grid_pitch & 0xFFF);
    int gptmp = grid_pitch >> 12;
    float grid_pitchV = (gptmp==0)? grid_pitchH : float( gptmp);
    if( grid_pitchV < 1.0f || grid_pitchH < 1.0f) return -1;
    if( gridH < 2 || gridW < 2 ) return -1;
    if( gridStride == 0) gridStride = gridW*2;
    
    // every for loop processes two columns
    float dxx = hMatrix[0] * grid_pitchH;       // amount xx changes from col to col
    float dyy = hMatrix[3] * grid_pitchH;
    float dzz = hMatrix[6] * grid_pitchH;

    float dxx_dv  = hMatrix[1] * grid_pitchV;   // amount changing from row to row.
    float dyy_dv  = hMatrix[4] * grid_pitchV;
    float dzz_dv  = hMatrix[7] * grid_pitchV;

    // we only do one float division per row of the grid;
    // all other 1/zz reciprocal are estimated from the previous one, and refined.
    // (each zz is calculated, for the purpose of doing that refinement).
    // Rounding of float-to-int must be done as 'to nearest'
    int old_rmode = rmode_set_tonearest();
    // now evaluate two rows at once (for better operation scheduling)
    //
    // if gridH is odd, we will do the second-last one twice
    // e.g. for gridH = 5, we wil do 3 passes; rows {0,1}, {2,3}, {3,4}
    //
    for( int irow = 0; irow < gridH; irow+=2)
    {
        int igridrow =  irow;
        if( igridrow > gridH-2) igridrow = gridH-2;

        int32_t *gpOut_0 = &gridArr[igridrow * gridStride];
        int32_t *gpOut_1 = gpOut_0+ gridStride;

        // for the upper row, find the 'v' coordinate and multiply by the second col of matrix,
        // adding the 3rd; this gives xx,yy,zz for the first grid point on the row (u=0).
        // offset for the lower row.

        float rowv = float(igridrow);
        float base_xx =  dxx_dv*rowv + hMatrix[2];
        float base_yy =  dyy_dv*rowv + hMatrix[5];
        float base_zz =  dzz_dv*rowv + hMatrix[8];
        float xx_0 = base_xx;
        float yy_0 = base_yy;
        float zz_0 = base_zz;
        float xx_1 = base_xx  + dxx_dv;
        float yy_1 = base_yy  + dyy_dv;
        float zz_1 = base_zz  + dzz_dv;

        float zz_0_recip = 1.0f/zz_0;
        float zz_1_recip = 1.0f/zz_1;

        // each loop does two columns. If gridW is odd, there will be an extra result
        // to store at the end.
        for( int jcol=1; jcol < gridW; jcol+= 2)
        {
            // store jcol-1 result
            gpOut_0[0] = round_to_int( xx_0 * zz_0_recip);
            gpOut_0[1] = round_to_int( yy_0 * zz_0_recip);
            gpOut_1[0] = round_to_int( xx_1 * zz_1_recip);
            gpOut_1[1] = round_to_int( yy_1 * zz_1_recip);

            // advance to next column 'jcol'
            xx_0 += dxx;
            yy_0 += dyy;
            zz_0 += dzz;
            xx_1 += dxx;
            yy_1 += dyy;
            zz_1 += dzz;

            // approx of 1/zz ...  previous + dzz * d (1/z)/dz  + (1/2)dzz^2 * d2(1/z)dz2
            zz_0_recip = recip_offset( zz_0_recip, dzz);
            // refine 1/zz
            // this will give float precision  if dzz <  6% of zz
            // 20- bit precision if dzz is <10% of zz
            zz_0_recip = refine_recip( zz_0_recip,zz_0);

            zz_1_recip = recip_offset( zz_1_recip, dzz);
            zz_1_recip = refine_recip( zz_1_recip,zz_1);
            // store 'jcol' result
            gpOut_0[2] = round_to_int( xx_0 * zz_0_recip);
            gpOut_0[3] = round_to_int( yy_0 * zz_0_recip);
            gpOut_1[2] = round_to_int( xx_1 * zz_1_recip);
            gpOut_1[3] = round_to_int( yy_1 * zz_1_recip);
            // now jcol+1 (recalc to avoid rounding creep)
            float jj = float(jcol+1);
            xx_0 = base_xx + jj*dxx;
            yy_0 = base_yy + jj*dyy;
            zz_0 = base_zz + jj*dzz;
            xx_1 = xx_0 + dxx_dv;
            yy_1 = yy_0 + dyy_dv;
            zz_1 = zz_0 + dzz_dv;
            zz_0_recip = recip_offset( zz_0_recip, dzz);
            zz_0_recip = refine_recip( zz_0_recip,zz_0);
            zz_1_recip = recip_offset( zz_1_recip, dzz);
            zz_1_recip = refine_recip( zz_1_recip,zz_1);
            // jcol+1 result is stored on the next loop.
            gpOut_0 += 4;
            gpOut_1 += 4;
        }
        
        if(gridW&1)
        {
            gpOut_0[0] = round_to_int( xx_0 * zz_0_recip);
            gpOut_0[1] = round_to_int( yy_0 * zz_0_recip);
            gpOut_1[0] = round_to_int( xx_1 * zz_1_recip);
            gpOut_1[1] = round_to_int( yy_1 * zz_1_recip);
        }
    }
    
    rmode_restore(old_rmode);
    return 0;
}

//
// Expand a 'grid' into individual pixel sample points.
// Input and output both consist of 32-bit (x,y) values,
// Generally these have fractional bits, but the input and output
// have the same number of fractional bits, so that information is not
// needed here (generally we'll use 6 fraction bits; for 4:2:0 chroma
// planes we will assume 7 fractional bits in the same data, relative to
// chroma pixels).
//
//
// The input array is wid_in by ht_in (both must be >= 2)
// The output array is (wid_in-1)*ratio  by (ht_in-1)*ratio
//
// both arrays have packed rows:  (int32 x, int32_y) across the row,
//    with a specified row pitch in units of int32.
//
// The 'ratio' must a power of 2 in range 2..64.
//
// The interpolation is done as follows, within each 'square' of the grid:
//   (1) interpolation across between UL and UR, and also between DL and DR
//     This is done with samples at (0.5/ratio, 1.5/ratio, 2.5/ratio ...   (ratio-0.5)/ratio)
//   (2) interpolate vertically between each pair of points generated in (1), using the same method.
//
// The 'offsx, offsy' values are added to all x/y values in the grid before processing;
// these are used to handle origin shifts needed by the downstream operation. They have
// the same # of fractional bits as the input.
// They should be set up so that x,y per-pixel coordinates are >= 0 if and only if
// they don't need to be clipped on the left/top.
//
// Note that no clipping is done. It is expected that the x,y input values
// be within the range +/-(2^23-1) (so we can do multiplies in 32-bits without overflow).
//
// returns -1 if there is a problem with the parameters
//
int interpolateSampleGridBilinear(
        int wid_in,                 // input width and height (both must be => 2
        int ht_in,
        int ratio,                  // upscale ratio.
        int offsx, int offsy,       // x,y offset.
        bool is_chroma,             // whether Chroma or Luma
        int32_t const * inarr,      // input array data
        unsigned in_arr_pitch,      // input row pitch, in int32 units
        int32_t *outarr,
        unsigned out_arr_pitch )    // output array pitch, in int32 units
{
    if( wid_in < 2 || ht_in < 2 || ratio < 2 || ratio > 64 || (ratio&(ratio-1)) != 0)
    {
        return -1;
    }
    
    // sanity check, to catch absurdly large output cases
    int64_t wid_out = int64_t(ratio) * (wid_in-1);
    int64_t ht_out = int64_t(ratio) * (ht_in-1);
    if( wid_out > (1<<24) || ht_out > (1<<24) || wid_out*ht_out > (1<<28))
    {
        return -1;
    }

    unsigned out_arr_pitch2 = out_arr_pitch;	// usually these are the same...

    // emulate 8x8 chroma interp?
    // we have a function for that; call it on each tile
    if( ratio == 8 && is_chroma )               
    {
        for( int  ibrow = 0; ibrow < ht_in-1; ibrow++)
        {
            for( int ibcol = 0; ibcol < wid_in-1; ibcol++)
            {
                int32_t const *pin0 = &inarr [in_arr_pitch*ibrow + ibcol*2];
                int32_t const *pin1 = pin0 + in_arr_pitch;
                int32_t * pout = &outarr[out_arr_pitch2*8*ibrow + ibcol*2*8];
                interpolate_tile_8_block_method<true>( pin0, pin1, offsx, offsy, pout, out_arr_pitch );
            }
        }
        
        return 0;
    }
    else if( ratio == 16 && !is_chroma )  
    // emulate 16x16 Luma interp?
    // we have a function for that; call it on each tile
    {
        for( int  ibrow = 0; ibrow < ht_in-1; ibrow++)
        {
            for( int ibcol = 0; ibcol < wid_in-1; ibcol++)
            {
                int32_t const *pin0 = &inarr [in_arr_pitch*ibrow + ibcol*2];
                int32_t const *pin1 = pin0 + in_arr_pitch;
                int32_t * pout = &outarr[out_arr_pitch2*16*ibrow + ibcol*2*16];
                interpolate_tile_8_block_method<false>( pin0, pin1, offsx, offsy, pout, out_arr_pitch );
            }
        }
        return 0;
    }
    else // only support the above two ratio and is_chroma combinations
    {    
        return -1;
    }
}

// Given an input image, an interpolated set of warp coords, and a place to store
// the output pixels, do pixel-by-pixel warp interpolation using cubic interpolation.
// You can specify # channels (e.g. 1 for gray, 3 for rgb)
// ... which must be the same across input and output
//
// The 'sampxy' array is that generated by the interpolateSampleGridBilinear routine;
// it is assumed to have the same [h,w] size as the output array, and contains pairs of int32 (x,y).
//
// The sample points are assumed to have 6 fractional bits
//
// The row widths of the arrays are:
//    inarr :   in_w * nchan * uint8
//    outarr:   out_w * nchan * uint8
//    sampxy:   out_w * 2 * int32
//
// Clipping:  all coords are first limited:
//      x :  0.. (in_w-3)*64-1
//      y :  0..(in_h*3)*64-1
// .. and then the integer pars of these values ( lower 6 bits removed) are considered
//  to be the upper-left corner of a 4x4 square of input pixels.  These will be
//   in range 0.. in_w-4 and 0..in_h-4 respectively.
// So the interpolation point is actually (x+1,y+1) (with [floor(x),floor(y)] being the top/left pixel used);
// input x,y values must be offset to compensate.
//
// This means no values will be read out-of-range, but it also means that the  effective sample points
// are clipped to a min of 1 and a max of wid-2 (or ht-2). If this is not acceptable, you can pad the input
// image on all sides (and offx, offy can be adjusted to compensate for the padding).
//
int interpolatePixelBicubic(
        int in_w, int in_h,                 // size of input image
        uint8_t const * inarr,              // pointer
        unsigned inarr_rowpitch,            // pitch in bytes
        int nchan,                          // # of channels (1..64)
        int out_w, int out_h,               // size of output image
        uint8_t  * outarr,                  // pointer
        unsigned outarr_rowpitch,           // pitch in bytes

        int32_t const * sampxy,
        unsigned sampxy_rowpitch
        )
{
    if ( in_w < 4 || in_h < 4 || out_w < 1 || out_h < 1 || nchan < 1 || nchan > 64 )
    {
        return -1;
    }
    
    int max_sample_x = (in_w-3)*64-1;
    int max_sample_y = (in_h-3)*64-1;

    pixel_interp_fp pixel_interp_func = (nchan ==1)? interp_cubic_1 : interp_cubic_N;

    for(int irow = 0; irow < out_h; irow++)
    {
        uint8_t * outp = &outarr[outarr_rowpitch*irow];
        for( int icol = 0; icol< out_w; icol++)
        {
            int sx = sampxy[icol*2];
            int sy = sampxy[icol*2+1];

            if( sx < 0) sx = 0 ;
            else if(sx > max_sample_x) sx = max_sample_x;
            if( sy < 0) sy = 0 ;
            else if(sy > max_sample_y) sy = max_sample_y;
            (*pixel_interp_func)(outp, inarr, inarr_rowpitch, sx, sy, nchan );
            outp += nchan;
        }
        sampxy += sampxy_rowpitch;
    }
    
    return 0;
}

// Given an input image, an interpolated set of warp coords, and a place to store
// the output pixels, do pixel-by-pixel warp interpolation using bilinear interpolation.
// You can specify # channels (e.g. 1 for gray, 3 for rgb)
// ... which must be the same across input and output
// This is intended for interpolation of chroma 4:2:0, so you would use nchan = 2
// if the U/V are interleaved.
//
// The 'sampxy' array is that generated by the interpolateSampleGridBilinear routine;
// it is assumed to have the same [h,w] size as the output array, and contains pairs of int32 (x,y).
//
// The sample points are assumed to have 6 fractional bits
//
// The row widths of the arrays are:
//    inarr :   in_w * nchan * uint8
//    outarr:   out_w * nchan * uint8
//    sampxy:   out_w * 2 * int32//*2 is for (x,y) pair
//
// Clipping:  all coords are first limited:
//      x :  0..(in_w-1)*64-1
//      y :  0..(in_h-1)*64-1
// .. and then the integer pars of these values ( lower 6 bits removed) are considered
//  to be the upper-left corner of a 2x2 square of input pixels.  These will be
//   in range 0.. in_w-2 and 0..in_h-2 respectively.
// This means no values will be read out-of-range, but it also means that 'edge extend' is the only
// edge handling option. If this is not acceptable, you can pad the input
// image on all sides (and offx, offy can be adjusted to compensate for the padding).
//
int interpolatePixelBilinear(
        int in_w, int in_h,                 // size of input image
        uint8_t const * inarr,              // pointer
        unsigned inarr_rowpitch,            // pitch in bytes
        int nchan,                          // # of channels (1..64)
        int out_w, int out_h,               // size of output image
        uint8_t  * outarr,                  // pointer
        unsigned outarr_rowpitch,           // pitch in bytes

        int32_t const * sampxy,
        unsigned sampxy_rowpitch
        )
{
    if ( in_w < 2 || in_h < 2 || out_w < 1 || out_h < 1 || nchan < 1 || nchan > 64 )
    {
        return -1;
    }
    
    int max_sample_x = (in_w-1)*64-1;
    int max_sample_y = (in_h-1)*64-1;

    pixel_interp_fp pixel_interp_func =
                (nchan ==1)? interp_bilinear_1:
                (nchan==2)? interp_bilinear_2:
                    interp_bilinear_func;

    for(int irow = 0; irow < out_h; irow++)
    {
        uint8_t * outp = &outarr[outarr_rowpitch*irow];

        for( int icol = 0; icol< out_w; icol++)
        {
            int sx = sampxy[icol*2];
            int sy = sampxy[icol*2+1];

            if( sx < 0) sx = 0 ;
            else if(sx > max_sample_x) sx = max_sample_x;
            if( sy < 0) sy = 0 ;
            else if(sy > max_sample_y) sy = max_sample_y;
            (*pixel_interp_func)(outp, inarr, inarr_rowpitch, sx, sy, nchan );
            outp += nchan;
        }
        sampxy += sampxy_rowpitch;
    }
    
    return 0;
}


void warp_ref(  unsigned char   *src,                
                int              srcStride, 
                int              srcWidth,
                int              srcHeight,
                int32_t         *gridarr,   // of size int32_t gridarr[meshH][meshW][2]              
                unsigned char   *dst,
                int              dstStride)                                
{
    const int meshW=int((srcWidth+15)/16)+1;
    const int meshH=int((srcHeight+15)/16)+1;

    assert( srcWidth >= 32 && srcHeight >= 32 );
    assert( srcWidth%2==0 && srcHeight%2==0 );// otherwise it will hard to handle Chroma part's srcWidth/2 and srcHeight/2.
    assert( srcStride%4==0 );
    
    // firstly process the Luma
    // -96=-32-64, -32 is the frame origin to pixel origin, -64 is an additional pixel used for 4x4 interpolation
    int offsX = -96;
    int offsY = -96;
    int upscale_ratio=16;

    int outW = srcWidth;
    int outH = srcHeight;
    ArrBufDesc_XY meshexp;
        
    // malloc within the allocate() for the meshgrid use
    // For Luma, there is a one to one mapping between meshexp and output image, so the size of 
    // the two matrix is the same.
    int meshexpRow=std::max(outH, upscale_ratio*meshH);
    meshexp.allocate(meshexpRow, dstStride);
    
    int res = interpolateSampleGridBilinear(
                                        meshW,              // input width and height (both must be => 2)
                                        meshH,
                                        upscale_ratio,      
                                        offsX, offsY,       // x,y offset.
                                        false,     
                                        gridarr,
                                        meshW*2,            
                                        &meshexp.data[0].x,
                                        meshexp.rowpitch*2);    
    if( res != 0)
    {
        printf("Luma: error return from interpolateSampleGridBilinear\n");
        meshexp.release();
        return;
    }
            
    res = interpolatePixelBicubic(
                                    srcWidth, srcHeight,                // size of input image
                                    src,                                // pointer
                                    srcStride,                          // pitch in bytes
                                    1,                                  // # of channels (1..64)
                                    outW, outH,                         // size of output image
                                    dst,                                // pointer
                                    dstStride,                          // pitch in bytes
                                    &meshexp.data[0].x,
                                    meshexp.rowpitch *2);
                                       
    
    meshexp.release();          
        
    // secondly process the Chroma    
    unsigned char *srcChroma=src+srcStride*srcHeight;
    unsigned char *dstChroma=dst+dstStride*outH; // let the pointer moves to the next region to accomodate the Chroma results
    
    offsX = -32;
    offsY = -64;  
    upscale_ratio=8;
    outW = srcWidth;
    outH = srcHeight/2;

    // The height of the expanded mesh is different between Luma and Chroma, so alloc/process/free one by one.
    // For Chroma, the width of meshexp is half of output image, widthx(X,Y) coordinates. e.g. If the output width is 1280, 
    // the 'Chroma' image row is 640 x {u,v} =  1280 bytes. But in the interpolation, only one interpolation point is used 
    // for each {u,v} pair, so the pixel-resolution mesh is only 640 wide for chroma.
    meshexpRow=std::max(outH, upscale_ratio*meshH);
    meshexp.allocate(meshexpRow, dstStride/2); 
    
    res = interpolateSampleGridBilinear(
                                        meshW,                      // input width and height (both must be => 2)
                                        meshH,
                                        upscale_ratio,              
                                        offsX, offsY,               // x,y offset.
                                        true,             
                                        gridarr,
                                        meshW*2,                    
                                        &meshexp.data[0].x,         // the output is pixel coordinate
                                        meshexp.rowpitch*2);
                                                         
    if( res != 0)
    {
        printf("Chroma: error return from interpolateSampleGridBilinear\n");
        meshexp.release();
        return;
    }
   
    res = interpolatePixelBilinear(
                                    srcWidth/2, srcHeight/2,            // size of input image
                                    srcChroma,                          // pointer
                                    srcStride,                          // pitch in bytes
                                    2,                                  // # of channels (1..64)
                                    outW/2, outH,                       // size of output image (width is in U,V pairs). Since U+V=outW, so each quantity should be divided by 2.      
                                    dstChroma,                          // pointer
                                    dstStride,                          // pitch in bytes
                                    &meshexp.data[0].x,
                                    meshexp.rowpitch *2);
                                    
    if( res != 0)
    {
        printf("error return from interpolate\n");       
    }

    meshexp.release();        
}
