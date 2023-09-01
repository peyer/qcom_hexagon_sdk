/**=============================================================================

@file
   qfxp.h

@brief
   API, macros and struct definitions for qfxp utilities available from C.
   
Copyright (c) 2017 QUALCOMM Technologies Incorporated.
All Rights Reserved Qualcomm Proprietary
=============================================================================**/

#ifndef QFXP_H
#define QFXP_H


#ifdef __cplusplus
extern "C"
{
#endif

#ifdef BUILDING_SO
/// MACRO enables function to be visible in shared-library case.
#define QFXP_API __attribute__ ((visibility ("default")))
#else
/// MACRO empty for non-shared-library case.
#define QFXP_API
#endif

/* Function modes */

// Highest-precision mode available.  Likely impractical for an actual implementation
#define QF_IDEAL           0x0000
// Recommended mode for an implementation targetting the DSP without HVX extensions
#define QF_DSP             0x0010
// Lower-resolution mode for an implementation targetting the DSP without HVX extensions
#define QF_DSP_APPROX      0x0011
// Recommended mode for an implementation targetting HVX
#define QF_HVX             0x0100
// Recommended mode for an implementation using higher-precision 32x32->32 multiply from V62 HVX extensions
#define QF_HVX_MULT_32_32  0x0200
// Recommended mode for an implementation using lower-precision 32x16->32 multiply from V62 HVX extensions
#define QF_HVX_MULT_32_16  0x0300


#define QF_NEW_STATS {0,0,-(1<<30),1<<30,0,0,0,0}

// Create signed fixed-point type with m bits for the mantissa 
#define S(size,m) {0,0,1,m,size-1-m,QF_NEW_STATS}

// Create unsigned fixed-point type with m bits for the mantissa 
#define U(size,m) {0,0,0,m,size-m,QF_NEW_STATS}

// Create mask with f bits in the LSBs
#define QF_FRACTIONAL_MASK(f) ((1<<f)-1)

// Display stats information for variable var
#define QF_PRINT_STATS(var) qf_printStats(var,#var)

// Default number of error and warning messages displayed by library at run time
#define QF_MAX_NUM_ERRORS_DISPLAYED 10
#define QF_MAX_NUM_WARNINGS_DISPLAYED 1

// Same as qf_floatToFix but for constants
#define QF_CLAMP(x, n) (x > ((1ul<<n)-1) ? ((1ul<<n)-1) :  (x < ((-1l<<n)) ? ((-1l<<n)) : x))
#define QF(ref, s, m, f)  ((int) QF_CLAMP( (int)((ref*(1ul<<f)+.5)  ) , m+f))

/* Stats on float variable and numerical errors resulting from use of
 * fixed-point arithmetic.
 */
typedef struct qf_stats_t {
	double acc;
	double accAbs;
	double max;
	double min;
	double accError;
	double accAbsError;
	double maxError;
	int count;
} qf_stats_t;

/* Internal representation of fixed-point type (s,m,f).
 *
 * Carries both a float representation of a number and its
 * corresponding fixed-point equivalent.
 *
 *        sign-extension mantissa   fraction
 * raw: [sssssssssssssss.mmmmmmmm.fffffffffff]
 * 
 * In simple terms, ref ~= (float)raw / (1<<f)
 */
typedef struct qf_t {
	float ref;      // float representation
	int raw;        // fixed-point representation
	int s;          // s==1?signed:unsigned
	int m;          // number of integer bits (sign excluded)
	int f;          // number of fractional bits
	qf_stats_t stats;  // stats on numerical approximations
} qf_t;

// Get integer approximating ref as (s,m,f) with optional rounding
QFXP_API int qf_getFxpApprox(float ref, int s, int m, int f, int rnd) ;

/* Print stats for fixed-point variable fp with title 'name' */	  
QFXP_API void qf_printStats(qf_t fp, char* name);

// Create fixed-point type approximating ref as (s,m,f) with optional rounding
QFXP_API float qf_init(qf_t *fp, float ref, int s, int m, int f, int rnd, int mode);

// Get and set fixed-point internal representation of fixed-point type 
QFXP_API int qf_getRaw(qf_t fp);
QFXP_API void qf_setRaw(qf_t* fp, int raw);

// Get float internal representation of fixed-point type 
QFXP_API float qf_getFloatRef(qf_t fp);

// Get float approximation derived from  fixed-point representation 
QFXP_API float qf_getFloatApprox(qf_t fp);

// Get double approximation derived from  fixed-point representation 
QFXP_API double qf_getDoubleApprox(qf_t fp);

// Get integer approximation derived from  fixed-point representation 
QFXP_API int qf_getIntApprox(qf_t fp, int rnd);

// Set fixed-point type representation of floating-point ref with optional rounding
QFXP_API float qf_floatToFix(qf_t *fp, float ref, int rnd, int mode);

// Set fixed-point type representation of another fixed-point type in with optional rounding
QFXP_API float qf_fixToFix(qf_t* res, qf_t in, int rnd);

/* Compute addition.
 *
 * Addition is performed by first aligning input operands to output format. Overflow
 * behavior is undefined.
 */
QFXP_API float qf_add(qf_t* res, qf_t in1, qf_t in2);

/* Compute subtraction.
 *
 * Subtraction is performed by first aligning input operands to output format. Overflow
 * behavior is undefined.
 */
QFXP_API float qf_sub(qf_t* res, qf_t in1, qf_t in2);

/* Multiply fixed-point type by a power of 2.
 *
 * Power of 2 can be positive or negative. If rnd is specified and conversion involves a right-shift.
 * rounding is used.  If the difference between the number of fractional bits in the input and output
 * fixed-point types equals the power of 2, the internal representation of the output will match that
 * of the output.
 */
QFXP_API float qf_multpwr2(qf_t *out, qf_t in, int shift, int rnd);

/* Create fixed-point number made of the fractional part of the input. */
QFXP_API float qf_frac(qf_t* res, qf_t in);	

/* Compute multiplication.
 *
 * The multiplication is a signed multiplication. The following modes are supported:
 *
 *  - QF_IDEAL
 * 
 *    Multiplication is a signed 32x32 -> 64 multiplication.  The 64-bit result is then shifted
 *    back to the relevant 32 bits with optional rounding.
 *
 *  - QF_DSP
 *
 *    Multiplication is a signed fractional 32x32 -> 32 multiplication with saturation performed on
 *    the DSP.  The 32-bit result is further shifted if necessary to the relevant 32 bits with
 *    optional rounding.
 *
 *  - QF_HVX_MULT_32_32
 *
 *    Multiplication is a signed fractional 32x32 -> 32 multiplication made of two 32x16 fractional
 *    multiplications.  The 32-bit result is further shifted if necessary to the relevant 32 bits
 *    with optional rounding.
 *
 *  - QF_HVX_MULT_32_16
 *
 *    Multiplication is a signed fractional 32x32 -> 32 multiplication made of one 32x16 fractional
 *    multiplication.  The 32-bit result is further shifted if necessary to the relevant 32 bits
 *    with optional rounding.
 */
QFXP_API float qf_mult(qf_t* res, qf_t in1, qf_t in2, int rnd, int mode);

/* Compute multiply-accumulate.
 *
 * The multiplication is a signed multiplication. The following modes are supported:
 *
 *  - QF_IDEAL
 * 
 *    Multiplication is a signed 32x32 -> 64 multiplication.  The 64-bit result is then shifted
 *    back to the relevant 32 bits with optional rounding prior to accumulation.
 *
 *  - QF_DSP
 *
 *    Mac is a signed fractional 32x32 -> 32 mac with saturation performed on the DSP.  If a shift
 *    different from a fractional left-shift-by-one is needed, the operation cannot leverage the mac
 *    instruction and a warning is generated. 
 *
 *  - QF_HVX_MULT_32_32 and QF_HVX_MULT_32_16
 *
 *    The mac operation is automatically expanded into a qf_mult followed by an qf_add.
 */
QFXP_API float qf_mac(qf_t* acc, qf_t in1, qf_t in2, int rnd, int mode);

/* Compute inverse.
 *
 * The following modes are supported:
 *
 *  - QF_IDEAL
 * 
 *    Inverse is performed internally using a floating-point divide operation.
 *
 *  - QF_DSP
 * 
 *    Inverse is performed with a library on the DSP without HVX extensions.  f+m must be
 *    no larger than 15.
 *
 *  - QF_DSP_APPROX
 * 
 *    Inverse is performed with a low-precisions library on the DSP without HVX extensions.
 *    f+m must be no larger than 15.
 *
 *  - QF_HVX
 * 
 *    Inverse is performed with a library in HVX extensions. f+m must be no larger than 15.
 *
 */
QFXP_API float qf_invert(qf_t* res, qf_t in, int mode); 

/* Compute square-root.
 *
 * The following modes are supported:
 *
 *  - QF_IDEAL
 * 
 *    Sqrt is performed internally using a floating-point sqrt operation.
 *
 *  - QF_DSP
 * 
 *    Sqrt is performed with a library on the DSP without HVX extensions.
 *
 *  - QF_HVX
 * 
 *    Sqrt is performed with a library in HVX extensions.
 *
 */
QFXP_API float qf_sqrt(qf_t* res, qf_t in, int mode);

#ifdef __cplusplus
}
#endif

#endif  // #ifndef QFXP_H
