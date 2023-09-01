/*****************************************************************
# Copyright (c) $Date: 2011/03/22 QUALCOMM INCORPORATED.
# All Rights Reserved.
*****************************************************************/
/*

    Streamer Interface Cosim

    This module emulates a camera streaming interface. The module
    can be configured as an input or output device.  Refer to the
    readme in the top level directory for more details.
*/

#include <HexagonWrapper.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#define strtoull _strtoui64
#define strcasecmp _stricmp
#endif

#define DEBUG 0
#if DEBUG == 1
#define DBGMSG(a) printf a
#else
#define DBGMSG(a)
#endif
#if DEBUG == 2
#define DBGMSG2(a) printf a
#else
#define DBGMSG2(a)
#endif
#define ERRMSG(a) printf a; exit(1);

#define MAXPATHLEN 1024
#define MAX_ARGS 6
#define DELAY (200)

enum {
    DIRECTION_IN,
    DIRECTION_OUT,
};

// Storage container type for bus requests.
typedef struct BusRequest_t {
    HEX_PA_t m_address;
 	HEX_4u_t m_numBytes;
 	HEX_1u_t* m_dataptr;
    HEX_4u_t m_reqID;
 	HEXAPI_BusAccessType m_type;
 	void *pCosim;
} BusRequest;

typedef struct {
    HexagonWrapper *pHexWrapper;
    const char *name;
    unsigned int id;
    HEX_4u_t running, rev_id;
    char filename[MAXPATHLEN];
    FILE *file;
    int direction;
 	// Time delay and units for timed callback.
 	HEX_8u_t time;
 	HEXAPI_Interval units;
} Streamer;

void Usage()
{
    ERRMSG(("Invalid number of arguments to streamer! There should be 6: \n" \
    "Streamer.(so/dll) 1:name 2:base_addr 3:end_addr 4:freq 5:direction 6:ID\n"));
}

// StreamerFile is a helper function for reads and writes to camera streamer
// registers visible on the bus. It handles the file I/O necessary to open
// input and output files for reading and writing.
void StreamerFile(Streamer *me, HEX_8u_t address)
{
   HEXAPI_Status status;
   int i = 0;
   char temp;
   const char *opentype = (me->direction == DIRECTION_IN) ? "r" : "w";

   // Read the input/output filename to be opened from simulated program memory.
   // The input/output filenames are passed as arguments to the memcpy test
   // when the simulator is launched. The test writes the filenames to a
   // memory location with a value of <streamer_base_address+0x4>, which is
   // defined by convention as a mailbox for passing filename arguments to this
   // cosim. This module will use the simulator's API to read the filename from
   // the program memory, and will then open the file for reading or writing of
   // pixel data.
   // Note that this approach is a convention used for the purpose of simulating
   // the streamer interface rather than anything enforced by hardware.
   do {
       status = me->pHexWrapper->ReadMemory((HEX_PA_t) address+i, 1, (void *) &temp);
       if (status != HEX_STAT_SUCCESS)
       {
           ERRMSG (("StreamerFile - Could not Read Memory! Error code: %d", status));
       }
       me->filename[i] = temp;
   } while ((temp) && (++i < (MAXPATHLEN-1)));
   me->filename[i] = 0;

   DBGMSG(("StreamerFile - file to be opened: %s\n", me->filename));

   if (me->file) fclose(me->file);

   if ((me->file = fopen(me->filename, opentype)) == NULL) {
       perror("StreamerFile - Error");
       ERRMSG(("StreamerFile - Attempt to open %s [%s] failed!\n",me->filename, opentype));
   } else {
       DBGMSG(("StreamerFile - %s [%s] opened successfully!\n",me->filename, opentype));
   }

}

// StreamerRead() is a bus callback helper function for reading from streamer
// registers visible on the bus. <streamer_base_addr> indicates whether or not
// the camera streamer is active. <streamer_bae_addr+0x4> is a mailbox
// indicating file IO is needed. No action is taken for a file IO operation in
// StreamerRead().
HEX_4u_t StreamerRead(Streamer *me, HEX_PA_t addr)
{
    int regno = (addr >> 2) & 1;
    DBGMSG(("StreamerRead - regno: %x\n", regno));
    if (regno == 0) /* RUNNING */ return me->running;
    else if (regno == 1) /* FILE */ return 0;
    else return 0;
};

// StreamerWrited() is a bus callback helper function for writing to streamer
// registers visible on the bus. <streamer_base_addr> indicates whether or not
// the camera streamer is active. <streamer_bae_addr+0x4> is a mailbox
// indicating file IO is needed. When a file IO operation is needed, the data
// variable contains a pointer to a file to be opened as input or output. The
// direction depends on the streamer registered for the address that covers
// the mailbox.
void StreamerWrite(Streamer *me, HEX_PA_t addr, HEX_8u_t data)
{
    int regno = (addr >> 2) & 1;
    DBGMSG(("StreamerWrite - regno: %x\n", regno));
    if (me->file) fflush(me->file);
    if (regno == 0) /* RUNNING */ me->running = data;
    else if (regno == 1) /* FILE */ StreamerFile(me, data);
    else return;
}

void TimedCallBack(void * pBusRequest)
{
    BusRequest *pReq = (BusRequest *)pBusRequest;
    Streamer *me = (Streamer*) pReq->pCosim;
    HEX_PA_t address = pReq->m_address;
    HEX_4u_t numBytes = pReq->m_numBytes;
    HEX_1u_t *pData = pReq->m_dataptr;
    HEX_4u_t reqID = pReq->m_reqID;
    HEXAPI_BusAccessType bat = pReq->m_type;
    HEX_8u_t data = 0;

    if (pData)
    {
        // When timing is enabled, the simulator sends a null data
        // transaction for timing marker purpose, no data service
        if ((bat == HEX_DATA_WRITE) || (bat == HEX_DATA_WRITE_LOCKED) ||
            (bat == HEX_DATA_CASTOUT))
        {
            for (HEX_4u_t i=0; i<numBytes; i++)
                data |= pData[i] << (i*8);
            StreamerWrite(me, address, data);
        }
        else if ((bat == HEX_DATA_READ) || (bat == HEX_DATA_READ_LOCKED))
        {
            StreamerRead(me, address);
        }
    }
    else
    {
        DBGMSG(("dataptr is NULL, return with no action taken by callback\n"));
    }

    // Signal completion of the transaction and return success code.
    me->pHexWrapper->BusTransactionFinished(pData, numBytes, reqID);
    me->pHexWrapper->RemoveTimedCallback(pReq);
    free(pReq);
    return;
}

// BusAccessCallback() is called by the simulator whenever a bus request is made
// to an address that was registered by the cosim. This function determines
// whether the access is a read or write and calls the appropriate helper
// function to handle the request. The callback is registered with the simulator
// in the RegisterCosimArgs() function further down below.
HEXAPI_TransactionStatus BusAccessCallback(void *pStreamer,
                            HEX_PA_t address,
                            HEX_4u_t numBytes,
                            HEX_1u_t* dataptr,
                            HEX_4u_t reqID,
                            HEXAPI_BusAccessType bat,
                            HEX_4u_t tnum,
                            HEXAPI_BusBurstType bt)
{
    Streamer *me = (Streamer*) pStreamer;
    HEX_8u_t data = 0;
    BusRequest *pReq;
    HEXAPI_TransactionStatus stat = (HEXAPI_TransactionStatus) -1;

    DBGMSG(("Streamer - Called bus access callback with address:0x%llx access type:%x\n", address, bat));

    // Do nothing for synch or barrier transactions.
    if ((bat == HEX_SYNCH) || (bat == HEX_BARRIER))
    {
        stat = TRANSACTION_SUCCESS;
    }

    // Perform the read or write.
    if ((bat == HEX_DEBUG_WRITE) || (bat == HEX_DEBUG_WRITE_LOCKED))
    {
        for (HEX_4u_t i=0; i<numBytes; i++)
            data |= dataptr[i] << (i*8);
        StreamerWrite(me, address, data);
        stat = TRANSACTION_SUCCESS;
    }
    else if ((bat == HEX_DEBUG_READ) || (bat == HEX_DEBUG_READ_LOCKED))
    {
        StreamerRead(me, address);
        stat = TRANSACTION_SUCCESS;
    }

    // Otherwise, accept the transaction and cause latency.
    if (stat != ((HEXAPI_TransactionStatus) -1))
    {
       me->pHexWrapper->BusTransactionFinished(dataptr, numBytes, reqID);
       return stat;
    }

    // Store the transaction details for handling in a timed callback.
    pReq = (BusRequest *) calloc(1, sizeof(BusRequest));
    pReq->m_address = address;
    pReq->m_numBytes = numBytes;
    pReq->m_dataptr = dataptr;
    pReq->m_reqID	= reqID;
    pReq->m_type = bat;
    pReq->pCosim = pStreamer;

    if ((me->rev_id & 0xff) < 0x60)
    {
        TimedCallBack(pReq);
    }
    else
    {
        me->pHexWrapper->AddTimedCallback((void *) pReq, me->time, me->units, TimedCallBack);
    }
    // do NOT terminate bus transaction now
    return TRANSACTION_SUCCESS;
}

// StreamerIn() is a timed callback helper function that handles
// reading pixel data in from the input file and pushing it into the
// Hexagon streamer interface.
void StreamerIn(Streamer *me)
{
    char linebuf[128] = "0x";
    HEX_8u_t data;
    linebuf[127] = 0;
    if (me->file == NULL) return;

    // Try reading next sample from input file.
    if (fgets(&linebuf[2],125,me->file) == NULL) return; // EOF or error
    data = strtoul(linebuf,NULL,0);
    DBGMSG2(("StreamerIn - read data from input file: 0x%x\n", data));

    // Push sample to Hexagon streamer interface.
    me->pHexWrapper->StreamerPush(me->id, data);
    return;
}

// StreamerOut() is a timed callback helper function that handles
// pulling pixel data out of the Hexagon streamer interface and
// writing it into an output file.
void StreamerOut(Streamer *me)
{
    HEX_8u_t data;
    if (me->file == NULL) return;

    // Try reading next sample from Hexagon streamer interface.
    data = me->pHexWrapper->StreamerPull(me->id);

    // Write sample to output file.
    DBGMSG2(("StreamerOut - read data from streamer interface: 0x%llx\n", data));
    fprintf(me->file, "0x%llx\n", data);
    return;
}

// StreamerTick() is a timed callback function that is called N times per
// simulated second, where N is a function of the streamer frequency given in the
// cosim configuration file versus the simulated clock frequency of the Hexagon
// core. This function determines the direction of the streamer cosim being
// called and calls the appropriate helper function.
void StreamerTick(void *pStreamer)
{
    Streamer *me = (Streamer*) pStreamer;

    // Do nothing if this streamer has not been enabled yet.
    if (!me->running) return;

    // Otherwise, check the direction and call the appropriate helper.
    if (me->direction == DIRECTION_IN) {
        return StreamerIn(me);
    } else if (me->direction == DIRECTION_OUT) {
        return StreamerOut(me);
    } else {
        return;
    }
}

extern "C" {

  // RegisterCosimArgs() is the first function called by the simulator after the
  // streamer shared library is loaded. This function is responsible for parsing
  // the arguments passed to the streamer library in the cosim configuration file
  // and using them to register for the appropriate callbacks and configure the
  // streamer. Refer to the Simulator System API User Guide for more details on
  // this function.
  void INTERFACE *RegisterCosimArgs(char **name, HexagonWrapper *pHexWrapper, char *args)
  {
      char *tok;
      HEXAPI_Status status;
      HEX_4u_t options = 0;
      char *argv[MAX_ARGS];

      if (!args) Usage();

      tok = strtok (args, " ");

      // Tokenize the arguments for use in configuring the streamer.
      while (tok != NULL)
      {
          if (options >= MAX_ARGS) Usage();
          argv[options++] = tok;
          tok = strtok(NULL, " ");

      }
      if (options != MAX_ARGS) Usage();

      // Instantiate a Streamer object.
      Streamer *me = (Streamer *) calloc(1, sizeof(Streamer));

      // Configure the streamer.
       me->pHexWrapper = pHexWrapper;
       me->name = argv[0];
       *name = argv[0];
       HEX_PA_t baseaddr = strtoull(argv[1], NULL, 0);
       HEX_PA_t endaddr  = strtoull(argv[2], NULL, 0);
       me->id            = strtoul(argv[5], NULL, 0);

       // Calculate time interval and units based on DELAY flag.
       me->pHexWrapper->CycleToTime((HEX_8u_t)DELAY, &(me->time), &(me->units));

       char buffer[4], *addr = buffer;
       int i = 4;
       me->pHexWrapper->GetInfo(HEX_INFO_MYARCH, &addr, (int *) &i);
       sscanf(buffer, "%x", &(me->rev_id));


      // Register for a bus access callback for this streamer's register space.
      status = pHexWrapper->AddBusAccessCallback((void *)me,
                                                  baseaddr,
                                                  endaddr,
                                                  BusAccessCallback);
      if (status != HEX_STAT_SUCCESS)
      {
          ERRMSG (("Could not register bus access callback! Error: %d", status));
      }

      // Register for a timed callback.
      double tick_ns = 1/((strtoull(argv[3], NULL, 0))/(double)1000000000);
      status = pHexWrapper->AddTimedCallback((void *)me, (HEX_8u_t)tick_ns, HEX_NANOSEC, StreamerTick);

      // Set streamer direction.
      if (strcasecmp(argv[4], "IN") == 0)
          me->direction = DIRECTION_IN;
      else
          me->direction = DIRECTION_OUT;

      DBGMSG(("Streamer Registration - %sput device %s initialized\n", argv[4], *name));
      DBGMSG((" baseaddr: 0x%llx endaddr: 0x%llx ID: %d\n", baseaddr, endaddr, me->id));

      return me;

  } //RegisterCosimArgs

  char INTERFACE * GetCosimVersion()
  { // Return the Hexagon wrapper version.
    return (char *)HEXAGON_WRAPPER_VERSION;
  }

  void INTERFACE UnRegisterCosim(void *me)
  {  // Release memory allocated for the Streamer.
     free(me);
  }

} // extern "C"
