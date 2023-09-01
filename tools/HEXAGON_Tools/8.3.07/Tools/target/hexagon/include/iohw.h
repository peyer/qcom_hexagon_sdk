/* iohw.h header for TR18015/TR18037 */
#ifndef _IOHW
#define _IOHW

		/* TYPES */
typedef unsigned int ioindex_t;
typedef unsigned int ioreg;	/* HARDWARE DEPENDENT */

		/* FUNCTIONS (all are masked by macros) */
unsigned int iord(ioreg);
unsigned long iordl(ioreg);

void iowr(ioreg, unsigned int);
void iowrl(ioreg, unsigned long);

void ioand(ioreg, unsigned int);
void ioandl(ioreg, unsigned long);

void ioor(ioreg, unsigned int);
void ioorl(ioreg, unsigned long);

void ioxor(ioreg, unsigned int);
void ioxorl(ioreg, unsigned long);

unsigned int iordbuf(ioreg, ioindex_t);
unsigned long iordbufl(ioreg, ioindex_t);

void iowrbuf(ioreg, ioindex_t, unsigned int);
void iowrbufl(ioreg, ioindex_t, unsigned long);

void ioandbuf(ioreg, ioindex_t, unsigned int);
void ioandbufl(ioreg, ioindex_t, unsigned long);

void ioorbuf(ioreg, ioindex_t, unsigned int);
void ioorbufl(ioreg, ioindex_t, unsigned long);

void ioxorbuf(ioreg, ioindex_t, unsigned int);
void ioxorbufl(ioreg, ioindex_t, unsigned long);

void iogroup_acquire(int group);
void iogroup_release(int group);
void iogroup_map(int group, int direct);

		/* MACROS */
#define _IOHW_CAT(x, y)	x##_##y	/* expand arguments and paste */

#define iord(dev)		_IOHW_CAT(dev, rd)()
#define iordl(dev)		_IOHW_CAT(dev, rdl)()

#define iowr(dev, val)	_IOHW_CAT(dev, wr)(val)
#define iowrl(dev, val)	_IOHW_CAT(dev, wrl)(val)

#define ioor(dev, val)	_IOHW_CAT(dev, or)(val)
#define ioorl(dev, val)	_IOHW_CAT(dev, orl)(val)

#define ioand(dev, val)		_IOHW_CAT(dev, and)(val)
#define ioandl(dev, val)	_IOHW_CAT(dev, andl)(val)

#define ioxor(dev, val)		_IOHW_CAT(dev, xor)(val)
#define ioxorl(dev, val)	_IOHW_CAT(dev, xorl)(val)

#define iordbuf(dev, idx)		_IOHW_CAT(dev, brd)(idx)
#define iordbufl(dev, idx)		_IOHW_CAT(dev, brdl)(idx)

#define iowrbuf(dev, idx, val)	_IOHW_CAT(dev, bwr)(idx, val)
#define iowrbufl(dev, idx, val)	_IOHW_CAT(dev, bwrl)(idx, val)

#define ioorbuf(dev, idx, val)	_IOHW_CAT(dev, bor)(idx, val)
#define ioorbufl(dev, idx, val)	_IOHW_CAT(dev, borl)(idx, val)

#define ioandbuf(dev, idx, val)		_IOHW_CAT(dev, band)(idx, val)
#define ioandbufl(dev, idx, val)	_IOHW_CAT(dev, bandl)(idx, val)

#define ioxorbuf(dev, idx, val)		_IOHW_CAT(dev, bxor)(idx, val)
#define ioxorbufl(dev, idx, val)	_IOHW_CAT(dev, bxorl)(idx, val)

#define iogroup_acquire(grp)	_IOHW_CAT(grp, acq)()
#define iogroup_release(grp)	_IOHW_CAT(grp, rel)()
#define iogroup_map(grp, dir)	_IOHW_CAT(grp, map)(dir)
#endif /* _IOHW */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.01:1566 */
