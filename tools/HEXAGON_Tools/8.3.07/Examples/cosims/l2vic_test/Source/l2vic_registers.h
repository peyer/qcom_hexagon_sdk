/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date: 2021/08/04 $
*****************************************************************/
#ifndef _L2VIC_REGISTERS_
#define _L2VIC_REGISTERS_

typedef unsigned int uint32;
typedef volatile uint32 vuint32;
typedef int int32;

#define MAX_SLICES  32

#define CSR_BASE					0x7b000000
#define L2VIC_BASE					((CSR_BASE) + 0x10000)
#define L2VIC_INT_ENABLE(n)			((vuint32 *) ((L2VIC_BASE) + 0x100 + 4 * (n/32)))
#define L2VIC_INT_ENABLE_CLEAR(n) 	((vuint32 *) ((L2VIC_BASE) + 0x180 + 4 * (n/32)))
#define L2VIC_INT_ENABLE_SET(n)		((vuint32 *) ((L2VIC_BASE) + 0x200 + 4 * (n/32)))
#define L2VIC_INT_TYPE(n)			((vuint32 *) ((L2VIC_BASE) + 0x280 + 4 * (n/32)))
#define L2VIC_INT_POLOARITY(n)		((vuint32 *) ((L2VIC_BASE) + 0x300 + 4 * (n/32)))
#define L2VIC_INT_STATUS(n)			((vuint32 *) ((L2VIC_BASE) + 0x380 + 4 * (n/32)))
#define L2VIC_INT_CLEAR(n)			((vuint32 *) ((L2VIC_BASE) + 0x400 + 4 * (n/32)))
#define L2VIC_SOFT_INT(n)			((vuint32 *) ((L2VIC_BASE) + 0x480 + 4 * (n/32)))
#define L2VIC_INT_PENDING(n)		((vuint32 *) ((L2VIC_BASE) + 0x500 + 4 * (n/32)))

// This example code tests the following Interrupts
#define IRQ1	36
#define IRQ2	38

#endif
