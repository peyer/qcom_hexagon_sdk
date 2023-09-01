//
// This example will program the
// Performance Monitor Unit and collect the
// number of read and write accesses done
// on the ahb and axi buses.
//
#include "pmu_ctrs_test.h"
#include <hexagon_standalone.h>

#define AXI_BUS		        0x0
#define AHB_BUS		        0x1
#define BUS_READ	        0x0
#define BUS_WRITE	        0x1

#define AXI_READ	        0x40
#define AXI_WRITE	        0x42
#define AHB_READ	        0x44
#define AHB_WRITE	        0x45

#define TLB_INDEX1	        1
#define TLB_INDEX2	        2
#define TLB_INDEX3	        3
#define TLB_INDEX4	        4
#define TLB_INDEX5	        5
#define TLB_INDEX6	        6

#define WRITE			1
#define READ			2
#define READ_WRITE		3
#define EXECUTE			4
#define READ_WRITE_EXECUTE	7

#define NON_CACHED_MEMORY	6


#define NUMTHREADS          1
#define THREADMASK          ((1<<NUMTHREADS)-1)
#define ITERATIONS          100

char *bus[2] = {"axi","ahb"};
char *access[2] = {"read","write"};

// The following locations are arbitrary and are used
// as an example.  Note that ahb_mem will become the
// value used to call the simulator with --ahb:lowaddr 0x28000000 --ahb:highaddr 0x29000000
// ahb:highaddr in this example is greater than the range we are accessing.
void *ahb_mem  = (void*) 0x28000000;
void *axi_mem  = (void*) 0x46000000;
void *bus_addr;
//
// This function does the bus read word accesses
// and increments the memory pointer by 4 bytes.
//
void do_bus_reads(int iterations, void *start_addr)
{
	asm volatile (
	".align 32\n"
        // get syscfg
        "r1 = syscfg\n"
	"r1 = setbit(r1, #9)\n"
	// enable pmu
        "syscfg = r1\n"
	"r1 = %0\n"
	// do reads
	"loop0(bus_reads, %1)\n"
	"bus_reads: { r5 = memw(r1)\n"
	"r1 = add(r1, #4) } : endloop0\n"
        // disable pmu
        "r1 = syscfg\n"
	"r1 = clrbit( r1, #9 )\n"
	"syscfg = r1\n"
        :
	: "r"(start_addr), "r"(iterations)
	: "r1", "r5" );
}
//
// This function does the bus write word accesses
// by storing the value of the address pointed at by
// the memory pointer and incrementing the pointer by 64 which correspondes to
// a v60 cache line size.
//
void do_bus_writes(int iterations, void *start_addr)
{
	asm volatile (
	".align 32\n"
        // get syscfg
        "r1 = syscfg\n"
	"r1 = setbit(r1, #9)\n"
	// enable pmu
        "syscfg = r1\n"
	"r1 = %0\n"
        // synchronization
	"isync\n"
        "syncht\n"
	"r5 = r1\n"
	// do writes
	"loop0(bus_writes, %1)\n"
	"bus_writes: {  memw(r1) = r5\n"
	"r1 = add(r1, #64)\n"
	"r5 = add(r5, #64) } : endloop0\n"
	// synchronization
	"isync\n"
	"syncht\n"
	// get syscfg
        "r1 = syscfg\n"
	"r1 = clrbit( r1, #9 )\n"
	// disable pmu
	"syscfg = r1\n"
        :
	: "r"(start_addr), "r"(iterations)
	: "r1", "r5" );
}
//
// This function sets up the pmu_config parameters,
// initializes the bus_addr, calls the do_bus_reads/writes
// and prints out the counter values.
//
void run_test(int bus_type,int access_type)
{
    unsigned int value;
    int i;
    if (bus_type == AXI_BUS)
    {
        // void *bus_addr = (void*)axi_mem;
        bus_addr = axi_mem;
	if (access_type == BUS_READ)
	{
	    pmu_config(THREADMASK, AXI_READ, AXI_READ, AXI_READ, AXI_READ);
	    pmu_config1(THREADMASK, AXI_READ, AXI_READ, AXI_READ, AXI_READ);
	}
	else
	{
	    pmu_config(THREADMASK, AXI_WRITE, AXI_WRITE, AXI_WRITE, AXI_WRITE);
            pmu_config1(THREADMASK, AXI_WRITE, AXI_WRITE, AXI_WRITE, AXI_WRITE);
	}

    }
    else
    {
        //void *bus_addr = (void*)ahb_mem;
        bus_addr = ahb_mem;
	if (access_type == BUS_READ)
	{
	    pmu_config(THREADMASK, AHB_READ, AHB_READ, AHB_READ, AHB_READ);
            pmu_config1(THREADMASK, AHB_READ, AHB_READ, AHB_READ, AHB_READ);
	}
	else
	{
	    pmu_config(THREADMASK, AHB_WRITE, AHB_WRITE, AHB_WRITE, AHB_WRITE);
            pmu_config1(THREADMASK, AHB_WRITE, AHB_WRITE, AHB_WRITE, AHB_WRITE);
	}
    }
    // Perform Bus Accesses
    if(access_type == BUS_READ)
    {
        (void) perf_run( do_bus_reads, (void*) bus_addr, (int) ITERATIONS);
    }
    else
    {
        (void) perf_run( do_bus_writes, (void*) bus_addr, (int) ITERATIONS);
    }
    // Print PMU counters
    for (i=0; i<8; i++)
    {
        value = get_pmu_counter(i);
        printf("%s %s test - pmu_counter%d = %d\n",bus[bus_type],access[access_type],i,value);
    }
    printf("\n");
}
//
// This function will dump the contents of the memory block you have written to.
//
void dump_mem(int *ptr, int iterations, int bus_type)
{
    int i;
    int count = iterations;
    int *myptr = ptr;
    char *mybus = bus[bus_type];
    printf("\nReading %s bus\n", mybus);
    for (i = 0; i < count; i++,myptr++)
    {
        printf("#%d %p = %x\n",i, myptr, *myptr);
    }
    }
//
// This is where it all begins
//
int main()
{
    // disable hw prefetching
    ihwprefetch_disable();
    dhwprefetch_disable();

    // axi address TLB entry
    add_translation_fixed(TLB_INDEX3, axi_mem, axi_mem, NON_CACHED_MEMORY, READ_WRITE);
    // ahb address TLB entry
    add_translation_fixed(TLB_INDEX4, ahb_mem, ahb_mem, NON_CACHED_MEMORY, READ_WRITE);
    // test for 100 ahb reads
    run_test(AHB_BUS, BUS_READ);
    // test for 100 axi reads
    run_test(AXI_BUS, BUS_READ);
    // test for 100 ahb writes
    run_test(AHB_BUS, BUS_WRITE);
    // test for 100 axi writes
    run_test(AXI_BUS, BUS_WRITE);
    // read memory
    //dump_mem(axi_mem, 100, AXI_BUS);
    //dump_mem(ahb_mem, 100, AHB_BUS);
}
