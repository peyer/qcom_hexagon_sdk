// Headers
#include "HAP_perf.h"
#include "hexagon_benchmark.h"
#include "sysmon_cachelock.h"
#include "hexagon_types.h"
#include "HAP_farf.h"
#include "HAP_power.h"
#include "qurt_error.h"
#include "hvx_interface.h"
//DMA Specific Headers
#include "dma_raw_blur_rw_async.h"
#include "dma_raw_blur_rw_async_halide.h"
#include "dma_def.h"
#include "dma_types.h"
#include "dmaWrapper.h"

#include "qurt_hvx.h"
#include "qurt.h"
#include <stdlib.h>
#include <string.h>
#include "HalideRuntime.h"
#include "HalideRuntimeHexagonDma.h"
#include "HalideRuntimeHexagonHost.h"

#ifndef SIMULATOR
#undef FARF_LOW
#define FARF_LOW 1
#undef FARF_HIGH
#define FARF_HIGH 1
#endif
// API to power on HVX
int dma_raw_blur_rw_async_power_on_hvx(void) {
    FARF(HIGH, "\npower_on\n");
    return power_on_hvx();
}
// API to power off HVX
int dma_raw_blur_rw_async_power_off_hvx(void) {
    FARF(HIGH, "\npower_off\n");
    return power_off_hvx();
}
// APIs to select HVX performance modes(Low/nomial/turbo)
int dma_raw_blur_rw_async_set_hvx_perf_mode_low(void) {
    FARF(HIGH, "\nperf_low\n");
    return set_hvx_perf_mode_low();
}
int dma_raw_blur_rw_async_set_hvx_perf_mode_nominal(void) {
    FARF(HIGH, "\nperf_nominal\n");
    return set_hvx_perf_mode_nominal();
}
int dma_raw_blur_rw_async_set_hvx_perf_mode_turbo(void) {
    FARF(HIGH, "\nperf_turbo\n");
    return set_hvx_perf_mode_turbo();
}
// This function Wraps the ION buffers to halide_buffer_t and invoke the Halide Pipeline code
int dma_raw_blur_rw_async_run(const unsigned char *input_buffer,
       int input_bufferLen, int width, int height,
       int stride, int format, short int is_ubwc, uint8_t *output_buffer, int output_bufferLen, int iterations, uint64_t* avg_time) {
    FARF(HIGH,"\ndma_raw_blur_rw_async_run\n");
    const uint8_t *internal_memory = input_buffer;
    const uint8_t *output_memory = output_buffer;
    bool is_ubwc_flag = (bool) is_ubwc;
    int nRet = 0;
    FARF(HIGH, "internal_memory %x width %d height %d\n", internal_memory, width, height);

    // Halide_buffer_t structures to hold input and output parameters of halide DMA pipeline function
    halide_buffer_t input = {0};
    halide_buffer_t output = {0};

    halide_dimension_t in_dim[2] = {{0,0,0},{0,0,0}};
    halide_dimension_t out[2]= {{0,0,0},{0,0,0}};
    // Populating the structure halide_buffer_t based on the input arguments
    input.type.code = halide_type_uint;
    input.type.bits = 8; 
    input.type.lanes = 1;
    input.dimensions = 2;
  
    output.type.code = halide_type_uint;
    output.type.bits = 8; 
    output.type.lanes = 1;
    output.dimensions = 2;

    input.dim = in_dim;
    output.dim = out;
    // Input Stride and range
    input.dim[0].min = 0;
    input.dim[0].stride = 1;
    input.dim[0].extent = width;
    input.dim[1].min = 0;
    input.dim[1].stride = width;
    input.dim[1].extent = height;

    // Output Stride and range
    output.dim[0].min = 0;
    output.dim[0].stride = 1;
    output.dim[0].extent = width;
    output.dim[1].min = 0;
    output.dim[1].stride = width;
    output.dim[1].extent = height;

    // In order to actually do a DMA transfer, we need to allocate a
    // DMA engine.
    void *dma_engine = nullptr;
    nRet = halide_hexagon_dma_allocate_engine(0, &dma_engine);
    if (nRet) {
        FARF(HIGH, "halide_hexagon_dma_allocate_engine returns %d \n", nRet);
        return nRet;
    }
    // DMA Power profile selection	
    nRet = halide_hexagon_dma_power_mode_voting(0, halide_hexagon_power_nominal);
    FARF(HIGH, "halide_hexagon_dma_power_mode_voting %d \n", nRet);

    uint64_t devicep = (uint64_t) internal_memory;
    input.device = 0;
    // Initialize DMA with input Frame details
    nRet = halide_hexagon_dma_device_wrap_native(0, &input, devicep);
    if (nRet) {
        // De-allocate the Allocated DMA engine if the frame initialization fails
        FARF(HIGH, "halide_hexagon_dma_device_wrap_native returns %d \n", nRet);
        halide_hexagon_dma_deallocate_engine(0, dma_engine);
        return nRet;
    } 
    // Set the Device Dirty Flag for choosing Halide Buffer Copy Direction for DMA Read
    input.flags = input.flags | halide_buffer_flag_device_dirty;
    // Complete the DMA Read Initialization
    nRet = halide_hexagon_dma_prepare_for_copy_to_host(0, &input, dma_engine, is_ubwc_flag, halide_hexagon_fmt_RawData);
    if (nRet) {
        FARF(HIGH, "halide_hexagon_dma_prepare_for_copy_to_host returns %d \n", nRet);
        halide_hexagon_dma_deallocate_engine(0, dma_engine);
        return nRet;
    }

    uint64_t devicey = (uint64_t) output_memory;
    output.device = 0;
    // Initialize DMA with Output Y Frame details
    nRet = halide_hexagon_dma_device_wrap_native(0, &output, devicey);
    if (nRet) {
        FARF(HIGH, "halide_hexagon_dma_device_wrap_native returns %d \n", nRet);
        halide_hexagon_dma_deallocate_engine(0, dma_engine);
        return nRet;
    }

    output.flags = output.flags | halide_buffer_flag_device_dirty;
    // Complete the DMA Write initialization
    nRet = halide_hexagon_dma_prepare_for_copy_to_device(0, &output, dma_engine, is_ubwc_flag, halide_hexagon_fmt_RawData);
    if (nRet) {
        FARF(HIGH, "halide_hexagon_dma_prepare_for_copy_to_device returns %d \n", nRet);
        halide_hexagon_dma_deallocate_engine(0, dma_engine);
        return nRet;
    }
    // Invoke the Halide Pipeline code and time the code for benchmarking purpose
    int result = 0;
    (*avg_time) = benchmark(iterations, [&]() {
        result = dma_raw_blur_rw_async_halide(&input, &output);
        if (result != 0) {
            FARF(HIGH, "dma_raw_blur_rw_async_test failed! %d\n", result);
        }
    });
    // de-init DMA, input side
    nRet = halide_hexagon_dma_unprepare(0, &input);
    if (nRet) {
        FARF(HIGH, "halide_hexagon_dma_unprepare returns %d \n", nRet);
        halide_hexagon_dma_deallocate_engine(0, dma_engine);
        return nRet;
    }

    // de-init DMA, output side
    nRet = halide_hexagon_dma_unprepare(0, &output);
    if (nRet) {
        FARF(HIGH, "halide_hexagon_dma_unprepare returns %d \n", nRet);
        halide_hexagon_dma_deallocate_engine(0, dma_engine);
        return nRet;
    }
    // DMA Transfer complete, Deallocate the engine
    nRet = halide_hexagon_dma_deallocate_engine(0, dma_engine);
    if (nRet) {
        FARF(HIGH, "halide_hexagon_dma_deallocate_engine returns %d \n", nRet);
        return nRet;
    }

    FARF(HIGH, "Success! \n");
    return 0;
}
