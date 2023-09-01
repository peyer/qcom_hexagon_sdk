//#ifndef VERIFY_PRINT_INFO
//#define VERIFY_PRINT_INFO
//#endif //VERIFY_PRINT_ERROR

#include <assert.h>
#include <stdio.h>
#include <sys/mman.h>

//! mocks
extern int myopen(const char* name, int flags, ...) {
   return 1;
}
#define open(arg, ...) myopen(arg, ##__VA_ARGS__)
int myclose(int fd) {
   return 0;
}
#define close(fd) myclose(fd)

int myioctl(int, int, ...);
#define ioctl(fd, flags, data) myioctl(fd, flags, data)

#define mmap(addr, length, prot, flags, fd, offset) mymmap(addr, length, prot, flags, fd, offset)
void *mymmap(void *addr, size_t length, int prot, int flags,
                               int fd, off_t offset) {
   return (void*)8;
}
#define munmap(addr, size) mymunmap(addr, size)
int mymunmap(void *addr, size_t length) {
   assert((int)(uintptr_t)addr == 8);
   return 0;
}
void *remote_register_fd(int fd, int size) {
   return 0;
}

#include "rpcmem_android.c"

int test_flags;
int test_heap_id_mask;
int test_ran;
int myioctl(int fd, int flags, ...) {
   va_list va;
   va_start(va, flags);

   struct ion_allocation_data_kk *ion = (struct ion_allocation_data_kk *)va_arg(va, struct ion_allocation_data_kk *);
   va_end(va);
   if((int)flags == (int)ION_IOC_ALLOC_KK) {
      if(ion->flags == test_flags && ion->heap_id_mask == test_heap_id_mask) {
         test_ran = 1;
         return 0;
      }
      return -1;
   } else if ((int)flags == (int)ION_IOC_ALLOC_ICS) {
      return -1;
   } else {
      return 0;
   }
   return 0;
}

int main(void) {
   rpcmem_init();
   test_flags = SET_FLAG(0, ION_FLAG_CACHED);
   test_heap_id_mask = HEAP_ID_TO_MASK(ION_HEAP_ID_SYSTEM);
   test_ran = 0;
   assert(0 != rpcmem_alloc(RPCMEM_DEFAULT_HEAP, RPCMEM_DEFAULT_FLAGS, 1024)); 
   assert(1 == test_ran);

   test_flags = SET_FLAG(0, ION_FLAG_CACHED);
   test_heap_id_mask = HEAP_ID_TO_MASK(ION_HEAP_ID_SYSTEM);
   test_ran = 0;
   assert(0 != rpcmem_alloc(0xc0de, RPCMEM_HEAP_DEFAULT, 1024)); 
   assert(1 == test_ran);

   test_flags = 0;
   test_heap_id_mask = HEAP_ID_TO_MASK(ION_HEAP_ID_SYSTEM);
   test_ran = 0;
   assert(0 != rpcmem_alloc(0xc0de, RPCMEM_HEAP_DEFAULT|RPCMEM_HEAP_UNCACHED, 1024)); 
   assert(1 == test_ran);

   test_flags = 0;
   test_heap_id_mask = HEAP_ID_TO_MASK(ION_HEAP_ID_SYSTEM);
   test_ran = 0;
   assert(0 != rpcmem_alloc(0xc0de, RPCMEM_HEAP_DEFAULT|RPCMEM_HEAP_UNCACHED|RPCMEM_HEAP_NOREG, 1024)); 
   assert(1 == test_ran);

   test_flags = SET_FLAG(0, ION_FLAG_CACHED);
   test_heap_id_mask = HEAP_ID_TO_MASK(18);
   test_ran = 0;
   assert(0 != rpcmem_alloc(18, RPCMEM_DEFAULT_FLAGS, 1024)); 
   assert(1 == test_ran);

   test_flags = SET_FLAG(0, ION_FLAG_CACHED);
   test_heap_id_mask = HEAP_ID_TO_MASK(18);
   test_ran = 0;
   assert(0 != rpcmem_alloc(18, ION_FLAG_CACHED, 1024)); 
   assert(1 == test_ran);

   test_flags = 0;
   test_heap_id_mask = HEAP_ID_TO_MASK(18);
   test_ran = 0;
   assert(0 != rpcmem_alloc(18, 0, 1024)); 
   assert(1 == test_ran);

   rpcmem_deinit();
   return 0;
}
