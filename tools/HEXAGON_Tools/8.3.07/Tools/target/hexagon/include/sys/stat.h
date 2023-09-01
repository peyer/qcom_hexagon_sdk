/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/

/*      $NetBSD: stat.h,v 1.48 2005/02/03 19:20:02 perry Exp $  */

/*-
 * Copyright (c) 1982, 1986, 1989, 1993
 *      The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
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
 *      @(#)stat.h      8.12 (Berkeley) 8/17/94
 */


/*
 stat.h

 Contains definitions for file and filesystem stat functions.

 */

#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

#include <types.h>

/*
 * This structure for containing what should be returned 
 * when doing a stat system call is defined here.
 * Its basic requirements are described by
 * the Posix standard.  The notion of uid and gid are
 * removed for our simplified environment.
 */
struct stat {
        dev_t     st_dev;               /* inode's device */
        ino_t     st_ino;               /* inode's number */
        mode_t    st_mode;              /* inode protection mode */
        nlink_t   st_nlink;             /* number of hard links */
        dev_t     st_rdev;              /* device type */
        off_t     st_size;              /* file size, in bytes */
        time_t    st_atime;             /* time of last access */
        time_t    st_mtime;             /* time of last data modification */
        time_t    st_ctime;             /* time of last file status change */
};

/*
 * The following values are what have been typically used
 * in BSD and other Unix platforms for interoperability's sake
 * we use the same here:
 */
#define S_IFMT     0170000   /* type of file */
#define S_IFSOCK   0140000   /* socket */
#define S_IFLNK    0120000   /* symbolic link */
#define S_IFREG    0100000   /* regular */
#define S_IFBLK    0060000   /* block special */
#define S_IFDIR    0040000   /* directory */
#define S_IFCHR    0020000   /* character special */
#define S_IFIFO    0010000   /* named pipe (fifo) */
#define S_ISUID    0004000   /* set user id on execution */
#define S_ISGID    0002000   /* set group id on execution */
#define S_ISVTX    0001000   /* save swapped text even after use */
#define S_IRWXU    00700     /* mask for owner permissions */
#define S_IRUSR    00400     /* read permission, owner */
#define S_IWUSR    00200     /* write permission, owner */
#define S_IXUSR    00100     /* execute permission, owner */
#define S_IRWXG    00070     /* mask for group permissions */
#define S_IRGRP    00040     /* read permission, group */
#define S_IWGRP    00020     /* write permission, group */
#define S_IXGRP    00010     /* execute permission, group */
#define S_IRWXO    00007     /* mask for permissions for others */
#define S_IROTH    00004     /* read permission, others */
#define S_IWOTH    00002     /* write permisson, others */
#define S_IXOTH    00001     /* execute permission, others */

#define	S_ISDIR(mode)	(((mode) & S_IFMT) == (S_IFDIR))
#define	S_ISCHR(mode)	(((mode) & S_IFMT) == (S_IFCHR))
#define	S_ISBLK(mode)	(((mode) & S_IFMT) == (S_IFBLK))
#define	S_ISREG(mode)	(((mode) & S_IFMT) == (S_IFREG))
#define S_ISFIFO(mode)	(((mode) & S_IFMT) == (S_IFIFO))
#define S_ISLNK(mode)	(((mode) & S_IFMT) == (S_IFLNK))
#define S_ISSOCK(mode)	(((mode) & S_IFMT) == (S_IFSOCK))

struct statvfs {
           unsigned long  f_bsize;    /* file system block size */
           unsigned long  f_frsize;   /* fragment size */
           fsblkcnt_t     f_blocks;   /* size of fs in f_frsize units */
           fsblkcnt_t     f_bfree;    /* # free blocks */
           fsblkcnt_t     f_bavail;   /* # free blocks for non-root */
           fsfilcnt_t     f_files;    /* # inodes */
           fsfilcnt_t     f_ffree;    /* # free inodes */
           fsfilcnt_t     f_favail;   /* # free inodes for non-root */
           unsigned long  f_fsid;     /* file system id */
           unsigned long  f_flag;     /* mount flags */
           unsigned long  f_namemax;  /* maximum filename length */
         };

_EXTERN_C	/* declarations */
int fstat(int filedes, struct stat *buf) _NO_THROW;
int fstatvfs(int fd, struct statvfs *buf) _NO_THROW;
int stat (const char *, struct stat *) _NO_THROW;
_END_EXTERN_C


#endif /*_SYS_STAT_H*/
