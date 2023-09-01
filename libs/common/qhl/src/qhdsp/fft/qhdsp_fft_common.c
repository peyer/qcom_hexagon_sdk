/**=============================================================================
@file
   qhdsp_fft_common.c

@brief
   Common routines for complex and real implementations of FFT and IFFT.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

#include "qhdsp_fft_internal.h"

/**
 * @brief           Generate twiddle factors for [float] complex FFT/IFFT functions.
 * @param[in,out]   twiddle - output buffer for storing generated twiddle factors
 * @param[in]       NP - number of samples on which FFT/IFFT is performed
 * @param[in]       log2NP - logarithm of power 2 of number of samples
 * @return
 * @note
 *                  - Generated twiddle factors are used for both FFT and IFFT.
 *                  - Assumptions:
 *                                  1. twiddle - buffer aligned by 8bytes
 *                                  2. twiddle buffer size is 3*NP/4 * [8bytes]
 */
void qhdsp_fft_gen_twiddles_complex_f(float complex* twiddle, int32_t NP, int32_t log2NP)
{
    int32_t i, k, k1, k2;
    float complex x[NP / 2];
    float complex y[NP / 4];

    float ar, ai;

    // Generate twiddles
    // arrange in bit reversed order

    for (i = 0; i < NP / 2; i++)
    {
        k = bitrev(i, log2NP - 1);
        ar =  COS((double)(k) * 2.0 * M_PI / (double)NP);
        ai = -SIN((double)(k) * 2.0 * M_PI / (double)NP);
        x[i] = ar + 1i * ai;
    }

    for (i = 0; i < NP / 4; i++)
    {
        k1 = bitrev((2 * i), log2NP - 1);
        k2 = bitrev(i, log2NP - 1);
        k = k1 + k2;
        ar =  COS((double)(k) * 2.0 * M_PI / (double)NP);
        ai = -SIN((double)(k) * 2.0 * M_PI / (double)NP);
        y[i] = ar + 1i * ai;
    }

    for (i = 0; i < NP / 4; i++)
    {
        twiddle[3 * i + 0] = x[i];
        twiddle[3 * i + 1] = x[2 * i];
        twiddle[3 * i + 2] = y[i];
    }
}

/**
 * @brief           Generate twiddle factors for [float] real FFT/IFFT functions.
 * @param[in,out]   twiddle1 - output buffer for storing generated twiddle factors - for N/2-point complex FFT
 * @param[in,out]   twiddle2 - output buffer for storing generated twiddle factors - for last stage
 * @param[in]       np - number of samples on which FFT/IFFT is performed
 * @param[in]       log2np - logarithm of power 2 of number of samples
 * @note
 *                  - Generated twiddle factors are used for both FFT and IFFT.
 *                  - Assumptions:
 *                                  1. twiddle1 & twiddle2 - buffers aligned by 8bytes
 *                                  2. twiddle1 buffer size is 3*np/8 * [8bytes]
 *                                  3. twiddle2 buffer size is np/2 * [8bytes]
 */
void qhdsp_fft_gen_twiddles_real_f(float complex *twiddle1, float complex *twiddle2, int32_t np, int32_t log2np)
{
    int32_t i, k, k1, k2;
    float ar, ai;

    np     = np / 2;
    log2np = log2np - 1;

    // Generate twiddles for complexFFT
    // arrange in bit reversed order
    for (i = 0; i < np / 4; i++)
    {
        k1 = bitrev(i, log2np - 1);
        ar =  COS((double)(k1) * 2.0 * M_PI / (double)np);
        ai = -SIN((double)(k1) * 2.0 * M_PI / (double)np);
        twiddle1[3 * i + 0] = ar + 1i * ai;

        k2 = bitrev(2 * i, log2np - 1);
        ar =  COS((double)(k2) * 2.0 * M_PI / (double)np);
        ai = -SIN((double)(k2) * 2.0 * M_PI / (double)np);
        twiddle1[3 * i + 1] = ar + 1i * ai;

        k = k1 + k2;
        ar =  COS((double)(k) * 2.0 * M_PI / (double)np);
        ai = -SIN((double)(k) * 2.0 * M_PI / (double)np);
        twiddle1[3 * i + 2] = ar + 1i * ai;
    }

    for (i = 1; i <= np; i++)
    {
        ar = -0.5 * SIN((double)(i) * M_PI / (double)np);
        ai = -0.5 * COS((double)(i) * M_PI / (double)np);
        twiddle2[i - 1] = ar + 1i * ai;
    }
}

/**
 * @brief           Generate twiddle factors for [fixed-point] complex FFT/IFFT functions.
 * @param[in,out]   twiddle - output buffer for storing generated twiddle factors
 * @param[in]       NP - number of samples on which FFT/IFFT is performed
 * @param[in]       log2NP - logarithm of power 2 of number of samples
 * @return
 * @note            
 *                  - Generated twiddle factors are used for both FFT and IFFT.
 *                  - twiddle factors format: Q15 
 *                  - Assumptions:
 *                                  1. twiddle - buffer aligned by 4bytes
 *                                  2. twiddle buffer size is 3*NP/4 * [4bytes]
 */
void qhdsp_fft_gen_twiddles_complex_h(int32_t* twiddle, int32_t NP, int32_t log2NP)
{
    int32_t i, k, k1, k2;

    float complex x[NP / 2 + 1];
    float complex y[NP / 4];
    int32_t fixedPointValue;

    float ar, ai;

    // Generate twiddles
    // arrange in bit reversed order
    
    // 1st stage - calculate float point values
    for (i = 0; i <= NP / 2; i++)
    {
        k = bitrev(i, log2NP - 1);
        ar =  COS((double)(k) * 2.0 * M_PI / (double)NP);
        ai = -SIN((double)(k) * 2.0 * M_PI / (double)NP);
        x[i] = ar + 1i * ai;
    }

    for (i = 0; i < NP / 4; i++)
    {
        k1 = bitrev((2 * i)+1, log2NP - 1);
        k2 = bitrev(i, log2NP - 1);
        k = k1 + k2;
        ar =  COS((double)(k) * 2.0 * M_PI / (double)NP);
        ai = -SIN((double)(k) * 2.0 * M_PI / (double)NP);
        y[i] = ar + 1i * ai;
    }

    // 2nd stage - convert float point values to 16-bit fixed-point values and pack real&imag parts of twiddle factors into one 32bit word
    for (i = 0; i < NP / 4; i++)
    {
        fixedPointValue = create_complex(   (__real__(x[i]) == (float)1.0)?(short)(0x7fff):(short)(__real__(x[i])*32768.0), \
                                            (__imag__(x[i]) == (float)1.0)?(short)(0x7fff):(short)(__imag__(x[i])*32768.0));
        twiddle[3 * i + 0] = fixedPointValue;

        fixedPointValue = create_complex(   (__real__(x[2*i+1]) == (float)1.0)?(int16_t)(0x7fff):(int16_t)(__real__(x[2*i+1])*32768.0)   ,  \
                                            (__imag__(x[2*i+1]) == (float)1.0)?(int16_t)(0x7fff):(int16_t)(__imag__(x[2*i+1])*32768.0)   );
        twiddle[3 * i + 1] = fixedPointValue;

        fixedPointValue = create_complex(   (__real__(y[i]) == (float)1.0)?(int16_t)(0x7fff):(int16_t)(__real__(y[i])*32768.0)   ,  \
                                            (__imag__(y[i]) == (float)1.0)?(int16_t)(0x7fff):(int16_t)(__imag__(y[i])*32768.0)   );
        twiddle[3 * i + 2] = fixedPointValue;
    }
}

/**
 * @brief           Generate twiddle factors for [fixed-point] real FFT/IFFT functions.
 * @param[in,out]   twiddle1 - output buffer for storing generated twiddle factors - for N/2-point complex FFT
 * @param[in,out]   twiddle2 - output buffer for storing generated twiddle factors - for last stage
 * @param[in]       NP - number of samples on which FFT/IFFT is performed
 * @param[in]       log2NP - logarithm of power 2 of number of samples
 * @note            
 *                  - Generated twiddle factors are used for both FFT and IFFT.
 *                  - twiddle factors format: Q15 
 *                  - Assumptions:
 *                                  1. twiddle1 & twiddle2 - buffers aligned by 4bytes
 *                                  2. twiddle1 buffer size is 3*NP/8 * [4bytes]
 *                                  3. twiddle2 buffer size is NP/2 * [4bytes]
 */
void qhdsp_fft_gen_twiddles_real_h(int32_t* twiddle1, int32_t* twiddle2, int32_t NP, int32_t log2NP)
{
    int32_t i, k, k1, k2;

    NP     = NP / 2;
    log2NP = log2NP - 1;

    float complex x[NP / 2 + 1];
    float complex y[NP / 4];
    int32_t fixedPointValue;

    float ar, ai;

    // Generate twiddles
    // arrange in bit reversed order
    
    // 1st stage - calculate float point values
    for (i = 0; i <= NP / 2; i++)
    {
        k = bitrev(i, log2NP - 1);
        ar =  COS((double)(k) * 2.0 * M_PI / (double)NP);
        ai = -SIN((double)(k) * 2.0 * M_PI / (double)NP);
        x[i] = ar + 1i * ai;
    }

    for (i = 0; i < NP / 4; i++)
    {
        k1 = bitrev((2 * i)+1, log2NP - 1);
        k2 = bitrev(i, log2NP - 1);
        k = k1 + k2;
        ar =  COS((double)(k) * 2.0 * M_PI / (double)NP);
        ai = -SIN((double)(k) * 2.0 * M_PI / (double)NP);
        y[i] = ar + 1i * ai;
    }

    // 2nd stage - convert float point values to 16-bit fixed-point values and pack real&imag parts of twiddle factors into one 32bit word
    for (i = 0; i < NP / 4; i++)
    {
        fixedPointValue = create_complex(   (__real__(x[i]) == (float)1.0)?(int16_t)(0x7fff):(int16_t)(__real__(x[i])*32768.0)   ,  \
                                            (__imag__(x[i]) == (float)1.0)?(int16_t)(0x7fff):(int16_t)(__imag__(x[i])*32768.0)   );
        twiddle1[3 * i + 0] = fixedPointValue;

        fixedPointValue = create_complex(   (__real__(x[2*i+1]) == (float)1.0)?(int16_t)(0x7fff):(int16_t)(__real__(x[2*i+1])*32768.0)   ,  \
                                            (__imag__(x[2*i+1]) == (float)1.0)?(int16_t)(0x7fff):(int16_t)(__imag__(x[2*i+1])*32768.0)   );
        twiddle1[3 * i + 1] = fixedPointValue;

        fixedPointValue = create_complex(   (__real__(y[i]) == (float)1.0)?(int16_t)(0x7fff):(int16_t)(__real__(y[i])*32768.0)   ,  \
                                            (__imag__(y[i]) == (float)1.0)?(int16_t)(0x7fff):(int16_t)(__imag__(y[i])*32768.0)   );
        twiddle1[3 * i + 2] = fixedPointValue;
    }

    for (i = 1; i <= NP; i++)
    {
        ar = SIN((double)(i) * M_PI / (double)NP);
        ai = COS((double)(i) * M_PI / (double)NP);

        fixedPointValue = create_complex(   (ar == (float)1.0)?(int16_t)(0x7fff):(int16_t)(ar*32768.0)   ,  \
                                            (ai == (float)1.0)?(int16_t)(0x7fff):(int16_t)(ai*32768.0)   );

        twiddle2[i - 1] = fixedPointValue;
    }
}
