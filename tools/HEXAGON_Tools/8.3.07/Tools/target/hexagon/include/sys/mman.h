/*  $NetBSD: mman.h,v 1.35 2005/02/03 19:20:01 perry Exp $  */

/*-
 * Copyright (c) 1982, 1986, 1993
 *  The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *  @(#)mman.h  8.2 (Berkeley) 1/9/95
 */

#ifndef _SYS_MMAN_H_
#define _SYS_MMAN_H_

#include <types.h>

#define __BEGIN_DECLS
#define __END_DECLS

/*
 * Protections are chosen from these bits, or-ed together
 */
#define PROT_NONE   0x00    /* no permissions */
#define PROT_READ   0x01    /* pages can be read */
#define PROT_WRITE  0x02    /* pages can be written */
#define PROT_EXEC   0x04    /* pages can be executed */

/*
 * Flags contain sharing type and options.
 * Sharing types; choose one.
 */
#define MAP_SHARED  0x0001  /* share changes */
#define MAP_PRIVATE 0x0002  /* changes are private */

/*
 * Other flags
 */
#define MAP_FIXED    0x0010 /* map addr must be exactly as requested */
#define MAP_RENAME   0x0020 /* Sun: rename private pages to file */
#define MAP_NORESERVE    0x0040 /* Sun: don't reserve needed swap area */
#define MAP_INHERIT  0x0080 /* region is retained after exec */
#define MAP_HASSEMAPHORE 0x0200 /* region may contain semaphores */
#define MAP_TRYFIXED     0x0400 /* attempt hint address, even within break */
#define MAP_WIRED    0x0800 /* mlock() mapping when it is established */

/*
 * Mapping type
 */
#define MAP_FILE    0x0000  /* map from file (default) */
#define MAP_ANON    0x1000  /* allocated from memory, swap space */

/*
 * Alignment (expressed in log2).  Must be >= log2(PAGE_SIZE) and
 * < # bits in a pointer (26 (acorn26), 32 or 64).
 */
#define MAP_ALIGNED(n)      ((n) << MAP_ALIGNMENT_SHIFT)
#define MAP_ALIGNMENT_SHIFT 24
#define MAP_ALIGNMENT_MASK  MAP_ALIGNED(0xff)
#define MAP_ALIGNMENT_64KB  MAP_ALIGNED(16) /* 2^16 */
#define MAP_ALIGNMENT_16MB  MAP_ALIGNED(24) /* 2^24 */
#define MAP_ALIGNMENT_4GB   MAP_ALIGNED(32) /* 2^32 */
#define MAP_ALIGNMENT_1TB   MAP_ALIGNED(40) /* 2^40 */
#define MAP_ALIGNMENT_256TB MAP_ALIGNED(48) /* 2^48 */
#define MAP_ALIGNMENT_64PB  MAP_ALIGNED(56) /* 2^56 */

/*
 * Error indicator returned by mmap(2)
 */
#define MAP_FAILED  ((void *) -1)   /* mmap() failed */

/*
 * Flags to msync
 */
#define MS_ASYNC    0x01    /* perform asynchronous writes */
#define MS_INVALIDATE   0x02    /* invalidate cached data */
#define MS_SYNC     0x04    /* perform synchronous writes */

/*
 * Flags to mlockall
 */
#define MCL_CURRENT 0x01    /* lock all pages currently mapped */
#define MCL_FUTURE  0x02    /* lock all pages mapped in the future */

__BEGIN_DECLS
void   *mmap(void *, size_t, int, int, int, off_t) _NO_THROW;
int     munmap(void *, size_t) _NO_THROW;
int     mprotect(void *, size_t, int) _NO_THROW;
//int     msync(void *, size_t, int) __RENAME(__msync13);
int     mlock(const void *, size_t) _NO_THROW;
int     munlock(const void *, size_t) _NO_THROW;
int     mlockall(int) _NO_THROW;
int     munlockall(void) _NO_THROW;
__END_DECLS

#endif /* !_SYS_MMAN_H_ */
