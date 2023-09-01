#ifndef ION_ALLOCATION_H
#define ION_ALLOCATION_H

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <android/log.h>

void alloc_init();
void alloc_finalize();
void* alloc_ion(size_t size);
void alloc_ion_free(void *ptr);
#endif
