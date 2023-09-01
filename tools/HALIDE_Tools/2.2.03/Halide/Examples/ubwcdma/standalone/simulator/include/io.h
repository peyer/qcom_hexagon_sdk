/* ======================================================================== */
/*  QUALCOMM TECHNOLOGIES, INC.                                             */
/*                                                                          */
/*  Halide for HVX Image Processing Example                                 */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*          Copyright (c) 2016 QUALCOMM TECHNOLOGIES Incorporated.          */
/*                           All Rights Reserved.                           */
/*                  QUALCOMM Confidential and Proprietary                   */
/* ======================================================================== */
#ifndef IO_H
#define IO_H
#include "q6sim_timer.h"
#if defined(__hexagon__)
#include <unistd.h>
#include <fcntl.h>
// This should be in the unistd for hexagon, but it's not?
extern "C" {
ssize_t      write(int, const void *, size_t);
}
#define FH int
#define O_CREAT_WRONLY_TRUNC (O_CREAT | O_WRONLY | O_TRUNC)
#define IS_INVALID_FILE_HANDLE(_a) (_a < 0)

#define RESET_PMU()     __asm__ __volatile__ (" r0 = #0x48 ; trap0(#0); \n" : : : "r0","r1","r2","r3","r4","r5","r6","r7","memory")
#define DUMP_PMU()      __asm__ __volatile__ (" r0 = #0x4a ; trap0(#0); \n" : : : "r0","r1","r2","r3","r4","r5","r6","r7","memory")
#define READ_PCYCLES    q6sim_read_pcycles

#else

#define RESET_PMU()
#define DUMP_PMU()
#define READ_PCYCLES()  0
#endif


int write_file(FH fp, unsigned char *src, int height, int width, int border_width) {
  int i;
#ifdef BORDERS
  for(i = 0; i < height; i++) {
#else
    for(i = border_width; i < height-border_width; i++) {
#endif

#ifdef BORDERS
      if(write(fp, &src[i*width], sizeof(unsigned char)*(width))!=(width)) {
#else
        if(write(fp, &src[(i*width)+border_width], sizeof(unsigned char)*(width-(border_width*2)))!=(width-(border_width*2))) {
#endif
      return 1;
    }
  }
  return 0;
}
#endif
