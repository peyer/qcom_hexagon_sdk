/* ======================================================================== */
/*  QUALCOMM TECHNOLOGIES, INC.                                             */
/*                                                                          */
/*  HEXAGON HVX Image/Video Processing Library                              */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*          Copyright (c) 2014 QUALCOMM TECHNOLOGIES Incorporated.          */
/*                           All Rights Reserved.                           */
/*                  QUALCOMM Confidential and Proprietary                   */
/* ======================================================================== */

/*[========================================================================]*/
/*[ FUNCTION                                                               ]*/
/*[     IntegrateImage                                                     ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function calculates a 2D integration of an image.             ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     AUG-01-2014                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/
#include "hexagon_types.h"
#include "hvx.cfg.h"
/* ======================================================================== */
/*  Intrinsic C version of IntegrateImage()                                 */
/* ======================================================================== */
const int SPLATpattern[] __attribute__((aligned(128))) = {
#if LOG2VLEN == 7
     0x40404040,
     0x40404040,
     0x40404040,
     0x40404040,
     0x40404040,
     0x40404040,
     0x40404040,
     0x40404040,
     0x40404040,
     0x40404040,
     0x40404040,
     0x40404040,
     0x40404040,
     0x40404040,
     0x40404040,
     0x40404040,
     0x20202020,
     0x20202020,
     0x20202020,
     0x20202020,
     0x20202020,
     0x20202020,
     0x20202020,
     0x20202020,
     0x10101010,
     0x10101010,
     0x10101010,
     0x10101010,
     0x08080808,
     0x08080808,
     0x04040404,
     0x00000000
#else
     0x20202020,
     0x20202020,
     0x20202020,
     0x20202020,
     0x20202020,
     0x20202020,
     0x20202020,
     0x20202020,
     0x10101010,
     0x10101010,
     0x10101010,
     0x10101010,
     0x08080808,
     0x08080808,
     0x04040404,
     0x00000000
#endif
};


/* ======================================================================== */
void IntegratePerRow(
    unsigned char   *restrict src,
    int              width,
    unsigned int    *restrict dst
    )
{
    int i;
    int const1 = 0x01010101;

    HVX_Vector sPixels, sEvenPixels;
    HVX_Vector sSum, sD1Sum, sD2Sum, sD4Sum, sD8Sum, sD16Sum;
#if LOG2VLEN == 7
    HVX_Vector sD32Sum;
#endif
    HVX_Vector sPIntgOdd, sPIntgEven, sD1PItgOdd, sPIntg0, sPIntg1;
    HVX_Vector sPrvIntg, sIntg0, sIntg1, sIntg2, sIntg3;
    HVX_Vector zero, mask, pattern;
    HVX_VectorPair dPIntg, dPIntguw01, dPIntguw23;

    HVX_Vector *pIn  = (HVX_Vector *)src;
    HVX_Vector *pOut = (HVX_Vector *)dst;

    zero = Q6_V_vzero();
    mask = Q6_V_vsplat_R(0x00FF00FF);
    pattern = *((HVX_Vector *)SPLATpattern);

    sIntg3 = Q6_V_vzero();

    for ( i=0; i< width; i+= VLEN )
    {
        sPixels= *pIn++;
        sSum = Q6_Vh_vdmpy_VubRb(sPixels,const1);
        sD1Sum = Q6_V_vlalign_VVI(sSum,zero,2);
        sSum   = Q6_Vh_vadd_VhVh(sSum,sD1Sum);
        sD2Sum = Q6_V_vlalign_VVI(sSum,zero,4);
        sSum   = Q6_Vh_vadd_VhVh(sSum,sD2Sum);
        sD4Sum = Q6_V_vlalign_VVR(sSum,zero,8);
        sSum   = Q6_Vh_vadd_VhVh(sSum,sD4Sum);
        sD8Sum = Q6_V_vlalign_VVR(sSum,zero,16);
        sSum   = Q6_Vh_vadd_VhVh(sSum,sD8Sum);
        sD16Sum = Q6_V_vlalign_VVR(sSum,zero,32);
#if LOG2VLEN == 6
        sPIntgOdd = Q6_Vh_vadd_VhVh(sSum,sD16Sum);
#else
        sSum   = Q6_Vh_vadd_VhVh(sSum,sD16Sum);
        sD32Sum = Q6_V_vlalign_VVR(sSum,zero,64);
        sPIntgOdd = Q6_Vh_vadd_VhVh(sSum,sD32Sum);
#endif
        sEvenPixels = Q6_V_vand_VV(sPixels,mask);
        sD1PItgOdd = Q6_V_vlalign_VVI(sPIntgOdd,zero,2);
        sPIntgEven = Q6_Vh_vadd_VhVh(sD1PItgOdd,sEvenPixels);

        dPIntg = Q6_W_vshuff_VVR(sPIntgOdd,sPIntgEven,-2);

        sPrvIntg = Q6_V_vrdelta_VV(sIntg3,pattern);
        dPIntguw01 = Q6_Wuw_vunpack_Vuh(Q6_V_lo_W(dPIntg));
        dPIntguw23 = Q6_Wuw_vunpack_Vuh(Q6_V_hi_W(dPIntg));

        sIntg0 = Q6_Vw_vadd_VwVw(sPrvIntg,Q6_V_lo_W(dPIntguw01));
        sIntg1 = Q6_Vw_vadd_VwVw(sPrvIntg,Q6_V_hi_W(dPIntguw01));
        sIntg2 = Q6_Vw_vadd_VwVw(sPrvIntg,Q6_V_lo_W(dPIntguw23));
        sIntg3 = Q6_Vw_vadd_VwVw(sPrvIntg,Q6_V_hi_W(dPIntguw23));

        *pOut++ = sIntg0;
        *pOut++ = sIntg1;
        *pOut++ = sIntg2;
        *pOut++ = sIntg3;
    }
}


/* ======================================================================== */
void IntegrateVertical(
    unsigned int    *iimgPrev,
    unsigned int    *iimg,
    int              stride,
    int              width,
    int              h
    )
{
    int i, j;
    unsigned int *cur;

    HVX_Vector sSum;
    HVX_Vector *prv = (HVX_Vector *)iimgPrev;

    for (i=0; i<width; i+=(VLEN/4))
    {
        cur  = &iimg[i];
        sSum = *prv++;

        for (j=0; j<h; j++)
        {
           sSum = Q6_Vw_vadd_VwVw(sSum, *((HVX_Vector *)cur));
           *((HVX_Vector *)cur) = sSum;
           cur += stride;
        }
    }
}


/* ======================================================================== */
void IntegrateImage(
    unsigned char   *src,
    int              srcStride,
    int              srcWidth,
    int              srcHeight,
    unsigned int    *dst,
    int              dstStride
    )
{
   int i, j, n, k;
   unsigned int sum;
   unsigned char *img;
   unsigned int  *iimg;

   // First row
   img  = src;
   iimg = dst;
   IntegratePerRow( img, srcWidth, iimg );

   iimg += dstStride;
   img  += srcStride;

   // remaining rows
   n = 4;

   for ( i=1; i<srcHeight; i+=n )
   {
      k = (srcHeight-i) > n ? n : (srcHeight-i);

      // split into horizontal/vertical integration in order to do MT implementation
      for (j=0; j<k; j++)
      {
        IntegratePerRow( img + j*srcStride, srcWidth, iimg + j*dstStride );
      }

      IntegrateVertical( iimg-dstStride, iimg, dstStride, srcWidth, k );

      img  += n*srcStride;
      iimg += n*dstStride;
   }
}


