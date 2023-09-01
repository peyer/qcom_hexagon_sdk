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
#include "dma_nv12_rw_halide.h"
#include "dma_nv12_rw.h"
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
int dma_nv12_rw_power_on_hvx(void) {
    FARF(HIGH, "\npower_on\n");
    return power_on_hvx();
}
// API to power off HVX
int dma_nv12_rw_power_off_hvx(void) {
    FARF(HIGH, "\npower_off\n");
    return power_off_hvx();
}
// APIs to select HVX performance modes(Low/nomial/turbo)
int dma_nv12_rw_set_hvx_perf_mode_low(void) {
    FARF(HIGH, "\nperf_low\n");
    return set_hvx_perf_mode_low();
}
int dma_nv12_rw_set_hvx_perf_mode_nominal(void) {
    FARF(HIGH, "\nperf_nominal\n");
    return set_hvx_perf_mode_nominal();
}
int dma_nv12_rw_set_hvx_perf_mode_turbo(void) {
    FARF(HIGH, "\nperf_turbo\n");
    return set_hvx_perf_mode_turbo();
}

// This function Wraps the ION buffers to halide_buffer_t and invoke the Halide Pipeline code 
int dma_nv12_rw_run(const unsigned char *input_buffer,
       int input_bufferLen, int width, int height,
       int stride, int format, short int is_ubwc, uint8_t *output_buffer, int output_bufferLen, int iterations, uint64_t* avg_time) {
    FARF(HIGH, "\ndma_nv12_rw_run\n");
    const uint8_t *input_memory = input_buffer;
    const uint8_t *output_memory = output_buffer;
    bool is_ubwc_flag = (bool) is_ubwc;
    int nRet = 0;
    FARF(HIGH, "input_memory %x output_memory %x width %d height %d\n", input_memory, output_memory, width, height);

    // Halide_buffer_t structures to hold input and output parameters of halide DMA pipeline function
    // Y and UV data need to be DMA'ed separately to allow seperate processing steps
    halide_buffer_t input_y = {0};
    halide_buffer_t input_uv = {0};    
    halide_buffer_t output_uv = {0};
    halide_buffer_t output_y = {0};

    halide_dimension_t in_dim_y[2] = {{0,0,0},{0,0,0}};
    halide_dimension_t in_dim_uv[3]= {{0,0,0},{0,0,0},{0,0,0}};
    halide_dimension_t out_y[2]= {{0,0,0},{0,0,0}};
    halide_dimension_t out_uv[3]={{0,0,0},{0,0,0},{0,0,0}};
    // Get the Pixel size based on the Format ( 8-bit or 16-bit)
    int pixelsize = 1;
    if (format == 4) 
        pixelsize = 2; 
    // Populating the structure halide_buffer_t based on the input arguments
    input_y.type.code = halide_type_uint;
    input_y.type.bits = 8 * pixelsize; 
    input_y.type.lanes = 1;
    input_y.dimensions = 2;

    input_uv.type.code = halide_type_uint;
    input_uv.type.bits = 8 * pixelsize; 
    input_uv.type.lanes = 1;
    input_uv.dimensions = 3;

    output_uv.type.code = halide_type_uint;
    output_uv.type.bits = 8 * pixelsize;
    output_uv.type.lanes = 1;
    output_uv.dimensions = 3;

    output_y.type.code = halide_type_uint;
    output_y.type.bits = 8 * pixelsize; 
    output_y.type.lanes = 1;
    output_y.dimensions = 2;

    input_y.dim = in_dim_y;
    input_uv.dim = in_dim_uv;    
    output_uv.dim = out_uv;
    output_y.dim = out_y;
    // Y Plane Stride and range
    input_y.dim[0].min = 0;
    input_y.dim[0].stride = 1;
    input_y.dim[0].extent = width; 
    input_y.dim[1].min = 0;
    input_y.dim[1].stride = width; 
    input_y.dim[1].extent = height;
    // UV Plane stride and range
    // U and V are interleaved, stride =2 will point to next U or V pixel
    // NV12 - UV Size is W * (H / 2 )
    input_uv.dim[0].min = 0;
    input_uv.dim[0].stride = 2;
    input_uv.dim[0].extent = width/2;
    input_uv.dim[1].min = height;
    input_uv.dim[1].stride = width;
    input_uv.dim[1].extent = height/2;
    input_uv.dim[2].min = 0;
    input_uv.dim[2].stride =1;
    input_uv.dim[2].extent = 2;

    // Y Plane Stride and range
    output_y.dim[0].min = 0;
    output_y.dim[0].stride = 1;
    output_y.dim[0].extent = width;
    output_y.dim[1].min = 0;
    output_y.dim[1].stride = width;
    output_y.dim[1].extent = height;

    // UV Plane stride and range
    // U and V are interleaved, stride =2 will point to next U or V pixel
    // NV12 - UV Size is W * (H / 2 )
    output_uv.dim[0].min = 0;
    output_uv.dim[0].stride = 2;
    output_uv.dim[0].extent = width/2;
    output_uv.dim[1].min = height;
    output_uv.dim[1].stride = width;
    output_uv.dim[1].extent = height/2;
    output_uv.dim[2].min = 0;
    output_uv.dim[2].stride = 1;
    output_uv.dim[2].extent = 2;
    // User format to DMA Format conversion
    halide_hexagon_image_fmt_t linear_format = (halide_hexagon_image_fmt_t)(format + 1);
    halide_hexagon_image_fmt_t chroma_format = (halide_hexagon_image_fmt_t)(format + 2);

    // In order to actually do a DMA transfer, we need to allocate a
    // DMA engine first.
    void *dma_engine = nullptr;
    nRet = halide_hexagon_dma_allocate_engine(0, &dma_engine);
    if (nRet) {
        FARF(HIGH, "halide_hexagon_dma_allocate_engine returns %d \n", nRet);
        return nRet;
    }
    // DMA Power profile selection
    nRet = halide_hexagon_dma_power_mode_voting(0, halide_hexagon_power_nominal);
    FARF(HIGH, "halide_hexagon_dma_power_mode_voting %d \n", nRet);
    
    uint64_t devicep = (uint64_t) input_memory;
    input_y.device = 0;
    // Initialize DMA with input Y Frame details
    nRet = halide_hexagon_dma_device_wrap_native(0, &input_y, devicep);
    if (nRet) {
        // De-allocate the Allocated DMA engine if the frame initialization fails 
        FARF(HIGH, "halide_hexagon_dma_device_wrap_native returns %d \n", nRet);
        halide_hexagon_dma_deallocate_engine(0, dma_engine); 
        return nRet;
    }

    // Initialize DMA with input UV Frame details
    input_uv.device = 0;
    nRet = halide_hexagon_dma_device_wrap_native(0, &input_uv, devicep);
    if (nRet) {
        // De-allocate the Allocated DMA engine if the frame initialization fails 
        FARF(HIGH, "halide_hexagon_dma_device_wrap_native returns %d \n", nRet);
        halide_hexagon_dma_deallocate_engine(0, dma_engine);
        return nRet;
    }

    // Initialize DMA with Output Y Frame details
    uint64_t devicey = (uint64_t) output_memory;
    output_y.device = 0;
    nRet = halide_hexagon_dma_device_wrap_native(0, &output_y, devicey);
    if (nRet) {
        // De-allocate the Allocated DMA engine if the frame initialization fails 
        FARF(HIGH, "halide_hexagon_dma_device_wrap_native returns %d \n", nRet);
        halide_hexagon_dma_deallocate_engine(0, dma_engine);
        return nRet;
    }

    // Initialize DMA with input UV Frame details
    uint64_t devicec = (uint64_t) output_memory;
    output_uv.device = 0;
    nRet = halide_hexagon_dma_device_wrap_native(0, &output_uv, devicec);
    if (nRet) {
        // De-allocate the Allocated DMA engine if the frame initialization fails 
        FARF(HIGH, "halide_hexagon_dma_device_wrap_native returns %d \n", nRet);
        halide_hexagon_dma_deallocate_engine(0, dma_engine);
        return nRet;
    }
    // Set the Device Dirty Flag for choosing Halide Buffer Copy Direction for DMA Read 
    input_y.flags = input_y.flags | halide_buffer_flag_device_dirty;
    nRet = halide_hexagon_dma_prepare_for_copy_to_host(0, &input_y, dma_engine, is_ubwc_flag, linear_format);
    if (nRet) {
        // De-allocate the Allocated DMA engine if the API fails
        FARF(HIGH, "halide_hexagon_dma_prepare_for_copy_to_host returns %d \n", nRet);
        halide_hexagon_dma_deallocate_engine(0, dma_engine);
        return nRet;
    }

    // Set the Device Dirty Flag for choosing Halide Buffer Copy Direction for DMA Read 
    input_uv.flags = input_uv.flags | halide_buffer_flag_device_dirty;
    nRet = halide_hexagon_dma_prepare_for_copy_to_host(0, &input_uv, dma_engine, is_ubwc_flag, chroma_format);
    if (nRet) {
        // De-allocate the Allocated DMA engine if the API fails
        FARF(HIGH, "halide_hexagon_dma_prepare_for_copy_to_host returns %d \n", nRet);
        halide_hexagon_dma_deallocate_engine(0, dma_engine);
        return nRet;
    }

    // Set the Device Dirty Flag for choosing Halide Buffer Copy Direction for DMA Read 
    output_y.flags = output_y.flags | halide_buffer_flag_device_dirty;
    nRet = halide_hexagon_dma_prepare_for_copy_to_device(0, &output_y, dma_engine, 0, linear_format);
    if (nRet) {
        // De-allocate the Allocated DMA engine if the API fails
        FARF(HIGH, "halide_hexagon_dma_prepare_for_copy_to_device returns %d \n", nRet);
        halide_hexagon_dma_deallocate_engine(0, dma_engine);
        return nRet;
    }

    // Set the Device Dirty Flag for choosing Halide Buffer Copy Direction for DMA Write
    output_uv.flags = output_uv.flags | halide_buffer_flag_device_dirty;
    nRet = halide_hexagon_dma_prepare_for_copy_to_device(0, &output_uv, dma_engine, 0, chroma_format);
    if (nRet) {
        // De-allocate the Allocated DMA engine if the API fails
        FARF(HIGH, "halide_hexagon_dma_prepare_for_copy_to_device returns %d \n", nRet);
        halide_hexagon_dma_deallocate_engine(0, dma_engine);
        return nRet;
    }
    // Invoke the Halide Pipeline code and time the code for benchmarking purpose
    int result = 0;
    (*avg_time) = benchmark(iterations, [&]() {
        result = dma_nv12_rw_halide(&input_y, &input_uv, &output_y, &output_uv);
        if (result != 0) {
            FARF(HIGH, "dma_nv12_rw_test failed! %d\n", result);
        }
    });
    // de-init DMA : Input Y- Frame  
    nRet = halide_hexagon_dma_unprepare(0, &input_y);
    if (nRet) {
        // De-allocate the Allocated DMA engine if the API fails
        FARF(HIGH, "halide_hexagon_dma_unprepare returns %d \n", nRet);
        halide_hexagon_dma_deallocate_engine(0, dma_engine);
        return nRet;
    }

    // de-init DMA : Output Y- Frame  
    nRet = halide_hexagon_dma_unprepare(0, &output_y);
    if (nRet) {
        // De-allocate the Allocated DMA engine if the API fails
        FARF(HIGH, "halide_hexagon_dma_unprepare returns %d \n", nRet);
        halide_hexagon_dma_deallocate_engine(0, dma_engine);
        return nRet;
    }
    // de-init DMA : Input  UV- Frame
    nRet = halide_hexagon_dma_unprepare(0, &input_uv);
    if (nRet) {
        FARF(HIGH, "halide_hexagon_dma_unprepare returns %d \n", nRet);
        halide_hexagon_dma_deallocate_engine(0, dma_engine);
        return nRet;
    }

    // de-init DMA : Output  UV- Frame
    nRet = halide_hexagon_dma_unprepare(0, &output_uv);
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
