/*	$NetBSD: search.h,v 1.16.2.2 2005/08/04 17:35:05 tron Exp $	*/

/*
 * Written by J.T. Conklin <jtc@NetBSD.org>
 * Public domain.
 */

#ifndef _SEARCH_H_
#define _SEARCH_H_

#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

#ifdef	_BSD_SIZE_T_
typedef	_BSD_SIZE_T_	size_t;
#undef	_BSD_SIZE_T_
#endif

typedef struct entry {
	char *key;
	void *data;
} ENTRY;

typedef enum {
	FIND, ENTER
} ACTION;

typedef enum {
	preorder,
	postorder,
	endorder,
	leaf
} VISIT;

#ifdef _SEARCH_PRIVATE
typedef struct node {
	char         *key;
	struct node  *llink, *rlink;
} node_t;
#endif

_EXTERN_C	/* declarations */
int hcreate (size_t ) _NO_THROW;
void hdestroy (void) _NO_THROW;
ENTRY *hsearch (ENTRY, ACTION) _NO_THROW;
void *tdelete (const void *vkey, void **vrootp, int (*compar) (const void *, const void *)) _NO_THROW;
void *tfind (const void *vkey, void *const *vrootp, int (*compar) (const void *, const void *)) _NO_THROW;
void *tsearch (const void *vkey, void **vrootp, int (*compar) (const void *, const void *)) _NO_THROW;
void twalk (const void *vroot, void (*action) (const void *, VISIT, int)) _NO_THROW;
_END_EXTERN_C

#endif /* !_SEARCH_H_ */
