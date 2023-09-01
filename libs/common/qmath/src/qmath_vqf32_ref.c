/**=============================================================================

@file
   qmath_vqf32_ref.c

@brief
   Some reference C implementations of vqf32 functions, for development/debug. 
   This is not a complete set of reference functions, rather presented as-is.

Copyright (c) 2017 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

//==============================================================================
// Include Files
//==============================================================================

// includes
#include "qmath.h"
#include <stdlib.h>

#include "hexagon_protos.h"

/*===========================================================================
    DEFINITIONS
===========================================================================*/
// (128-byte is only mode supported)
#define VLEN 128 

// define max/min integers
const int64_t MAXINT64 = 0x7FFFFFFFFFFFFFFFLL;
const int64_t MININT64 = 0x8000000000000000LL;
const int32_t MAXINT32 = 0x7FFFFFFFL;
const int32_t MININT32 = 0x80000000L;

// define exponent values to represent infinities
const int32_t MAXEXP32 = 0x20000000UL;

/*===========================================================================
    DECLARATIONS
===========================================================================*/

/*===========================================================================
    TYPEDEF
===========================================================================*/

/*===========================================================================
    LOCAL FUNCTION
===========================================================================*/
#define MAX(a,b)       ((a) > (b) ? (a) : (b))
#define MIN(a,b)       ((a) < (b) ? (a) : (b))

/*===========================================================================
    GLOBAL FUNCTION
===========================================================================*/
// input data must be scaled by 2.000000 ^ i for each coefficient
// input data must be generated for each interval by subtracting range value
int inv32Table[16][4] = {
//---------------- 0.500000 to 0.531250 ----------------------
// { 0.999999897538, -0.999946763580, 0.995662398063, -0.885402947309, },
{2147483428, -2147369324, 2138168719, -1901388351 },
//---------------- 0.531250 to 0.562500 ----------------------
// { 0.941176394255, -0.885773523170, 0.830481318262, -0.699613491641, },
{2021160917, -1902184157, 1783445051, -1502408533 },
//---------------- 0.562500 to 0.593750 ----------------------
// { 0.888888831082, -0.790093472092, 0.699893791654, -0.560102570728, },
{1908874230, -1696712812, 1503010473, -1202811112 },
//---------------- 0.593750 to 0.625000 ----------------------
// { 0.842105218735, -0.709118248022, 0.595300844669, -0.453699735854, },
{1808407187, -1522819842, 1278398830, -974312764 },
//---------------- 0.625000 to 0.656250 ----------------------
// { 0.799999965409, -0.639982081493, 0.510545336364, -0.371407617757, },
{1717986844, -1374351055, 1096387761, -797591786 },
//---------------- 0.656250 to 0.687500 ----------------------
// { 0.761904734646, -0.580484754270, 0.441140008785, -0.306956789956, },
{1636177959, -1246581518, 947340955, -659184687 },
//---------------- 0.687500 to 0.718750 ----------------------
// { 0.727272705558, -0.528914384077, 0.383762246038, -0.255900086363, },
{1561806243, -1135834991, 824123148, -549541251 },
//---------------- 0.718750 to 0.750000 ----------------------
// { 0.695652156443, -0.483922911916, 0.335916215412, -0.215031606647, },
{1493901631, -1039216540, 721374580, -461776859 },
//---------------- 0.750000 to 0.781250 ----------------------
// { 0.666666652484, -0.444437112397, 0.295702514502, -0.182006067887, },
{1431655735, -954421431, 635016315, -390855055 },
//---------------- 0.781250 to 0.812500 ----------------------
// { 0.639999988389, -0.409593999962, 0.261658327884, -0.155084593527, },
{1374389510, -879596417, 561906980, -333041629 },
//---------------- 0.812500 to 0.843750 ----------------------
// { 0.615384605806, -0.378693276726, 0.232644717038, -0.132961948343, },
{1321528378, -813237619, 499600726, -285533610 },
//---------------- 0.843750 to 0.875000 ----------------------
// { 0.592592584633, -0.351161870848, 0.207765969417, -0.114647229230, },
{1272582885, -754114375, 446174022, -246203050 },
//---------------- 0.875000 to 0.906250 ----------------------
// { 0.571428564771, -0.326527175735, 0.186311103640, -0.099380249251, },
{1227133499, -701211771, 400100049, -213417460 },
//---------------- 0.906250 to 0.937500 ----------------------
// { 0.551724132328, -0.304396633127, 0.167710912816, -0.086572134063, },
{1184818552, -653686792, 360156443, -185912242 },
//---------------- 0.937500 to 0.968750 ----------------------
// { 0.533333328591, -0.284441997861, 0.151506053294, -0.075762585405, },
{1145324602, -610834539, 325356772, -162698913 },
//---------------- 0.968750 to 1.000000 ----------------------
// { 0.516129028222, -0.266387096426, 0.137323084110, -0.066588773433, },
{1108378648, -572061934, 294899078, -142998302 }
};

// Creating order 4 polynomial approximation of 0.5/X, in interval 0.500000 to 1.000000 in 16 sections
// Quantization is 2147483648.000000 on grid of 32768 points
// input data must be scaled by 2.000000 ^ i for each ceofficient
// input data must be generated for each interval by subtracting range value
// max error <= 3.3312394618988 / 2^31
int inv32Table4[16][5] = {
//---------------- 0.500000 to 0.531250 ----------------------
{2147483645, -2147480949, 2147133394, -2131482747, 1844192256 },
//---------------- 0.531250 to 0.562500 ----------------------
{2021161078, -1902267358, 1790125413, -1673845791, 1373909894 },
//---------------- 0.562500 to 0.593750 ----------------------
{1908874352, -1696775847, 1508070670, -1332652643, 1040458237 },
//---------------- 0.593750 to 0.625000 ----------------------
{1808407281, -1522868302, 1282288281, -1074101491, 799566904 },
//---------------- 0.625000 to 0.656250 ----------------------
{1717986917, -1374388803, 1099416950, -875300862, 622602786 },
//---------------- 0.656250 to 0.687500 ----------------------
{1636178017, -1246611273, 949728418, -720425167, 490622030 },
//---------------- 0.687500 to 0.718750 ----------------------
{1561806289, -1135858702, 826025313, -598328994, 390832872 },
//---------------- 0.718750 to 0.750000 ----------------------
{1493901668, -1039235621, 722905146, -501030369, 314436642 },
//---------------- 0.750000 to 0.781250 ----------------------
{1431655765, -954436926, 636259083, -422725162, 255278697 },
//---------------- 0.781250 to 0.812500 ----------------------
{1374389534, -879609105, 562924512, -359133817, 208987249 },
//---------------- 0.812500 to 0.843750 ----------------------
{1321528399, -813248089, 500440275, -307060463, 172412890 },
//---------------- 0.843750 to 0.875000 ----------------------
{1272582902, -754123076, 446871672, -264090393, 143257171 },
//---------------- 0.875000 to 0.906250 ----------------------
{1227133513, -701219049, 400683629, -228379302, 119822467 },
//---------------- 0.906250 to 0.937500 ----------------------
{1184818564, -653692919, 360647620, -198504392, 100840945 },
//---------------- 0.937500 to 0.968750 ----------------------
{1145324612, -610839726, 325772560, -173357799, 85355940 },
//---------------- 0.968750 to 1.000000 ----------------------
{1108378657, -572066348, 295252944, -152069396, 72638597 }
};

int norm32(int x) {
    int l0, l1;
    if(x == 0) return(0);
    for(l0=0; l0<32; l0++) if(!((x<<l0) & 0x80000000)) break;
    for(l1=0; l1<32; l1++) if( ((x<<l1) & 0x80000000)) break;
    if(l1 > l0) l0 = l1;
    return(l0-1);
}

int cl0(int x) {
    int l1;
    if(x == 0) return(0);
    for(l1=0; l1<32; l1++) if( ((x<<l1) & 0x80000000)) break;
    return(l1);
}

int smpy32(int x, int y)
{
    long long int a = (long long int )x;
    long long int b = (long long int )y;
    a = ((a * b) + 0x40000000LL) >> 31;
    return((int) a);
}

HVXMATH_API vqf32_t
vqf32_inverse(vqf32_t a)
{
    HVX_Vector cMant, cExp;
    int i, v, e;

    int32_t *vptr = (int32_t*)&a;
    int32_t *eptr = vptr + 32;

    int32_t* cmPtr = (int32_t*)&cMant;
    int32_t* cePtr = (int32_t*)&cExp;
    int x, range, y, exp, mant;

    for (i = 0; i < 32; i++)
    {
        v = *vptr++;
        e = *eptr++;
        int neg = (v < 0);
        int zero = (v == 0);
        int inf = (e >= MAXEXP32);
        if (neg) v = -v;

        exp  = norm32(v);
        mant = v << exp;

        range = (mant >> 26) & 0xf;
        x = (mant<<1) & 0x7ffffff;
#ifdef HIGH_QUALITY_INVERSE
        y = inv32Table4[range][4];
        y = smpy32(x , y ) + inv32Table4[range][3];
        y = smpy32(x , y ) + inv32Table4[range][2];
        y = smpy32(x , y ) + inv32Table4[range][1];
        y = smpy32(x , y ) + inv32Table4[range][0];
#else
        y = inv32Table[range][3];
        y = smpy32(x , y ) + inv32Table[range][2];
        y = smpy32(x , y ) + inv32Table[range][1];
        y = smpy32(x , y ) + inv32Table[range][0];
#endif
        exp = exp + 1 - e;

        if (neg) y = -y;

        int k = norm32(y);
        y = y << k;
        exp = exp - k;

        y = (inf ? 0 : y);
        exp = (inf ? -MAXEXP32 : exp);
        *cmPtr++ = (zero ? MAXINT32 : y);
        *cePtr++ = (zero ? MAXEXP32 : exp);
    }
    return Q6_W_vcombine_VV(cExp, cMant);
}

int invsqrt32Table[16][4] = {
// Quantization is 2147483648.000000 on grid of 32768 points
// input data must be scaled by 2.000000 ^ i for each coefficient
// input data must be generated for each interval by subtracting range value
//---------------- 0.500000 to 0.531250 ----------------------
// { 0.707106761074, -0.353542956837, 0.264316495870, -0.198623667315, },
 {1518500207, -759227719, 567615353, -426541078 },
//---------------- 0.531250 to 0.562500 ----------------------
// { 0.685994325139, -0.322812867116, 0.227223638279, -0.161636476470, },
 {1473161596, -693235353, 487959048, -347111690 },
//---------------- 0.562500 to 0.593750 ----------------------
// { 0.666666654651, -0.296290072230, 0.197025596423, -0.133053863560, },
 {1431655740, -636278085, 423109247, -285730996 },
//---------------- 0.593750 to 0.625000 ----------------------
// { 0.648885675043, -0.273210117459, 0.172158765282, -0.110654733042, },
 {1393471377, -586714260, 369708133, -237629230 },
//---------------- 0.625000 to 0.656250 ----------------------
// { 0.632455524465, -0.252978296796, 0.151471889192, -0.092879647834, },
 {1358187897, -543266756, 325283405, -199457525 },
//---------------- 0.656250 to 0.687500 ----------------------
// { 0.617213393740, -0.235125755556, 0.134103492786, -0.078613435495, },
 {1325455670, -504928715, 287985058, -168821067 },
//---------------- 0.687500 to 0.718750 ----------------------
// { 0.603022684178, -0.219278404983, 0.119399492559, -0.067045324377, },
 {1294981354, -470896789, 256408458, -143978738 },
//---------------- 0.718750 to 0.750000 ----------------------
// { 0.589767820527, -0.205134520071, 0.106856692061, -0.057576959380, },
 {1266516751, -440523027, 229472999, -123645579 },
//---------------- 0.750000 to 0.781250 ----------------------
// { 0.577350265797, -0.192448337517, 0.096083318836, -0.049760692093, },
 {1239850255, -413279658, 206337356, -106860273 },
//---------------- 0.781250 to 0.812500 ----------------------
// { 0.565685422115, -0.181017873049, 0.086771005317, -0.043257426522, },
 {1214800194, -388732922, 186339315, -92894616 },
//---------------- 0.812500 to 0.843750 ----------------------
// { 0.554700193842, -0.170675753512, 0.078674593838, -0.037807344728, },
 {1191209596, -366523390, 168952404, -81190655 },
//---------------- 0.843750 to 0.875000 ----------------------
// { 0.544331051934, -0.161282234505, 0.071597379298, -0.033209267428, },
 {1168942033, -346350961, 153754201, -71316359 },
//---------------- 0.875000 to 0.906250 ----------------------
// { 0.534522482107, -0.152719823738, 0.065380188086, -0.029305898484, },
 {1147878290, -327963324, 140402885, -62933938 },
//---------------- 0.906250 to 0.937500 ----------------------
// { 0.525225729968, -0.144889098598, 0.059893201535, -0.025973142448, },
 {1127913667, -311146970, 128619671, -55776899 },
//---------------- 0.937500 to 0.968750 ----------------------
// { 0.516397778228, -0.137705422005, 0.055029767032, -0.023112271705, },
 {1108955785, -295720142, 118175525, -49633226 },
//---------------- 0.968750 to 1.000000 ---------------------
//{ 0.508000506906, -0.131096341086, 0.050701667453, -0.020644127954, },
 {1090922782, -281527249, 108881002, -44332927 }
};

HVXMATH_API vqf32_t
vqf32_invsqrt(vqf32_t a)
{
    int i, v, e;
    int x, range, y, exp, mant;
    HVX_Vector cMant, cExp;

    int32_t *vptr = (int32_t*)&a;
    int32_t *eptr = vptr + 32;

    int32_t* cmPtr = (int32_t*)&cMant;
    int32_t* cePtr = (int32_t*)&cExp;
    const int32_t isqrt2 = 0x5a827999;

    for (i = 0; i < 32; i++)
    {
        v = *vptr++;
        e = *eptr++;
        int neg = (v < 0);
        int zero = (v == 0);
        int inf = (e >= MAXEXP32);

        // treat negative numbers like infinity, i.e. invsqrt(negative number) = 0.
        inf = inf | neg;

        exp  = norm32(v);
        mant = v << exp;

        range = (mant >> 26) & 0xf;
        x = (mant<<1) & 0x7ffffff;
        y = invsqrt32Table[range][3];
        y = smpy32(x , y ) + invsqrt32Table[range][2];
        y = smpy32(x , y ) + invsqrt32Table[range][1];
        y = smpy32(x , y ) + invsqrt32Table[range][0];
        int32_t y2 = smpy32(isqrt2 , y);
        exp = exp + 1 - e;

        if(!(exp & 1)) y = y2;

        exp = (exp >> 1) + 1;

        int k = norm32(y);
        y = y << k;
        exp = exp - k;

        y = (inf ? 0 : y);
        exp = (inf ? -MAXEXP32 : exp);
        *cmPtr++ = (zero ? MAXINT32 : y);
        *cePtr++ = (zero ? MAXEXP32 : exp);
    }
    return Q6_W_vcombine_VV(cExp, cMant);
}

int sqrt32Table[16][4] = {
//---------------- 0.500000 to 0.531250 ----------------------
// { 0.707106784148, 0.353551858947, -0.088264243640, 0.040946416567, },
 {1518500256, 759246836, -189546020, 87931760 },
//---------------- 0.531250 to 0.562500 ----------------------
// { 0.728868989266, 0.342995924593, -0.080604357564, 0.035342854566, },
 {1565234236, 736578139, -173096540, 75898202 },
//---------------- 0.562500 to 0.593750 ----------------------
// { 0.750000001984, 0.333332308461, -0.073991149025, 0.030756930419, },
 {1610612740, 715825682, -158894783, 66050005 },
//---------------- 0.593750 to 0.625000 ----------------------
// { 0.770551752021, 0.324441990325, -0.068234864911, 0.026962800974, },
 {1654747287, 696733869, -146533257, 57902174 },
//---------------- 0.625000 to 0.656250 ----------------------
// { 0.790569416427, 0.316227051238, -0.063187783956, 0.023792993232, },
 {1697734894, 679092422, -135694733, 51095064 },
//---------------- 0.656250 to 0.687500 ----------------------
// { 0.810092588473, 0.308606095169, -0.058733375324, 0.021121376156, },
 {1739660587, 662726543, -126128963, 45357810 },
//---------------- 0.687500 to 0.718750 ----------------------
// { 0.829156198589, 0.301510828997, -0.054778613245, 0.018851620952, },
 {1780599378, 647489575, -117636176, 40483548 },
//---------------- 0.718750 to 0.750000 ----------------------
// { 0.847791248749, 0.294883469684, -0.051248432777, 0.016909222672, },
 {1820617844, 633257429, -110055171, 36312279 },
//---------------- 0.750000 to 0.781250 ----------------------
// { 0.866025404526, 0.288674752171, -0.048081667108, 0.015235888898, },
 {1859775395, 619924310, -103254594, 32718822 },
//---------------- 0.781250 to 0.812500 ----------------------
// { 0.883883477128, 0.282842380122, -0.045228027711, 0.013785529559, },
 {1898125314, 607399386, -97126450, 29604199},
//---------------- 0.812500 to 0.843750 ----------------------
// { 0.901387819430, 0.277349807708, -0.042645830302, 0.012521349252, },
 {1935715603, 595604177, -91581223, 26889393 },
//---------------- 0.843750 to 0.875000 ----------------------
// { 0.918558654039, 0.272165271950, -0.040300261887, 0.011413710595, },
 {1972589689, 584470471, -86544153, 24510757 },
//---------------- 0.875000 to 0.906250 ----------------------
// { 0.935414347131, 0.267261016911, -0.038162045635, 0.010438544939, },
 {2008787015, 573938664, -81952369, 22416605 },
//---------------- 0.906250 to 0.937500 ----------------------
// { 0.951971638620, 0.262612666347, -0.036206401817, 0.009576156784, },
 {2044343527, 563956407, -77752656, 20564640 },
//---------------- 0.937500 to 0.968750 ----------------------
// { 0.968245836897, 0.258198712369, -0.034412231562, 0.008810314802, },
 {2079292102, 554477513, -73899705, 18920007 },
//---------------- 0.968750 to 1.000000 ----------------------
// { 0.984250984559, 0.254000095593, -0.032761470073, 0.008127554142, },
 {2113662895, 545461052, -70354721, 17453790 }
};

HVXMATH_API vqf32_t
vqf32_sqrt(vqf32_t a)
{
    int i, v, e;
    int x, range, y, exp, mant;
    HVX_Vector cMant, cExp;

    int32_t *vptr = (int32_t*)&a;
    int32_t *eptr = vptr + 32;

    int32_t* cmPtr = (int32_t*)&cMant;
    int32_t* cePtr = (int32_t*)&cExp;
    const int32_t isqrt2 = 0x5a827999;

    for (i = 0; i < 32; i++)
    {
        v = *vptr++;
        e = *eptr++;

        int invalid = (v <= 0) || (abs(e) >= MAXEXP32);

        // leave 0, infinity, and negative numbers unchanged

        exp  = norm32(v);
        mant = v << exp;

        range = (mant >> 26) & 0xf;
        x = (mant<<1) & 0x7ffffff;
        y = sqrt32Table[range][3];
        y = smpy32(x , y ) + sqrt32Table[range][2];
        y = smpy32(x , y ) + sqrt32Table[range][1];
        y = smpy32(x , y ) + sqrt32Table[range][0];

        if (y < 0)
        {
            y = 0x40000000;
            exp -= 2;
        }

        int32_t y2 = smpy32(isqrt2, y);

        exp = e - exp - 1;

        if(!(exp & 1)) y = y2;

        exp = (exp >> 1) + 1;

        int k = norm32(y);
        y = y << k;
        exp = exp - k;

        *cmPtr++ = (invalid ? v : y);
        *cePtr++ = (invalid ? e : exp);
    }
    return Q6_W_vcombine_VV(cExp, cMant);
}

vqf32_t vqf32_from_float(float* src)
{
    vqf32_t v;
    int32_t *mptr = (int32_t*) &v;
    int32_t *eptr = mptr + VLEN/sizeof(*mptr);

    for (uint32_t i = 0; i < 32; i++)
    {
        uint32_t raw = *(uint32_t*)(&src[i]);
        int negative = ((raw >> 31) & 1);
        int32_t tmp_mant = 0x40000000UL;            // implied integer bit
        int32_t tmp_exp = ((raw >> 23) & 0xFF);     // exponent
        if (255 == tmp_exp)
        {
            mptr[i] = (negative ? MININT32 : MAXINT32);
            eptr[i] = MAXEXP32;
        }
        else if (0 == tmp_exp)                           // treat 0 and denormalized numbers as 0
        {
            mptr[i] = 0;
            eptr[i] = -MAXEXP32;
        }
        else
        {
            tmp_mant |= ((raw & 0x7FFFFF) << 7);        // or in the significand
            tmp_exp -= 126;                             // per IEEE format
            if (negative) tmp_mant = -tmp_mant;         // move sign bit into mantissa
            mptr[i] = tmp_mant;
            eptr[i] = tmp_exp;
        }
    }
    return v;
}

void vqf32_to_float(vqf32_t src, float* dst)
{
    int32_t *mptr = (int32_t*) &src;
    int32_t *eptr = mptr + VLEN/sizeof(*mptr);

    for (uint32_t i = 0; i < 32; i++)
    {
        int32_t tmp_mant = mptr[i];
        int32_t tmp_exp =  eptr[i] + 126;

        uint32_t sign = (tmp_mant < 0 ? 1 : 0);
        uint32_t raw = sign << 31;

        // check for +/- infinity
        if (tmp_exp > 254)
        {
            raw |= 0x7F800000; // exponent = 0xFF
        }
        // check for 0 (or very small numbers)
        else if (tmp_exp <= 0)
        {
            raw = 0;
        }
        else
        {
            if (MININT32 == tmp_mant) tmp_mant = MAXINT32;
            else tmp_mant = abs(tmp_mant);

            // add rounding constant
            if (tmp_mant > MAXINT32 - 0x40) tmp_mant = MAXINT32;
            else tmp_mant += 0x40;

            // extract fractional portion
            tmp_mant = (tmp_mant >> 7) & 0x7FFFFF;
            raw |= ((tmp_exp << 23) | tmp_mant);
        }
        dst[i] = *((float*)&raw);
    }
}

vqf32_t vqf32_from_double(double* src)
{
    vqf32_t c;
    int32_t *mptr = (int32_t*) &c;
    int32_t *eptr = mptr + VLEN/sizeof(*mptr);

    for (uint32_t i = 0; i < 32; i++)
    {
        uint64_t raw = *(uint64_t*)(&src[i]);
        int negative = ((raw >> 63) & 1);
        int32_t tmp_mant = 0x40000000UL;            // implied integer bit
        int32_t tmp_exp = ((raw >> 52) & 0x7FF);     // exponent

        if (2047 == tmp_exp)
        {
            mptr[i] = (negative ? MININT32 : MAXINT32);
            eptr[i] = MAXEXP32;
        }
        else if (0 == tmp_exp)                           // treat 0 and denormalized numbers as 0
        {
            mptr[i] = 0;
            eptr[i] = -MAXEXP32;
        }
        else
        {
            tmp_mant |= ((raw & 0xFFFFFFFFFFFFFULL) >> 22);        // or in the significand
            tmp_exp -= 1022;                             // per IEEE format
            if (negative) tmp_mant = -tmp_mant;         // move sign bit into mantissa
            mptr[i] = tmp_mant;
            eptr[i] = tmp_exp;
        }
    }
    return c;
}

void vqf32_to_double(vqf32_t v, double* dst)
{
    int32_t *mptr = (int32_t*) &v;
    int32_t *eptr = mptr + VLEN/sizeof(*mptr);

    for (uint32_t i = 0; i < 32; i++)
    {
        int64_t tmp_mant = mptr[i];
        int64_t tmp_exp =  eptr[i] + 1022;
        uint64_t sign = (tmp_mant < 0 ? 1 : 0);
        uint64_t raw = sign << 63;

        // check for +/- infinity
        if (tmp_exp > 2046)
        {
            raw |= 0x7FF0000000000000ULL; // exponent = 0x7FF
        }
        // check for 0 (or very small numbers)
        else if (tmp_exp <= 0)
        {
            raw = 0;
        }
        else
        {
            if (MININT64 == tmp_mant) tmp_mant = MAXINT64;
            else tmp_mant = llabs(tmp_mant);

            // extract fractional portion
            tmp_mant = (tmp_mant & 0x3FFFFFFFULL) << 22;
            raw |= ((tmp_exp << 52) | tmp_mant);
        }
        dst[i] = *(double*)&raw;
    }
}

static int32_t normamt(int32_t x)
{
    if (0 == x) return 0;

    int32_t inverse = ~x;
    int32_t leading_ones = 0;
    int32_t leading_zeros = 0;

    for (int i = 0; i < 32; i++)
    {
        if ((x & MININT32) == 0) break;
        else
        {
            leading_ones++;
            x <<= 1;
        }
    }

    for (int i = 0; i < 32; i++)
    {
        if ((inverse & MININT32) == 0) break;
        else
        {
            leading_zeros++;
            inverse <<= 1;
        }
    }

    return MAX(leading_ones, leading_zeros) - 1;
}

vqf32_t vqf32_add(vqf32_t a, vqf32_t b)
{
    int32_t *_a_m = (int32_t*) &a;
    int32_t *_a_e = _a_m + VLEN/sizeof(*_a_m);
    int32_t *_b_m = (int32_t*) &b;
    int32_t *_b_e = _b_m + VLEN/sizeof(*_b_m);

    vqf32_t c;
    int32_t *_c_m = (int32_t*) &c;
    int32_t *_c_e = _c_m + VLEN/sizeof(*_c_m);

    for (int j = 0; j < 32; j++)
    {
        int32_t amant = *_a_m++;
        int32_t aexp  = *_a_e++;
        int32_t bmant = *_b_m++;
        int32_t bexp  = *_b_e++;

        // conditional swap so that aexp >= bexp
        if (aexp < bexp)
        {
            int32_t temp = aexp;
            aexp = bexp;
            bexp = temp;
            temp = amant;
            amant = bmant;
            bmant = temp;
        }

        aexp++;
        int32_t cexp = aexp - bexp;

        amant >>= 1;
        bmant >>= cexp;
        int32_t cmant = amant;
        if (cexp <= 31) cmant += bmant;

        int32_t k = normamt(cmant);
        if (0 == cmant) cexp = -MAXEXP32;
        else cexp = aexp - k;
        cmant = cmant << k;

        *_c_m++ = cmant;
        *_c_e++ = cexp;
    }
    return c;
}

vqf32_t vqf32_sub(vqf32_t a, vqf32_t b)
{
    int32_t *_a_m = (int32_t*) &a;
    int32_t *_a_e = _a_m + VLEN/sizeof(*_a_m);
    int32_t *_b_m = (int32_t*) &b;
    int32_t *_b_e = _b_m + VLEN/sizeof(*_b_m);

    vqf32_t c;
    int32_t *_c_m = (int32_t*) &c;
    int32_t *_c_e = _c_m + VLEN/sizeof(*_c_m);

    for (int j = 0; j < 32; j++)
    {
        int32_t amant = *_a_m++;
        int32_t aexp  = *_a_e++;
        int32_t bmant = *_b_m++;
        int32_t bexp  = *_b_e++;

        bmant = (bmant == MININT32) ? MAXINT32 : -bmant;
        // conditional swap so that aexp >= bexp
        if (aexp < bexp)
        {
            int32_t temp = aexp;
            aexp = bexp;
            bexp = temp;
            temp = amant;
            amant = bmant;
            bmant = temp;
        }
        aexp++;
        int32_t cexp = aexp - bexp;

        amant >>= 1;
        bmant >>= cexp;
        int32_t cmant = amant;
        if (cexp <= 31) cmant += bmant;

        int32_t k = normamt(cmant);
        if (0 == cmant) cexp = -MAXEXP32;
        else cexp = aexp - k;
        cmant = cmant << k;

        *_c_m++ = cmant;
        *_c_e++ = cexp;
    }
    return c;
}

vqf32_t vqf32_mpy(vqf32_t a, vqf32_t b)
{
    int32_t *_a_m = (int32_t*) &a;
    int32_t *_a_e = _a_m + VLEN/sizeof(*_a_m);
    int32_t *_b_m = (int32_t*) &b;
    int32_t *_b_e = _b_m + VLEN/sizeof(*_b_m);

    vqf32_t c;
    int32_t *_c_m = (int32_t*) &c;
    int32_t *_c_e = _c_m + VLEN/sizeof(*_c_m);

    for (int j = 0; j < 32; j++)
    {
        int32_t amant = *_a_m++;
        int32_t aexp  = *_a_e++;
        int32_t bmant = *_b_m++;
        int32_t bexp  = *_b_e++;

        int64_t mant_prod = (int64_t)amant * bmant + ((int64_t)1 << 30);
        mant_prod >>= 31;
        mant_prod = MIN(mant_prod, (int64_t)MAXINT32);
        mant_prod = MAX(mant_prod, (int64_t)MININT32);

        int64_t exp_prod = aexp + bexp;
        if (exp_prod < -MAXEXP32 || 0 == mant_prod)
        {
            *_c_e++ = -MAXEXP32;
            *_c_m++ = 0;
        }
        else
        {
            int32_t k = normamt((int32_t)mant_prod);
            *_c_m++ = ((int32_t)mant_prod) << k;
            *_c_e++ = MIN(exp_prod - k,MAXEXP32);
        }
    }
    return c;
}

vqf32_t vqf32_mpyadd(vqf32_t acc, vqf32_t a, vqf32_t b)
{
    int32_t *_a_m = (int32_t*) &a;
    int32_t *_a_e = _a_m + VLEN/sizeof(*_a_m);
    int32_t *_b_m = (int32_t*) &b;
    int32_t *_b_e = _b_m + VLEN/sizeof(*_b_m);

    vqf32_t c;
    int32_t *_c_m = (int32_t*) &c;
    int32_t *_c_e = _c_m + VLEN/sizeof(*_c_m);

    for (int j = 0; j < 32; j++)
    {
        int32_t amant = *_a_m++;
        int32_t aexp  = *_a_e++;
        int32_t bmant = *_b_m++;
        int32_t bexp  = *_b_e++;

        int64_t mant_prod = (int64_t)amant * bmant + ((int64_t)1 << 30);
        mant_prod >>= 31;
        mant_prod = MIN(mant_prod, (int64_t)MAXINT32);
        mant_prod = MAX(mant_prod, (int64_t)MININT32);

        int64_t exp_prod = aexp + bexp;
        if (exp_prod < -MAXEXP32 || 0 == mant_prod)
        {
            *_c_e++ = -MAXEXP32;
            *_c_m++ = 0;
        }
        else if (exp_prod >= MAXEXP32)
        {
            *_c_e++ = MAXEXP32;
            *_c_m++ = (int32_t)mant_prod;
        }
        else
        {
            int32_t k = normamt((int32_t)mant_prod);
            *_c_e++ = exp_prod - k;
            *_c_m++ = ((int32_t)mant_prod) << k;
        }
    }
    return vqf32_add(acc, c);
}

vqf32_t vqf32_mpysub(vqf32_t acc, vqf32_t a, vqf32_t b)
{
    int32_t *_a_m = (int32_t*) &a;
    int32_t *_a_e = _a_m + VLEN/sizeof(*_a_m);
    int32_t *_b_m = (int32_t*) &b;
    int32_t *_b_e = _b_m + VLEN/sizeof(*_b_m);

    vqf32_t c;
    int32_t *_c_m = (int32_t*) &c;
    int32_t *_c_e = _c_m + VLEN/sizeof(*_c_m);

    for (int j = 0; j < 32; j++)
    {
        int32_t amant = *_a_m++;
        int32_t aexp  = *_a_e++;
        int32_t bmant = *_b_m++;
        int32_t bexp  = *_b_e++;

        int64_t mant_prod = (int64_t)amant * bmant + ((int64_t)1 << 30);
        mant_prod >>= 31;
        mant_prod = MIN(mant_prod, (int64_t)MAXINT32);
        mant_prod = MAX(mant_prod, (int64_t)MININT32);

        int64_t exp_prod = aexp + bexp;
        if (exp_prod < -MAXEXP32 || 0 == mant_prod)
        {
            *_c_e++ = -MAXEXP32;
            *_c_m++ = 0;
        }
        else if (exp_prod >= MAXEXP32)
        {
            *_c_e++ = MAXEXP32;
            *_c_m++ = (int32_t)mant_prod;
        }
        else
        {
            int32_t k = normamt((int32_t)mant_prod);
            *_c_e++ = exp_prod - k;
            *_c_m++ = ((int32_t)mant_prod) << k;
        }
    }
    return vqf32_sub(acc, c);
}

vqf32_t vqf32_negate(vqf32_t a)
{
    int32_t *_a_m = (int32_t*) &a;
    int32_t *_a_e = _a_m + VLEN/sizeof(*_a_m);

    vqf32_t c;
    int32_t *_c_m = (int32_t*) &c;
    int32_t *_c_e = _c_m + VLEN/sizeof(*_c_m);

    for (int j = 0; j < 32; j++)
    {
        int32_t amant = *_a_m++;
        int32_t aexp  = *_a_e++;
        if (MININT32 == amant)
        {
            amant >>= 1;
            aexp += 1;
        }
        amant = -amant;
        *_c_m++ = amant;
        *_c_e++ = aexp;
    }
    return c;
}

vqf32_t vqf32_abs(vqf32_t a)
{
    int32_t *_a_m = (int32_t*) &a;
    int32_t *_a_e = _a_m + VLEN/sizeof(*_a_m);

    vqf32_t c;
    int32_t *_c_m = (int32_t*) &c;
    int32_t *_c_e = _c_m + VLEN/sizeof(*_c_m);

    for (int j = 0; j < 32; j++)
    {
        int32_t amant = *_a_m++;
        int32_t aexp  = *_a_e++;
        if (MININT32 == amant)
        {
            amant >>= 1;
            aexp += 1;
        }
        if (amant < 0) amant = -amant;

        *_c_m++ = amant;
        *_c_e++ = aexp;
    }
    return c;
}

