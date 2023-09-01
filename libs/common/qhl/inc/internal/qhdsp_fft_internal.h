/**=============================================================================
@file
   qhdsp_fft_internal.h

@brief
   Header file for common routines used internally in C implementations of FFT.

Copyright (c) 2019 Qualcomm Technologies Incorporated.
All Rights Reserved. Qualcomm Proprietary and Confidential.
=============================================================================**/

#ifndef _QHML_FFT_INTERNAL_H
#define _QHML_FFT_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#if __hexagon__
#include <hexagon_protos.h>     // intrinsics
#include "qhmath.h"
#include "qhcomplex.h"
#else
#include <math.h>
#include <complex.h>
#endif


// reused from q6basic_op.h
#define   extract_l(x)       ((int16_t)(x))

#if __hexagon__
#define   extract_h(x)       ((int16_t)Q6_R_asrh_R( (x) ))
#else
#define   extract_h(x)       ((x) >> 16)
#endif

#if __hexagon__
#define   cadd(x, y)         (Q6_R_vaddh_RR_sat( (x), (y) ))
#else
#define	  cadd(x, y)	\
({ 				\
	int32_t z; \
	z = saturate((extract_l(x) + extract_l(y))); \
	z |= (saturate((extract_h(x) + extract_h(y))) << 16); \
	z; \
})
#endif

#if __hexagon__
#define   csub(x, y)         (Q6_R_vsubh_RR_sat( (x), (y) ))
#else
#define   csub(x, y) \
({ 				\
	int32_t z; \
	z = saturate((extract_l(x) - extract_l(y))); \
	z |= (saturate((extract_h(x) - extract_h(y))) << 16); \
	z; \
})
#endif

#if __hexagon__
#define   cmult_cr(x, y)      (Q6_R_cmpy_RR_conj_s1_rnd_sat( (x), (y) ))
#else
#define   cmult_cr(x, y) \
({ \
	int32_t z; \
	z =  extract_h(saturate32(((extract_l(x)*extract_l(y))<<1) + ((extract_h(x)*extract_h(y))<<1) + 0x8000)); \
	z |= (extract_h(saturate32(((extract_h(x)*extract_l(y))<<1) - ((extract_l(x)*extract_h(y))<<1) + 0x8000))<<16); \
	z; \
})
#endif

#if __hexagon__
#define   cmult_r(x, y)      (Q6_R_cmpy_RR_s1_rnd_sat( (x), (y) ))
#else
#define   cmult_r(x, y) \
({ \
	int32_t z; \
	z =  extract_h(saturate32(((extract_l(x)*extract_l(y))<<1) - ((extract_h(x)*extract_h(y))<<1) + 0x8000)); \
	z |= (extract_h(saturate32(((extract_h(x)*extract_l(y))<<1) + ((extract_l(x)*extract_h(y))<<1) + 0x8000))<<16); \
	z; \
})
#endif

#if __hexagon__
#define   creal_f(x)      (qhcomplex_creal_f( (x) ) )
#else
#define   creal_f(x)      ( crealf( (x) ) )
#endif

#if __hexagon__
#define   conj_f(x)      (qhcomplex_conj_f( (x) ) )
#else
#define   conj_f(x)      ( conjf( (x) ) )
#endif

#if __hexagon__
#define   cimag_f(x)      (qhcomplex_cimag_f( (x) ) )
#else
#define   cimag_f(x)      ( cimagf( (x) ) )
#endif

static inline int32_t brev(int32_t word)
{
#if __hexagon__
	return Q6_R_brev_R(word);
#else
	__asm__ __volatile__("rbit %0,%1" : "=r" (word) : "r" (word));
	return word;
#endif
}

//#if __hexagon__
//#define   brev(x)            (Q6_R_brev_R( (x) ))
//#else
//#define   brev(x)            (__builtin_rbit( (x) ))
//#endif

#if __hexagon__
#define   SIN(x)            (qhmath_sin_f( (x) ))
#else
#define   SIN(x)            (sinf( (x) ))
#endif

#if __hexagon__
#define   COS(x)            (qhmath_cos_f( (x) ))
#else
#define   COS(x)            (cosf( (x) ))
#endif

#if __hexagon__
#define   LOG2(x)            (qhmath_log2_f( (x) ))
#else
#define   LOG2(x)            (log2f( (x) ))
#endif

#if __hexagon__
#define   LOG10(x)            (qhmath_log10_f( (x) ))
#else
#define   LOG10(x)            (log10f( (x) ))
#endif

#if __hexagon__ // bitfield extract
#define   bf_extr(s, w, o)   (Q6_R_extract_RII( (s), (w), (o) ))
#else
#define   bf_extr(s, w, o)   ((((int32_t)(s)) >> (o)) & ((1 << (w)) - 1))
#endif

//static inline int32_t bf_extr(int32_t src, const int32_t width, const int32_t offset)
//{
//#if __hexagon__
//	return Q6_R_extract_RII(src, width, offset);
//#else
//	if (offset == 0)
//		__asm__ __volatile__("sbfx %[dest], %[src], %[offset], %[width]":[dest]"=r"(src):[src]"r"(src),[offset]"I"(0),[width]"I"(16));
//	else if (offset == 16)
//		__asm__ __volatile__("sbfx %[dest], %[src], %[offset], %[width]":[dest]"=r"(src):[src]"r"(src),[offset]"I"(16),[width]"I"(16));
//	else
//		return 0;
//	return src;
//#endif
//}

#define   real(L_var1)       (extract_l((L_var1)))                   /* Extract real part -- low  16 bits */
#define   imag(L_var1)       (extract_h((L_var1)))                   /* Extract imag part -- high 16 bits */

static inline uint16_t saturate(uint32_t word)
{
#if __hexagon__
	return (int16_t)Q6_R_sath_R(word);
#else
	__asm__ __volatile__("ssat %[srcr], %[satv], %[srcr]\n\t"
				"sxth %[srcr], %[srcr]\n\t"
				:[srcr]"+r"(word):[satv]"I"(16));
	return word;
#endif
}

static inline uint32_t saturate32(uint32_t word)
{
#if __hexagon__
	return (int32_t)Q6_R_sath_R(word);
#else
	__asm__ __volatile__("ssat %[srcr], %[satv], %[srcr]\n\t"
				:[srcr]"+r"(word):[satv]"I"(32));
	return word;
#endif
}

static inline uint32_t sign_extend_16(uint32_t word)
{
#if __hexagon__
	return Q6_R_sxth_R(word);
#else
	__asm__ __volatile__("sxth %0,%1" : "=r" (word) : "r" (word));
	return word;
#endif
}

#define   negate(x)          (saturate(-sign_extend_16((x))))

static inline int32_t conjugate(int64_t in)
{
#if __hexagon__
	return (int32_t)Q6_P_vconj_P_sat(in);
#else
	int32_t l32 = (int32_t) in;
	int16_t lh16 = negate(extract_h(l32));
	int16_t ll16 = extract_l(l32);
	return (((int32_t)lh16) << 16) | ((int32_t)ll16);

#endif
}

#if __hexagon__
#define   combine(h, l)      (Q6_R_combine_RlRl( (h), (l) ))
#else
#define   combine(h, l)      ((l) | ((h) << 16))
#endif
#define   create_complex(var_r, var_i)       (combine((var_i),(var_r)))              /* Construct complex number         */


/** @addtogroup fft_internal_routines FFT internal routines
  * @{
 */

/**
 * @brief       [HELPER FUNCTION] C implementation of bit-reversal process
 * @param[in]   x - number on which bit-reversal is performed
 * @param[in]   bits - number of bits on which bit-reverse is performed
 * @return      y - bit-reversed number
 */
int32_t bitrev(int32_t x, int32_t bits);

/**
 * @brief           [HELPER FUNCTION] Float Radix-2 FFT/IFFT butterfly operation
 * @param[in,out]   x - input/output buffer of 2 complex values on which butterfly operation is done
 * @return
 */
void radix2_btfly_f(float complex *x);

/**
 * @brief           [HELPER FUNCTION] Float Radix-4 FFT butterfly operation
 * @param[in,out]   x - input/output buffer of 4 complex values on which butterfly operation is done
 * @return
 */
void radix4_btfly_f(float complex *x);

/**
 * @brief           [HELPER FUNCTION] Float Radix-4 IFFT butterfly operation
 * @param[in,out]   x - input/output buffer of 4 complex values on which butterfly operation is done
 * @return
 */
void ifft_radix4_btfly_f(float complex *x);

/**
 * @brief           [HELPER FUNCTION] Fixed point complex average of two numbers
 * @param[in,out]   x - input complex num 1, y - input complex num 2
 * @return          (complex) ((x+y)/2)
 */
int32_t cavg( int32_t x, int32_t y );

/**
 * @brief           [HELPER FUNCTION] Fixed point complex negative average of two numbers
 * @param[in,out]   x - input complex num 1, y - input complex num 2
 * @return          (complex) ((x-y)/2)
 */
int32_t cnavg( int32_t x, int32_t y );

/**
 * @brief           [HELPER FUNCTION] Fixed point Radix-2 FFT butterfly operation
 * @param[in,out]   x - input/output buffer of 2 complex values on which butterfly operation is done
 * @return
 */
void radix2_btfly_h(int32_t *x);

/**
 * @brief           [HELPER FUNCTION] Fixed point Radix-4 FFT butterfly operation
 * @param[in,out]   x - input/output buffer of 4 complex values on which butterfly operation is done
 * @return
 */
void radix4_btfly_h_qv3(int32_t *x);

/**
 * @brief           [HELPER FUNCTION] Fixed point Radix-4 FFT butterfly operation
 * @param[in,out]   x - input/output buffer of 4 complex values on which butterfly operation is done
 * @return
 */
void radix4_btfly_h(int32_t *x);

/**
 * @brief           [HELPER FUNCTION] Fixed point Radix-2 IFFT butterfly operation
 * @param[in,out]   x - input/output buffer of 2 complex values on which butterfly operation is done
 * @return
 */
void ifft_radix2_btfly_h( int32_t *x );

/**
 * @brief           [HELPER FUNCTION] Fixed point Radix-4 IFFT butterfly operation
 * @param[in,out]   x - input/output buffer of 4 complex values on which butterfly operation is done
 * @return
 */
void ifft_radix4_btfly_h_qv3( int32_t *x );

/**
 * @brief           [HELPER FUNCTION] Fixed point Radix-4 IFFT butterfly operation
 * @param[in,out]   x - input/output buffer of 4 complex values on which butterfly operation is done
 * @return
 */
void ifft_radix4_btfly_h( int32_t *x );

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* _QHML_FFT_INTERNAL_H */
