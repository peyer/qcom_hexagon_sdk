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
/*[     wiener9x9                                                          ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function applies a 9x9 kernel of Wiener filter to an image.   ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     AUG-01-2014                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/
#include <assert.h>
#include <stdlib.h>
#include "hvx.cfg.h"
#include "wiener9x9.h"
#include "io.h"
#include "hexagon_types.h"

/* ======================================================================== */
/*  Intrinsic C version.                                                    */
/* ======================================================================== */
void deltainit(
    unsigned char  * __restrict src,
    unsigned short * __restrict rgm,
    unsigned       * __restrict rgm2,
    int             width,
    int             stride
    )
{
    const int window_2 = WINDOWS / 2;
    int loop = (width + VLEN - 1) >> LOG2VLEN;
    int x;
    int const1 = 0x1010101;
    HVX_Vector *pin = (HVX_Vector*)(&src[-5*stride]);
    HVX_Vector *prgm = (HVX_Vector*)rgm;
    HVX_Vector *prgm2 = (HVX_Vector*)rgm2;
    int vec_stride = stride/VLEN;
    int ones = 0x01010101;

    for (x = 0; x < loop; x++)
    {
        HVX_Vector in_5, in_4, in_3, in_2, in_1, in0, in1, in2, in3;
        HVX_Vector tmp0L, tmp0H, sqsum0, sqsum1, sqsum2, sqsum3;
        HVX_VectorPair tmp0, tmp1, tmp2, tmp3, sumx0, sqsum02, sqsum13;

        in_5 = pin[0*vec_stride + x];
        in_4 = pin[1*vec_stride + x];
        in_3 = pin[2*vec_stride + x];
        in_2 = pin[3*vec_stride + x];
        in_1 = pin[4*vec_stride + x];
        in0  = pin[5*vec_stride + x];
        in1  = pin[6*vec_stride + x];
        in2  = pin[7*vec_stride + x];
        in3  = pin[8*vec_stride + x];

        sumx0 = Q6_Wh_vmpa_WubRb(Q6_W_vcombine_VV(in_4, in_5), ones);
        sumx0 = Q6_Wh_vmpaacc_WhWubRb(sumx0, Q6_W_vcombine_VV(in_2, in_3), ones);
        sumx0 = Q6_Wh_vmpaacc_WhWubRb(sumx0, Q6_W_vcombine_VV(in0, in_1), ones);
        sumx0 = Q6_Wh_vmpaacc_WhWubRb(sumx0, Q6_W_vcombine_VV(in2, in1), ones);
        sumx0 = Q6_Wh_vmpyacc_WhVubRb(sumx0, in3, ones);

        tmp0 = Q6_Wb_vshuffoe_VbVb(in_4, in_5);
        tmp1 = Q6_Wb_vshuffoe_VbVb(in_2, in_3);
        in_5 = Q6_Vh_vshuffe_VhVh(Q6_V_lo_W(tmp1), Q6_V_lo_W(tmp0));
        in_4 = Q6_Vh_vshuffo_VhVh(Q6_V_lo_W(tmp1), Q6_V_lo_W(tmp0));
        in_3 = Q6_Vh_vshuffe_VhVh(Q6_V_hi_W(tmp1), Q6_V_hi_W(tmp0));
        in_2 = Q6_Vh_vshuffo_VhVh(Q6_V_hi_W(tmp1), Q6_V_hi_W(tmp0));
        sqsum0 = Q6_Vuw_vrmpy_VubVub(in_5, in_5);
        sqsum2 = Q6_Vuw_vrmpy_VubVub(in_4, in_4);
        sqsum1 = Q6_Vuw_vrmpy_VubVub(in_3, in_3);
        sqsum3 = Q6_Vuw_vrmpy_VubVub(in_2, in_2);

        tmp0L = Q6_Vb_vshuffe_VbVb(in0, in_1);
        tmp0H = Q6_Vb_vshuffo_VbVb(in0, in_1);
        tmp1 = Q6_Wb_vshuffoe_VbVb(in1, in2);
        in_1 = Q6_Vh_vshuffe_VhVh(Q6_V_lo_W(tmp1), tmp0L);
        in0 = Q6_Vh_vshuffo_VhVh(Q6_V_lo_W(tmp1), tmp0L);
        in1 = Q6_Vh_vshuffe_VhVh(Q6_V_hi_W(tmp1), tmp0H);
        in2 = Q6_Vh_vshuffo_VhVh(Q6_V_hi_W(tmp1), tmp0H);
        sqsum0 = Q6_Vuw_vrmpyacc_VuwVubVub(sqsum0, in_1, in_1);
        sqsum2 = Q6_Vuw_vrmpyacc_VuwVubVub(sqsum2, in0, in0);
        sqsum1 = Q6_Vuw_vrmpyacc_VuwVubVub(sqsum1, in1, in1);
        sqsum3 = Q6_Vuw_vrmpyacc_VuwVubVub(sqsum3, in2, in2);

        tmp2 = Q6_Wuh_vmpy_VubVub(in3, in3);
        tmp3 = Q6_Wuw_vzxt_Vuh(Q6_V_lo_W(tmp2));
        tmp2 = Q6_Wuw_vzxt_Vuh(Q6_V_hi_W(tmp2));
        sqsum3 = Q6_Vw_vadd_VwVw(sqsum3, Q6_V_hi_W(tmp2));
        sqsum0 = Q6_Vw_vadd_VwVw(sqsum0, Q6_V_lo_W(tmp3));
        sqsum1 = Q6_Vw_vadd_VwVw(sqsum1, Q6_V_lo_W(tmp2));
        sqsum2 = Q6_Vw_vadd_VwVw(sqsum2, Q6_V_hi_W(tmp3));
        sqsum02 = Q6_W_vshuff_VVR(sqsum2, sqsum0, -4);
        sqsum13 = Q6_W_vshuff_VVR(sqsum3, sqsum1, -4);

        prgm[2*x+0] = Q6_V_lo_W(sumx0);
        prgm[2*x+1] = Q6_V_hi_W(sumx0);

        prgm2[4*x+0] = Q6_V_lo_W(sqsum02);
        prgm2[4*x+1] = Q6_V_lo_W(sqsum13);
        prgm2[4*x+2] = Q6_V_hi_W(sqsum02);
        prgm2[4*x+3] = Q6_V_hi_W(sqsum13);
    }
}

/* ======================================================================== */
void vertboxfiltervarcomp(
    unsigned char  * __restrict src,
    unsigned short * __restrict rgm,
    unsigned       * __restrict rgm2,
    int             width,
    int             stride
    )
{
    int x;
    int loop = (width + VLEN - 1) >> LOG2VLEN;
    HVX_Vector *prgm = (HVX_Vector*)rgm;
    HVX_Vector *prgm2 = (HVX_Vector*)rgm2;
    HVX_Vector *pinold = (HVX_Vector*)(&src[-5*stride]);
    HVX_Vector *pinnew = (HVX_Vector*)(&src[4*stride]);

    for (x = 0; x < loop; x++)
    {
        HVX_Vector in0, in0p, in1, in1p, rgma, rgmb, rgm2a, rgm2b, rgm2c, rgm2d;
        HVX_VectorPair m, xsq0, xsq1, delta0, delta1;

        in0 = pinold[x];
        in1 = pinnew[x];
        in0p = Q6_Vb_vshuff_Vb(in0);
        in1p = Q6_Vb_vshuff_Vb(in1);

        m = Q6_Wh_vsub_VubVub(in1, in0);

        xsq0 = Q6_Wuh_vmpy_VubVub(in0p, in0p);
        xsq1 = Q6_Wuh_vmpy_VubVub(in1p, in1p);

        rgma = prgm[2*x+0];
        rgmb = prgm[2*x+1];
        prgm[2*x+0] = Q6_Vh_vadd_VhVh(Q6_V_lo_W(m), rgma);
        prgm[2*x+1] = Q6_Vh_vadd_VhVh(Q6_V_hi_W(m), rgmb);

        delta0 = Q6_Ww_vsub_VuhVuh(Q6_V_lo_W(xsq1), Q6_V_lo_W(xsq0));
        delta1 = Q6_Ww_vsub_VuhVuh(Q6_V_hi_W(xsq1), Q6_V_hi_W(xsq0));
        rgm2a = prgm2[4*x+0];
        rgm2b = prgm2[4*x+1];
        rgm2c = prgm2[4*x+2];
        rgm2d = prgm2[4*x+3];
        prgm2[4*x+0] = Q6_Vw_vadd_VwVw(rgm2a, Q6_V_lo_W(delta0));
        prgm2[4*x+1] = Q6_Vw_vadd_VwVw(rgm2b, Q6_V_hi_W(delta0));
        prgm2[4*x+2] = Q6_Vw_vadd_VwVw(rgm2c, Q6_V_lo_W(delta1));
        prgm2[4*x+3] = Q6_Vw_vadd_VwVw(rgm2d, Q6_V_hi_W(delta1));
    }
}

/* ======================================================================== */
void horboxfilter(
    unsigned char  * __restrict rgmean,
    unsigned short * __restrict rgm,
    int             width
    )
{
    const int window_2 = WINDOWS / 2;
    int loop = (width + VLEN - 1) >> LOG2VLEN;
    int x;
    HVX_Vector *prgm = (HVX_Vector *)rgm;
    HVX_Vector odd0, odd1, even0s, even0, even1, L0, evenodd0, evenodd1, s10, sum, sumo, sume;
    HVX_Vector outalign, outdelay, outL, delay1, delay2, delay3;
    HVX_Vector *prgmean = (HVX_Vector*)rgmean;
    int const809 = (809<<16)+809;

    even0 = *prgm++;
    odd1 = *prgm++;
    L0 = *prgm++;
    even0s = Q6_V_valign_VVR(L0, even0, 2);
    even0 = L0;
    evenodd1 = Q6_Vh_vadd_VhVh(even0s, odd1);

    outL = Q6_V_vzero();
    outdelay = outL;

    for (x = 0; x < loop; x++)
    {
        L0 = prgm[1];
        even0s = Q6_V_valign_VVR(L0, even0, 2);

        odd0 = *prgm; prgm += 2;
        evenodd0 = Q6_Vh_vadd_VhVh(even0s, odd0);

        delay1 = Q6_V_valign_VVR(evenodd0, evenodd1, 2);
        delay2 = Q6_V_valign_VVR(evenodd0, evenodd1, 4);
        delay3 = Q6_V_valign_VVR(evenodd0, evenodd1, 6);

        sum = Q6_Vh_vadd_VhVh(evenodd1, delay1);
        sum = Q6_Vh_vadd_VhVh(sum, delay2);
        sum = Q6_Vh_vadd_VhVh(sum, delay3);

        even0 = L0;
        evenodd1 = evenodd0;

        s10 = Q6_V_valign_VVR(odd0, odd1, 8);
        odd1 = odd0;

        even1 = prgm[-5];
        sumo = Q6_Vh_vadd_VhVh(sum, s10);
        sume = Q6_Vh_vadd_VhVh(sum, even1);

        sume = Q6_Vh_vmpy_VhRh_s1_sat(sume, const809);
        sumo = Q6_Vh_vmpy_VhRh_s1_sat(sumo, const809);
        outL = Q6_Vub_vasr_VhVhR_rnd_sat(sumo, sume, 1);

        outalign = Q6_V_valign_VVR(outL, outdelay, VLEN-4);
        outdelay = outL;

        prgmean[x] = outalign;
    }
}

/* ======================================================================== */
void horvarcomp(
    unsigned short * __restrict rgvar,
    unsigned       * __restrict rgm2,
    int             width,
    unsigned char  * __restrict rgmean
    )
{
    const int window_2 = WINDOWS / 2;
    int loop = (width + VLEN - 1) >> LOG2VLEN;
    int x;
    int scale_factor = ((809)<<16) + 809;
    HVX_Vector *prgm2 = (HVX_Vector *)rgm2;
    HVX_Vector *prgvar = (HVX_Vector *)rgvar;
    HVX_Vector *prgmean = (HVX_Vector*)rgmean;
    HVX_Vector odd0, odd1, even0s, even0, even1, L0, evenodd0, evenodd1, s10, sum, sumo, sume;
    HVX_Vector varavge, varavgo, mean, half, maxval, outalign, outdelay, outL, delay1, delay2, delay3;
    HVX_VectorPair mean2;
    int *psumo = (int *)&sumo;
    int *psume = (int *)&sume;

    even0 = *prgm2++;
    odd1 = *prgm2++;
    L0 = *prgm2++;
    even0s = Q6_V_valign_VVR(L0, even0, 4);
    even0 = L0;
    evenodd1 = Q6_Vw_vadd_VwVw(even0s, odd1);

    outL = Q6_V_vzero();
    outdelay = outL;

    half = Q6_V_vsplat_R(0x8000);
    maxval = Q6_V_vsplat_R(0x7fff7fff);

    for (x = 0; x < loop; x++)
    {
        L0 = prgm2[1];
        even0s = Q6_V_valign_VVR(L0, even0, 4);

        odd0 = *prgm2; prgm2 += 2;
        evenodd0 = Q6_Vw_vadd_VwVw(even0s, odd0);

        delay1 = Q6_V_valign_VVR(evenodd0, evenodd1, 4);
        delay2 = Q6_V_valign_VVR(evenodd0, evenodd1, 8);
        delay3 = Q6_V_valign_VVR(evenodd0, evenodd1, 12);

        sum = Q6_Vw_vadd_VwVw(evenodd1, delay1);
        sum = Q6_Vw_vadd_VwVw(sum, delay2);
        sum = Q6_Vw_vadd_VwVw(sum, delay3);

        even0 = L0;
        evenodd1 = evenodd0;

        s10 = Q6_V_valign_VVR(odd0, odd1, 16);
        odd1 = odd0;

        even1 = prgm2[-5];
        sume = Q6_Vw_vadd_VwVw(sum, even1);
        sumo = Q6_Vw_vadd_VwVw(sum, s10);

        varavge = half;
        varavge = Q6_Vw_vmpyiacc_VwVwRh(varavge, sume, scale_factor);
        varavgo = half;
        varavgo = Q6_Vw_vmpyiacc_VwVwRh(varavgo, sumo, scale_factor);

        mean = Q6_Vb_vshuff_Vb(prgmean[x]);
        mean2 = Q6_Wuh_vmpy_VubVub(mean, mean);
        outL = Q6_Vh_vshuffo_VhVh(varavgo, varavge);

        outalign = Q6_V_vlalign_VVR(outL, outdelay, 8);
        outdelay = outL;
        outalign = Q6_Vuh_vsub_VuhVuh_sat(outalign, Q6_V_lo_W(mean2));
        outalign = Q6_Vuh_vmin_VuhVuh(outalign, maxval);
        prgvar[2*x+0] = outalign;

        L0 = prgm2[1];
        even0s = Q6_V_valign_VVR(L0, even0, 4);

        odd0 = *prgm2; prgm2 += 2;
        evenodd0 = Q6_Vw_vadd_VwVw(even0s, odd0);

        delay1 = Q6_V_valign_VVR(evenodd0, evenodd1, 4);
        delay2 = Q6_V_valign_VVR(evenodd0, evenodd1, 8);
        delay3 = Q6_V_valign_VVR(evenodd0, evenodd1, 12);

        sum = Q6_Vw_vadd_VwVw(evenodd1, delay1);
        sum = Q6_Vw_vadd_VwVw(sum, delay2);
        sum = Q6_Vw_vadd_VwVw(sum, delay3);

        even0 = L0;
        evenodd1 = evenodd0;

        s10 = Q6_V_valign_VVR(odd0, odd1, 16);
        odd1 = odd0;

        even1 = prgm2[-5];
        sume = Q6_Vw_vadd_VwVw(sum, even1);
        sumo = Q6_Vw_vadd_VwVw(sum, s10);

        varavge = half;
        varavge = Q6_Vw_vmpyiacc_VwVwRh(varavge, sume, scale_factor);
        varavgo = half;
        varavgo = Q6_Vw_vmpyiacc_VwVwRh(varavgo, sumo, scale_factor);

        outL = Q6_Vh_vshuffo_VhVh(varavgo, varavge);

        outalign = Q6_V_vlalign_VVR(outL, outdelay, 8);
        outdelay = outL;
        outalign = Q6_Vuh_vsub_VuhVuh_sat(outalign, Q6_V_hi_W(mean2));
        outalign = Q6_Vuh_vmin_VuhVuh(outalign, maxval);
        prgvar[2*x+1] = outalign;
    }
}

/* ======================================================================== */
unsigned char dSlope_table[32] __attribute__((aligned(VLEN))) =
{    // delta of val_table
    125, 116, 111, 104, 98, 93, 89, 84,
     80,  76,  72,  70, 66, 63, 61, 58,
     56,  53,  51,  50, 47, 46, 44, 43,
     41,  40,  38,  37, 36, 34, 34, 32
};


unsigned short val_table[32] __attribute__((aligned(VLEN))) =
{
    4096, 3971, 3855, 3744, 3640, 3542, 3449, 3360,
    3276, 3196, 3120, 3048, 2978, 2912, 2849, 2788,
    2730, 2674, 2621, 2570, 2520, 2473, 2427, 2383,
    2340, 2299, 2259, 2221, 2184, 2148, 2114, 2080
};



void reciprocal(
    unsigned short * __restrict input,
    unsigned short * __restrict recipval,
    short          * __restrict recipshft,
    int width
)
{
    HVX_Vector  x0, x1, clb0, clb1;
    HVX_Vector  frac0, frac1, idx0, idx1, subidx;
    HVX_Vector  slopeTab, slope, valueLo, valueHi, tmp, mask, const12;
    HVX_VectorPair valTab, dTmp, dValue, dT4;
    HVX_Vector  *pInput, *pValue, *pNshft;

    int i;

    pInput = (HVX_Vector *)input;
    pValue = (HVX_Vector *)recipval;
    pNshft = (HVX_Vector *)recipshft;

    mask    = Q6_V_vsplat_R(0x001F001F);
    const12 = Q6_V_vsplat_R(0x000C000C);

    slopeTab = Q6_Vb_vshuff_Vb(*((HVX_Vector *)dSlope_table));
    valTab   = Q6_Wuh_vzxt_Vub(*((HVX_Vector *)val_table));

    for (i = 0; i<width; i+=VLEN)
    {
        x0 = *pInput++;
        x1 = *pInput++;

        clb0 = Q6_Vuh_vcl0_Vuh(x0);
        clb1 = Q6_Vuh_vcl0_Vuh(x1);

        *pNshft++ = Q6_Vh_vsub_VhVh(const12,clb0);
        *pNshft++ = Q6_Vh_vsub_VhVh(const12,clb1);

        x0 = Q6_Vh_vasl_VhVh(x0,clb0);
        x1 = Q6_Vh_vasl_VhVh(x1,clb1);

        frac0 = Q6_Vuh_vlsr_VuhR(x0,5);
        frac1 = Q6_Vuh_vlsr_VuhR(x1,5);

        idx0 = Q6_Vuh_vlsr_VuhR(frac0,5);
        idx1 = Q6_Vuh_vlsr_VuhR(frac1,5);
        idx0 = Q6_V_vand_VV(idx0,mask);
        idx1 = Q6_V_vand_VV(idx1,mask);
        subidx = Q6_Vb_vshuffe_VbVb(idx1,idx0);

        frac0 = Q6_V_vand_VV(frac0, mask);
        frac1 = Q6_V_vand_VV(frac1, mask);

        slope   = Q6_Vb_vlut32_VbVbR(subidx,slopeTab,0);
        valueLo = Q6_Vb_vlut32_VbVbR(subidx,Q6_V_lo_W(valTab),0);
        valueHi = Q6_Vb_vlut32_VbVbR(subidx,Q6_V_hi_W(valTab),0);

        dTmp = Q6_Wuh_vmpy_VubVub(slope,Q6_Vb_vshuffe_VbVb(frac1,frac0));

        tmp = Q6_Vub_vasr_VhVhR_rnd_sat(Q6_V_hi_W(dTmp),Q6_V_lo_W(dTmp),5);

        dT4 = Q6_Wuh_vzxt_Vub(tmp);
        dValue = Q6_Wb_vshuffoe_VbVb(valueHi,valueLo);

        dValue = Q6_Wh_vsub_WhWh(dValue,dT4);

        *pValue++ = Q6_V_lo_W(dValue);
        *pValue++ = Q6_V_hi_W(dValue);
    }
}

/* ======================================================================== */
void blending(
    unsigned char  * __restrict src,
    unsigned char  * __restrict rgmean,
    unsigned short * __restrict rgvar,
    short          * __restrict rgshft,
    unsigned char  * __restrict dst,
    unsigned char   noise,
    int             width
    )
{
#define BIT 7
    const int window_2 = WINDOWS / 2;
    int x;
    HVX_Vector *pval = (HVX_Vector *)rgvar;
    HVX_Vector *pshft = (HVX_Vector *)rgshft;
    HVX_Vector *pout = (HVX_Vector *)dst;
    HVX_Vector *pin = (HVX_Vector *)src;
    HVX_Vector *pmean = (HVX_Vector *)rgmean;
    int loop = (width + VLEN - 1) >> LOG2VLEN;
    int noise32 = (noise<<16) | noise;
    HVX_Vector const8, zero, maxvalh, maxvalb;

    maxvalb = Q6_V_vsplat_R((1<<(BIT+24)) | (1<<(BIT+16)) | (1<<(BIT+8)) | (1<<(BIT)));
    maxvalh = Q6_V_vsplat_R((1<<(BIT+16)) | (1<<(BIT)));
    const8 = Q6_V_vsplat_R((8<<16) | 8);
    zero = Q6_V_vzero();

    for (x = 0; x < loop; x++)
    {
        HVX_Vector var0, var1, nvar0L, nvar0H, nvar1L, nvar1H, nv0, nv1, alpha, one_alpha;
        HVX_VectorPair nvar0, nvar1, shft0, shft1, dstv ;
        HVX_VectorPred Q0, Q1;
        var0 = pval[2*x+0];
        var1 = pval[2*x+1];

        nvar0 = Q6_Wuw_vmpy_VuhRuh(var0, noise32);
        nvar1 = Q6_Wuw_vmpy_VuhRuh(var1, noise32);

        shft0 = Q6_Ww_vadd_VuhVuh(const8, pshft[2*x+0]);
        shft1 = Q6_Ww_vadd_VuhVuh(const8, pshft[2*x+1]);

        nvar0L = Q6_Vw_vlsr_VwVw(Q6_V_lo_W(nvar0), Q6_V_lo_W(shft0));
        nvar0H = Q6_Vw_vlsr_VwVw(Q6_V_hi_W(nvar0), Q6_V_hi_W(shft0));
        nvar1L = Q6_Vw_vlsr_VwVw(Q6_V_lo_W(nvar1), Q6_V_lo_W(shft1));
        nvar1H = Q6_Vw_vlsr_VwVw(Q6_V_hi_W(nvar1), Q6_V_hi_W(shft1));

        nv0 = Q6_Vh_vshuffe_VhVh(nvar0H, nvar0L);
        nv1 = Q6_Vh_vshuffe_VhVh(nvar1H, nvar1L);

        Q0 = Q6_Q_vcmp_eq_VhVh(var0, zero);
        Q1 = Q6_Q_vcmp_eq_VhVh(var1, zero);

        nv0 = Q6_V_vmux_QVV(Q0, maxvalh, nv0);
        nv1 = Q6_V_vmux_QVV(Q1, maxvalh, nv1);

        nv0 = Q6_Vub_vpack_VhVh_sat(nv1, nv0);
        alpha = Q6_Vub_vmin_VubVub(nv0, maxvalb);

        one_alpha = Q6_Vub_vsub_VubVub_sat(maxvalb, alpha);
        dstv = Q6_Wuh_vmpy_VubVub(pmean[x], alpha);
        dstv = Q6_Wuh_vmpyacc_WuhVubVub(dstv, pin[x], one_alpha);
        pout[x] = Q6_Vub_vasr_VhVhR_rnd_sat(Q6_V_hi_W(dstv), Q6_V_lo_W(dstv), BIT);
    }
    for (x = 0; x < window_2; x++)
    {
        dst[x] = src[x];
        dst[width - 1 - x] = src[width - 1 - x];
    }
}

/* ======================================================================== */
void filter_wiener9x9 (
    unsigned char      *out,
    unsigned char      *in,
    int                 width,
    int                 stride,
    int                 height,
    const unsigned char noise)
{
    const int window = WINDOWS;
    const int window_2 = window / 2;
    int y;
    int stride1 = (stride + 2*VLEN-1) & (-2*VLEN);

    unsigned char *input = &in[window_2*stride];
    unsigned char *output = &out[window_2*stride];

    unsigned char *rgmean = (unsigned char*)memalign(VLEN, sizeof(rgmean[0]) * stride1);
    unsigned short *rgvar = (unsigned short*)memalign(VLEN, sizeof(rgvar[0]) * stride1);
    unsigned short *rgval = (unsigned short*)memalign(VLEN, sizeof(rgval[0]) * stride1);
    short *rgshft = (short*)memalign(VLEN, sizeof(rgshft[0]) * stride1);
    unsigned short *m = (unsigned short*)memalign(VLEN, sizeof(m[0]) * stride1);
    unsigned *m2 = (unsigned *)memalign(VLEN, sizeof(m2[0]) * stride1);

    assert(rgmean);
    assert(rgvar);
    assert(rgval);
    assert(rgshft);
    assert(m);
    assert(m2);

    if (window != 9) return;

    deltainit(
        input,
        m,
        m2,
        width,
        stride);

    for (y = window_2; y < height - window_2; y++)
    {
        vertboxfiltervarcomp(input, m, m2, width, stride);
        horboxfilter(rgmean, m, width);
        horvarcomp(rgvar, m2, width, rgmean);
        reciprocal(rgvar, rgval, rgshft, stride1);
        blending(input, rgmean, rgval, rgshft, output, noise,width);

        input += stride;
        output += stride;
    }

    free(rgmean);
    free(rgvar);
    free(rgval);
    free(rgshft);
    free(m);
    free(m2);
}
