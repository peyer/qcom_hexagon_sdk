//
// This header file supports global_reg_access.c
// which reads/writes registers.
//
// Revision 0.1  Date 5/2/2013
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>

// by default we have no-inline as a gcc switch to these functions.
// this is because gcc might inline functions twice which causes assembly label conflicts.
// it's also nice to see when we are in a different function for debug purposes.
// however sometimes we really want inlining, so we can force that with forceinline.
#define noinline __attribute__((noinline))
#define forceinline __attribute__((always_inline))

static int ihwprefetch_global = -1;
static int dhwprefetch_global = -1;

/* reset pmu counters to 0 */
static forceinline void pmu_reset()
{
	asm volatile(
		"r0 = #0\n"
		"pmucnt0 = r0\n"
		"pmucnt1 = r0\n"
		"pmucnt2 = r0\n"
		"pmucnt3 = r0\n"
		"s44 = r0\n"
		"s45 = r0\n"
		"s46 = r0\n"
		"s47 = r0\n"
		"isync\n"
		: : : "r0" );
}

// turn off dc hwprefetch
static void dhwprefetch_disable()
{
    dhwprefetch_global = 0;
	asm volatile (
		"r1 = usr\n"
		"r1 = clrbit(r1, #13)\n"
		"r1 = clrbit(r1, #14)\n"
		"usr = r1\n"
		"r1 = ccr\n"
		"r1 = clrbit(r1, #17)\n"
		"ccr = r1\n"
		"isync\n"
		: : : "r1" );
}

// turn off ic hwprefetch
static void ihwprefetch_disable()
{
    ihwprefetch_global = 0;
	asm volatile (
		"r1 = usr\n"
		"r1 = clrbit(r1, #15)\n"
		"r1 = clrbit(r1, #16)\n"
		"usr = r1\n"
		"r1 = ccr\n"
		"r1 = clrbit(r1, #16)\n"
		"ccr = r1\n"
		"isync\n"
		:
		:
		: "r1" );
}

static void l2wa_disable()
{
	asm volatile (
		"k0lock\n"
		"r1 = syscfg\n"
		"r1 = setbit( r1, #21 )\n"
		"syscfg = r1\n"
		"k0unlock\n"
		"isync\n"
		: : : "r1" );
}

/* enable credit QoS mode */
static void qos_enable()
{
	asm volatile (
		"k0lock\n"
		"r0 = syscfg\n"
		"r0 = clrbit(r0, #13)\n"
		"syscfg = r0\n"
		"k0unlock\n"
		"isync\n"
		: : : "r0" );
}

/* configure the PMU with a threadmask and 4 events to observe */
static forceinline void pmu_config( int tmask, char event0, char event1,
		char event2, char event3 )
{
	unsigned int pmuevtcfg = (event3<<24) + (event2<<16) + (event1<<8) + event0;
	asm volatile(
		"pmucfg = %0\n"
		"pmuevtcfg = %1\n"
		:
		: "r"(tmask), "r"(pmuevtcfg)
		: "r0" );
}

/* configure the PMU with a threadmask and 4 events to observe */
static forceinline void pmu_config1( int tmask, char event0, char event1,
		char event2, char event3 )
{
	unsigned int pmuevtcfg1 = (event3<<24) + (event2<<16) + (event1<<8) + event0;
	asm volatile(
		"pmucfg = %0\n"
		"s54 = %1\n"
		:
		: "r"(tmask), "r"(pmuevtcfg1)
		: "r0" );
}

/* get PMU counter 0, 1, 2, 3 */
static forceinline unsigned int get_pmu_counter( int event )
{
	unsigned int counter;
	switch (event) {
	case 0:
		asm volatile ("%0 = pmucnt0\n" : "=r"(counter));
		break;
	case 1:
		asm volatile ("%0 = pmucnt1\n" : "=r"(counter));
		break;
	case 2:
		asm volatile ("%0 = pmucnt2\n" : "=r"(counter));
		break;
	case 3:
		asm volatile ("%0 = pmucnt3\n" : "=r"(counter));
		break;
	case 4:
		asm volatile ("%0 = s44\n" : "=r"(counter));
		break;
	case 5:
		asm volatile ("%0 = s45\n" : "=r"(counter));
		break;
	case 6:
		asm volatile ("%0 = s46\n" : "=r"(counter));
		break;
	case 7:
		asm volatile ("%0 = s47\n" : "=r"(counter));
		break;
	}
	return counter;
}

static void perf_run( void (*function)(int, void*), void *start_addr, int loop_count )
{
	unsigned int syscfg;

	pmu_reset();

	asm volatile(
		// get syscfg value
		"r1 = syscfg\n"
		"r1 = setbit(r1, #9)\n"
		"hintjr (%1)\n"
		// start pmu
		"syscfg = r1\n"
		"isync\n"
		// call function with (r1=start_addr),(r0=iterations)
		"r1 = %1\n"
		"r0 = %2\n"
		"callr %0\n"
		// stop pmu
		"r1 = syscfg\n"
		"isync\n"
		"r1 = clrbit( r1, #9 )\n"
		"syscfg = r1\n"
		:
		: "r"(function), "r"(start_addr), "r"(loop_count)
		: "r0", "r1", "r2", "sp", "lr", "fp", "lc0" );
}
