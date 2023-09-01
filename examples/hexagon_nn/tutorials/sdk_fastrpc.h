

#ifdef NO_SDK

// This code is disabled by default.
//   It allows compiling monolithically outside SDK
//   e.g. for archsim/H2/linux, and is only used for internal testing.
#define fastrpc_setup()
#define fastrpc_teardown()
#define rpcmem_alloc(_a, _b, _c) malloc(_c)

#else

// These are the includes and definitions needed to make fastRPC work.
// FastRPC is the communication channel that allows the ARM core
//   (running the main application) to send requests to the DSP core
//   (which does all the heavy math).
#define adspmsgd_start(_a, _b, _c)
#define adspmsgd_stop()
#include "rpcmem.h"
#include "AEEStdErr.h"
#include <sys/types.h>
#ifndef __hexagon__
#include <sys/time.h>
#endif
#define ION_HEAP_ID_SYSTEM 25



int fastrpc_setup()
{
    int retVal=0;

    adspmsgd_start(0,RPCMEM_HEAP_DEFAULT,4096);
    rpcmem_init();
	return retVal;
}

void fastrpc_teardown()
{
        rpcmem_deinit();
        adspmsgd_stop();
}

#endif
