#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <float.h>
#include <hexagon_sim_timer.h>

#define COMPUTE_THREADS 3
#define STACK_SIZE 16384

volatile int exit_flag, fast_irq_enable[4] = {0,0,0,0};
unsigned int FAST_L2VIC_PA;
unsigned long long start_cycles, current_cycles;
unsigned long long pcycle_count;
static char stack [COMPUTE_THREADS][STACK_SIZE] __attribute__ ((__aligned__(8)));

unsigned long long hexagon_sim_read_pcycles(void);
void register_interrupt(int intno, void (*IRQ_handler)(int intno));
void thread_create(void (*pc) (void *), void (*sp), int threadno, void (*data_struct));
void thread_join(int mask);
void add_translation(void (*va), void (*pa), int cacheability);

int add_translation_extended(int index, \
void *va, uint64_t pa, unsigned int page_size, unsigned int xwru, \
unsigned int cccc, unsigned int asid, unsigned int aa, unsigned int vg);

typedef volatile unsigned long u32;
typedef volatile unsigned long long u64;
typedef volatile unsigned long vu32;
typedef volatile unsigned long long vu64;

// L2_INT1-4 defined in makefile call to hexagon-clang
static int L2_INT[ 4 ] = { L2_INT1, L2_INT2, L2_INT3, L2_INT4 };
#define THREAD0_ENABLE				0xfe
#define THREAD1_ENABLE				0xfd
#define THREAD2_ENABLE				0xfb
#define THREAD3_ENABLE				0xf7
#define IRQ2						2
#define IRQ3						3
#define IRQ4						4
#define IRQ5						5

#define CSR_BASE1					0xfab00000
#define CSR_BASE2					0xfab40000

#define QTMR_BASE					((CSR_BASE1) + 0x20000)
#define QTMR_AC_CNTACR				((vu32 *) ((QTMR_BASE) + 0x40))
#define QTMR_CNTPCT1_LO				((vu32 *) ((QTMR_BASE) + 0x1000))
#define QTMR_CNTPCT1_HI				((vu32 *) ((QTMR_BASE) + 0x1004))
#define QTMR_CNTP1_CVAL_LO			((vu32 *) ((QTMR_BASE) + 0x1020))
#define QTMR_CNTP1_CVAL_HI			((vu32 *) ((QTMR_BASE) + 0x1024))
#define QTMR_CNTP1_TVAL				((vu32 *) ((QTMR_BASE) + 0x1028))
#define QTMR_CNTP1_CTL				((vu32 *) ((QTMR_BASE) + 0x102c))

#define QTMR_CNTPCT2_LO				((vu32 *) ((QTMR_BASE) + 0x2000))
#define QTMR_CNTPCT2_HI				((vu32 *) ((QTMR_BASE) + 0x2004))
#define QTMR_CNTP2_CVAL_LO			((vu32 *) ((QTMR_BASE) + 0x2020))
#define QTMR_CNTP2_CVAL_HI			((vu32 *) ((QTMR_BASE) + 0x2024))
#define QTMR_CNTP2_TVAL				((vu32 *) ((QTMR_BASE) + 0x2028))
#define QTMR_CNTP2_CTL				((vu32 *) ((QTMR_BASE) + 0x202c))

#define QTMR_CNTPCT3_LO				((vu32 *) ((QTMR_BASE) + 0x3000))
#define QTMR_CNTPCT3_HI				((vu32 *) ((QTMR_BASE) + 0x3004))
#define QTMR_CNTP3_CVAL_LO			((vu32 *) ((QTMR_BASE) + 0x3020))
#define QTMR_CNTP3_CVAL_HI			((vu32 *) ((QTMR_BASE) + 0x3024))
#define QTMR_CNTP3_TVAL				((vu32 *) ((QTMR_BASE) + 0x3028))
#define QTMR_CNTP3_CTL				((vu32 *) ((QTMR_BASE) + 0x302c))

#define QTMR_CNTPCT4_LO				((vu32 *) ((QTMR_BASE) + 0x4000))
#define QTMR_CNTPCT4_HI				((vu32 *) ((QTMR_BASE) + 0x4004))
#define QTMR_CNTP4_CVAL_LO			((vu32 *) ((QTMR_BASE) + 0x4020))
#define QTMR_CNTP4_CVAL_HI			((vu32 *) ((QTMR_BASE) + 0x4024))
#define QTMR_CNTP4_TVAL				((vu32 *) ((QTMR_BASE) + 0x4028))
#define QTMR_CNTP4_CTL				((vu32 *) ((QTMR_BASE) + 0x402c))

#define QTMR2_BASE					((CSR_BASE2) + 0x20000)
#define QTMR2_AC_CNTACR				((vu32 *) ((QTMR2_BASE) + 0x40))
#define QTMR2_CNTPCT1_LO			((vu32 *) ((QTMR2_BASE) + 0x1000))
#define QTMR2_CNTPCT1_HI			((vu32 *) ((QTMR2_BASE) + 0x1004))
#define QTMR2_CNTP1_CVAL_LO			((vu32 *) ((QTMR2_BASE) + 0x1020))
#define QTMR2_CNTP1_CVAL_HI			((vu32 *) ((QTMR2_BASE) + 0x1024))
#define QTMR2_CNTP1_TVAL			((vu32 *) ((QTMR2_BASE) + 0x1028))
#define QTMR2_CNTP1_CTL				((vu32 *) ((QTMR2_BASE) + 0x102c))

#define QTMR2_CNTPCT2_LO			((vu32 *) ((QTMR2_BASE) + 0x2000))
#define QTMR2_CNTPCT2_HI			((vu32 *) ((QTMR2_BASE) + 0x2004))
#define QTMR2_CNTP2_CVAL_LO			((vu32 *) ((QTMR2_BASE) + 0x2020))
#define QTMR2_CNTP2_CVAL_HI			((vu32 *) ((QTMR2_BASE) + 0x2024))
#define QTMR2_CNTP2_TVAL			((vu32 *) ((QTMR2_BASE) + 0x2028))
#define QTMR2_CNTP2_CTL				((vu32 *) ((QTMR2_BASE) + 0x202c))

#define L2VIC_BASE					((CSR_BASE1) + 0x10000)
#define L2VIC_INT_ENABLE(n)			((vu32 *) ((L2VIC_BASE) + 0x100 + 4 * (n/32)))
#define L2VIC_INT_ENABLE_CLEAR(n)	((vu32 *) ((L2VIC_BASE) + 0x180 + 4 * (n/32)))
#define L2VIC_INT_ENABLE_SET(n)		((vu32 *) ((L2VIC_BASE) + 0x200 + 4 * (n/32)))
#define L2VIC_INT_TYPE(n)			((vu32 *) ((L2VIC_BASE) + 0x280 + 4 * (n/32)))
#define L2VIC_INT_STATUS(n)			((vu32 *) ((L2VIC_BASE) + 0x380 + 4 * (n/32)))
#define L2VIC_INT_CLEAR(n)			((vu32 *) ((L2VIC_BASE) + 0x400 + 4 * (n/32)))
#define L2VIC_SOFT_INT(n)			((vu32 *) ((L2VIC_BASE) + 0x480 + 4 * (n/32)))
#define L2VIC_INT_PENDING(n)		((vu32 *) ((L2VIC_BASE) + 0x500 + 4 * (n/32)))

#define L2VIC_INT_GRP(n)			((vu32 *) ((L2VIC_BASE) + 0x600 + 4 * n ))
#define FAST_L2VIC_VA 				((vu32 *) 0xef560000)
#define EDGE_TRIGGER				1
#define LEVEL_TRIGGER				0
#define ON							1
#define OFF							0
#define FAST						1
#define NORMAL						0
#define PAGE_4KB					1
#define PAGE_64KB					4
#define XWRU						0xf
#define DEVICE_UNCACHED				4

#define ticks_per_qtimer1			((QTMR_FREQ)/3000)
#define ticks_per_qtimer2			((QTMR_FREQ)/1000)
#define ticks_per_qtimer3			((QTMR_FREQ)/6000)
#define ticks_per_qtimer4			((QTMR_FREQ)/600)

static volatile int qtimer_cnt[4]= {0,0,0,0};
static u32 tval, tval2;

// read pcyclelo and pcyclehi counters
unsigned long long my_read_pcycles(void)
{
  u32 lo, hi;
  u64 pcyc;

  __asm__ __volatile__ (
    "%0 = pcyclelo\n"
    "%1 = pcyclehi\n"
    : "=r" (lo), "=r" (hi)
  );
  pcyc = (((u64) hi)<<32) + lo;
  // printf(" pcyc 0x%x 0x%x\n", hi, lo); fflush(stdout);
  return pcyc;
}

static void thread_resume(int thread_num)
{
	asm volatile (
		".align 32\n"
		// get thread number
		"r1 = %0\n"
		// put thread in wait mode
		"    resume(r1)\n"
		" isync\n"
		:
		: "r"(thread_num)
		: "r1"
	);
}

static void asm_wait()
{
	//printf("All threads going into wait mode\n");
	asm volatile (
		"r0 = #7\n"
		"wait(r0)\n"
		"isync\n"
		: : : "r0"
	);
}

/*
	this function assigns an interrupt to a thread
*/
void iassignw(int irq_to_thread)
{
	__asm__ __volatile__ (
		"r0 = %0\n"
		"isync\n"
		"iassignw(r0)\n"
		"isync\n"
		:
		: "r" (irq_to_thread)
		: "r0"
	);

}
/*
	this function returns an interrupt assigned to a thread
*/
u32 iassignr(int thread)
{
	u32 interrupt;
	__asm__ __volatile__ (
		"r1 = %1\n"
		"isync\n"
		"r0 = iassignr(r1)\n"
		"isync\n"
		"%0 = r0\n"
		: "=r" (interrupt)
		: "r" (thread)
		: "r0"
	);
	return interrupt;
}
/*
    This function allows thread to initialize its imask reg for irqs.
*/
void imaskw(int unmask_irq)
{
	int thread=0xff;
	__asm__ __volatile__ (
		"r0 = %0\n"
		"p0 = %1\n"
		"imask = r0\n"
		"isync\n"
		:
		: "r" (unmask_irq), "r" (thread)
		: "r0", "r1"
	);
}

/*
    This function allows thread to read its imask reg.
*/
int imaskr(int thread)
{
	int imask;
	__asm__ __volatile__ (
		"isync\n"
		"r0 = getimask(%1)\n"
		"isync\n"
		"%0 = r0\n"
		: "=r" (imask)
		: "r" (thread)
		: "r0"
	);
	return imask;
}
/*
    This function is executed by thread 1.
*/
void thread1()
{
	// unmask irq3 for thread 1
	imaskw(0xf7);
	// assign irq3 for thread 1
	// and put in guest mode, ie, GM=1, UM=1
	u32 pcyclo, GM_UM = 0x90000, set = (IRQ3 << 16) | THREAD1_ENABLE;
	iassignw(set);
	set = (iassignr((IRQ3<<16)) ^ 0xf);
	printf("iassignr for IRQ%x is thread %ld\n", 3, set-1);
	// lets put this thread into guest mode for later testing the fast_irq
	__asm__ __volatile__ (
		"r0 = ssr\n"
		"isync\n"
		"r1 = %0\n"
		"r0 = or(r0,r1)\n"
		"ssr = r0\n"
		:
		: "r" (GM_UM)
		: "r0", "r1"
	);
	for (;;)
	{
		__asm__ __volatile__ (
			"%0 = gpcyclelo\n"
			"isync\n"
			: "=r" (pcyclo)
		);
	}
}
/*
    This function is executed by thread 2.
*/
void thread2()
{
	// unmask irq4 for thread 2
	imaskw(0xef);
	// assign irq4 for thread 2
	u32 pcyclo, set = (IRQ4 << 16) | THREAD2_ENABLE;
	iassignw(set);
	set = (iassignr((IRQ4 << 16)) ^ 0xf);
	printf("iassignr for IRQ%x is thread %ld\n", 4, set-2);
	for (;;)
	{
		__asm__ __volatile__ (
			"%0 = pcyclelo\n"
			"isync\n"
			: "=r" (pcyclo)
		);
	}
}
/*
    This function is executed by thread 3.
*/
void thread3()
{
	// unmask irq5 for thread 3
	imaskw(0xdf);
	// enable irq5 for thread 3
	u32 pcyclo, set = (IRQ5 << 16) | THREAD3_ENABLE;
	iassignw(set);
	set = (iassignr((IRQ5<<16)) ^ 0xf);
	printf("iassignr for IRQ%x is thread %ld\n", 5, set-5);
	for (;;)
	{
		__asm__ __volatile__ (
			"%0 = pcyclelo\n"
			"isync\n"
			: "=r" (pcyclo)
		);
	}
}

void update_qtimer1()
{
	u64 cval = (((u64) *QTMR_CNTP1_CVAL_HI) << 32) |
	((u64) *QTMR_CNTP1_CVAL_LO);
	cval += ticks_per_qtimer1;
	*QTMR_CNTP1_CVAL_LO = (u32) (cval & 0xffffffff);
	*QTMR_CNTP1_CVAL_HI = (u32) (cval >> 32);
}

void update_qtimer2()
{
	u64 cval = (((u64) *QTMR_CNTP2_CVAL_HI) << 32) |
	((u64) *QTMR_CNTP2_CVAL_LO);
	cval += ticks_per_qtimer2;
	*QTMR_CNTP2_CVAL_LO = (u32) (cval & 0xffffffff);
	*QTMR_CNTP2_CVAL_HI = (u32) (cval >> 32);
}

void update_qtimer3()
{
	u64 cval = (((u64) *QTMR2_CNTP1_CVAL_HI) << 32) |
	((u64) *QTMR2_CNTP1_CVAL_LO);
	cval += ticks_per_qtimer3;
	*QTMR2_CNTP1_CVAL_LO = (u32) (cval & 0xffffffff);
	*QTMR2_CNTP1_CVAL_HI = (u32) (cval >> 32);
}

void update_qtimer4()
{
	u64 cval = (((u64) *QTMR2_CNTP2_CVAL_HI) << 32) |
	((u64) *QTMR_CNTP2_CVAL_LO);
	cval += ticks_per_qtimer4;
	*QTMR2_CNTP2_CVAL_LO = (u32) (cval & 0xffffffff);
	*QTMR2_CNTP2_CVAL_HI = (u32) (cval >> 32);
}

// this function turns on/off the qtimers.
// ctrl = 1 is enable, ctrl = 0 is disable
void init_qtimers(int ctr, int ctrl)
{
	// enable read/write access to all timers
	*QTMR_AC_CNTACR = 0x3f;
	if (ctr & 1)
	{
		// set up timer 1
		*QTMR_CNTP1_TVAL = ticks_per_qtimer1;
		*QTMR_CNTP1_CTL = ctrl;
	}
	if (ctr & 2)
	{
		// set up timer 2
		*QTMR_CNTP2_TVAL = ticks_per_qtimer2;
		*QTMR_CNTP2_CTL = ctrl;
	}
	if (ctr & 4)
	{
		// set up timer 3
		*QTMR2_CNTP1_TVAL = ticks_per_qtimer3;
		*QTMR_CNTP1_CTL = ctrl;
	}
	if (ctr & 8)
	{
		// set up timer 4
		*QTMR2_CNTP2_TVAL = ticks_per_qtimer4;
		*QTMR2_CNTP2_CTL = ctrl;
	}
}

//
// for normal interrupts, re-enable L2VIC interrupts.
// fast_irq does not use this function.
//
void update_l2vic(u32 irq)
{
    u32 irq_bit = 1 << (irq % 32);
    *L2VIC_INT_ENABLE_SET(irq)  =  irq_bit;
}



// assign L2irq to specific L1INT
void assign_l2irq_to_l1int(int *irq_array, int how_many)
{
  int i, irq, word_id, set_id;

  for (i = 0; i < how_many; i++)
  {
    irq = irq_array[i];
    word_id = irq / 8;
    set_id = irq % 8;
    *L2VIC_INT_GRP(word_id) = (8 + i) << (set_id * 4);
	//printf("\nIRQ#%d goes to L2VIC_INT_GRP(word_id) = 0x%p, word_id = %d, set_id = %d, value = %x\n", irq_array[i], L2VIC_INT_GRP(word_id), word_id, set_id, ((8 + i) << (set_id * 4))); 
  }
}

/*
  All l2vic interrupts will be set to level-sensetive type
q
q
q
q
  INT_TYPE = 0 for level-sensitive
  INT_TYPE = 1 for edge-sensitive
*/
void init_l2vic(int type)
{

    u32 irq1_bit, irq2_bit, irq3_bit, irq4_bit, read_;

    irq1_bit = (1 << (L2_INT1 % 32));
    irq2_bit = (1 << (L2_INT2 % 32));
	irq3_bit = (1 << (L2_INT3 % 32));
	irq4_bit = (1 << (L2_INT4 % 32));

	*L2VIC_INT_ENABLE_CLEAR(L2_INT1)  = irq1_bit;
	*L2VIC_INT_TYPE(L2_INT1)        = (unsigned int) type;
	//assign_l2irq_to_l1int_v66(L2_INT1, 0);
	*L2VIC_INT_ENABLE_SET(L2_INT1)    = irq1_bit;

	*L2VIC_INT_ENABLE_CLEAR(L2_INT2) = irq2_bit;
	*L2VIC_INT_TYPE(L2_INT2)        = (unsigned int) type;
	//assign_l2irq_to_l1int_v66(L2_INT2, 1);
	*L2VIC_INT_ENABLE_SET(L2_INT2)   = irq2_bit;

	*L2VIC_INT_ENABLE_CLEAR(L2_INT3) = irq3_bit;
	*L2VIC_INT_TYPE(L2_INT3)        = (unsigned int) type;
	//assign_l2irq_to_l1int_v66(L2_INT3, 2);
	*L2VIC_INT_ENABLE_SET(L2_INT3)   = irq3_bit;

	*L2VIC_INT_ENABLE_CLEAR(L2_INT4) = irq4_bit;
	*L2VIC_INT_TYPE(L2_INT4)        = (unsigned int) type;
	//assign_l2irq_to_l1int_v66(L2_INT4, 3);
	*L2VIC_INT_ENABLE_SET(L2_INT4)   = irq4_bit;

	assign_l2irq_to_l1int(L2_INT, 4);
}
/*
    This function is executed by thread 0 which services IRQ2.
*/
void intr_handler0 (int irq)
{
    u32 vid, vid1, htid, pcycle_cnt;

    float time_in_sec = ((float) (*QTMR_CNTPCT1_LO)) / ((float) (QTMR_FREQ));

    __asm__ __volatile__ (
		"%0 = vid\n"
		"isync\n"
		"%1 = vid1\n"
		"isync\n"
		"%2 = htid\n"
		"isync\n"
		"%3 = pcyclelo\n"
		"isync\n"
		: "=r" (vid), "=r" (vid1), "=r" (htid), "=r"(pcycle_cnt)
    );

//    if ((vid & 0xffff) == L2_INT1)
    if ((vid & 0x07ff) == L2_INT1)
    {
		qtimer_cnt[0]++;
		printf ("qtimer1 interrupt #%d, irq=0x%x, VID=0x%lx, VID1=0x%lx, HTID=0x%lx, imask=0x%x, pcycle ctr = %lu  \n",qtimer_cnt[0], irq, vid, vid1, htid, imaskr(0), pcycle_cnt);
        update_qtimer1();
		if (!fast_irq_enable[0])
			update_l2vic((vid & 0x07ff));
		else *FAST_L2VIC_VA = (0x07ff & vid);
    }
    else
    {
        printf ("Intr_handler0 discovered Other IRQ interrupt #%d, irq=0x%x, VID=0x%lx, VID1=0x%lx, HTID=0x%lx, imask=0x%x, pcycle ctr = %lu  \n",qtimer_cnt[0], irq, vid, vid1, htid, imaskr(0), pcycle_cnt);
    }
}
/*
    This function is executed by thread 1 which services IRQ3.
*/
void intr_handler1 (int irq)
{
    u32 vid, vid1, htid, pcycle_cnt;

    float time_in_sec = ((float) (*QTMR_CNTPCT1_LO)) / ((float) (QTMR_FREQ));

    __asm__ __volatile__ (
		"%0 = vid\n"
		"isync\n"
		"%1 = vid1\n"
		"isync\n"
		"%2 = htid\n"
		"isync\n"
		"%3 = pcyclelo\n"
		"isync\n"
		: "=r" (vid), "=r" (vid1), "=r" (htid), "=r"(pcycle_cnt)
    );

    if ((vid >> 16) == L2_INT2)
    {
		qtimer_cnt[1]++;
		printf ("qtimer2 interrupt #%d, irq=0x%x, VID=0x%lx, VID1=0x%lx, HTID=0x%lx, imask=0x%x, pcycle ctr = %lu  \n",qtimer_cnt[1], irq, vid, vid1, htid, imaskr(1), pcycle_cnt);
        update_qtimer2();
		if (!fast_irq_enable[1])
			update_l2vic((vid &0x07ff0000)>>16);
		else *FAST_L2VIC_VA = ((vid &0x07ff0000)>>16);
    }
    else
    {
        printf ("Intr_handler1 discovered Other IRQ VID=0x%lx, VID1=0x%lx\n", vid, vid1);
    }
}
/*
    This function is executed by thread 2 which services IRQ4.
*/
void intr_handler2 (int irq)
{
    u32 vid, vid1, htid, pcycle_cnt;

    float time_in_sec = ((float) (*QTMR_CNTPCT1_LO)) / ((float) (QTMR_FREQ));

    __asm__ __volatile__ (
		"%0 = vid\n"
		"isync\n"
		"%1 = vid1\n"
		"isync\n"
		"%2 = htid\n"
		"isync\n"
		"%3 = pcyclelo\n"
		"isync\n"
		: "=r" (vid), "=r" (vid1), "=r" (htid), "=r"(pcycle_cnt)
    );

    if ((vid1 & 0xffff) == L2_INT3)
    {
		qtimer_cnt[2]++;
		printf ("qtimer3 interrupt #%d, irq=0x%x, VID=0x%lx, VID1=0x%lx, HTID=0x%lx, imask=0x%x, pcycle ctr = %lu  \n",qtimer_cnt[2], irq, vid, vid1, htid, imaskr(2), pcycle_cnt);
        update_qtimer3();
		if (!fast_irq_enable[2])
			update_l2vic((vid1 & 0x07ff));
		else *FAST_L2VIC_VA = (0x07ff |  L2_INT3);
    }
    else
    {
        printf ("Intr_handler2 discovered Other IRQ VID=0x%lx, VID1=0x%lx\n", vid, vid1);
    }
}
/*
    This function is executed by thread 3 which services IRQ5.
*/
void intr_handler3 (int irq)
{
    u32 vid, vid1, htid, pcycle_cnt;

    float time_in_sec = ((float) (*QTMR_CNTPCT1_LO)) / ((float) (QTMR_FREQ));

    __asm__ __volatile__ (
		"%0 = vid\n"
		"isync\n"
		"%1 = vid1\n"
		"isync\n"
		"%2 = htid\n"
		"isync\n"
		"%3 = pcyclelo\n"
		"isync\n"
		: "=r" (vid), "=r" (vid1), "=r" (htid), "=r"(pcycle_cnt)
    );

    if ((vid1 >> 16)== L2_INT4)
    {
		qtimer_cnt[3]++;
		printf ("qtimer4 interrupt #%d, irq=0x%x, VID=0x%lx, VID1=0x%lx, HTID=0x%lx, imask=0x%x, pcycle ctr = %lu  \n",qtimer_cnt[3], irq, vid, vid1, htid, imaskr(3), pcycle_cnt);
        update_qtimer4();
		if (!fast_irq_enable[3])
			update_l2vic((vid1 & 0x07ff0000)>>16);
		else *FAST_L2VIC_VA = (0x07ff |  L2_INT4);
    }
    else
    {
        printf ("Intr_handler3 discovered Other IRQ VID=0x%lx, VID1=0x%lx\n", vid, vid1);
    }
}
/*
    This function will create a swi interrupt.
*/
void swi_handler(int irq)
{


}
/*
    This function is executed by thread 0.
*/
void enable_core_interrupt()
{
	u32 set = (IRQ2 <<16) | THREAD0_ENABLE;  // assign irq2 only for thread 0

	if(Q6VERSION <65)
	{
		int irq = 2;
		__asm__ __volatile__ (
		   "     r0 = #0\n"
		   "     r0 = setbit(r0,%0)\n"
		   "     r1 = iahl\n"
		   "isync\n"
		   "     r1 = or(r1,r0)\n"
		   "     iahl = r1\n"
		   "isync\n"
		   "     r1 = iel\n"
		   "isync\n"
		   "     r1 = or(r1,r0)\n"
		   "     iel = r1\n"
		   "isync\n"
		   :
		   : "r" (irq)
		   : "r0", "r1"
		   );
		register_interrupt (irq, intr_handler0);
	}
	else
	{
		// unmask irq2 only for thread0
		imaskw(0xfb);
		// irq2 only for thread0
		iassignw(set);
		register_interrupt (IRQ2, intr_handler0);
		register_interrupt (IRQ3, intr_handler1);
		register_interrupt (IRQ4, intr_handler2);
		register_interrupt (IRQ5, intr_handler3);
	}
}
/*
    This function is executed by thread 0 to enable
	fast_irq re-enable for all threads.
*/
int enable_fast_irq(int irq_num)
{
	unsigned int cfgbase, pa, j;

	// turn off qtimers 1-4
	init_qtimers(0xf, OFF);


	// get cfgbase to set up fastl2vic
	__asm__ __volatile__ (
		"%0 = cfgbase\n"
		"isync\n"
		: "=r"(cfgbase)
	);
	// tlb entry 3, asid=0, aa=0, valid bit and global bit = 1 (ignore asid)
	if (add_translation_extended(3, (void *) (cfgbase<<16), cfgbase<<16, PAGE_4KB, XWRU, DEVICE_UNCACHED, 0, 0, 3))
	{
		printf("FAIL: could not create mapping 4KB for CFGtable\n");
		fflush(stdout);
		return -1;
	}
 
	// get fast_irq physical address from cfgbase
	__asm__ __volatile__ (
		"r0 = cfgbase\n"
		"isync\n"
		"r0 = asl(r0, #5)\n"
		"r1 = #0x28\n"
		// get physical address of fast_irq
		"r2 = memw_phys(r1, r0)\n"
		"isync\n"
		"%0 = r2\n"
		: "=r"(pa)
		:
		: "r0", "r1", "r2", "r3"
	);

	// get physical address
	FAST_L2VIC_PA = ((unsigned int) pa << 16);

	printf("fastl2vic is at PA=0x%x\n", (unsigned int) FAST_L2VIC_PA);
	// tlb entry 4, asid=0, aa=0, valid bit and global bit = 1 (ignore asid)
	if (add_translation_extended(4, (void *)FAST_L2VIC_VA, FAST_L2VIC_PA, PAGE_64KB, XWRU, DEVICE_UNCACHED, 0, 0, 3))
	{
		printf("FAIL: could not create mapping 64KB for FastL2vic\n");
		fflush(stdout);
		exit(-1);
	}

	init_l2vic(EDGE_TRIGGER);

	// fast_irq re-enable command type = 0
	for (j =0; j < 4; j++)
		*FAST_L2VIC_VA = 0xffff |  IRQ2+j;

	// turn on qtimers 1-4
	init_qtimers(0xf, ON);

	return 0;
}

/*
	this function tests normal and fast irqs.
	On = test fast, Off = test normal
*/
int test_irqs(int type)
{

	enable_core_interrupt();
    init_l2vic(EDGE_TRIGGER);
	// turn on  qtimers 1-4
	if(type == FAST) {
		qtimer_cnt[1]=0;
		if(enable_fast_irq(1))
		{
			printf("failed to enable fast irq\n");
			return -1;
		}
	}
    init_qtimers(0xf, ON);
	// Wait till qtimer 2 has caused 2 interrupts
    while (qtimer_cnt[1] < 2)
    {
		// Thread 0 waits for interrupts
		//asm_wait();
    }
	return 0 ;
}

/*
    need to finish working on this
*/
int test_SWI_fast_irq()
{
	int i,j;

	// Lets test the fast_irq using IRQ3
	if(enable_fast_irq(1))
	{
		printf("failed to enable fast irq\n");
		return -1;
	}

	for (i = 0; i < 6; i++) // disable(1)-enable(4)-disable(1)
	{
	   // clear off all pending interrupts
		for (j =0; j < 4; j++)
		{
		  unsigned int bit = 1 << (IRQ2+j % 32);
		  *L2VIC_INT_CLEAR(IRQ2+j) = bit;
		}

		// set up
		printf(" iteration S1 %d pcycle ctr = 0x%llx\n", i, my_read_pcycles()); //fflush(stdout);
		if ((i == 0) || (i == 5))
		{
			for (j =0; j < 4; j++)
				*FAST_L2VIC_VA = (1 << 16) | IRQ2+j;
		}
		if (i == 1)
		{
			//for (j =0; j < 4; j++)
			//{
				__asm__ __volatile__ (
					"r0 = cfgbase\n"
					"isync\n"
					"r2 = memw_phys(r1, r0)\n"
					"isync\n"
					:
					:
					: "r0", "r1", "r2", "r3"
				);
				*FAST_L2VIC_VA = (0 << 16) | 3; //IRQ[j]; // enable them
			//}
		}

		// action
		printf(" iteration S2 %d pcycle ctr = 0x%llx\n", i, my_read_pcycles()); //fflush(stdout);
#if 0
		if (i & 0x1)
		{
		  *L2VIC_SOFT_INT(L2_INT[0]) = 0xf0; // kick off 36-39
		}
		else
#endif
		{
		  //for (j =0; j < 4; j++)
		  //{
			  //*L2VIC_INT_TYPE(L2_INT[j]) = 1;
			*FAST_L2VIC_VA = (unsigned int)(0 << 16) | 3; //L2_INT[j];
			//printf("l2vic_int_type(%x) = %lx\n", (unsigned int) L2VIC_INT_TYPE(L2_INT[j]), *L2VIC_INT_TYPE(L2_INT[j]) );
		  //}
		}
	}
	return 0;
}
