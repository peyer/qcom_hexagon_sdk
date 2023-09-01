/* ctype.h standard header */
#ifndef _CTYPE
#define _CTYPE
#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

_C_STD_BEGIN

		/* _Ctype code bits */
#define _XB		0x400 /* extra blank */
#define _XA		0x200 /* extra alphabetic */
#define _XS		0x100 /* extra space */
#define _BB		0x80 /* BEL, BS, etc. */
#define _CN		0x40 /* CR, FF, HT, NL, VT */
#define _DI		0x20 /* '0'-'9' */
#define _LO		0x10 /* 'a'-'z' */
#define _PU		0x08 /* punctuation */
#define _SP		0x04 /* space */
#define _UP		0x02 /* 'A'-'Z' */
#define _XD		0x01 /* '0'-'9', 'A'-'F', 'a'-'f' */

_C_LIB_DECL
typedef const short *_Ctype_t;

_Ctype_t _Getpctype(void);
_Ctype_t _Getptolower(void);
_Ctype_t _Getptoupper(void);

 #if !_MULTI_THREAD || _COMPILER_TLS && !_GLOBAL_LOCALE
extern _Ctype_t _TLS_QUAL _Ctype;
extern _Ctype_t _TLS_QUAL _Tolotab;
extern _Ctype_t _TLS_QUAL _Touptab;

  #define _Getpctype()		_Ctype
  #define _Getptolower()	_Tolotab
  #define _Getptoupper()	_Touptab
 #endif /* !_MULTI_THREAD || _COMPILER_TLS && !_GLOBAL_LOCALE */

#define _Getchrtype(x)	_Getpctype()[(int)(x)]
#define _CTolower(x)	_Getptolower()[(int)(x)]
#define _CToupper(x)	_Getptoupper()[(int)(x)]
_END_C_LIB_DECL

 #if _HAS_INLINE
		// INLINES
_C_LIB_DECL
_GNUC_EXTERN_INLINE inline int isalnum(int _Ch)
	{	// check alnum bits
	return (_Getchrtype(_Ch) & (_DI|_LO|_UP|_XA));
	}

_GNUC_EXTERN_INLINE inline int isalpha(int _Ch)
	{	// check alpha bits
	return (_Getchrtype(_Ch) & (_LO|_UP|_XA));
	}

_GNUC_EXTERN_INLINE inline int iscntrl(int _Ch)
	{	// check cntrl bits
	return (_Getchrtype(_Ch) & _BB);
	}

_GNUC_EXTERN_INLINE inline int isdigit(int _Ch)
	{	// check digit bits
	return (_Getchrtype(_Ch) & _DI);
	}

_GNUC_EXTERN_INLINE inline int isgraph(int _Ch)
	{	// check graph bits
	return (_Getchrtype(_Ch) & (_DI|_LO|_PU|_UP|_XA));
	}

_GNUC_EXTERN_INLINE inline int islower(int _Ch)
	{	// check alnum bits
	return (_Getchrtype(_Ch) & _LO);
	}

_GNUC_EXTERN_INLINE inline int isprint(int _Ch)
	{	// check print bits
	return (_Getchrtype(_Ch) & (_DI|_LO|_PU|_SP|_UP|_XA));
	}

_GNUC_EXTERN_INLINE inline int ispunct(int _Ch)
	{	// check punct bits
	return (_Getchrtype(_Ch) & _PU);
	}

_GNUC_EXTERN_INLINE inline int isspace(int _Ch)
	{	// check space bits
	return (_Getchrtype(_Ch) & (_CN|_SP|_XS));
	}

_GNUC_EXTERN_INLINE inline int isupper(int _Ch)
	{	// check upper bits
	return (_Getchrtype(_Ch) & _UP);
	}

_GNUC_EXTERN_INLINE inline int isxdigit(int _Ch)
	{	// check xdigit bits
	return (_Getchrtype(_Ch) & _XD);
	}

 #if _HAS_C9X
_GNUC_EXTERN_INLINE inline int isblank(int _Ch)
	{	// check blank bits
	return (_Getchrtype(_Ch) & (_SP|_XB));
	}
 #endif /* _IS_C9X */

_GNUC_EXTERN_INLINE inline int tolower(int _Ch)
	{	// convert to lower case
	return (_CTolower(_Ch));
	}

_GNUC_EXTERN_INLINE inline int toupper(int _Ch)
	{	// convert to upper case
	return (_CToupper(_Ch));
	}
int _toupper(int);
int _tolower(int);
int isascii (int);
int toascii (int);
_END_C_LIB_DECL

 #else
		/* declarations and macro overrides, for C90 */
_C_LIB_DECL
int isalnum(int) _NO_THROW;
int isalpha(int) _NO_THROW;
int iscntrl(int) _NO_THROW;
int isdigit(int) _NO_THROW;
int isgraph(int) _NO_THROW;
int islower(int) _NO_THROW;
int isprint(int) _NO_THROW;
int ispunct(int) _NO_THROW;
int isspace(int) _NO_THROW;
int isupper(int) _NO_THROW;
int isxdigit(int) _NO_THROW;
int tolower(int) _NO_THROW;
int toupper(int) _NO_THROW;
int _toupper(int) _NO_THROW;
int _tolower(int) _NO_THROW;
int isascii (int) _NO_THROW;
int toascii (int) _NO_THROW;

_END_C_LIB_DECL

  #define isalnum(c)	(_Getchrtype(c) & (_DI|_LO|_UP|_XA))
  #define isalpha(c)	(_Getchrtype(c) & (_LO|_UP|_XA))
  #define iscntrl(c)	(_Getchrtype(c) & _BB)
  #define isdigit(c)	(_Getchrtype(c) & _DI)
  #define isgraph(c)	(_Getchrtype(c) & (_DI|_LO|_PU|_UP|_XA))
  #define islower(c)	(_Getchrtype(c) & _LO)
  #define isprint(c)	(_Getchrtype(c) & (_DI|_LO|_PU|_SP|_UP|_XA))
  #define ispunct(c)	(_Getchrtype(c) & _PU)
  #define isspace(c)	(_Getchrtype(c) & (_CN|_SP|_XS))
  #define isupper(c)	(_Getchrtype(c) & _UP)
  #define isxdigit(c)	(_Getchrtype(c) & _XD)

 #ifndef _DNK_RENAME
  #define tolower(c)	_CTolower(c)
  #define toupper(c)	_CToupper(c)
 #endif /* _DNK_RENAME */

 #endif /* _HAS_INLINE */

_C_STD_END
#endif /* _CTYPE */

#ifdef _STD_USING
using _CSTD isalnum; using _CSTD isalpha; using _CSTD iscntrl;
using _CSTD isdigit; using _CSTD isgraph; using _CSTD islower;
using _CSTD isprint; using _CSTD ispunct; using _CSTD isspace;
using _CSTD isupper; using _CSTD isxdigit; using _CSTD tolower;
using _CSTD toupper;

 #if _HAS_C9X
using _CSTD isblank;
 #endif /* _IS_C9X */

#endif /* _STD_USING */


/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.01:1566 */
