/*=====================================================================
               Copyright (c) 2013 QUALCOMM Incorporated.
                          All Rights Reserved.
                 QUALCOMM Confidential and Proprietary
======================================================================*/
#pragma once

#if defined(__hexagon__)
#include "q6sim_timer.h"
#include <unistd.h>
#include <fcntl.h>
// This should be in the unistd for hexagon, but it's not?
ssize_t      write(int, const void *, size_t);

#define FH int
#define O_CREAT_WRONLY_TRUNC (O_CREAT | O_WRONLY | O_TRUNC)
#define IS_INVALID_FILE_HANDLE(_a) (_a < 0)

#define SETUP()
#define RESET_PMU()     __asm__ __volatile__ (" r0 = #0x48 ; trap0(#0); \n" : : : "r0","r1","r2","r3","r4","r5","r6","r7","memory")
#define DUMP_PMU()      __asm__ __volatile__ (" r0 = #0x4a ; trap0(#0); \n" : : : "r0","r1","r2","r3","r4","r5","r6","r7","memory")
#define READ_PCYCLES    q6sim_read_pcycles


#else
// file I/O
#define FH FILE*
#define O_RDONLY "rb"
#define O_CREAT_WRONLY_TRUNC "wb"
#define open(_a, _b, ...) fopen(_a, _b)
#define close(_a) fclose(_a)
#define read(_a, _b, _c) fread(_b, sizeof(char), _c, _a)
#define write(_a, _b, _c) fwrite(_b, sizeof(char), _c, _a)
#define IS_INVALID_FILE_HANDLE(_a) (_a == 0)

// memory allocation
#define memalign(_a, _b) malloc(_b)
#define __attribute__(_a)

#define READ_PCYCLES()  0
#define RESET_PMU()
#define DUMP_PMU()
#define SETUP()

#endif

