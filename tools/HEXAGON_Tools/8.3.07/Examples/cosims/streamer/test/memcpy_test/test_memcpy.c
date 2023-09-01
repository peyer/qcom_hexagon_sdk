/***************************************************************************
 * Copyright (c) Date: Mon Nov 24 2008 QUALCOMM INCORPORATED
 * All Rights Reserved
 * Modified by QUALCOMM INCORPORATED on Mon Oct 20 2014
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "streamer_swif.h"
#include <hexagon_standalone.h>

#ifndef TCM_BASE
#error TCM_BASE must be defined in makefile!
#endif
#ifndef STREAMER_BASE
#error STREAMER_BASE must be defined in makefile!
#endif

#define BUF_NUM_LINES 16
#define PIXEL_WIDTH 2
#define MAXPATHLEN 1024

// The following values are used to configure the data format the
// streamer should expect to process. Refer to the Camera Streaming
// Programming Interface guide for more details.
#define  RX_PIXEL_SIZE_ON_INPUT_LINK  0x003 // 14-bit pixel data
#define  RX_PIXEL_SIZE_TO_RX_BUFFER   0x004 // 16-bit pixel data
#define  TX_PIXEL_SIZE_FROM_TX_BUFFER 0x400 // 16-bit pixel data
#define  TX_PIXEL_SIZE_ON_OUTPUT_LINK 0x300 // 14-bit pixel data

// Test assumes camera configuration registers are visible on the bus.
// The pointers to streamer_input/output/0/1 are used to start the
// streamer devices modeled by the cosim and to pass input/output file
// names to the cosim. On actual hardware, the input and output could
// be started by the Hexagon or by another processor. The filename
// passing via a register is a convention used strictly for simulation.
volatile unsigned int *streamer_input0  = (volatile unsigned int *)(STREAMER_BASE + 0x000);
volatile unsigned int *streamer_input1  = (volatile unsigned int *)(STREAMER_BASE + 0x100);
volatile unsigned int *streamer_output0 = (volatile unsigned int *)(STREAMER_BASE + 0x200);
volatile unsigned int *streamer_output1 = (volatile unsigned int *)(STREAMER_BASE + 0x300);

// The streamer control registers are all modeled internally to the Hexagon.
// The pointers to streamer_interface0/1 reflect the location where the
// test expects the interface for each streamer to live in the memory map.
// See the implementation of streamer_if_t in streamer_swif.h for a full
// listing of all the registers. Refer to the Camera Streaming Progamming
// Interface guide for more details on the interface.
volatile streamer_if_t *streamer_interface0 = (volatile streamer_if_t *)(TCM_BASE + 0x001c0000);
volatile streamer_if_t *streamer_interface1 = (volatile streamer_if_t *)(TCM_BASE + 0x001c2000);

void hvx_copy_line(unsigned char *, unsigned char *, unsigned int);

void streamer_newfile(volatile unsigned int *streamaddr, const char *filename)
{
    streamaddr[1] = (unsigned int)filename;
}

void streamer_start(volatile unsigned int *streamaddr)
{
    streamaddr[0] = 1;
}

void streamer_stop(volatile unsigned int *streamaddr)
{
    streamaddr[0] = 0;
}

void *tcm_alloc(unsigned int size)
{
    static char *tcm_addr = (char *)TCM_BASE;
    void *ret;
    ret = tcm_addr;
    tcm_addr += size;
    return ret;
}

// Starting addresses for the image are precomputed. Image is subdivided
// into equal sized horizontal slices (height/number of threads).
void memcpy_test( int width, int height)
{
    unsigned char *inbuf0;
    unsigned char *inbuf1;
    unsigned char *outbuf0;
    unsigned char *outbuf1;
    int linecount0 = 0;
    int linecount1 = 0;
    int processed_linecount0 = 2;
    int processed_linecount1 = 2;
    int bufsize  = 0;
    int linesize = 0;
    int in_idx0  = 0;
    int in_idx1  = 0;
    int out_idx0 = 0;
    int out_idx1 = 0;

    // Each Hexagon can process two output rows simultaneously. For a 3x3
    // convolution, at least 4 input rows are needed before starting any
    // processing in order to produce two output rows.
    int runahead_linecount = 4;

    streamer_config_t config0;
    streamer_config_t config1;

    linesize = width*PIXEL_WIDTH;
    bufsize = width*PIXEL_WIDTH*BUF_NUM_LINES;

    // Allocate input/output buffers for the streamer.
    // The buffers must be allocated in TCM.
    inbuf0  = tcm_alloc(bufsize);
    inbuf1  = tcm_alloc(bufsize);
    outbuf0 = tcm_alloc(bufsize);
    outbuf1 = tcm_alloc(bufsize);

    // config0/1 will be used to configure the streamer devices.
    streamer_config_init(&config0);
    streamer_config_init(&config1);
    streamer_config_tx(&config0,(uint32_t)outbuf0, bufsize, width, height);
    streamer_config_tx(&config1,(uint32_t)outbuf1, bufsize, width, height);
    streamer_config_rx(&config0,(uint32_t) inbuf0, bufsize, width, height);
    streamer_config_rx(&config1,(uint32_t) inbuf1, bufsize, width, height);

    // The format is based on the image type, which is predefined
    // in this case.
    config0.format = config1.format =
        RX_PIXEL_SIZE_TO_RX_BUFFER
        | TX_PIXEL_SIZE_FROM_TX_BUFFER
        | TX_PIXEL_SIZE_ON_OUTPUT_LINK
        | RX_PIXEL_SIZE_ON_INPUT_LINK;

    streamer_init(streamer_interface0, &config0);
    streamer_init(streamer_interface1, &config1);

    // The following code starts the various streamer interfaces
    // and devices. As mentioned in comments above, this can be done
    // with code on the Hexagon directing the devices to start, or by
    // another processor that starts both the input/output devices and
    // signals the Hexagon software to begin processing.
    //
    // Start the Hexagon streamers.
    streamer_tx_start(streamer_interface0);
    streamer_tx_start(streamer_interface1);
    streamer_rx_start(streamer_interface0);
    streamer_rx_start(streamer_interface1);
    // Start the input/output devices.
    streamer_start(streamer_input0);
    streamer_start(streamer_input1);
    streamer_start(streamer_output0);
    streamer_start(streamer_output1);

    // The test will poll the Hexagon streamer interface waiting
    // for at least 4 lines before starting.
    linecount0 = linecount1 = runahead_linecount - 1;
    processed_linecount0 = processed_linecount1 = 0;

    // Begin line processing. The code polls each Hexagon streamer
    // interface for an available line count in its respective input
    // buffer, waiting for at least 4 lines to be available before
    // starting the processing for the first iteration through the
    // loop. Once the lines are processed, all of the input/output
    // indexes are updated. This continues until there are no more
    // lines to process as defined by the predetermined image height.
    while (linecount0 < height) {
        linecount0 = streamer_rx_wait_for_line(streamer_interface0, (linecount0|1));
        do {
            hvx_copy_line(outbuf0 + out_idx0, inbuf0 + in_idx0, 2*linesize);
            streamer_rx_done(streamer_interface0, in_idx0);
            streamer_tx_done(streamer_interface0, out_idx0);
            in_idx0 = streamer_rx_wrap_idx(in_idx0 + 2*linesize, &config0);
            out_idx0 = streamer_tx_wrap_idx(out_idx0 + 2*linesize, &config0);
            processed_linecount0 += 2;
        } while (linecount0 > processed_linecount0);
        linecount1 = streamer_rx_wait_for_line(streamer_interface1, (linecount1|1));
        do {
            hvx_copy_line(outbuf1 + out_idx1, inbuf1 + in_idx1, 2*linesize);
            streamer_rx_done(streamer_interface1, in_idx1);
            streamer_tx_done(streamer_interface1, out_idx1);
            in_idx1 = streamer_rx_wrap_idx(in_idx1 + 2*linesize, &config1);
            out_idx1 = streamer_tx_wrap_idx(out_idx1 + 2*linesize, &config1);
            processed_linecount1 += 2;
        } while (linecount1 > processed_linecount1);
    }

    // Wait for Hexagon streamer interface to signal output EOF.
    streamer_tx_done(streamer_interface0, out_idx0);
    streamer_tx_done(streamer_interface1, out_idx1);
    streamer_tx_wait_for_eof(streamer_interface0);
    streamer_tx_wait_for_eof(streamer_interface1);
}

int main(int argc, char* argv[])
{
    uint32_t width, height;
    char buf[MAXPATHLEN];

    // Translation for camera streamer registers. Configured as device-type memory.
    add_translation_extended(1   /*TLB index*/,
                             (void*)STREAMER_BASE /*VA*/,
                             (unsigned long long)STREAMER_BASE /*PA*/,
                             16  /*size*/,
                             0xF /*XWRU*/,
                             4   /*CCCC*/,
                             0   /*ASID*/,
                             0   /*AA*/,
                             3   /*VG*/);
    // Translation for TCM. Configured as uncached memory.
    add_translation_extended(2   /*TLB index*/,
                             (void*)TCM_BASE /*VA*/,
                             (unsigned long long)TCM_BASE /*PA*/,
                             16   /*size*/,
                             0xF  /*XWRU*/,
                             6    /*CCCC*/,
                             0    /*ASID*/,
                             0    /*AA*/,
                             3    /*VG*/);
    // Translation for streamer register space. Configured as device-type memory.
    add_translation_extended(3   /*TLB index*/,
                             (void*) (TCM_BASE+0x100000) /*VA*/,
                             (unsigned long long) (TCM_BASE+0x100000) /*PA*/,
                             16  /*size*/,
                             0xF /*XWRU*/,
                             4   /*CCCC*/,
                             0   /*ASID*/,
                             0   /*AA*/,
                             3   /*VG*/);

    printf("Using values %s %s %s %s\n",argv[1],argv[2],argv[3],argv[4]);
    width  = atoi(argv[1]);
    height = atoi(argv[2]);

    // Write the input/output filenames to the streamer device mailbox
    // registers for reading by the cosim. Note that this is a convention
    // used purely for simplifying the simulation and does not reflect
    // any actual hardware implementation.
    streamer_newfile(streamer_input0, argv[3]);
    streamer_newfile(streamer_input1, argv[3]);
    snprintf(buf, MAXPATHLEN-1, "%s.0", argv[4]);
    streamer_newfile(streamer_output0, buf);
    snprintf(buf, MAXPATHLEN-1, "%s.1", argv[4]);
    streamer_newfile(streamer_output1, buf);

    printf("starting...\n");
    SIM_ACQUIRE_HVX;
    memcpy_test(width, height);
    SIM_RELEASE_HVX;
    return 0;
}

