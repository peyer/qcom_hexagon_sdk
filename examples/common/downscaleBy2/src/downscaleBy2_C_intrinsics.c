// this file contains a C implementation (with intrinsics and pre-fetching) of the
// assembly function down2. This is for
// illustration/comparative profiling only, and should not be linked into the image
// unless the assembly version is removed

#define L2FETCH(ADDR, REG)   asm("	l2fetch(%0, %1)\n"	:: "r" (ADDR), "r" (REG)	);

#include "downscaleBy2_asm.h"
#include "dspcache.h"               // contains assembly for cache pre-fetching.
#include "hexagon_types.h"
#include "hexagon_protos.h"         // part of Q6 tools, contains intrinsics definitions
#include <assert.h>

void
down2( const unsigned  char *imgSrc, unsigned int width, unsigned int height, 
    unsigned int stride, unsigned  char *imgDst, unsigned int dstStride)
{
    // make sure alignements meet requirements
    assert((((unsigned int)imgSrc & 3) == 0) && (((unsigned int)imgDst & 7) == 0) && ((stride & 3) == 0) && ((dstStride & 7) == 0));

    unsigned int i,j;
    for (i = 0; i < height; i+=2)
    {
        unsigned int *topPtr = (unsigned int *)(&(imgSrc[i * stride]));

        // Issue L2 prefetch for current 2 and next 2 source rows
        dspcache_box_l2fetch((unsigned char*)topPtr, width, 4, stride);

        unsigned int *botPtr = topPtr + (stride / 4);
        unsigned int *dstPtr = (unsigned int *)imgDst;

        // clear cache lines for current destination row.
        dspcache_linear_dczeroa((unsigned char*)dstPtr, width / 2);
        for (j = 0; j < width/8; j++)
        {
            // Issue L1 prefetch for next cache line. Alternate between top and bottom rows.
            unsigned char *dcfetchAddr = (0 == (j & 1)) ? (unsigned char *)topPtr : (unsigned char*)botPtr;
            Q6_dcfetch_A(dcfetchAddr + 32);

            // Read 8x2 source pixels and vector zero extend to 16 bits each
            unsigned long long top1 = Q6_P_vzxtbh_R(*topPtr++);
            unsigned long long bot1 = Q6_P_vzxtbh_R(*botPtr++);
            unsigned long long top2 = Q6_P_vzxtbh_R(*topPtr++);
            unsigned long long bot2 = Q6_P_vzxtbh_R(*botPtr++);

            // Vector-reduce add into 4 32-bit sums, one for each 2x2 pixel block
            top1 = Q6_P_vraddub_PP(top1, bot1);
            top2 = Q6_P_vraddub_PP(top2, bot2);

            // vector shift to divide each sum by 4, and pack each to 16 bits
            top1 = Q6_R_vasrw_PI(top1,2) | ((unsigned long long)Q6_R_vasrw_PI(top2,2) << 32);

            // pack to 8 bits and output 4 dst pixels
            *dstPtr++ = Q6_R_vtrunehb_P(top1);
        }

        // finish any extra samples if (width % 8) != 0
        if (0 != (width % 8))
        {
            unsigned short *tPtr = (unsigned short*) topPtr;
            unsigned short *bPtr = (unsigned short*) topPtr;
            unsigned char *dPtr = (unsigned char*) dstPtr;
            for (j = 0; j < width % 8; j+=2)
            {
                unsigned short tp = *tPtr++;
                unsigned short bp = *bPtr++;
                unsigned int sum = (tp & 0xFF) + (bp & 0xFF) + ((tp >> 8) & 0xFF) + ((bp >> 8) & 0xFF);
                *dPtr++ = ((sum >> 2) & 0xFF);
            }
        }
        imgDst += dstStride;
    }
    return;
}

void
down2_hvx( const unsigned  char *__restrict imgSrc, unsigned int width, unsigned int height,
    unsigned int stride, unsigned  char *__restrict imgDst, unsigned int dstStride, unsigned int VLEN)
{
#if (__HEXAGON_ARCH__ >= 60)
    HEXAGON_Vect32 const_0x40404040 = 0x40404040;

    uint64_t L2FETCH_REGISTER = (1ULL <<48) | ((uint64_t)stride<<32) | ((uint64_t)width<<16) | 2ULL;
    L2FETCH(imgSrc, L2FETCH_REGISTER);

    int i,j;
    unsigned int log2VLEN = Q6_R_ct0_R(VLEN);
    unsigned int widthIterations = width >> (log2VLEN + 1);
    unsigned int remainingWidth = width - (widthIterations * 2 * VLEN); // width remaining beyond blocks of 2*VLEN
    HVX_VectorPred tailMask = Q6_Q_vsetq_R(remainingWidth >> 1); 		// bitmask for remaining dst width (after downscaling)

    for (i = 0; i < height; i += 2)
    {
        if (i+2 < height)
        {
        	L2FETCH(imgSrc + (2*stride), L2FETCH_REGISTER);
        }
        HVX_Vector *dst = (HVX_Vector *)imgDst;
        HVX_Vector *src0 = (HVX_Vector *)imgSrc;
        HVX_Vector *src1 = (HVX_Vector *)(imgSrc + stride);
        HVX_Vector Vsrc00, Vsrc01, Vsrc10, Vsrc11, Vsum0, Vsum1;

        for (j = 0; j < widthIterations; j++)
        {
        	Vsrc00 = *src0++;
        	Vsrc10 = *src1++;
        	Vsrc01 = *src0++;
        	Vsrc11 = *src1++;

        	Vsum0 = Q6_Vh_vdmpy_VubRb(Vsrc00, const_0x40404040);
        	Vsum1 = Q6_Vh_vdmpy_VubRb(Vsrc01, const_0x40404040);

        	Vsum0 = Q6_Vh_vdmpyacc_VhVubRb(Vsum0, Vsrc10, const_0x40404040);
        	Vsum1 = Q6_Vh_vdmpyacc_VhVubRb(Vsum1, Vsrc11, const_0x40404040);

        	Vsum0 = Q6_Vb_vpacko_VhVh(Vsum1, Vsum0);

        	*dst++ = Vsum0;
        }

        if (remainingWidth > 0)
        {
        	Vsrc00 = *src0++;
        	Vsrc10 = *src1++;

        	if (remainingWidth > VLEN)
        	{
            	Vsrc01 = *src0++;
            	Vsrc11 = *src1++;
        	}

        	Vsum0 = Q6_Vh_vdmpy_VubRb(Vsrc00, const_0x40404040);
        	Vsum1 = Q6_Vh_vdmpy_VubRb(Vsrc01, const_0x40404040);

        	Vsum0 = Q6_Vh_vdmpyacc_VhVubRb(Vsum0, Vsrc10, const_0x40404040);
        	Vsum1 = Q6_Vh_vdmpyacc_VhVubRb(Vsum1, Vsrc11, const_0x40404040);

        	Vsum0 = Q6_Vb_vpacko_VhVh(Vsum1, Vsum0);

        	Vsum1 = *dst;
        	Vsum0 = Q6_V_vmux_QVV(tailMask, Vsum0, Vsum1);
        	*dst++ = Vsum0;
        }

        imgSrc += 2 * stride;
        imgDst += dstStride;
    }
#endif
}
