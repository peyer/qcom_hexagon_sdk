#include "AEEatomic.h"
#include <assert.h>

static uint32 q_atomic_Add(uint32* puDest, int nAdd) {
   *puDest += nAdd;
   return *puDest;
}

static uint32 q_atomic_Exchange(uint32* puDest, uint32 uVal) {
   int previous = *puDest;
   *puDest = uVal;
   return previous;
}
static uint32 q_atomic_CompareAndExchange(uint32 *puDest, uint32 uExchange, uint32 uCompare) {
   int previous = *puDest;
   if (*puDest == uCompare) {
      *puDest = uExchange;
   }
   return previous;
}

static uint32 q_atomic_CompareOrAdd(uint32* puDest, uint32 uCompare, int nAdd) {
   if (*puDest != uCompare) {
      *puDest += nAdd;
   }
   return *puDest;
}

int main(void) {
   uint32 dest = 0, qdest = 0, result, qresult;
   uintptr_t destup = 0;
   qresult = q_atomic_Add(&qdest, 10);
   result = atomic_Add(&dest, 10);
   assert(qdest == dest);
   assert(result == qresult);


   qresult = q_atomic_Exchange(&qdest, 20);
   result = atomic_Exchange(&dest, 20);

   assert(qdest == dest);
   assert(result == qresult);

   qdest = 20;
   dest = 20;
   qresult = q_atomic_CompareAndExchange(&qdest, 30, 0);
   result = atomic_CompareAndExchange(&dest, 30, 0);
   assert(qdest == 20);
   assert(dest == 20);
   assert(qdest == dest);
   assert(result == qresult);

   qresult = q_atomic_CompareAndExchange(&qdest, 40, 20);
   result = atomic_CompareAndExchange(&dest, 40, 20);

   assert(qdest == 40);
   assert(dest == 40);
   assert(qdest == dest);
   assert(result == qresult);

   qdest = 20;
   dest = 20;
   qresult = q_atomic_CompareOrAdd(&qdest, 20, 10);
   result = atomic_CompareOrAdd(&dest, 20, 10);
   assert(qdest == 20);
   assert(dest == 20);
   assert(qdest == dest);
   assert(result == qresult);

   qresult = q_atomic_CompareOrAdd(&qdest, 0, 10);
   result = atomic_CompareOrAdd(&dest, 0, 10);
   assert(qdest == 30);
   assert(dest == 30);
   assert(qdest == dest);
   assert(result == qresult);

   assert(0 == atomic_CompareAndExchangeUP(&destup, 1, 0));
   assert(destup == 1);
   return 0;
}
