#ifndef QTESTENV_H
#define QTESTENV_H
/*
=======================================================================
              Copyright ©  2008 Qualcomm Technologies Incorporated.
                        All Rights Reserved.
                Qualcomm Confidential and Proprietary
=======================================================================
*/

#include "AEEIEnv.h"

//--------------------------------------------------------------------
// QTestEnv : an implementation of IEnv for qtests.
//
// QTestEnv is implemented on top of C stdlib and stdio.  Obvious error
// conditions terminate program execution and print an error message.
// Its memory allocation functions detect common heap usage errors
// (buffer overrun, underrun, freeing bogus pointers, using memory after
// free, etc.).  Its CreateInstance() by default responds to no class IDs,
// but it is easy to customize.
//--------------------------------------------------------------------

// Argument to OverrideCI.  This behaves the same as IEnv_CreateInstance().
//
typedef int QTestEnvCIFunc(void *pvCxt, IEnv *piEnv, AEECLSID id, void **ppOut);

typedef struct QTestEnv QTestEnv;


// Create an implementation of IEnv for a qtest.
//
// If ppiEnv is not NULL, *ppiEnv will be filled with an IEnv*.  This will *not*
// constitute a second reference count.
//
extern QTestEnv * QTestEnv_New(IEnv **ppiEnv);


// Orderly cleanup; same as final IEnv_Release():
//
//  - Calls AtExit callbacks
//  - ASSERTs there are no leaks of memory
//  - ASSERTs the QTestEnv reference count is equal to 1
//  - delete the QTestEnv.
//
extern void QTestEnv_Done(QTestEnv *me);


// Delete an instance of QTestEnv regardless of the reference count; do not
// ASSERT; do not call AtExit callbacks.
//
extern void QTestEnv_Delete(QTestEnv *);


// Return the number of bytes that have been allocated.
//
extern int QTestEnv_Allocated(QTestEnv *me);


// Return an IEnv* interface to QTestEnv.
//
// For reference counting purposes, treat this like a cast rather than like
// QueryInterface.  It does not increment the object's reference count.
//
extern IEnv *QTestEnv_ToIEnv(QTestEnv *me);


// Return an IRealloc* interface to QTestEnv.
//
// For reference counting purposes, treat this like a cast rather than like
// QueryInterface.  It does not increment the object's reference count.
//
extern IRealloc *QTestEnv_ToIRealloc(QTestEnv *p);


// Override this QTestEnv's CreateInstance() behavior.
//
// The provided function is placed at the front of the list of override
// functions.  This function will be called when the QTestEnv's
// IEnv_CreateInstance() method is called with the same 'id' and 'ppOut'
// parameters.
//
// Override functions can return -1 to defer to the next override in the list.
// If any other value is returned, the CI operation is considered complete and
// that value will be returned to the caller of IEnv_CreateInstance().
//
extern void QTestEnv_OverrideCI(QTestEnv *pte, QTestEnvCIFunc *pfnCI, void *pvCI);


// Cause allocation to fail.
//
// Bits in uFailMask determine which succeeding malloc and realloc-larger
// requests will succeed.  If LSB is 0, the next malloc succeeds.  If 1, the
// next malloc fails.  The value is shifted right after each attempt, and the
// MSB bit is preserved (a la signed shift).
//
// value  consequence
// -----  -----------------------------------------------------------
//   1    next mallocs fails, all others succeed
//  ~1    next malloc succeeds, all others fail
//   2    next malloc succeeds, second fails, all others succeed
//
extern void QTestEnv_SetFailureMask(QTestEnv *me, uint32 uFailMask);
extern void QTestEnv_GetFailureMask(QTestEnv *me, uint32* puFailMask);
extern int QTestEnv_TestFailure(QTestEnv *me);

#endif /* QTESTENV_H */
