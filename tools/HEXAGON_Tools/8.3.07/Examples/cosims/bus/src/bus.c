/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
*****************************************************************/
/*

    Bus interface cosim module 

    This cosim module models latency incurred by a bus access. 
    The access latency can be configured via the DELAY flag
	in bus.h, which indicates the number of cycles the bus transaction
	will be delayed. The depth of the bus queue can be changed via the
	MAX_QUEUE_DEPTH flag in bus.h. Note that changing the values of these
	flags in the header file requires a clean make in order for the 
	changes to take effect.
	
	This is a simple implementation that registers itself for a bus 
	access callback across the entire memory map. When a bus access 
	occurs, debug accesses are serviced immediately. For non-debug 
	accesses, a BusRequest container is set up to store the details of 
	the transaction, and a timed callback is registered with a pointer 
	to the newly created BusRequest container. The interval of the timed 
	callback is derived from the value of the DELAY flag. The callback 
	function uses the pointer to the BusRequest container passed back to 
	it to service the request, and then deletes the container. 
*/

#include <algorithm>
#include <assert.h>
#include <string.h>
using namespace std;
#include "bus.h"

extern "C" {

  void INTERFACE *RegisterCosim(char **name, HexagonWrapper *pHexWrapper)
  {
	// Register the name of the cosim.
	*name="BusCosim"; 

	// Instantiate a BusCosim class. 
	BusCosim *p_BusCosim = new BusCosim(pHexWrapper); 
	
	// Register for a bus access callback across the entire memory map.
	pHexWrapper->AddBusAccessCallback((void *)p_BusCosim,
										0x0, 
										0xffffffff, 
										BusAccessCallback);

    printf("Registered 0x0-0xffffffff for bus access callback...\n");
    return p_BusCosim;
  }
  
  char INTERFACE * GetCosimVersion()
  { // Return the Hexagon wrapper version.
	return (char *)HEXAGON_WRAPPER_VERSION;
  }

  void INTERFACE UnRegisterCosim(void *pBusCosim)
  {  // Call the destructor for BusCosim. 
	 BusCosim *p_BusCosim = (BusCosim *)pBusCosim;
     delete p_BusCosim;
  }
}
// BusCosim constructor.
BusCosim::BusCosim(HexagonWrapper *pHexWrapper)
{
    printf("Constructing bus cosim...\n");
	// Store pointer to simulator for use in cosim.
	m_simHandle=pHexWrapper;
	// Set initial queue depth.
    queue_depth=0;
	// Calculate time interval and units based on DELAY flag.
    m_simHandle->CycleToTime((HEX_8u_t)DELAY, &time, &units);
	// Initialize queue list to NULL pointers
	for(i=0; i<MAX_QUEUE_DEPTH; i++)
		queue_list[i]=NULL;
}
// BusCosim destructor.
BusCosim::~BusCosim()
{	
    printf("Destructing bus cosim...\n");
	// Free memory allocated for local storage.
	for(i=0;i<64*1024;i++)
	{
		if(mem_entries[i]) free(mem_entries[i]);
	}
	for(i=0; i<MAX_QUEUE_DEPTH; i++)
	{
		if(queue_list[i]) free(queue_list[i]);
	}
}

HEXAPI_TransactionStatus BusAccessCallback(void *pBusCosim, 
							HEX_PA_t address, 
							HEX_4u_t numBytes,
							HEX_1u_t* dataptr,
							HEX_4u_t reqID, 
							HEXAPI_BusAccessType bat,
							HEX_4u_t tnum, 
							HEXAPI_BusBurstType bt)
{
  // p_BusCosim points to the BusCosim class.
  BusCosim *p_BusCosim = (BusCosim*) pBusCosim;
  // p_BusRequest will point to a BusRequest structure type.
  BusRequest *p_BusRequest;

  // Do nothing for synch or barrier transactions.
  if((bat == HEX_SYNCH) || (bat == HEX_BARRIER)){
	// Signal completion of transaction.
	p_BusCosim->m_simHandle->BusTransactionFinished(dataptr, 
													numBytes, 
													reqID);
	// Signal acceptance of transaction.
	return TRANSACTION_SUCCESS;
  }

  // Debug transactions must be handled immediately.
  if((bat == HEX_DEBUG_READ) || (bat == HEX_DEBUG_WRITE))
  { // Perform the read or write.
	if(bat == HEX_DEBUG_WRITE)
		p_BusCosim->mem_access(address, numBytes, dataptr, WRITE);
	else
		p_BusCosim->mem_access(address, numBytes, dataptr, READ);
	// Signal completion of the debug transaction.
	p_BusCosim->m_simHandle->BusTransactionFinished(dataptr, 
													numBytes, 
													reqID);
	// Signal acceptance of the transaction.
	return TRANSACTION_SUCCESS;
  }
  // Non-debug transaction, can it be accepted?
  else 
  { // Signal rejection of the transaction if the queue is full.
	if(!p_BusCosim->check_queue())
		return TRANSACTION_REPLAY;
	// Otherwise, accept the transaction.
	else
	{   // Create a new bus request container.
		p_BusRequest = (BusRequest *) calloc(1, sizeof(BusRequest));
		// Store the transaction details for handling in a timed callback.
		p_BusRequest->m_address		= address;
		p_BusRequest->m_numBytes	= numBytes;
		p_BusRequest->m_dataptr		= dataptr;
		p_BusRequest->m_reqID		= reqID;
		p_BusRequest->m_type		= bat;
		p_BusRequest->pBusCosim		= p_BusCosim;

		// Increment queue by 1.
		p_BusCosim->increment_queue(p_BusRequest);

		// Register for a timed callback.
		p_BusCosim->m_simHandle->AddTimedCallback((void *) 
									p_BusRequest, 
									p_BusCosim->time, 
									p_BusCosim->units, 
									TimedCallBack);
		// Signal acceptance of the transaction.
		return TRANSACTION_SUCCESS;	
	}
  }
}


void TimedCallBack(void * pBusRequest)
{	// p_BusRequest points to a BusRequest structure type.
	BusRequest *p_BusRequest = (BusRequest *)pBusRequest;
	// p_BusCosim points to the BusCosim class.
	BusCosim *p_BusCosim = (BusCosim *)p_BusRequest->pBusCosim;

	// Check to make sure data pointer is non-NULL.
	// See release notes errata for details.
	if(p_BusRequest->m_dataptr)
	{	
		if((p_BusRequest->m_type == HEX_DATA_WRITE) 
		|| (p_BusRequest->m_type == HEX_DATA_CASTOUT)
		|| (p_BusRequest->m_type == HEX_DATA_WRITE_LOCKED))
		    // Perform write. 
		    p_BusCosim->mem_access(p_BusRequest->m_address,
 		 						   p_BusRequest->m_numBytes, 
    							   p_BusRequest->m_dataptr,
								   WRITE);
		else 			
			// Perform read.
			p_BusCosim->mem_access(p_BusRequest->m_address, 
								   p_BusRequest->m_numBytes, 
								   p_BusRequest->m_dataptr,
								   READ);
	} 
	// Signal completion of the transaction.
    p_BusCosim->m_simHandle->BusTransactionFinished(p_BusRequest->m_dataptr,
 													p_BusRequest->m_numBytes,
 													p_BusRequest->m_reqID);
	// Remove the timed callback.
    p_BusCosim->m_simHandle->RemoveTimedCallback(p_BusRequest);

	// Decrement the queue by 1.
    p_BusCosim->decrement_queue(p_BusRequest);

	// Free the memory used to store the bus request.
    free(p_BusRequest);

	return;
}
	
void BusCosim::mem_access(unsigned int address, 
						  unsigned int numBytes, 
						  unsigned char* data, 
						  bool write)
{   // Memory is partitioned as an index of 64k pages.
	// The upper 16 bits of the address are used as an 
	// index into a page array. The lower bits are used 
	// as offset into the page
	unsigned short tindex = address >> 16;
	unsigned short offset = address & 0xFFFF;

	unsigned int size;
	memblock_t *memptr;

	// The 'do' loop handles accesses across a 64k boundary.
	do {
		// Calculate how much space is left in the current page.
		size = (~offset & 0xffff) + 1;
		if(numBytes < size) size = numBytes;

		// Set the pointer to the top of the requested page.
		memptr = mem_entries[tindex];

		// Check if the requested page already exists.
		if(!memptr)
		{	// If not, allocate it from the heap.
			memptr = (memblock_t *)calloc (1, sizeof(memblock_t));
			mem_entries[tindex] = memptr;
		}
		// Peform read or write depending on how mem_access was called.
		if(write)	
			// Write the data to local storage.
			memcpy((unsigned char *)memptr + offset, data, size);
		else
			// Read the data from local storage.
			memcpy(data, (unsigned char *)memptr + offset, numBytes);

		// Set loop variables for access to the next 64k page if needed.
		data += size;
		tindex++;
		offset = 0;
		numBytes -= size;

	}while(numBytes > 0); 
		
	return;
}







