###############################################################################
##
## Performance Monitor Unit (PMU) Counters Example
##
###############################################################################

The pmu_ctrs_test example demonstrates how the Hexagon pmu counters can be used
to monitor specific events in the hexagon architecture to provide on-target
performance tracking.

The V55 and V60 pmu contain 8 32-bit counters which when enabled count events
defined in the pmu_cfg register.  This example will monitor and count axi
and ahb bus activity.  They are designated as NON_CACHED_MEMORY in the mmu and
the simulator is invoked with a defined address space for axi and ahb memory.

The pmu registers are set to zero, enabled and disabled before and after doing
the bus accesses to insure correct count in this example.

It should be noted that in this example the axi and ahb writes are to addresses
that are 64 bytes apart whereas the axi and ahb reads are to addresses that are
4 bytes apart.  This demonstrates that even though the NON_CACHED_MEMORY attribute
is applied for both buses, the v60 architecture L2 cache reserves the right to
coalesce stores and send them to the bus in a single transaction.  When a value
smaller than 64 is applied for writes the pmu counters will have count less than
100.

To run the example type "make" to clean, build and simulate the elf file. You
can also view the contents written to the axi/ahb memory by uncommenting the
last two lines in main().
