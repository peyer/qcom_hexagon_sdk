/*==============================================================================
  Copyright (c) 2012-2013, 2017 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#if defined(WINNT) || defined (_WIN32_WINNT)
#ifndef __MINGW32__
#define vsnprintf(a,b,c,d) vsnprintf_s(a, b, b,c,d)
#pragma warning( disable : 4068 )
#endif

void* qurt_thread_get_tls_base(void *base) {
  return NULL;
}

#else

void* __attribute__((weak)) qurt_thread_get_tls_base(void *base) {
  return NULL;
}

#endif

#include "HAP_mem.h"
#include "HAP_debug.h"
#include "HAP_perf.h"
#include "HAP_power.h"
#include "HAP_pls.h"

int HAP_mem_set_grow_size(uint64 min, uint64 max) {return 0;}

int HAP_cx_malloc(void* pctx, uint32 bytes, void** pptr)
{
  (void)pctx;

  *pptr = malloc(bytes);
  if (*pptr) {
    return 0;
  }
  return 1;
}

int HAP_cx_free(void* pctx, void* ptr)
{
  (void)pctx;

  free(ptr);
  return 1;
}

void* HAP_mmap(void *addr, int len, int prot, int flags, int fd, long offset)
{
	(void)addr;
	(void)len;
	(void)prot;
	(void)flags;
	(void)offset;
	
	return (void *)fd;
	
	
}

int HAP_munmap(void *addr, int len)
{
	(void)addr;
	(void)len;
	
	return (0);
}

int HAP_mmap_get(int fd, void **vaddr, uint64 *paddr)
{
	(void)fd;
	(void)paddr;
	(void)vaddr;
	return AEE_EUNSUPPORTED;
}

int HAP_mmap_put(int fd)
{
	(void)fd;
	return AEE_EUNSUPPORTED;
}

int HAP_mem_get_stats(struct HAP_mem_stats *stats)
{
	(void)stats;
	return AEE_EUNSUPPORTED;	
}



#if defined(WINNT) || defined (_WIN32_WINNT)
void *memalign(size_t block_size, size_t size) {
   return 0;
}
#else
void* __attribute__((weak)) __wrap_malloc(size_t size);
void __attribute__((weak))__wrap_free(void *ptr);
void * __attribute__((weak))__wrap_calloc(size_t num_elem, size_t elem_size);
void * __attribute__((weak))__wrap_realloc(void *ptr, size_t size);
void * __attribute__((weak))__wrap_memalign(size_t blocksize, size_t size);
#endif

#ifdef LEAK_TRACKER

#define MEM_HEAP_CALLER_ADDRESS(level) ((void *)__builtin_return_address(level))

typedef struct leak_tracker_leak leak_tracker_leak;
struct leak_tracker_leak {
  leak_tracker_leak* next;
  void* addr;
  int size;
  void* caddr[5];
};

static leak_tracker_leak* leak_tracker_head = 0;
static int leak_tracker_enabled = 0;

extern void __real_free(void*);
extern void* __real_malloc(size_t);

void leak_tracker_init(void)
{
  printf("\n");
  while (leak_tracker_head != 0) {
    leak_tracker_leak* leak = leak_tracker_head;
    leak_tracker_head = leak->next;
    __real_free(leak);
  }
  leak_tracker_enabled = 1;
}

int leak_tracker_leaks(void)
{
  int i = 0;
  leak_tracker_leak* leak;

  for (i = 0, leak = leak_tracker_head; leak != 0; i++, leak = leak->next) {
    printf("leak %d: %p (%d bytes)\n", i, leak->addr, leak->size);
    printf("leak %d addr 0 = %p\n", i, leak->caddr[0]);
    printf("leak %d addr 1 = %p\n", i, leak->caddr[1]);
    printf("leak %d addr 2 = %p\n", i, leak->caddr[2]);
    printf("leak %d addr 3 = %p\n", i, leak->caddr[3]);
    printf("leak %d addr 4 = %p\n", i, leak->caddr[4]);
  }
  leak_tracker_enabled = 0;
  return i;
}
#endif

size_t memsmove(void* dst, size_t dst_size, const void* src, size_t src_size)
{
  size_t  copy_size = (dst_size <= src_size) ? dst_size : src_size;
  memmove(dst, src, copy_size);
  return copy_size;
}

size_t memscpy(void* dst, size_t dst_size, const void* src, size_t src_size)
{
  size_t  copy_size = (dst_size <= src_size) ? dst_size : src_size;
  memcpy(dst, src, copy_size);
  return copy_size;
}

void* __wrap_malloc(size_t size)
{
#ifdef LEAK_TRACKER
  void* addr = __real_malloc(size);
  if (addr && leak_tracker_enabled) {
    leak_tracker_leak* leak = (leak_tracker_leak*)__real_malloc(sizeof(leak_tracker_leak));
    if (leak) {
      memset(leak, 0, sizeof(leak_tracker_leak));
      leak->addr = addr;
      leak->size = size;
      leak->caddr[0] =  __builtin_extract_return_addr(MEM_HEAP_CALLER_ADDRESS(0));
      leak->caddr[1] =  __builtin_extract_return_addr(MEM_HEAP_CALLER_ADDRESS(1));
      leak->caddr[2] =  __builtin_extract_return_addr(MEM_HEAP_CALLER_ADDRESS(2));
      leak->caddr[3] =  __builtin_extract_return_addr(MEM_HEAP_CALLER_ADDRESS(3));
      /*leak->caddr[4] =  __builtin_extract_return_addr(MEM_HEAP_CALLER_ADDRESS(4));*/

      leak->next = leak_tracker_head;
      leak_tracker_head = leak;
    }
  }
  return addr;
#else
  return malloc(size);
#endif
}

void __wrap_free(void* ptr)
{
#ifdef LEAK_TRACKER
  leak_tracker_leak* leak = 0;
  leak_tracker_leak* last_leak = 0;

  for (leak = leak_tracker_head; leak != 0; leak = leak->next) {
    if (leak->addr == ptr) {
      if (leak == leak_tracker_head) {
        leak_tracker_head = leak->next;
      } else {
        last_leak->next = leak->next;
      }
      __real_free(leak);
      break;
    } else {
      last_leak = leak;
    }
  }
  __real_free(ptr);
#else
  free(ptr);
#endif
}

void* __wrap_calloc(size_t num, size_t size)
{
  return calloc(num, size);
}

void* __wrap_realloc(void* ptr, size_t size)
{
  return realloc(ptr, size);
}
#ifndef __APPLE__
extern void* memalign(size_t boundary, size_t size);

void* __wrap_memalign(size_t blocksize, size_t size)
{
  return memalign(blocksize, size);
}
#endif

#include "pls.h"
static struct pls_table pls = {0};

void test_util_pls_deinit(void) {
   pls_thread_deinit(&pls);
}
static void pls_init(void) {
   if(pls.uRefs == 0) {
      pls_ctor(&pls, 0);
   }
}
/*
** Stub function to avoid linker error from the libc.a that calls this function. 
** Not needed for any functionality.
*/
int sys_ftrunc(int fd, int offset) 
{
   printf("test_utils.c::sys_ftrunc stubbed out\n");
   return -1;
}
int sys_opendir (char *pathname)
{
   printf("test_utils.c::sys_opendir stubbed out\n");
   return -1;
} 
int adsp_mmap_fd_getinfo(int fd, uint32_t *len) {
   printf("test_utils.c::adsp_mmap_fd_getinfo stubbed out\n");
   return 0;
} 
 
int HAP_pls_add_lookup(uintptr_t type, uintptr_t key, int size, int (*ctor)(void* ctx, void* data), void* ctx, void (*dtor)(void* ctx), void** ppo) {
   pls_init();
   return pls_add_lookup_singleton(&pls, type, key, size, ctor, ctx, dtor, ppo);
}


int HAP_pls_add(uintptr_t type, uintptr_t key, int size, int (*ctor)(void* ctx, void* data), void* ctx, void (*dtor)(void*), void** ppo)
{
   pls_init();
   return pls_add(&pls, type, key, size, ctor, ctx, dtor, ppo);
}

int HAP_pls_lookup(uintptr_t type, uintptr_t key, void** ppo)
{
   pls_init();
   return pls_lookup(&pls, type, key, ppo);
}

long HAP_debug_ptrace(int req, unsigned int pid, void* addr, void* data)
{
  return 0;
}

uint64 HAP_perf_get_time_us(void)
{
   static long long start = 0;
  // TODO
  // assume 500 MHz on simulator
  //return HAP_perf_get_pcycles() / 500;
  return start++;
}

#ifdef __hexagon__
#include "hexagon_sim_timer.h"
uint64 HAP_perf_get_pcycles(void)
{
  uint64_t pcycle;
  asm volatile ("%[pcycle] = C15:14" : [pcycle] "=r"(pcycle));
  return pcycle;
}
#else
uint64 HAP_perf_get_pcycles(void)
{
 // TODO
  return (uint64)0;
}
#endif

uint64 HAP_perf_get_qtimer_count(void)
{
  // TODO
  return (uint64)0;
}

uint64 HAP_perf_qtimer_count_to_us(uint64 count)
{
  /* Converts ticks into microseconds
        1 tick = 1/19.2MHz seconds
        Micro Seconds = Ticks * 10ULL/192ULL */
  return (uint64)(count * 10ull / 192ull);
}

int HAP_power_request(int clock, int bus, int latency)
{
  return 0;
}

int HAP_power_request_abs(int clock, int bus, int latency)
{
  return 0;
}

int HAP_power_get_max_speed(int* clock_max, int* bus_max)
{
  return 0;
}
int HAP_HVX_power_request(void)
{
  return 0;
}
int HAP_HVX_power_release(void)
{
  return 0;
}
int HAP_power_set(void* context, HAP_power_request_t* request)
{
  return 0;
}
int HAP_power_get(void* context, HAP_power_response_t* response)
{
  // fill with some reasonable values.
  switch (response->type)
  {
    case HAP_power_get_max_mips:
        response->max_mips = 800;
        break;

    case HAP_power_get_max_bus_bw:
        response->max_bus_bw = 4000000000ULL;
        break;

    case HAP_power_get_client_class:
        response->client_class = 0;
        break;

    case HAP_power_get_clk_Freq:
        response->clkFreqHz = 800;
        break;

    case HAP_power_get_aggregateAVSMpps:
        response->aggregateAVSMpps = 0;
        break;

    default:
        break;
  }
  return 0;
}

int adsp_power_boost_on(void);
void adsp_power_boost_off(void);

int adsp_power_boost_on(void)
{
  return HAP_power_request(100, 100, 1000);
}

void adsp_power_boost_off(void)
{
  (void)HAP_power_request(0, 0, 10000);
}

int perf_add_usec(uintptr_t ix, uint64_t usec) {
   return 0;
}

uint32_t HAP_get_chip_family_id(void)
{
    return 0;
}
uint32_t HAP_get_dsp_domain(void)
{
	return 3;
}

#ifdef __hexagon__
#if (__HEXAGON_ARCH__ >= 65)
//This is the global varaible that is used by QuRT kernel to set up the
//default heapSize at bootstraping time. Here we override it with 512Mhz 
unsigned heapSize = 0x40000000;      /* Set a 1024MB heap */
unsigned int __attribute__((section(".data"))) qurt_usermalloc_size = 0;

int sys_get_cmdline(char *, int);
int qtest_get_cmdline(char *outBuf, int outSize)
{
    const int bufsize = 1024;
    char *buf = malloc(bufsize);
    if (!buf) return -1;
//    char buf[1024];  // for holding & parsing the command line

    // system call to retrieve the command line, supported by q6 simulator.
    int r = sys_get_cmdline(buf, bufsize);
    if(r == 0){
        size_t i = 0;
        //Search until first non-delimiter
        for(i = 0; (i < bufsize && buf[i] == ' '); i++){}

        //Loop until first delimiter 
        for(; (i < bufsize && buf[i] != ' ' && buf[i] != '\0'); i++){}

        size_t copySize=(outSize<=(bufsize-i)?outSize:(bufsize-i));

        memcpy(outBuf, buf+i, copySize);
        printf("qtest heapSize: <%d>\n",heapSize);
        printf("qtest cmdline: <%s>\n",outBuf);
    }
    free(buf);
    return r;
}


#else
int sys_get_cmdline(char *, int);
int qtest_get_cmdline(char *outBuf, int outSize)
{
    return sys_get_cmdline(outBuf, outSize); 
}
#endif

//Stubbing for simulation target
typedef uint64_t remote_handle64;
int remote_handle64_control(remote_handle64 h, uint32_t req, void* data, uint32_t datalen) {
    return 0;
}

#endif //#ifdef __hexagon__

//linker cant find this object
#include "HAP_debug_printf.c"
