#ifndef FARF_HIGH
#define FARF_HIGH  1
#endif // FARF_HIGH  1

#ifndef VERIFY_PRINT_ERROR
#define VERIFY_PRINT_ERROR
#endif //VERIFY_PRINT_ERROR

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "AEEStdErr.h"
#include "HAP_farf.h"
#include "AEEstd.h"
#include "remote.h"
#include "rpcmem.h"
#include "rpcperf.h"
#include "verify.h"

#ifndef CALLOC
#define CALLOC calloc
#endif

#ifndef MALLOC
#define MALLOC malloc
#endif

#ifndef FREE
#define FREE free
#endif

static int uncached;

uint64_t diff_time(struct timeval *start, struct timeval * end) {
   uint64_t s = start->tv_sec * 1000000 + start->tv_usec;
   uint64_t e = end->tv_sec * 1000000 + end->tv_usec;
   return e - s;
}

static int rpcperf_alloc(int size, int contig, void **ppo) {
   int nErr = 0;

   if(!contig) {
      VERIFY(0 != (*ppo = CALLOC(size, 1)));
   } else {
      const char* eheap = getenv("RPCPERF_HEAP_ID");
      int heapid = eheap == 0 ? -1 : atoi(eheap);
      const char* eflags = getenv("RPCPERF_HEAP_FLAGS");
      uint32 flags = eflags == 0 ? RPCMEM_DEFAULT_FLAGS : atoi(eflags);
      if(uncached) {
         flags |= RPCMEM_HEAP_UNCACHED;
      }
      FARF(HIGH, "rpcmem_alloc(%x, %x, %d)", heapid, flags, size); 
      VERIFY(0 != (*ppo = rpcmem_alloc(heapid, flags, size)));
   }
bail:
   return nErr;
}

static void rpcperf_free(void *po, int contig) {
   if(!contig) {
      FREE(po);
   } else {
      rpcmem_free(po);
   }
}

int rpcperf_main(int argc, char* argv[]) {
   int nErr = 0;
   remote_handle handle = 0;
   uint32 ii, cnt;

   rpcmem_init();
   while(1) {
      VERIFY(argc > 1);
      if (0 == strcmp(argv[1], "power_boost_on")) {
         VERIFY(0 == rpcperf_power_boost(1));
         argv++;
         argc--;
         continue;
      } else if (0 == strcmp(argv[1], "uncached")) {
         uncached = 1;
         argv++;
         argc--;
         continue;
      } else {
         break;
      }
   }
   VERIFY(argc > 2);
   cnt = atol(argv[2]);
   if (0 == strcmp(argv[1], "noop")) {
      for(ii = 0; ii < cnt; ++ii) {
         VERIFY(0 == rpcperf_noop());
      }
   } else if(0 == strcmp(argv[1], "inbuf")) {
      uint8 *x;
      struct timeval start, end;
      int size = 16*16, rw = 0, contig = 0;
      if(argc > 3) {
         size = atol(argv[3]);
      }
      if(argc > 4) {
         contig = atol(argv[4]);
      }
      if(argc > 5) {
         rw = atol(argv[5]);
      }
      gettimeofday(&start, 0);
      VERIFY(0 == rpcperf_alloc(size, contig, (void **)&x));
      for(ii = 0; ii < cnt; ++ii) {
         if(rw == 1) {
            int ii;
            memset(x, 0, size);
            for(ii = 0; ii < size; ii += 32) {
               x[ii] = (uint8)ii;
            }
         }
         if(0 == (ii % 1000)) {
            gettimeofday(&end, 0);
            FARF(HIGH, "time: %llu\n", (long long unsigned int)diff_time(&start, &end));
            gettimeofday(&start, 0);
         }
         nErr |= rpcperf_inbuf(x, size, rw);
      }
      VERIFY(0 == nErr);
      rpcperf_free(x, contig);
   } else if(0 == strcmp(argv[1], "routbuf")) {
      struct timeval start, end;
      uint8 *x;
      int size = 16*16, rw = 0, contig = 0;
      if(argc > 3) {
         size = atol(argv[3]);
      }
      if(argc > 4) {
         contig = atol(argv[4]);
      }
      if(argc > 5) {
         rw = atol(argv[5]);
      }
      VERIFY(0 == rpcperf_alloc(size, contig, (void **)&x));
      for(ii = 0; ii < cnt; ++ii) {
         if(rw == 1) {
            memset(x, 0, size);
         }
         if(0 == (ii % 1000)) {
            gettimeofday(&end, 0);
            FARF(HIGH, "time: %llu\n", (long long unsigned int)diff_time(&start, &end));
            gettimeofday(&start, 0);
         }
         VERIFY(0 == (nErr = rpcperf_routbuf(x, size, rw)));
         if(rw == 1) {
            int ii;
            for(ii = 0; ii < size; ii += 32) {
               VERIFY(x[ii] == (uint8)ii);
            }
         }
      }
      rpcperf_free(x, contig);
   } else {
      printf("bad command %s\n", argv[1]);
      nErr = -1;
   }
   if (0 == strcmp(argv[argc - 1], "power_boost_off")) {
      VERIFY(0 == rpcperf_power_boost(0));
   } 
bail:
   if(handle) {
      remote_handle_close(handle);
   }
   rpcmem_deinit();
   return nErr;
}
