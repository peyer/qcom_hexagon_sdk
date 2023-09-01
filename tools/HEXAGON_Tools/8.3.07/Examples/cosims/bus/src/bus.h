/*****************************************************************
# Copyright (c) $Date: 2021/08/04 $ QUALCOMM INCORPORATED.
# All Rights Reserved.
*****************************************************************/
#ifndef __BUS_H
#define __BUS_H
#include "HexagonWrapper.h"

// Delay unit is core clock cycles.
#define DELAY 100
#define MAX_QUEUE_DEPTH 10

// Bool values for use with mem_access() helper function.
#define WRITE 1
#define READ  0

// Memory is partitioned as an index of 64k pages.
typedef unsigned char memblock_t[64*1024];
memblock_t *mem_entries[64*1024];

// Forward declaration of BusCosim class for use
// in declaration of BusRequest structure type.
class BusCosim;

// Storage container type for bus requests.
typedef struct BusRequest_t {
  	HEX_PA_t m_address;
	HEX_4u_t m_numBytes;
	HEX_1u_t* m_dataptr;
    HEX_4u_t m_reqID;
	HEXAPI_BusAccessType m_type;
	BusCosim *pBusCosim;
}BusRequest;

class BusCosim {
private:
	// Counter used to keep track of queue depth.
	unsigned int queue_depth;
	// List of pending transactions. This list
	// is only used by the destructor to free
	// memory of unhandled transactions
	// upon simulation termination.
	BusRequest* queue_list[MAX_QUEUE_DEPTH];
	// Variable used in assorted 'for' loops.
	int i;
public:
	// Pointer to HexagonWrapper class.
	HexagonWrapper *m_simHandle;
	// Time delay and units for timed callback.
	HEX_8u_t time;
	HEXAPI_Interval units;
	// BusCosim constructor
	BusCosim(HexagonWrapper *);
	// BusCosim destructor
	~BusCosim();
	// Helper function which handles reads and writes.
	void mem_access(unsigned int, unsigned int, unsigned char*, bool);
	// Helper functions to manage queue depth.
	// The queue_list array is used to store pointers
	// to BusRequest structure types as they are created.
	// Once the request is serviced, it is removed from the list.
	// The array is only used to keep track of pending bus requests
	// for use in freeing memory on simulation termination.
	void decrement_queue(BusRequest* pBusRequest)
	{
		queue_depth--;
		assert(queue_depth >= 0);
		for(i=0; i<MAX_QUEUE_DEPTH; i++)
		{
            if(queue_list[i] == pBusRequest)
			{
                queue_list[i] = NULL;
                break;
            }
		}
	}
	void increment_queue(BusRequest* pBusRequest)
	{
		queue_depth++;
		assert(queue_depth < MAX_QUEUE_DEPTH);
		for (i=0; i<MAX_QUEUE_DEPTH; i++)
		{
			if (queue_list[i] == NULL)
            {
                queue_list[i] = pBusRequest;
                break;
            }
		}

	}

	bool check_queue(){return ((MAX_QUEUE_DEPTH-1) - queue_depth);}
};

// Timed callback prototype.
void TimedCallBack(void *);

// Bus access callback prototype.
HEXAPI_TransactionStatus BusAccessCallback(void *,
										  HEX_PA_t,
										  HEX_4u_t,
										  HEX_1u_t*,
										  HEX_4u_t,
										  HEXAPI_BusAccessType,
										  HEX_4u_t,
										  HEXAPI_BusBurstType);

#endif // __BUS_H

