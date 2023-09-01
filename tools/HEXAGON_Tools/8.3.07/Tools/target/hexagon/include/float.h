/* float.h standard header -- IEEE 754 version */
#ifndef _FLOAT
#define _FLOAT
#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

_C_STD_BEGIN

		/* TYPE DEFINITIONS */
 #ifndef _DVALS
  #define _DVALS
typedef struct
	{	/* parameters for a floating-point type */
	int _Ddig, _Dmdig, _Dmax10e, _Dmaxe, _Dmin10e, _Dmine;
	union
		{	/* union of short array and all floats */
		unsigned short _Us[8];
		float _Float;
		double _Double;
		long double _Long_double;
		} _Deps, _Dmax, _Dmin;
	} _Dvals;
 #endif /* _DVALS */

		/* DECLARATIONS */
_C_LIB_DECL
int _Fltrounds(void);
extern /* const */ _Dvals _Dbl, _Flt, _Ldbl;
_END_C_LIB_DECL

		/* COMMON PROPERTIES */
#define FLT_RADIX		2
#define FLT_ROUNDS		(_CSTD _Fltrounds())

 #if _HAS_C9X

 #ifdef _FEVAL
  #define FLT_EVAL_METHOD	_FEVAL

 #else /* _FEVAL */
  #define FLT_EVAL_METHOD	-1	/* indeterminable */
 #endif /* _FEVAL */

 #if _DLONG == 0
  #define DECIMAL_DIG	17	/* 64-bit long double */

 #elif _DLONG == 1
  #define DECIMAL_DIG	21	/* 80-bit long double */

 #else /* 1 < _DLONG */
  #define DECIMAL_DIG	36	/* 128-bit SPARC long double */
 #endif /* _DLONG */

 #define FLT_DECIMAL_DIG 9      /* Spec says 6 but its suppose to be this:
                                   ceil(1+FLT_MANT_DIG*log10(FLT_RADIX)))
                                   which is 9 */
 #define DBL_DECIMAL_DIG 10
 #define LDBL_DECIMAL_DIG 10

 #define FLT_HAS_SUBNORM 1     /* ISO/IEC 9899:201x spec */
 #define DBL_HAS_SUBNORM 1
 #define LDBL_HAS_SUBNORM 1

 /* pow(FLT_RADIX, FLT_MIN_EXP-FLT_MANT_DIG */
 #define FLT_TRUE_MIN 1.40129846432481707092e-45F
/* pow(FLT_RADIX, DBL_MIN_EXP-DBL_MANT_DIG) */
 #define DBL_TRUE_MIN 4.94065645841246544177e-324
 #define LDBL_TRUE_MIN DBL_TRUE_MIN
 #endif /* _IS_C9X */

		/* float PROPERTIES */

 #if 199901L <= __STDC_VERSION__ && !defined(__APPLE__)

 #if defined(__cplusplus) || defined(__GNUC__) && __GNUC__ < 3
	/* IEEE 4 byte */
  #define FLT_EPSILON		_CSTD _Flt._Deps._Float
  #define FLT_MAX			_CSTD _Flt._Dmax._Float
  #define FLT_MIN			_CSTD _Flt._Dmin._Float

 #else /* defined(__cplusplus) || defined(__GNUC__) && __GNUC__ < 3 */
  #define FLT_EPSILON		0x8p-26
  #define FLT_MAX			0xf.fffffp+124
  #define FLT_MIN			0x8p-129
 #endif /* defined(__cplusplus) || defined(__GNUC__) && __GNUC__ < 3 */

 #else /* 199901L <= __STDC_VERSION__ */
 #define FLT_EPSILON		_CSTD _Flt._Deps._Float
 #define FLT_MAX			_CSTD _Flt._Dmax._Float
 #define FLT_MIN			_CSTD _Flt._Dmin._Float
 #endif /* 199901L <= __STDC_VERSION__ */

#define FLT_DIG			6	/* _CSTD _Flt._Ddig */
#define FLT_MANT_DIG	24	/* _CSTD _Flt._Dmdig */
#define FLT_MAX_10_EXP	38	/* _CSTD _Flt._Dmax10e */
#define FLT_MAX_EXP		128	/* _CSTD _Flt._Dmaxe */
#define FLT_MIN_10_EXP	-37	/* _CSTD _Flt._Dmin10e */
#define FLT_MIN_EXP		-125	/* _CSTD _Flt._Dmine */

		/* double PROPERTIES */

 #if 199901L <= __STDC_VERSION__

 #if  defined(__cplusplus) || defined(__GNUC__) && __GNUC__ < 3
	/* IEEE 8 byte */
  #define DBL_EPSILON		_CSTD _Dbl._Deps._Double
  #define DBL_MAX			_CSTD _Dbl._Dmax._Double
  #define DBL_MIN			_CSTD _Dbl._Dmin._Double

 #else /* defined(__cplusplus) || defined(__GNUC__) && __GNUC__ < 3 */
  #define DBL_EPSILON		0x8p-55
  #define DBL_MAX			0xf.ffffffffffff8p+1020
  #define DBL_MIN			0x8p-1025
 #endif /* defined(__cplusplus) || defined(__GNUC__) && __GNUC__ < 3 */

 #else /* 199901L <= __STDC_VERSION__ */
 #define DBL_EPSILON		_CSTD _Dbl._Deps._Double
 #define DBL_MAX			_CSTD _Dbl._Dmax._Double
 #define DBL_MIN			_CSTD _Dbl._Dmin._Double
 #endif /* 199901L <= __STDC_VERSION__ */

#define DBL_DIG			15 	/* _CSTD _Dbl._Ddig */
#define DBL_MANT_DIG	53	/* _CSTD _Dbl._Dmdig */
#define DBL_MAX_10_EXP	308	/* _CSTD _Dbl._Dmax10e */
#define DBL_MAX_EXP		1024	/* _CSTD _Dbl._Dmaxe */
#define DBL_MIN_10_EXP	-307	/* _CSTD _Dbl._Dmin10e */
#define DBL_MIN_EXP		-1021	/* _CSTD _Dbl._Dmine */

		/* long double PROPERTIES */

 #if _DLONG == 0

 #if 199901L <= __STDC_VERSION__

  #if defined(__cplusplus) || defined(__GNUC__) && __GNUC__ < 3
	/* IEEE 8 byte */
   #define LDBL_EPSILON	_CSTD _Ldbl._Deps._Long_double
   #define LDBL_MAX		_CSTD _Ldbl._Dmax._Long_double
   #define LDBL_MIN		_CSTD _Ldbl._Dmin._Long_double

  #else /* defined(__cplusplus) || defined(__GNUC__) && __GNUC__ < 3 */
   #define LDBL_EPSILON	0x8p-55L
   #define LDBL_MAX		0xf.ffffffffffff8p+1020L
   #define LDBL_MIN		0x8p-1025L
  #endif /* defined(__cplusplus) || defined(__GNUC__) && __GNUC__ < 3 */

 #else /* 199901L <= __STDC_VERSION__ */
   #define LDBL_EPSILON	_CSTD _Ldbl._Deps._Long_double
   #define LDBL_MAX		_CSTD _Ldbl._Dmax._Long_double
   #define LDBL_MIN		_CSTD _Ldbl._Dmin._Long_double
 #endif /* 199901L <= __STDC_VERSION__ */

  #define LDBL_DIG			15	/* _CSTD _Ldbl._Ddig */
  #define LDBL_MANT_DIG		53	/* _CSTD _Ldbl._Dmdig */
  #define LDBL_MAX_10_EXP	308	/* _CSTD _Ldbl._Dmax10e */
  #define LDBL_MAX_EXP		1024	/* _CSTD _Ldbl._Dmaxe */
  #define LDBL_MIN_10_EXP	-307	/* _CSTD _Ldbl._Dmin10e */
  #define LDBL_MIN_EXP		-1021	/* _CSTD _Ldbl._Dmine */

 #elif _DLONG == 1

 #if 199901L <= __STDC_VERSION__

  #if defined(__cplusplus) || defined(__GNUC__) && __GNUC__ < 3
	/* IEEE 10 byte, no hidden bit */
   #define LDBL_EPSILON	_CSTD _Ldbl._Deps._Long_double
   #define LDBL_MAX		_CSTD _Ldbl._Dmax._Long_double
   #define LDBL_MIN		_CSTD _Ldbl._Dmin._Long_double

  #else /* defined(__cplusplus) || defined(__GNUC__) && __GNUC__ < 3 */
   #define LDBL_EPSILON	0x8p-66L
   #define LDBL_MAX		0xf.fffffffffffffffp+16380L
   #define LDBL_MIN		0x8p-16385L
  #endif /* defined(__cplusplus) || defined(__GNUC__) && __GNUC__ < 3 */

 #else /* 199901L <= __STDC_VERSION__ */
   #define LDBL_EPSILON	_CSTD _Ldbl._Deps._Long_double
   #define LDBL_MAX		_CSTD _Ldbl._Dmax._Long_double
   #define LDBL_MIN		_CSTD _Ldbl._Dmin._Long_double
 #endif /* 199901L <= __STDC_VERSION__ */

  #define LDBL_DIG			18	/* _CSTD _Ldbl._Ddig */
  #define LDBL_MANT_DIG		64	/* _CSTD _Ldbl._Dmdig */
  #define LDBL_MAX_10_EXP	4932	/* _CSTD _Ldbl._Dmax10e */
  #define LDBL_MAX_EXP		16384	/* _CSTD _Ldbl._Dmaxe */
  #define LDBL_MIN_10_EXP	-4931	/* _CSTD _Ldbl._Dmin10e */
  #define LDBL_MIN_EXP		-16381	/* _CSTD _Ldbl._Dmine */

 #else /* 1 < _DLONG */

 #if 199901L <= __STDC_VERSION__
	/* IEEE 16 byte, hidden bit */

  #if defined(__cplusplus) || defined(__GNUC__) && __GNUC__ < 3
   #define LDBL_EPSILON	_CSTD _Ldbl._Deps._Long_double
   #define LDBL_MAX		_CSTD _Ldbl._Dmax._Long_double
   #define LDBL_MIN		_CSTD _Ldbl._Dmin._Long_double

  #else /* defined(__cplusplus) || defined(__GNUC__) && __GNUC__ < 3 */
   #define LDBL_EPSILON	0x8p-115L
   #define LDBL_MAX		0xf.fffffffffffffffffffffffffff8p+16380L

   #if !defined(__EDG__) || 245 < __EDG_VERSION__
    #define LDBL_MIN		0x8p-16385L

   #else /* !defined(__EDG__) || 245 < __EDG_VERSION__ */
    #define LDBL_MIN		_CSTD _Ldbl._Dmin._Long_double
   #endif /* !defined(__EDG__) || 245 < __EDG_VERSION__ */

  #endif /* defined(__cplusplus) || defined(__GNUC__) && __GNUC__ < 3 */

 #else /* 199901L <= __STDC_VERSION__ */
   #define LDBL_EPSILON	_CSTD _Ldbl._Deps._Long_double
   #define LDBL_MAX		_CSTD _Ldbl._Dmax._Long_double
   #define LDBL_MIN		_CSTD _Ldbl._Dmin._Long_double
 #endif /* 199901L <= __STDC_VERSION__ */

  #define LDBL_DIG			33	/* _CSTD _Ldbl._Ddig */
  #define LDBL_MANT_DIG		113	/* _CSTD _Ldbl._Dmdig */
  #define LDBL_MAX_10_EXP	4932	/* _CSTD _Ldbl._Dmax10e */
  #define LDBL_MAX_EXP		16384	/* _CSTD _Ldbl._Dmaxe */
  #define LDBL_MIN_10_EXP	-4931	/* _CSTD _Ldbl._Dmin10e */
  #define LDBL_MIN_EXP		-16381	/* _CSTD _Ldbl._Dmine */
 #endif	/* _DLONG */

_C_STD_END
#endif /* _FLOAT */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.01:1566 */
