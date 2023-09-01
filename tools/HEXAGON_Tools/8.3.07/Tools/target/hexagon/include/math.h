/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
/* math.h standard header */
#ifndef _MATH
#define _MATH
 #include <xtgmath.h>
#ifndef _YMATH
 #include <ymath.h>
#endif /* _YMATH */

_C_STD_BEGIN
		/* CODES FOR is* FUNCTIONS */
#define _FP_LT	1
#define _FP_EQ	2
#define _FP_GT	4

		/* CODES FOR ilogb FUNCTIONS */

 #if _ILONG
  #define _FP_ILOGB0	(-0x7fffffff - _C2)
  #define _FP_ILOGBNAN	0x7fffffff

 #else /* _ILONG */
  #define _FP_ILOGB0	(-0x7fff - _C2)
  #define _FP_ILOGBNAN	0x7fff
 #endif /* _ILONG */

 #if _HAS_C9X
		/* TYPES */

 #if _FEVAL == 1
typedef double float_t;
typedef double double_t;

 #elif _FEVAL == 2
typedef long double float_t;
typedef long double double_t;

 #else /* _FEVAL */
typedef float float_t;
typedef double double_t;
 #endif /* _FEVAL */

		/* MACROS */

 #if 245 < __EDG_VERSION__ && !defined(__cplusplus)
  #define HUGE_VAL	((double)__INFINITY__)
  #define HUGE_VALF	__INFINITY__
  #define HUGE_VALL	((long double)__INFINITY__)
  #define INFINITY	__INFINITY__
  #define NAN		__NAN__

 #elif defined(__EDG__) || 0 < _MSC_VER
  #define HUGE_VAL	_CSTD _Inf._Double
  #define HUGE_VALF	_CSTD _FInf._Float
  #define HUGE_VALL	_CSTD _LInf._Long_double
  #define INFINITY	_CSTD _FInf._Float
  #define NAN		_CSTD _FNan._Float

 #else /* !defined(__EDG__) */
  #ifndef _HUGE_ENUF
   #define _HUGE_ENUF	1e+300	/* _HUGE_ENUF*_HUGE_ENUF must overflow */
  #endif /* _HUGE_ENUF */

  #define HUGE_VAL	((double)INFINITY)
  #define HUGE_VALF	((float)INFINITY)
  #define HUGE_VALL	((long double)INFINITY)
  #define INFINITY	((float)(_HUGE_ENUF * _HUGE_ENUF))
  #define NAN		((float)(INFINITY * 0.0F))
 #endif /* 245 < __EDG_VERSION__ */

#define FP_INFINITE		_INFCODE
#define FP_NAN			_NANCODE
#define FP_NORMAL		_FINITE
#define FP_SUBNORMAL	_DENORM
#define FP_ZERO			0

 #if _HAS_C9X_FAST_FMA
  #define FP_FAST_FMA	1
  #define FP_FAST_FMAF	1
  #define FP_FAST_FMAL	1
 #endif /* _HAS_C9X_FAST_FMA */

 #define FP_ILOGB0		_FP_ILOGB0
 #define FP_ILOGBNAN	_FP_ILOGBNAN

#define MATH_ERRNO			1
#define MATH_ERREXCEPT		2
#define math_errhandling	(MATH_ERRNO | MATH_ERREXCEPT)	/* do both */

_C_LIB_DECL
int _FFpcomp(float, float) _NO_THROW;
int _Fpcomp(double, double) _NO_THROW;
int _LFpcomp(long double, long double) _NO_THROW;

int _FDclass(float) _NO_THROW __attribute__ ((__const__));
int _Dclass(double) _NO_THROW __attribute__ ((__const__));
int _LDclass(long double) _NO_THROW __attribute__ ((__const__));

int _FDsign(float) _NO_THROW __attribute__ ((__const__));
int _Dsign(double) _NO_THROW __attribute__ ((__const__));
int _LDsign(long double) _NO_THROW __attribute__ ((__const__));
_END_C_LIB_DECL

 #if _HAS_GENERIC_TEMPLATES
extern "C++" {
		// TEMPLATE FUNCTION _FPCOMP
inline int _FPCOMP(float _Left, float _Right)
	{	// compare _Left and _Right
	return (_FFpcomp(_Left, _Right));
	}

inline int _FPCOMP(double _Left, double _Right)
	{	// compare _Left and _Right
	return (_Fpcomp(_Left, _Right));
	}

inline int _FPCOMP(long double _Left, long double _Right)
	{	// compare _Left and _Right
	return (_LFpcomp(_Left, _Right));
	}

template<class _T1, class _T2> inline
	int _FPCOMP(_T1 _Left, _T2 _Right)
	{	// compare _Left and _Right
	typedef typename _Combined_type<float,
		typename _Real_widened<
			typename _Real_type<_T1>::_Type,
			typename _Real_type<_T2>::_Type>::_Type>::_Type _Tw;
	return (_FPCOMP((_Tw)_Left, (_Tw)_Right));
	}

		// FUNCTION fpclassify
inline int fpclassify(float _Left)
	{	// classify argument
	return (_FDtest(&_Left));
	}

inline int fpclassify(double _Left)
	{	// classify argument
	return (_Dtest(&_Left));
	}

inline int fpclassify(long double _Left)
	{	// classify argument
	return (_LDtest(&_Left));
	}

		// FUNCTION signbit
inline bool signbit(float _Left)
	{	// test sign bit
	return (_FDsign(_Left) != 0);
	}

inline bool signbit(double _Left)
	{	// test sign bit
	return (_Dsign(_Left) != 0);
	}

inline bool signbit(long double _Left)
	{	// test sign bit
	return (_LDsign(_Left) != 0);
	}

template<class _Ty> inline
	bool isfinite(_Ty _Left)
	{	// test for finite
	return (fpclassify(_Left) <= 0);
	}

template<class _Ty> inline
	bool isinf(_Ty _Left)
	{	// test for infinite
	return (fpclassify(_Left) == FP_INFINITE);
	}

template<class _Ty> inline
	bool isnan(_Ty _Left)
	{	// test for NaN
	return (fpclassify(_Left) == FP_NAN);
	}

template<class _Ty> inline
	bool isnormal(_Ty _Left)
	{	// test for normal
	return (fpclassify(_Left) == FP_NORMAL);
	}

template<class _Ty1, class _Ty2> inline
	bool isgreater(_Ty1 _Left, _Ty2 _Right)
	{	// test for _Left > _Right
	return ((_FPCOMP(_Left, _Right) & _FP_GT) != 0);
	}

template<class _Ty1, class _Ty2> inline
	bool isgreaterequal(_Ty1 _Left, _Ty2 _Right)
	{	// test for _Left >= _Right
	return ((_FPCOMP(_Left, _Right) & (_FP_EQ | _FP_GT)) != 0);
	}

template<class _Ty1, class _Ty2> inline
	bool isless(_Ty1 _Left, _Ty2 _Right)
	{	// test for _Left < _Right
	return ((_FPCOMP(_Left, _Right) & _FP_LT) != 0);
	}

template<class _Ty1, class _Ty2> inline
	bool islessequal(_Ty1 _Left, _Ty2 _Right)
	{	// test for _Left <= _Right
	return ((_FPCOMP(_Left, _Right) & (_FP_LT | _FP_EQ)) != 0);
	}

template<class _Ty1, class _Ty2> inline
	bool islessgreater(_Ty1 _Left, _Ty2 _Right)
	{	// test for _Left != _Right
	return ((_FPCOMP(_Left, _Right) & (_FP_LT | _FP_GT)) != 0);
	}

template<class _Ty1, class _Ty2> inline
	bool isunordered(_Ty1 _Left, _Ty2 _Right)
	{	// test for _Left unorderd w.r.t. _Right
	return (_FPCOMP(_Left, _Right) == 0);
	}

  #define fpclassify(x) 		(_CSTD fpclassify(x))
  #define signbit(x)			(_CSTD signbit(x))
  #define isfinite(x)			(_CSTD isfinite(x))
  #define isinf(x)				(_CSTD isinf(x))
  #define isnan(x)				(_CSTD isnan(x))
  #define isnormal(x)			(_CSTD isnormal(x))
  #define isgreater(x, y)		(_CSTD isgreater(x, y))
  #define isgreaterequal(x, y)	(_CSTD isgreaterequal(x, y))
  #define isless(x, y)			(_CSTD isless(x, y))
  #define islessequal(x, y)		(_CSTD islessequal(x, y))
  #define islessgreater(x, y)	(_CSTD islessgreater(x, y))
  #define isunordered(x, y)		(_CSTD isunordered(x, y))
}	// extern "C++"

 #else /* _HAS_GENERIC_TEMPLATES */

 #if __clang__
#define _Tg_overload	__attribute__((__overloadable__, __always_inline__))

static int _Tg_overload _Tg_FPCOMP(double _Left, double _Right)
	{return (_Fpcomp(_Left, _Right));}
static int _Tg_overload _Tg_FPCOMP(float _Left, float _Right)
	{return (_FFpcomp(_Left, _Right));}
static int _Tg_overload _Tg_FPCOMP(long double _Left, long double _Right)
	{return (_LFpcomp(_Left, _Right));}

static int _Tg_overload _Tg_fpclassify(double _Left)
	{return (_Dclass(_Left));}
static int _Tg_overload _Tg_fpclassify(float _Left)
	{return (_FDclass(_Left));}
static int _Tg_overload _Tg_fpclassify(long double _Left)
	{return (_LDclass(_Left));}

static int _Tg_overload _Tg_signbit(double _Left)
	{return (_Dsign(_Left));}
static int _Tg_overload _Tg_signbit(float _Left)
	{return (_FDsign(_Left));}
static int _Tg_overload _Tg_signbit(long double _Left)
	{return (_LDsign(_Left));}

  #undef _FPCOMP
  #define _FPCOMP(x, y)	_Tg_FPCOMP(x, y)

  #undef fpclassify
  #define fpclassify(x)	_Tg_fpclassify(x)

  #undef signbit
  #define signbit(x)	_Tg_signbit(x)

 #else /* __clang__ */
  #if __EDG__
   #define _CARGI(x, fd, ff, fl) \
	__generic(x,,, fd, ff, fl,,,)(x)
   #define _CARG2I(x, y, fd, ff, fl) \
	__generic(x, y,, fd, ff, fl,,,)(x, y)

  #elif 2 <= __GNUC__
   #define _FLT_TYPE(expression)	_FLT_OR_DBL(__typeof__ (expression))

  #define _FLT_OR_DBL(mytype)	__typeof__ (*(0 \
	? (__typeof__ (0 ? (mytype *)0 \
		: (void *)!((mytype)0.5 && (mytype)0.5 - 1)))0 \
	: (__typeof__ (0 ? (double *)0 \
		: (void *)((mytype)0.5 && (mytype)0.5 - 1)))0))

   #define _CARGI(x, fd, ff, fl) \
	(__extension__({ \
	int _Ans; \
	if (sizeof (_FLT_TYPE(x)) == sizeof (double)) \
		_Ans = fd(x); \
	else if (sizeof (_FLT_TYPE(x)) == sizeof (float)) \
	_Ans = ff(x); \
	else \
		_Ans = fl(x); \
	_Ans; \
	}))
   #define _CARG2I(x, y, fd, ff, fl) \
	(__extension__({ \
	int _Ans; \
	if (sizeof (_FLT_TYPE((x) + (y))) == sizeof (double)) \
		_Ans = fd(x, y); \
	else if (sizeof (_FLT_TYPE((x) + (y))) == sizeof (float)) \
		_Ans = ff(x, y); \
	else \
		_Ans = fl(x, y); \
	_Ans; \
	}))

  #else /* compiler type */
   #define _ARG(x)	(sizeof ((x) + (float)0) == sizeof (float) ? 'f' \
	: sizeof ((x) + (double)0) == sizeof (double) ? 'd' \
	: 'l')
   #define _CARGI(x, fd, ff, fl)	\
	(_ARG(x) == 'f' ? ff((float)(x)) \
	: _ARG(x) == 'd' ? fd((double)(x)) \
	: fl((long double)(x)))
   #define _CARG2I(x, y, fd, ff, fl)	\
	(_ARG((x) + (y)) == 'f' ? ff((float)(x), (float)(y)) \
	: _ARG((x) + (y)) == 'd' ? fd((double)(x), (double)(y)) \
	: fl((long double)(x), (long double)(y)))
  #endif /* compiler type */

  #define _FPCOMP(x, y) \
	_CARG2I(x, y, _Fpcomp, _FFpcomp, _LFpcomp)
  #define fpclassify(x) \
	_CARGI(x, _Dclass, _FDclass, _LDclass)
  #define signbit(x) \
	_CARGI(x, _Dsign, _FDsign, _LDsign)
 #endif /* __clang__ */

#define isfinite(x)	(fpclassify(x) <= 0)
#define isinf(x)	(fpclassify(x) == FP_INFINITE)
#define isnan(x)	(fpclassify(x) == FP_NAN)
#define isnormal(x)	(fpclassify(x) == FP_NORMAL)

#define isgreater(x, y)	((_FPCOMP(x, y) & _FP_GT) != 0)
#define isgreaterequal(x, y)	\
	((_FPCOMP(x, y) & (_FP_EQ | _FP_GT)) != 0)
#define isless(x, y)	((_FPCOMP(x, y) & _FP_LT) != 0)
#define islessequal(x, y)	((_FPCOMP(x, y) & (_FP_LT | _FP_EQ)) != 0)
#define islessgreater(x, y)	\
	((_FPCOMP(x, y) & (_FP_LT | _FP_GT)) != 0)
#define isunordered(x, y)	(_FPCOMP(x, y) == 0)
 #endif /* _HAS_GENERIC_TEMPLATES */

 #else /* _HAS_C9X */
		/* MACROS */
 #define HUGE_VAL	_CSTD _Hugeval._Double
 #endif /* _HAS_C9X */

_C_LIB_DECL
		/* double declarations */
double acos(double) _NO_THROW;
double asin(double) _NO_THROW;
double atan(double) _NO_THROW;
double atan2(double, double) _NO_THROW;
double ceil(double) _NO_THROW __attribute__ ((__const__));
double exp(double) _NO_THROW;
double fabs(double) _NO_THROW __attribute__ ((__const__));
double floor(double) _NO_THROW __attribute__ ((__const__));
double fmod(double, double) _NO_THROW;
double frexp(double, int *) _NO_THROW;
double ldexp(double, int) _NO_THROW;
double modf(double, double *) _NO_THROW;
double pow(double, double) _NO_THROW;
double sqrt(double) _NO_THROW;
double tan(double) _NO_THROW;
double tanh(double) _NO_THROW;

 #if _HAS_C9X
double acosh(double) _NO_THROW;
double asinh(double) _NO_THROW;
double atanh(double) _NO_THROW;
double cbrt(double) _NO_THROW;
double copysign(double, double) _NO_THROW __attribute__ ((__const__));
double erf(double) _NO_THROW;
double erfc(double) _NO_THROW;
double exp2(double) _NO_THROW;
double expm1(double) _NO_THROW;
double fdim(double, double) _NO_THROW;
double fma(double, double, double) _NO_THROW;
double fmax(double, double) _NO_THROW;
double fmin(double, double) _NO_THROW;
double hypot(double, double) _NO_THROW;
int ilogb(double) _NO_THROW;
double lgamma(double) _NO_THROW;
_Longlong llrint(double) _NO_THROW;
_Longlong llround(double) _NO_THROW;
double log1p(double) _NO_THROW;
double logb(double) _NO_THROW;
long lrint(double) _NO_THROW;
long lround(double) _NO_THROW;
double nan(const char *) _NO_THROW __attribute__ ((__const__));
double nearbyint(double) _NO_THROW;
double nextafter(double, double) _NO_THROW __attribute__ ((__const__));
double nexttoward(double, long double) _NO_THROW __attribute__ ((__const__));
double remainder(double, double) _NO_THROW;
double remquo(double, double, int *) _NO_THROW;
double rint(double) _NO_THROW;
double round(double) _NO_THROW __attribute__ ((__const__));
double scalbn(double, int) _NO_THROW;
double scalbln(double, long) _NO_THROW;
double tgamma(double) _NO_THROW;
double trunc(double) _NO_THROW __attribute__ ((__const__));
 #endif /* _HAS_C9X */

 #if _HAS_TR1_DECLARATIONS
double assoc_laguerre(unsigned, unsigned, double) _NO_THROW;
double assoc_legendre(unsigned, unsigned, double) _NO_THROW;
double beta(double, double) _NO_THROW;
double comp_ellint_1(double) _NO_THROW;
double comp_ellint_2(double) _NO_THROW;
double comp_ellint_3(double, double) _NO_THROW;
double cyl_bessel_i(double, double) _NO_THROW;
double cyl_bessel_j(double, double) _NO_THROW;
double cyl_bessel_k(double, double) _NO_THROW;
double cyl_neumann(double, double) _NO_THROW;
double ellint_1(double, double) _NO_THROW;
double ellint_2(double, double) _NO_THROW;
double ellint_3(double, double, double) _NO_THROW;
double expint(double) _NO_THROW;
double hermite(unsigned, double) _NO_THROW;
double laguerre(unsigned, double) _NO_THROW;
double legendre(unsigned, double) _NO_THROW;
double riemann_zeta(double) _NO_THROW;
double sph_bessel(unsigned, double) _NO_THROW;
double sph_legendre(unsigned, unsigned, double) _NO_THROW;
double sph_neumann(unsigned, double) _NO_THROW;

 #if _HAS_ADDED_FUNCTIONS
double besseli0(double) _NO_THROW;
double besseli1(double) _NO_THROW;
double besselj0(double) _NO_THROW;
double besselj1(double) _NO_THROW;
double besselk0(double) _NO_THROW;
double besselk1(double) _NO_THROW;
double bessely0(double) _NO_THROW;
double bessely1(double) _NO_THROW;

double j0(double) _NO_THROW;
double j1(double) _NO_THROW;
double jn(double) _NO_THROW;
double y0(double) _NO_THROW;
double y1(double) _NO_THROW;
double yn(double) _NO_THROW;
 #elif _HAS_POSIX_C_LIB
double j0(double) _NO_THROW;
double j1(double) _NO_THROW;
double jn(int, double) _NO_THROW;
double y0(double) _NO_THROW;
double y1(double) _NO_THROW;
double yn(int, double) _NO_THROW;
 #endif /* _HAS_ADDED_FUNCTIONS */

 #endif /* _HAS_TR1_DECLARATIONS */

		/* float declarations */
float exp10f(float); // Qualcomm optimized - not part of the original library interface.
// These should be C99 as well, but that causes problems for C++98 builds
float acosf(float) _NO_THROW;
float asinf(float) _NO_THROW;
float atanf(float) _NO_THROW;
float atan2f(float, float) _NO_THROW;
float ceilf(float) _NO_THROW __attribute__ ((__const__));
float cosf(float) _NO_THROW;
float expf(float) _NO_THROW;
float fabsf(float) _NO_THROW __attribute__ ((__const__));
float floorf(float) _NO_THROW __attribute__ ((__const__));
float fmodf(float, float) _NO_THROW;
float frexpf(float, int *) _NO_THROW;
float ldexpf(float, int) _NO_THROW;
float logf(float) _NO_THROW;
float log10f(float) _NO_THROW;
float modff(float, float *) _NO_THROW;
float powf(float, float) _NO_THROW;
float sinf(float) _NO_THROW;
float sqrtf(float) _NO_THROW;
float tanf(float) _NO_THROW;
float tanhf(float) _NO_THROW;

 #if _HAS_C9X
float acoshf(float) _NO_THROW;
float asinhf(float) _NO_THROW;
float atanhf(float) _NO_THROW;
float cbrtf(float) _NO_THROW;
float copysignf(float, float) _NO_THROW __attribute__ ((__const__));
float erff(float) _NO_THROW;
float erfcf(float) _NO_THROW;
float expm1f(float) _NO_THROW;
float exp2f(float) _NO_THROW;
float fdimf(float, float) _NO_THROW;
float fmaf(float, float, float) _NO_THROW;
float fmaxf(float, float) _NO_THROW;
float fminf(float, float) _NO_THROW;
float hypotf(float, float) _NO_THROW;
int ilogbf(float) _NO_THROW;
float lgammaf(float) _NO_THROW;
_Longlong llrintf(float) _NO_THROW;
_Longlong llroundf(float) _NO_THROW;
float log1pf(float) _NO_THROW;
float log2f(float) _NO_THROW;
float logbf(float) _NO_THROW;
float logf(float) _NO_THROW;
long lrintf(float) _NO_THROW;
long lroundf(float) _NO_THROW;
float nanf(const char *) _NO_THROW __attribute__ ((__const__));
float nearbyintf(float) _NO_THROW;
float nextafterf(float, float) _NO_THROW __attribute__ ((__const__));
float nexttowardf(float, long double) _NO_THROW __attribute__ ((__const__));
float remainderf(float, float) _NO_THROW;
float remquof(float, float, int *) _NO_THROW;
float rintf(float) _NO_THROW;
float roundf(float) _NO_THROW __attribute__ ((__const__));
float scalbnf(float, int) _NO_THROW;
float scalblnf(float, long) _NO_THROW;
float tgammaf(float) _NO_THROW;
float truncf(float) _NO_THROW __attribute__ ((__const__));
 #endif /* _HAS_C9X */

 #if _HAS_TR1_DECLARATIONS
float assoc_laguerref(unsigned, unsigned, float) _NO_THROW;
float assoc_legendref(unsigned, unsigned, float) _NO_THROW;
float betaf(float, float) _NO_THROW;
float comp_ellint_1f(float) _NO_THROW;
float comp_ellint_2f(float) _NO_THROW;
float comp_ellint_3f(float, float) _NO_THROW;
float cyl_bessel_if(float, float) _NO_THROW;
float cyl_bessel_jf(float, float) _NO_THROW;
float cyl_bessel_kf(float, float) _NO_THROW;
float cyl_neumannf(float, float) _NO_THROW;
float ellint_1f(float, float) _NO_THROW;
float ellint_2f(float, float) _NO_THROW;
float ellint_3f(float, float, float) _NO_THROW;
float expintf(float) _NO_THROW;
float hermitef(unsigned, float) _NO_THROW;
float laguerref(unsigned, float) _NO_THROW;
float legendref(unsigned, float) _NO_THROW;
float riemann_zetaf(float) _NO_THROW;
float sph_besself(unsigned, float) _NO_THROW;
float sph_legendref(unsigned, unsigned, float) _NO_THROW;
float sph_neumannf(unsigned, float) _NO_THROW;

 #if _HAS_ADDED_FUNCTIONS
float besseli0f(float) _NO_THROW;
float besseli1f(float) _NO_THROW;
float besselj0f(float) _NO_THROW;
float besselj1f(float) _NO_THROW;
float besselk0f(float) _NO_THROW;
float besselk1f(float) _NO_THROW;
float bessely0f(float) _NO_THROW;
float bessely1f(float) _NO_THROW;

double j0f(double) _NO_THROW;
double j1f(double) _NO_THROW;
double jnf(double) _NO_THROW;
double y0f(double) _NO_THROW;
double y1f(double) _NO_THROW;
double ynf(double) _NO_THROW;
 #endif /* _HAS_ADDED_FUNCTIONS */

 #endif /* _HAS_TR1_DECLARATIONS */

		/* long double declarations */
long double acosl(long double) _NO_THROW;
long double asinl(long double) _NO_THROW;
long double atanl(long double) _NO_THROW;
long double atan2l(long double, long double) _NO_THROW;
long double ceill(long double) _NO_THROW __attribute__ ((__const__));
long double expl(long double) _NO_THROW;
long double fabsl(long double) _NO_THROW __attribute__ ((__const__));
long double floorl(long double) _NO_THROW __attribute__ ((__const__));
long double fmodl(long double, long double) _NO_THROW;
long double frexpl(long double, int *) _NO_THROW;
long double ldexpl(long double, int) _NO_THROW;
long double modfl(long double, long double *) _NO_THROW;
long double powl(long double, long double) _NO_THROW;
long double sqrtl(long double) _NO_THROW;
long double tanl(long double) _NO_THROW;
long double tanhl(long double) _NO_THROW;

 #if _HAS_C9X
long double acoshl(long double) _NO_THROW;
long double asinhl(long double) _NO_THROW;
long double atanhl(long double) _NO_THROW;
long double cbrtl(long double) _NO_THROW;
long double copysignl(long double, long double) _NO_THROW __attribute__ ((__const__));
long double erfl(long double) _NO_THROW;
long double erfcl(long double) _NO_THROW;
long double exp2l(long double) _NO_THROW;
long double expm1l(long double) _NO_THROW;
long double fdiml(long double, long double) _NO_THROW;
long double fmal(long double, long double, long double) _NO_THROW;
long double fmaxl(long double, long double) _NO_THROW;
long double fminl(long double, long double) _NO_THROW;
long double hypotl(long double, long double) _NO_THROW;
int ilogbl(long double) _NO_THROW;
long double lgammal(long double) _NO_THROW;
_Longlong llrintl(long double) _NO_THROW;
_Longlong llroundl(long double) _NO_THROW;
long double log1pl(long double) _NO_THROW;
long double logbl(long double) _NO_THROW;
long lrintl(long double) _NO_THROW;
long lroundl(long double) _NO_THROW;
long double nanl(const char *) _NO_THROW __attribute__ ((__const__));
long double nearbyintl(long double) _NO_THROW;
long double nextafterl(long double, long double) _NO_THROW __attribute__ ((__const__));
long double nexttowardl(long double, long double) _NO_THROW __attribute__ ((__const__));
long double remainderl(long double, long double) _NO_THROW;
long double remquol(long double, long double, int *) _NO_THROW;
long double rintl(long double) _NO_THROW;
long double roundl(long double) _NO_THROW __attribute__ ((__const__));
long double scalbnl(long double, int) _NO_THROW;
long double scalblnl(long double, long) _NO_THROW;
long double tgammal(long double) _NO_THROW;
long double truncl(long double) _NO_THROW __attribute__ ((__const__));
 #endif /* _HAS_C9X */

 #if _HAS_TR1_DECLARATIONS
long double assoc_laguerrel(unsigned, unsigned, long double) _NO_THROW;
long double assoc_legendrel(unsigned, unsigned, long double) _NO_THROW;
long double betal(long double, long double) _NO_THROW;
long double comp_ellint_1l(long double) _NO_THROW;
long double comp_ellint_2l(long double) _NO_THROW;
long double comp_ellint_3l(long double, long double) _NO_THROW;
long double cyl_bessel_il(long double, long double) _NO_THROW;
long double cyl_bessel_jl(long double, long double) _NO_THROW;
long double cyl_bessel_kl(long double, long double) _NO_THROW;
long double cyl_neumannl(long double, long double) _NO_THROW;
long double ellint_1l(long double, long double) _NO_THROW;
long double ellint_2l(long double, long double) _NO_THROW;
long double ellint_3l(long double, long double, long double) _NO_THROW;
long double expintl(long double) _NO_THROW;
long double hermitel(unsigned, long double) _NO_THROW;
long double laguerrel(unsigned, long double) _NO_THROW;
long double legendrel(unsigned, long double) _NO_THROW;
long double riemann_zetal(long double) _NO_THROW;
long double sph_bessell(unsigned, long double) _NO_THROW;
long double sph_legendrel(unsigned, unsigned, long double) _NO_THROW;
long double sph_neumannl(unsigned, long double) _NO_THROW;

 #if _HAS_ADDED_FUNCTIONS
long double besseli0l(long double) _NO_THROW;
long double besseli1l(long double) _NO_THROW;
long double besselj0l(long double) _NO_THROW;
long double besselj1l(long double) _NO_THROW;
long double besselk0l(long double) _NO_THROW;
long double besselk1l(long double) _NO_THROW;
long double bessely0l(long double) _NO_THROW;
long double bessely1l(long double) _NO_THROW;

double j0l(double) _NO_THROW;
double j1l(double) _NO_THROW;
double jnl(double) _NO_THROW;
double y0l(double) _NO_THROW;
double y1l(double) _NO_THROW;
double ynl(double) _NO_THROW;
 #endif /* _HAS_ADDED_FUNCTIONS */

 #endif /* _HAS_TR1_DECLARATIONS */
double scalb (double, double) _NO_THROW;
_END_C_LIB_DECL

 #if _HAS_INLINE

 _C_LIB_DECL
 // BEGIN SHARED C++ / C90 inlines
_GNUC_EXTERN_INLINE inline double cos(double _Left)
	{	// return cosine
	return (_Sin(_Left, 1));
	}

_GNUC_EXTERN_INLINE inline double cosh(double _Left)
	{	// return hyperbolic cosine
	return (_Cosh(_Left, 1.0));
	}

_GNUC_EXTERN_INLINE inline double log(double _Left)
	{	// return natural logarithm
	return (_Log(_Left, 0));
	}

_GNUC_EXTERN_INLINE inline double log10(double _Left)
	{	// return base-10 logarithm
	return (_Log(_Left, 1));
	}

_GNUC_EXTERN_INLINE inline double sin(double _Left)
	{	// return sine
	return (_Sin(_Left, 0));
	}

_GNUC_EXTERN_INLINE inline double sinh(double _Left)
	{	// return hyperbolic sine
	return (_Sinh(_Left, 1.0));
	}

_GNUC_EXTERN_INLINE inline double log2(double _Left)
	{	// return base-2 logarithm
	return (_Log(_Left, -1));
	}

_GNUC_EXTERN_INLINE inline float coshf(float _Left)
	{	// return hyperbolic cosine
	return (_FCosh(_Left, 1.0F));
	}

_GNUC_EXTERN_INLINE inline float sinhf(float _Left)
	{	// return hyperbolic sine
	return (_FSinh(_Left, 1.0F));
	}

_GNUC_EXTERN_INLINE inline long double cosl(long double _Left)
	{	// return cosine
	return (_LSin(_Left, 1));
	}

_GNUC_EXTERN_INLINE inline long double coshl(long double _Left)
	{	// return hyperbolic cosine
	return (_LCosh(_Left, 1.0L));
	}

_GNUC_EXTERN_INLINE inline long double logl(long double _Left)
	{	// return natural logarithm
	return (_LLog(_Left, 0));
	}

_GNUC_EXTERN_INLINE inline long double log10l(long double _Left)
	{	// return base-10 logarithm
	return (_LLog(_Left, 1));
	}

_GNUC_EXTERN_INLINE inline long double sinl(long double _Left)
	{	// return sine
	return (_LSin(_Left, 0));
	}

_GNUC_EXTERN_INLINE inline long double sinhl(long double _Left)
	{	// return hyperbolic sine
	return (_LSinh(_Left, 1.0L));
	}

_GNUC_EXTERN_INLINE inline long double log2l(long double _Left)
	{	// return base-2 logarithm
	return (_LLog(_Left, -1));
	}
 // END SHARED C++ / C90 inlines
 _END_C_LIB_DECL

#if defined(__cplusplus) && !defined(_NO_CPP_INLINES)
extern "C++" {

  #if _IS_EMBEDDED

  #else /* _IS_EMBEDDED */
		// TEMPLATE FUNCTION _Pow_int
template<class _Ty> inline
	_Ty _Pow_int(_Ty _Left, int _Right)
	{	// raise to integer power
	unsigned int _Num = _Right;
	if (_Right < 0)
		_Num = 0 - _Num;

	for (_Ty _Ans = 1; ; _Left *= _Left)
		{	// scale and fold in factors
		if ((_Num & 1) != 0)
			_Ans *= _Left;
		if ((_Num >>= 1) == 0)
			return ((_Ty)(0 <= _Right ? _Ans
				: _Ans == _Ty(0) ? HUGE_VAL : _Ty(1) / _Ans));
		}
	}
  #endif /* _IS_EMBEDDED */

inline double abs(double _Left)	// OVERLOADS
	{	// return absolute value
	return (fabs(_Left));
	}

 #if _IS_EMBEDDED
inline double pow(double _Left, int _Right)
	{	// raise to integer power
	unsigned int _Num = _Right;
	if (_Right < 0)
		_Num = 0 - _Num;

	for (double _Ans = 1; ; _Left *= _Left)
		{if ((_Num & 1) != 0)
			_Ans *= _Left;
		if ((_Num >>= 1) == 0)
			return (_Right < 0 ? (double)(1) / _Ans : _Ans);
		}
	}

 #else /* _IS_EMBEDDED */
inline double pow(double _Left, int _Right)
	{	// raise to integer power
	return (_Pow_int(_Left, _Right));
	}
 #endif /* _IS_EMBEDDED */

		// float INLINES, FOR C++
inline float abs(float _Left)	// OVERLOADS
	{	// return absolute value
	return (fabsf(_Left));
	}

inline float acos(float _Left)
	{	// return arccosine
	return (acosf(_Left));
	}

inline float asin(float _Left)
	{	// return arcsine
	return (asinf(_Left));
	}

inline float atan(float _Left)
	{	// return arctangent
	return (atanf(_Left));
	}

inline float atan2(float _Left, float _Right)
	{	// return arctangent
	return (atan2f(_Left, _Right));
	}

inline float ceil(float _Left)
	{	// return ceiling
	return (ceilf(_Left));
	}

inline float cos(float _Left)
	{	// return cosine
	return (_FSin(_Left, 1));
	}

inline float cosh(float _Left)
	{	// return hyperbolic cosine
	return (_FCosh(_Left, 1.0F));
	}

inline float exp(float _Left)
	{	// return exponential
	return (expf(_Left));
	}

inline float fabs(float _Left)
	{	// return absolute value
	return (fabsf(_Left));
	}

inline float floor(float _Left)
	{	// return floor
	return (floorf(_Left));
	}

inline float fmod(float _Left, float _Right)
	{	// return modulus
	return (fmodf(_Left, _Right));
	}

inline float frexp(float _Left, int *_Right)
	{	// unpack exponent
	return (frexpf(_Left, _Right));
	}

inline float ldexp(float _Left, int _Right)
	{	// pack exponent
	return (ldexpf(_Left, _Right));
	}

inline float log(float _Left)
	{	// return natural logarithm
	return (_FLog(_Left, 0));
	}

inline float log10(float _Left)
	{	// return base-10 logarithm
	return (_FLog(_Left, 1));
	}

inline float modf(float _Left, float *_Right)
	{	// unpack fraction
	return (modff(_Left, _Right));
	}

inline float pow(float _Left, float _Right)
	{	// raise to power
	return (powf(_Left, _Right));
	}

 #if _IS_EMBEDDED
inline float pow(float _Left, int _Right)
	{	// raise to integer power
	unsigned int _Num = _Right;
	if (_Right < 0)
		_Num = 0 - _Num;

	for (float _Ans = 1; ; _Left *= _Left)
		{if ((_Num & 1) != 0)
			_Ans *= _Left;
		if ((_Num >>= 1) == 0)
			return (_Right < 0 ? (float)(1) / _Ans : _Ans);
		}
	}

 #else /* _IS_EMBEDDED */
inline float pow(float _Left, int _Right)
	{	// raise to integer power
	return (_Pow_int(_Left, _Right));
	}
 #endif /* _IS_EMBEDDED */

inline float sin(float _Left)
	{	// return sine
	return (_FSin(_Left, 0));
	}

inline float sinh(float _Left)
	{	// return hyperbolic sine
	return (_FSinh(_Left, 1.0F));
	}

inline float sqrt(float _Left)
	{	// return square root
	return (sqrtf(_Left));
	}

inline float tan(float _Left)
	{	// return tangent
	return (tanf(_Left));
	}

inline float tanh(float _Left)
	{	// return hyperbolic tangent
	return (tanhf(_Left));
	}

 #if _HAS_C9X
inline float acosh(float _Left)
	{	// return hyperbolic arccosine
	return (acoshf(_Left));
	}

inline float asinh(float _Left)
	{	// return hyperbolic arcsine
	return (asinhf(_Left));
	}

inline float atanh(float _Left)
	{	// return hyperbolic arctangent
	return (atanhf(_Left));
	}

inline float cbrt(float _Left)
	{	// return cube root
	return (cbrtf(_Left));
	}

inline float copysign(float _Left, float _Right)
	{	// return copysign
	return (copysignf(_Left, _Right));
	}

inline float erf(float _Left)
	{	// return erf
 	return (erff(_Left)); }

inline float erfc(float _Left)
	{	// return erfc
	return (erfcf(_Left));
	}

inline float exp2(float _Left)
	{	// return exp2
	return (exp2f(_Left));
	}

inline float expm1(float _Left)
	{	// return expml
	return (expm1f(_Left));
	}

inline float fdim(float _Left, float _Right)
	{	// return fdim
	return (fdimf(_Left, _Right));
	}

inline float fma(float _Left, float _Right, float _Addend)
	{	// return fma
	return (fmaf(_Left, _Right, _Addend));
	}

inline float fmax(float _Left, float _Right)
	{	// return fmax
	return (fmaxf(_Left, _Right));
	}

inline float fmin(float _Left, float _Right)
	{	// return fmin
	return (fminf(_Left, _Right));
	}

inline float hypot(float _Left, float _Right)
	{	// return hypot
	return (hypotf(_Left, _Right));
	}

inline int ilogb(float _Left)
	{	// return ilogb
	return (ilogbf(_Left));
	}

inline float lgamma(float _Left)
	{	// return lgamma
	return (lgammaf(_Left));
	}

inline _Longlong llrint(float _Left)
	{	// return llrint
	return (llrintf(_Left));
	}

inline _Longlong llround(float _Left)
	{	// return llround
	return (llroundf(_Left));
	}

inline float log1p(float _Left)
	{	// return loglp
	return (log1pf(_Left));
	}

inline float log2(float _Left)
	{	// return log2
	return (_FLog(_Left, -1));
	}

inline float logb(float _Left)
	{	// return logb
	return (logbf(_Left));
	}

inline long lrint(float _Left)
	{	// return lrint
	return (lrintf(_Left));
	}

inline long lround(float _Left)
	{	// return lround
	return (lroundf(_Left));
	}

inline float nearbyint(float _Left)
	{	// return nearbyint
	return (nearbyintf(_Left));
	}

inline float nextafter(float _Left, float _Right)
	{	// return nextafter
	return (nextafterf(_Left, _Right));
	}

inline float nexttoward(float _Left, long double _Right)
	{	// return nexttoward
	return (nexttowardf(_Left, _Right));
	}

inline float remainder(float _Left, float _Right)
	{	// return remainder
	return (remainderf(_Left, _Right));
	}

inline float remquo(float _Left, float _Right, int *_Pval)
	{	// return remquo
	return (remquof(_Left, _Right, _Pval));
	}

inline float rint(float _Left)
	{	// return rint
	return (rintf(_Left));
	}

inline float round(float _Left)
	{	// return round
	return (roundf(_Left));
	}

inline float scalbn(float _Left, int _Right)
	{	// return scalbn
	return (scalbnf(_Left, _Right));
	}

inline float scalbln(float _Left, long _Right)
	{	// return scalbln
	return (scalblnf(_Left, _Right));
	}

inline float tgamma(float _Left)
	{	// return tgamma
	return (tgammaf(_Left));
	}

inline float trunc(float _Left)
	{	// return trunc
	return (truncf(_Left));
	}
 #endif /* _HAS_C9X */

 #if _HAS_TR1_DECLARATIONS
inline float assoc_laguerre(unsigned _Nv, unsigned _Mv,
	float _Xv)
	{	// return assoc_laguerre
	return (assoc_laguerref(_Nv, _Mv, _Xv));
	}

inline float assoc_legendre(unsigned _Nv, unsigned _Mv,
	float _Xv)
	{	// return assoc_legendre
	return (assoc_legendref(_Nv, _Mv, _Xv));
	}

inline float beta(float _Xv, float _Yv)
	{	// return beta
	return (betaf(_Xv, _Yv));
	}

inline float comp_ellint_1(float _Kv)
	{	// return comp_ellint_1
	return (comp_ellint_1f(_Kv));
	}

inline float comp_ellint_2(float _Kv)
	{	// return comp_ellint_2
	return (comp_ellint_2f(_Kv));
	}

inline float comp_ellint_3(float _Kv, float _Nu)
	{	// return comp_ellint_3
	return (comp_ellint_3f(_Kv, _Nu));
	}

inline float cyl_bessel_i(float _Nu, float _Xv)
	{	// return cyl_bessel_i
	return (cyl_bessel_if(_Nu, _Xv));
	}

inline float cyl_bessel_j(float _Nu, float _Xv)
	{	// return cyl_bessel_j
	return (cyl_bessel_jf(_Nu, _Xv));
	}

inline float cyl_bessel_k(float _Nu, float _Xv)
	{	// return cyl_bessel_k
	return (cyl_bessel_kf(_Nu, _Xv));
	}

inline float cyl_neumann(float _Nu, float _Xv)
	{	// return cyl_neumann
	return (cyl_neumannf(_Nu, _Xv));
	}

inline float ellint_1(float _Kv, float _Phi)
	{	// return ellint_1
	return (ellint_1f(_Kv, _Phi));
	}

inline float ellint_2(float _Kv, float _Phi)
	{	// return ellint_2
	return (ellint_2f(_Kv, _Phi));
	}

inline float ellint_3(float _Kv, float _Nu, float _Phi)
	{	// return ellint_3
	return (ellint_3f(_Kv, _Nu, _Phi));
	}

inline float expint(float _Xv)
	{	// return expint
	return (expintf(_Xv));
	}

inline float hermite(unsigned _Nv, float _Xv)
	{	// return hermite
	return (hermitef(_Nv, _Xv));
	}

inline float laguerre(unsigned _Nv, float _Xv)
	{	// return laguerre
	return (laguerref(_Nv, _Xv));
	}

inline float legendre(unsigned _Lv, float _Xv)
	{	// return legendre
	return (legendref(_Lv, _Xv));
	}

inline float riemann_zeta(float _Xv)
	{	// return riemann_zeta
	return (riemann_zetaf(_Xv));
	}

inline float sph_bessel(unsigned _Nv, float _Xv)
	{	// return sph_bessel
	return (sph_besself(_Nv, _Xv));
	}

inline float sph_legendre(unsigned _Lv, unsigned _Mv, float _Xv)
	{	// return sph_legendre
	return (sph_legendref(_Lv, _Mv, _Xv));
	}

inline float sph_neumann(unsigned _Nv, float _Xv)
	{	// return sph_neumann
	return (sph_neumannf(_Nv, _Xv));
	}

 #if _HAS_ADDED_FUNCTIONS
inline float besseli0(float _Xv)
	{	// return besseli0
	return (besseli0f(_Xv));
	}

inline float besseli1(float _Xv)
	{	// return besseli1
	return (besseli1f(_Xv));
	}

inline float besselj0(float _Xv)
	{	// return besselj0
	return (besselj0f(_Xv));
	}

inline float besselj1(float _Xv)
	{	// return besselj1
	return (besselj1f(_Xv));
	}

inline float besselk0(float _Xv)
	{	// return besselk0
	return (besselk0f(_Xv));
	}

inline float besselk1(float _Xv)
	{	// return besselk1
	return (besselk1f(_Xv));
	}

inline float bessely0(float _Xv)
	{	// return bessely0
	return (bessely0f(_Xv));
	}

inline float bessely1(float _Xv)
	{	// return bessely1
	return (bessely1f(_Xv));
	}
 #endif /* _HAS_ADDED_FUNCTIONS */

 #endif /* _HAS_TR1_DECLARATIONS */

		// long double INLINES, FOR C++
_C_LIB_DECL

_END_C_LIB_DECL

inline long double abs(long double _Left)	// OVERLOADS
	{	// return absolute value
	return (fabsl(_Left));
	}

inline long double acos(long double _Left)
	{	// return arccosine
	return (acosl(_Left));
	}

inline long double asin(long double _Left)
	{	// return arcsine
	return (asinl(_Left));
	}

inline long double atan(long double _Left)
	{	// return arctangent
	return (atanl(_Left));
	}

inline long double atan2(long double _Left, long double _Right)
	{	// return arctangent
	return (atan2l(_Left, _Right));
	}

inline long double ceil(long double _Left)
	{	// return ceiling
	return (ceill(_Left));
	}

inline long double cos(long double _Left)
	{	// return cosine
	return (_LSin(_Left, 1));
	}

inline long double cosh(long double _Left)
	{	// return hyperbolic cosine
	return (_LCosh(_Left, 1.0L));
	}

inline long double exp(long double _Left)
	{	// return exponential
	return (expl(_Left));
	}

inline long double fabs(long double _Left)
	{	// return absolute value
	return (fabsl(_Left));
	}

inline long double floor(long double _Left)
	{	// return floor
	return (floorl(_Left));
	}

inline long double fmod(long double _Left, long double _Right)
	{	// return modulus
	return (fmodl(_Left, _Right));
	}

inline long double frexp(long double _Left, int *_Right)
	{	// unpack exponent
	return (frexpl(_Left, _Right));
	}

inline long double ldexp(long double _Left, int _Right)
	{	// pack exponent
	return (ldexpl(_Left, _Right));
	}

inline long double log(long double _Left)
	{	// return natural logarithm
	return (_LLog(_Left, 0));
	}

inline long double log10(long double _Left)
	{	// return base-10 logarithm
	return (_LLog(_Left, 1));
	}

inline long double modf(long double _Left, long double *_Right)
	{	// unpack fraction
	return (modfl(_Left, _Right));
	}

inline long double pow(long double _Left, long double _Right)
	{	// raise to power
	return (powl(_Left, _Right));
	}

 #if _IS_EMBEDDED
inline long double pow(long double _Left, int _Right)
	{	// raise to integer power
	unsigned int _Num = _Right;
	if (_Right < 0)
		_Num = 0 - _Num;

	for (long double _Ans = 1; ; _Left *= _Left)
		{if ((_Num & 1) != 0)
			_Ans *= _Left;
		if ((_Num >>= 1) == 0)
			return (_Right < 0 ? (long double)(1) / _Ans : _Ans);
		}
	}

 #else /* _IS_EMBEDDED */
inline long double pow(long double _Left, int _Right)
	{	// raise to integer power
	return (_Pow_int(_Left, _Right));
	}
 #endif /* _IS_EMBEDDED */

inline long double sin(long double _Left)
	{	// return sine
	return (_LSin(_Left, 0));
	}

inline long double sinh(long double _Left)
	{	// return hyperbolic sine
	return (_LSinh(_Left, 1.0L));
	}

inline long double sqrt(long double _Left)
	{	// return square root
	return (sqrtl(_Left));
	}

inline long double tan(long double _Left)
	{	// return tangent
	return (tanl(_Left));
	}

inline long double tanh(long double _Left)
	{	// return hyperbolic tangent
	return (tanhl(_Left));
	}

 #if _HAS_C9X
inline long double acosh(long double _Left)
	{	// return acosh
	return (acoshl(_Left));
	}

inline long double asinh(long double _Left)
	{	// return asinh
	return (asinhl(_Left));
	}

inline long double atanh(long double _Left)
	{	// return atanh
	return (atanhl(_Left));
	}

inline long double cbrt(long double _Left)
	{	// return cbrt
	return (cbrtl(_Left));
	}

inline long double copysign(long double _Left, long double _Right)
	{	// return copysign
	return (copysignl(_Left, _Right));
	}

inline long double erf(long double _Left)
	{	// return erf
	return (erfl(_Left));
	}

inline long double erfc(long double _Left)
	{	// return erfc
	return (erfcl(_Left));
	}

inline long double exp2(long double _Left)
	{	// return exp2
	return (exp2l(_Left));
	}

inline long double expm1(long double _Left)
	{	// return expml
	return (expm1l(_Left));
	}

inline long double fdim(long double _Left, long double _Right)
	{	// return fdim
	return (fdiml(_Left, _Right));
	}

inline long double fma(long double _Left, long double _Right,
	long double _Addend)
	{	// return fma
	return (fmal(_Left, _Right, _Addend));
	}

inline long double fmax(long double _Left, long double _Right)
	{	// return fmax
	return (fmaxl(_Left, _Right));
	}

inline long double fmin(long double _Left, long double _Right)
	{	// return fmin
	return (fminl(_Left, _Right));
	}

inline long double hypot(long double _Left, long double _Right)
	{	// return hypot
	return (hypotl(_Left, _Right));
	}

inline int ilogb(long double _Left)
	{	// return ilogb
	return (ilogbl(_Left));
	}

inline long double lgamma(long double _Left)
	{	// return lgamma
	return (lgammal(_Left));
	}

inline _Longlong llrint(long double _Left)
	{	// return llrint
	return (llrintl(_Left));
	}

inline _Longlong llround(long double _Left)
	{	// return llround
	return (llroundl(_Left));
	}

inline long double log1p(long double _Left)
	{	// return loglp
	return (log1pl(_Left));
	}

inline long double log2(long double _Left)
	{	// return log2
	return (_LLog(_Left, -1));
	}

inline long double logb(long double _Left)
	{	// return logb
	return (logbl(_Left));
	}

inline long lrint(long double _Left)
	{	// return lrint
	return (lrintl(_Left));
	}

inline long lround(long double _Left)
	{	// return lround
	return (lroundl(_Left));
	}

inline long double nearbyint(long double _Left)
	{	// return nearbyint
	return (nearbyintl(_Left));
	}

inline long double nextafter(long double _Left, long double _Right)
	{	// return nextafter
	return (nextafterl(_Left, _Right));
	}

inline long double nexttoward(long double _Left, long double _Right)
	{	// return nexttoward
	return (nexttowardl(_Left, _Right));
	}

inline long double remainder(long double _Left, long double _Right)
	{	// return remainder
	return (remainderl(_Left, _Right));
	}

inline long double remquo(long double _Left, long double _Right,
	int *_Pval)
	{	// return remquo
	return (remquol(_Left, _Right, _Pval));
	}

inline long double rint(long double _Left)
	{	// return rint
	return (rintl(_Left));
	}

inline long double round(long double _Left)
	{	// return round
	return (roundl(_Left));
	}

inline long double scalbn(long double _Left, int _Right)
	{	// return scalbn
	return (scalbnl(_Left, _Right));
	}

inline long double scalbln(long double _Left, long _Right)
	{	// return scalbln
	return (scalblnl(_Left, _Right));
	}

inline long double tgamma(long double _Left)
	{	// return tgamma
	return (tgammal(_Left));
	}

inline long double trunc(long double _Left)
	{	// return trunc
	return (truncl(_Left));
	}
 #endif /* _HAS_C9X */

 #if _HAS_TR1_DECLARATIONS
inline long double assoc_laguerre(unsigned _Nv, unsigned _Mv,
	long double _Xv)
	{	// return assoc_laguerre
	return (assoc_laguerrel(_Nv, _Mv, _Xv));
	}

inline long double assoc_legendre(unsigned _Nv, unsigned _Mv,
	long double _Xv)
	{	// return assoc_legendre
	return (assoc_legendrel(_Nv, _Mv, _Xv));
	}

inline long double beta(long double _Xv, long double _Yv)
	{	// return beta
	return (betal(_Xv, _Yv));
	}

inline long double comp_ellint_1(long double _Kv)
	{	// return comp_ellint_1
	return (comp_ellint_1l(_Kv));
	}

inline long double comp_ellint_2(long double _Kv)
	{	// return comp_ellint_2
	return (comp_ellint_2l(_Kv));
	}

inline long double comp_ellint_3(long double _Kv, long double _Nu)
	{	// return comp_ellint_3
	return (comp_ellint_3l(_Kv, _Nu));
	}

inline long double cyl_bessel_i(long double _Nu, long double _Xv)
	{	// return cyl_bessel_i
	return (cyl_bessel_il(_Nu, _Xv));
	}

inline long double cyl_bessel_j(long double _Nu, long double _Xv)
	{	// return cyl_bessel_j
	return (cyl_bessel_jl(_Nu, _Xv));
	}

inline long double cyl_bessel_k(long double _Nu, long double _Xv)
	{	// return cyl_bessel_k
	return (cyl_bessel_kl(_Nu, _Xv));
	}

inline long double cyl_neumann(long double _Nu, long double _Xv)
	{	// return cyl_neumann
	return (cyl_neumannl(_Nu, _Xv));
	}

inline long double ellint_1(long double _Kv, long double _Phi)
	{	// return ellint_1
	return (ellint_1l(_Kv, _Phi));
	}

inline long double ellint_2(long double _Kv, long double _Phi)
	{	// return ellint_2
	return (ellint_2l(_Kv, _Phi));
	}

inline long double ellint_3(long double _Kv, long double _Nu,
	long double _Phi)
	{	// return ellint_3
	return (ellint_3l(_Kv, _Nu, _Phi));
	}

inline long double expint(long double _Xv)
	{	// return expint
	return (expintl(_Xv));
	}

inline long double hermite(unsigned _Nv, long double _Xv)
	{	// return hermite
	return (hermitel(_Nv, _Xv));
	}

inline long double laguerre(unsigned _Nv, long double _Xv)
	{	// return laguerre
	return (laguerrel(_Nv, _Xv));
	}

inline long double legendre(unsigned _Lv, long double _Xv)
	{	// return legendre
	return (legendrel(_Lv, _Xv));
	}

inline long double riemann_zeta(long double _Xv)
	{	// return riemann_zeta
	return (riemann_zetal(_Xv));
	}

inline long double sph_bessel(unsigned _Nv, long double _Xv)
	{	// return sph_bessel
	return (sph_bessell(_Nv, _Xv));
	}

inline long double sph_legendre(unsigned _Lv, unsigned _Mv,
	long double _Xv)
	{	// return sph_legendre
	return (sph_legendrel(_Lv, _Mv, _Xv));
	}

inline long double sph_neumann(unsigned _Nv, long double _Xv)
	{	// return sph_neumann
	return (sph_neumannl(_Nv, _Xv));
	}

 #if _HAS_ADDED_FUNCTIONS
inline long double besseli0(long double _Xv)
	{	// return besseli0
	return (besseli0l(_Xv));
	}

inline long double besseli1(long double _Xv)
	{	// return besseli1
	return (besseli1l(_Xv));
	}

inline long double besselj0(long double _Xv)
	{	// return besselj0
	return (besselj0l(_Xv));
	}

inline long double besselj1(long double _Xv)
	{	// return besselj1
	return (besselj1l(_Xv));
	}

inline long double besselk0(long double _Xv)
	{	// return besselk0
	return (besselk0l(_Xv));
	}

inline long double besselk1(long double _Xv)
	{	// return besselk1
	return (besselk1l(_Xv));
	}

inline long double bessely0(long double _Xv)
	{	// return bessely0
	return (bessely0l(_Xv));
	}

inline long double bessely1(long double _Xv)
	{	// return bessely1
	return (bessely1l(_Xv));
	}
 #endif /* _HAS_ADDED_FUNCTIONS */

 #endif /* _HAS_TR1_DECLARATIONS */
}	// extern "C++"
 #endif /* defined(__cplusplus) && !defined(_NO_CPP_INLINES) */

 #else /* _HAS_INLINE */

_C_LIB_DECL
		/* double MACRO OVERRIDES, FOR C90 */
double cos(double) _NO_THROW;
double cosh(double) _NO_THROW;
double log(double) _NO_THROW;
double log10(double) _NO_THROW;
double sin(double) _NO_THROW;
double sinh(double) _NO_THROW;

 #define cos(x)		_Sin(x, 1)
 #define cosh(x)	_Cosh(x, 1.0)
 #define log(x)		_Log(x, 0)
 #define log10(x)	_Log(x, 1)
 #define sin(x)		_Sin(x, 0)
 #define sinh(x)	_Sinh(x, 1.0)

_END_C_LIB_DECL
#endif /* _HAS_INLINE */

_C_STD_END

		/* GENERIC TEMPLATES */

 #if _HAS_GENERIC_TEMPLATES
extern "C++" {
_C_STD_BEGIN
	_TGEN_RC(acos)	// real or complex generic overloads
	_TGEN_RC(acosh)
	_TGEN_RC(asin)
	_TGEN_RC(asinh)
	_TGEN_RC(atan)
	_TGEN_RC(atanh)
	_TGEN_RC(cos)
	_TGEN_RC(cosh)
	_TGEN_RC(exp)
	_TGEN_RC0(fabs)
	_TGEN_RC(log)
	_TGEN_RC(log10)
	_TGEN_RC2(pow)
	_TGEN_RC(sin)
	_TGEN_RC(sinh)
	_TGEN_RC(sqrt)
	_TGEN_RC(tan)
	_TGEN_RC(tanh)

	_TGEN_R2(atan2)	// real generic overloads
	_TGEN_R(cbrt)
	_TGEN_R(ceil)
	_TGEN_R2(copysign)
	_TGEN_R(erf)
	_TGEN_R(erfc)
	_TGEN_R(exp2)
	_TGEN_R(expm1)
	_TGEN_R2(fdim)
	_TGEN_R(floor)
	_TGEN_R3(fma)
	_TGEN_R2(fmax)
	_TGEN_R2(fmin)
	_TGEN_R2(fmod)
	_TGEN_RX(frexp, int *)
	_TGEN_R2(hypot)
	_TGEN_RI(ilogb, int)
	_TGEN_RX(ldexp, int)
	_TGEN_R(lgamma)
	_TGEN_RI(llrint, _Longlong)
	_TGEN_RI(llround, _Longlong)
	_TGEN_R(log1p)
	_TGEN_R(log2)
	_TGEN_R(logb)
	_TGEN_RI(lrint, long)
	_TGEN_RI(lround, long)
	_TGEN_R(nearbyint)
	_TGEN_R2(nextafter)
	_TGEN_RX(nexttoward, long double)
	_TGEN_R2(remainder)
	_TGEN_R2X(remquo, int *)
	_TGEN_R(rint)
	_TGEN_R(round)
	_TGEN_RX(scalbln, long)
	_TGEN_RX(scalbn, int)
	_TGEN_R(tgamma)
	_TGEN_R(trunc)
_C_STD_END
}	// extern "C++"
 #endif /* _HAS_GENERIC_TEMPLATES */

#endif /* _MATH */

 #if defined(_STD_USING)

  #if !defined(_NO_CPP_INLINES)
using _CSTD abs;
  #endif /* !defined(_NO_CPP_INLINES) */

using _CSTD acos; using _CSTD asin;
using _CSTD atan; using _CSTD atan2; using _CSTD ceil;
using _CSTD cos; using _CSTD cosh; using _CSTD exp;
using _CSTD fabs; using _CSTD floor; using _CSTD fmod;
using _CSTD frexp; using _CSTD ldexp; using _CSTD log;
using _CSTD log10; using _CSTD modf; using _CSTD pow;
using _CSTD sin; using _CSTD sinh; using _CSTD sqrt;
using _CSTD tan; using _CSTD tanh;

using _CSTD acosf; using _CSTD asinf;
using _CSTD atanf; using _CSTD atan2f; using _CSTD ceilf;
using _CSTD cosf; using _CSTD coshf; using _CSTD expf;
using _CSTD fabsf; using _CSTD floorf; using _CSTD fmodf;
using _CSTD frexpf; using _CSTD ldexpf; using _CSTD logf;
using _CSTD log10f; using _CSTD modff; using _CSTD powf;
using _CSTD sinf; using _CSTD sinhf; using _CSTD sqrtf;
using _CSTD tanf; using _CSTD tanhf;
using _CSTD exp10f;

using _CSTD acosl; using _CSTD asinl;
using _CSTD atanl; using _CSTD atan2l; using _CSTD ceill;
using _CSTD cosl; using _CSTD coshl; using _CSTD expl;
using _CSTD fabsl; using _CSTD floorl; using _CSTD fmodl;
using _CSTD frexpl; using _CSTD ldexpl; using _CSTD logl;
using _CSTD log10l; using _CSTD modfl; using _CSTD powl;
using _CSTD sinl; using _CSTD sinhl; using _CSTD sqrtl;
using _CSTD tanl; using _CSTD tanhl;

 #if _HAS_C9X

 #if _HAS_GENERIC_TEMPLATES
using _CSTD _FPCOMP; using _CSTD fpclassify;
using _CSTD signbit; using _CSTD isfinite; using _CSTD isinf;
using _CSTD isnan; using _CSTD isnormal;
using _CSTD isgreater; using _CSTD isgreaterequal;
using _CSTD isless; using _CSTD islessequal; using _CSTD islessgreater;
using _CSTD isunordered;
 #endif	/* _HAS_GENERIC_TEMPLATES */

using _CSTD float_t; using _CSTD double_t;

using _CSTD acosh; using _CSTD asinh; using _CSTD atanh;
using _CSTD cbrt; using _CSTD erf; using _CSTD erfc;
using _CSTD expm1; using _CSTD exp2;
using _CSTD hypot; using _CSTD ilogb; using _CSTD lgamma;
using _CSTD log1p; using _CSTD log2; using _CSTD logb;
using _CSTD llrint; using _CSTD lrint; using _CSTD nearbyint;
using _CSTD rint; using _CSTD llround; using _CSTD lround;
using _CSTD fdim; using _CSTD fma; using _CSTD fmax; using _CSTD fmin;
using _CSTD round; using _CSTD trunc;
using _CSTD remainder; using _CSTD remquo;
using _CSTD copysign; using _CSTD nan; using _CSTD nextafter;
using _CSTD scalbn; using _CSTD scalbln;
using _CSTD nexttoward; using _CSTD tgamma;

using _CSTD acoshf; using _CSTD asinhf; using _CSTD atanhf;
using _CSTD cbrtf; using _CSTD erff; using _CSTD erfcf;
using _CSTD expm1f; using _CSTD exp2f;
using _CSTD hypotf; using _CSTD ilogbf; using _CSTD lgammaf;
using _CSTD log1pf; using _CSTD log2f; using _CSTD logbf;
using _CSTD llrintf; using _CSTD lrintf; using _CSTD nearbyintf;
using _CSTD rintf; using _CSTD llroundf; using _CSTD lroundf;
using _CSTD fdimf; using _CSTD fmaf; using _CSTD fmaxf; using _CSTD fminf;
using _CSTD roundf; using _CSTD truncf;
using _CSTD remainderf; using _CSTD remquof;
using _CSTD copysignf; using _CSTD nanf;
using _CSTD nextafterf; using _CSTD scalbnf; using _CSTD scalblnf;
using _CSTD nexttowardf; using _CSTD tgammaf;

using _CSTD acoshl; using _CSTD asinhl; using _CSTD atanhl;
using _CSTD cbrtl; using _CSTD erfl; using _CSTD erfcl;
using _CSTD expm1l; using _CSTD exp2l;
using _CSTD hypotl; using _CSTD ilogbl; using _CSTD lgammal;
using _CSTD log1pl; using _CSTD log2l; using _CSTD logbl;
using _CSTD llrintl; using _CSTD lrintl; using _CSTD nearbyintl;
using _CSTD rintl; using _CSTD llroundl; using _CSTD lroundl;
using _CSTD fdiml; using _CSTD fmal; using _CSTD fmaxl; using _CSTD fminl;
using _CSTD roundl; using _CSTD truncl;
using _CSTD remainderl; using _CSTD remquol;
using _CSTD copysignl; using _CSTD nanl;
using _CSTD nextafterl; using _CSTD scalbnl; using _CSTD scalblnl;
using _CSTD nexttowardl; using _CSTD tgammal;
 #endif /* _HAS_C9X */

 #if _HAS_TR1_DECLARATIONS
using _CSTD assoc_laguerre; using _CSTD assoc_legendre; using _CSTD beta;
using _CSTD comp_ellint_1; using _CSTD comp_ellint_2;
using _CSTD comp_ellint_3; using _CSTD cyl_bessel_i;
using _CSTD cyl_bessel_j; using _CSTD cyl_bessel_k;
using _CSTD cyl_neumann; using _CSTD ellint_1; using _CSTD ellint_2;
using _CSTD ellint_3; using _CSTD expint; using _CSTD hermite;
using _CSTD laguerre; using _CSTD legendre; using _CSTD riemann_zeta;
using _CSTD sph_bessel; using _CSTD sph_legendre; using _CSTD sph_neumann;

using _CSTD assoc_laguerref; using _CSTD assoc_legendref; using _CSTD betaf;
using _CSTD comp_ellint_1f; using _CSTD comp_ellint_2f;
using _CSTD comp_ellint_3f; using _CSTD cyl_bessel_if;
using _CSTD cyl_bessel_jf; using _CSTD cyl_bessel_kf;
using _CSTD cyl_neumannf; using _CSTD ellint_1f; using _CSTD ellint_2f;
using _CSTD ellint_3f; using _CSTD expintf; using _CSTD hermitef;
using _CSTD laguerref; using _CSTD legendref; using _CSTD riemann_zetaf;
using _CSTD sph_besself; using _CSTD sph_legendref; using _CSTD sph_neumannf;

using _CSTD assoc_laguerrel; using _CSTD assoc_legendrel; using _CSTD betal;
using _CSTD comp_ellint_1l; using _CSTD comp_ellint_2l;
using _CSTD comp_ellint_3l; using _CSTD cyl_bessel_il;
using _CSTD cyl_bessel_jl; using _CSTD cyl_bessel_kl;
using _CSTD cyl_neumannl; using _CSTD ellint_1l; using _CSTD ellint_2l;
using _CSTD ellint_3l; using _CSTD expintl; using _CSTD hermitel;
using _CSTD laguerrel; using _CSTD legendrel; using _CSTD riemann_zetal;
using _CSTD sph_bessell; using _CSTD sph_legendrel; using _CSTD sph_neumannl;

 #if _HAS_ADDED_FUNCTIONS
using _CSTD besseli0; using _CSTD besseli1;
using _CSTD besselj0; using _CSTD besselj1;
using _CSTD besselk0; using _CSTD besselk1;
using _CSTD bessely0; using _CSTD bessely1;
using _CSTD j0; using _CSTD j1; using _CSTD jn;
using _CSTD y0; using _CSTD y1; using _CSTD yn;

using _CSTD besseli0f; using _CSTD besseli1f;
using _CSTD besselj0f; using _CSTD besselj1f;
using _CSTD besselk0f; using _CSTD besselk1f;
using _CSTD bessely0f; using _CSTD bessely1f;
using _CSTD j0f; using _CSTD j1f; using _CSTD jnf;
using _CSTD y0f; using _CSTD y1f; using _CSTD ynf;

using _CSTD besseli0l; using _CSTD besseli1l;
using _CSTD besselj0l; using _CSTD besselj1l;
using _CSTD besselk0l; using _CSTD besselk1l;
using _CSTD bessely0l; using _CSTD bessely1l;
using _CSTD j0l; using _CSTD j1l; using _CSTD jnl;
using _CSTD y0l; using _CSTD y1l; using _CSTD ynl;
 #endif /* _HAS_ADDED_FUNCTIONS */

 #endif /* _HAS_TR1_DECLARATIONS */
 #endif /* defined(_STD_USING) */


/*
 * Copyright (c) 1992-2010 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.30:1566 */
