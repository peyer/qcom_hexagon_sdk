// new.h standard header
#ifndef _NEW_H_
#define _NEW_H_
#include <new>

 #if _HAS_NAMESPACE
using std::bad_alloc;
using std::new_handler;
using std::nothrow;
using std::nothrow_t;
using std::set_new_handler;
 #endif /* _HAS_NAMESPACE */

#endif /* _NEW_ */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.01:1566 */
