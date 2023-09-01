/***************************************************************************
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 ****************************************************************************/

#ifndef HWIO_H
#define HWIO_H 1

#include <stdint.h>

static inline uint32_t HWIO_IN(long addr) {
	return *((volatile uint32_t *)addr);
}

static inline void HWIO_OUT(long addr, uint32_t val) {
	*((volatile uint32_t *)addr) = val;
}


#endif

