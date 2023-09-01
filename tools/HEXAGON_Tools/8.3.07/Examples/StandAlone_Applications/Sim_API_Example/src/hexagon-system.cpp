#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <assert.h>
#include <cstring>

//#define BUS_COSIM 1

using namespace std;

#include <HexagonTypes.h>
#include <HexagonWrapper.h>

#ifdef WIN32
  #define HEXFUNC							\
      << (strstr(__FUNCTION__,"::") ? strstr(__FUNCTION__,"::") : __FUNCTION__)\
      << ": "
#else
  #define HEXFUNC	__FUNCTION__ << ": "
#endif


#define print_msg(a)	cout << HEXFUNC << a << endl
#define print_err(a)	cerr << HEXFUNC << a << endl

#define CHECK_RC(func)							    \
{									    \
    HEXAPI_Status rc = func;						    \
									    \
    if (rc != HEX_STAT_SUCCESS)						    \
    {									    \
	print_err ("Simulator API Error: " << #func << " returned " << rc); \
    }									    \
}

#define CHECK_FATAL_RC(func, error_code)				\
{									\
    HEXAPI_Status rc = func;						\
									\
    if (rc != HEX_STAT_SUCCESS)						\
    {									\
	print_err ("Simulator API Fatal Error: "			\
				    << #func << " returned " << rc); 	\
	exit (error_code);						\
    }									\
}

#define ISS(x)                         CHECK_RC(iss->x)
#define IF_ISS(x)       { if (iss) { CHECK_RC(iss->x) }}



// Delay unit is core clock cycles.
#define DELAY 100
#define MAX_QUEUE_DEPTH 10

// Bool values for use with mem_access() helper function.
#define WRITE 1
#define READ  0

// Memory is partitioned as an index of 64k pages.
typedef unsigned char memblock_t[64*1024];
memblock_t *mem_entries[64*1024];

// Storage container type for bus requests.
typedef struct BusRequest_t {
    HexagonWrapper *     iss;
    HEX_PA_t             addr;
    HEX_4u_t             nBytes;
    HEX_1u_t *           dataptr;
    HEX_4u_t             reqID;
    HEXAPI_BusAccessType type;
} BusRequest;

// Counter used to keep track of queue depth.
HEX_4u_t queue_depth = 0;

// List of pending transactions. This list
// is only used by the destructor to free
// memory of unhandled transactions
// upon simulation termination.
BusRequest * queue_list[MAX_QUEUE_DEPTH];

void
decrement_queue (BusRequest *pBusRequest)
{
    int i;
    queue_depth--;
    assert (queue_depth >= 0);

    for (i = 0; i < MAX_QUEUE_DEPTH; i++)
    {
	if (queue_list[i] == pBusRequest) queue_list[i] = NULL;
    }
}

void
increment_queue (BusRequest *pBusRequest)
{
    int i;
    queue_depth++;
    assert(queue_depth < MAX_QUEUE_DEPTH);

    for (i = 0; i < MAX_QUEUE_DEPTH; i++)
    {
	if (queue_list[i] == NULL) queue_list[i] = pBusRequest;
    }
}

bool
check_queue()
{
    return ((MAX_QUEUE_DEPTH-1) - queue_depth);
}


void
mem_access (HEX_4u_t addr,
	    HEX_4u_t nBytes,
	    unsigned char *data,
	    bool write)
{
    /*
     * Memory is partitioned as an index of 64k pages.
     * The upper 16 bits of the address are used as an
     * index into a page array. The lower bits are used
     * as offset into the page
     */
    HEX_2u_t tindex = addr >> 16;
    HEX_2u_t offset = addr & 0xFFFF;
    HEX_4u_t size;
    memblock_t *pMemBlock;
    unsigned char *memptr;

    // The 'do' loop handles accesses across a 64k boundary.
    do
    {
	// Calculate how much space is left in the current page.
	size = (~offset & 0xffff) + 1;
	if (nBytes < size) size = nBytes;

	// Set the pointer to the top of the requested page.
	pMemBlock = mem_entries[tindex];

	// Check if the requested page already exists.
	if (!pMemBlock)
	{
	    // If not, allocate it from the heap.
	    pMemBlock = (memblock_t *)calloc (1, sizeof(memblock_t));
	    mem_entries[tindex] = pMemBlock;
	}

	memptr = ((unsigned char *) pMemBlock) + offset;

	if (write)
	{
	    // Write the data to local storage.
	    memcpy (memptr, data, size);
	}
	else
	{
	    // Read the data from local storage.
	    memcpy (data, memptr, size);
	}

	// Set loop variables for access to the next 64k page if needed.
	data += size;
	tindex++;
	offset = 0;
	nBytes -= size;
    }
    while (nBytes > 0);
}

void
TimedCallBack (void *pBusRequest)
{
    BusRequest *pReq = (BusRequest *) pBusRequest;
    HexagonWrapper *iss = pReq->iss;

    if (pReq->dataptr)
    {
	bool accessType = ((pReq->type == HEX_DATA_WRITE)   ||
			   (pReq->type == HEX_DATA_CASTOUT) ||
			   (pReq->type == HEX_DATA_WRITE_LOCKED)) ? WRITE : READ;

	mem_access (pReq->addr, pReq->nBytes, pReq->dataptr, accessType);
    }

    iss->BusTransactionFinished (pReq->dataptr, pReq->nBytes, pReq->reqID);
    ISS (RemoveTimedCallback(pReq));

    decrement_queue(pReq);

    free (pReq);
}


HEXAPI_TransactionStatus
BusAccessCallback (void *handle,
		   HEX_PA_t addr,
		   HEX_4u_t nBytes,
		   HEX_1u_t* dataptr,
		   HEX_4u_t reqID,
		   HEXAPI_BusAccessType bat,
		   HEX_4u_t tnum,
		   HEXAPI_BusBurstType bt)
{
    HexagonWrapper *iss = (HexagonWrapper *) handle;

    if ((bat == HEX_SYNCH) || (bat == HEX_BARRIER))
    {
	iss->BusTransactionFinished (dataptr, nBytes, reqID);
	return TRANSACTION_SUCCESS;
    }

    if ((bat == HEX_DEBUG_READ) || (bat == HEX_DEBUG_WRITE))
    {
	bool accessType = (bat == HEX_DEBUG_WRITE) ? WRITE : READ;

	mem_access(addr, nBytes, dataptr, accessType);
	iss->BusTransactionFinished(dataptr, nBytes, reqID);
	return TRANSACTION_SUCCESS;
    }
    else
    {
	if (!check_queue())
	{
	    return TRANSACTION_REPLAY;
	}
	else
	{
	    HEX_8u_t time;
	    HEXAPI_Interval units;
	    BusRequest *pReq;

	    pReq = (BusRequest *) calloc(1, sizeof(BusRequest));

	    pReq->iss      = iss;
	    pReq->addr     = addr;
	    pReq->nBytes   = nBytes;
	    pReq->dataptr  = dataptr;
	    pReq->reqID	   = reqID;
	    pReq->type     = bat;

	    increment_queue(pReq);

	    iss->CycleToTime(DELAY, &time, &units);

	    ISS (AddTimedCallback((void *) pReq, time, units, TimedCallBack));
	    return TRANSACTION_SUCCESS;
	}
    }
}


int
main(int argc, char **argv)
{
    HexagonWrapper *iss;
    HEXAPI_CoreState state;
    HEXAPI_Status rc;
    HEX_4u_t runResult;
    HEX_4u_t hw_threads = 4;
    HEX_4u_t dbgPort;
    #define BUFSIZ_STATS 10240
    char buf[BUFSIZ_STATS];
    time_t starttime;
    time_t endtime;
    char *elf;
    char *cpuStr;

	// expand argc test to 6 if you add a bus.cfg_path
    if (argc < 5)
    {
        print_msg ("Syntax:\n" << argv[0] << " -mv<version> -G <port> [<elf>] [<bus.cfg_path>]");
		return -1;
    }

    cpuStr = argv[1];
    dbgPort = atoi (argv[3]);
    elf = argv[4];

    iss = new HexagonWrapper(cpuStr);

    print_msg ("New Hexagon instance: " << cpuStr << " iss=" << hex << iss << dec);

    ISS (ConfigureExecutableBinary(elf));
    ISS (ConfigureCoreFrequency(600000000));
    ISS (ConfigureTimingMode(HEX_TIMING_NODBC));
    ISS (ConfigureExecutableBinary(elf));

	// This demonstrates that you can either provide a bus cosim
	// or make one available in this system code via AddBusAccessCallback.
	// You will need to add the cfg_file_path to your makefile if you
	// choose to use external cosim.
#ifdef BUS_COSIM
	ISS (ConfigureCosim(argv[5]));
#else
    ISS (AddBusAccessCallback((void *) iss, 0, 0xffffffff, BusAccessCallback));
    print_msg ("Registered 0x0 to 0xffffffff for bus access callback");
#endif

    if (dbgPort)
    {
        print_msg ("dbgPort=" << dbgPort);
        ISS (ConfigureRemoteDebug(dbgPort));
    }

    CHECK_FATAL_RC (iss->EndOfConfiguration(), -1);

    ISS (LoadExecutableBinary());

    starttime = time(NULL);

    /*
     *  As long as the simulator is active and at least one thread
     *  is running or in wait state, we will not return from the
     *  'iss->Run' function.
     */

    print_msg ("Run");

    state = iss->Run(&runResult);

    switch (state)
    {
	case HEX_CORE_FINISHED:
	{
		HEX_4u_t modectl;
		HEX_4u_t mask = (1 << hw_threads) - 1;

		print_msg ("core state = HEX_CORE_FINISHED");

		if (dbgPort)
		{
		    /*  A debugger was attached, and now the debugger has
		     *  exited.
		     */
		    print_msg ("Debugger has exited\n");
		}

		ISS (ReadGlobalRegister(G_REG_MODECTL, &modectl));

		if ((mask & (modectl | (modectl >> 16))) == 0)
		{
		    /*
		     *  The Hexagon program has finished.  All hardware threads
		     *  are now in 'off' mode, but the AXI slave port is still
		     *  enabled.
		     *
		     *  According to the Hexagon Core HDD, the only way that
		     *  the core itself can be woken up from this state is by
		     *  toggling the reset pin.  However, the AXI slave port
		     *  will still wake up the simulator to service TCM
		     *  requests.
		     *
		     *  IMPORTANT NOTE: An incoming IRQ will NOT wake up the
		     *                  core from this state!
		     *
		     *  Set the pins to idle, but don't assert the actual core
		     *  because it's already asserted
		     */

		    print_msg ("The Hexagon program has exited -- exit code = "
								<< runResult);
		}
		}
		break;

	    case HEX_CORE_RESET:
		/*
		 *  The Hexagon core is in RESET (i.e. HALTED).
		 *
		 *  All Hexagon threads are off and the AXI slave port
		 *  is disabled.
		 *
		 *  The only way that the core can be woken up from this
		 *  state is by toggling the reset pin.  TCM cannot be accessed
		 *  when the core is in this state.
		 *
		 *  IMPORTANT NOTE: An incoming IRQ will NOT wake up the core
		 *                  from this state!
		 */
		print_msg ("core state = HEX_CORE_RESET");
		break;

	    case HEX_CORE_SUCCESS:
	    case HEX_CORE_BREAKPOINT:
	    case HEX_CORE_ASYNCHRONOUS_BREAK:
	    case HEX_CORE_ERROR:
	        print_msg ("Hexagon simulation completed; state=" << state);
	    	break;
	}

    endtime = time(NULL);

    /* Summarize stats */
    ISS (EmitStatistics());

    rc = iss->EmitPerfStatistics (starttime, 0, endtime, 0, buf, sizeof(buf));

    if (rc == HEX_STAT_SUCCESS)
    {
        printf("%s\n", buf);
    }
	return 0;
}
