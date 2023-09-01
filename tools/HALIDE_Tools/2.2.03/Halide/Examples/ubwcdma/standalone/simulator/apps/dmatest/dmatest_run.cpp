#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <stdlib.h>
#include "hexagon_types.h"
#include "sysmon_cachelock.h"
#include "HalideRuntimeHexagonDma.h"
#include "HalideRuntimeHexagonHost.h"
#include "HalideBuffer.h"
#include "dmatest_i.h"
#include "HAP_farf.h"
#include "HAP_power.h"
#include "dma_def.h"
#include "dma_types.h"
#include "dmaWrapper.h"
#include "qurt.h"
#include "../../include/io.h"
#include "simulator_benchmark.h"


#define CEILING(num, div) ((num + div -1)/(div))
#define ALIGN(x, a) CEILING(x, a) * a

void halide_print(void *user_context, const char *msg) {

    printf("%s\n", msg);
}

int main(int argc, char **argv) {

    const int width = 1024;
    const int height = 1024;

    // Initialize the register region for the DMA (must always be first).
    int nRet;
    nRet = nDma_InitializeUbwcRegisterMemory();
    if (nRet != QURT_EOK){
        return 1;
    }
    // Initialize the DMA block and firmware.
    nRet = nDma_Initialize();
    if (nRet != QURT_EOK){
        return 1;
    }

    t_StDmaWrapper_FrameProp pStFrameProp;
    pStFrameProp.aAddr = 0;
    pStFrameProp.u16H = height;
    pStFrameProp.u16W = width;
    pStFrameProp.u16Stride = width;
    int frameSize = nDmaWrapper_GetFramesize(eDmaFmt_NV12, &pStFrameProp, false);

    printf("frameSize %d \n", frameSize);
    qurt_size_t region_ddr_size = ALIGN(frameSize * 2, 0x1000);

    qurt_mem_pool_t pool_ddr;
    qurt_mem_region_t region_ddr;
    qurt_mem_region_attr_t region_ddr_attr;
    // Attach to the default physical pool.
    nRet = qurt_mem_pool_attach ("DEFAULT_PHYSPOOL", &pool_ddr);
    if(nRet != QURT_EOK) {
        return nRet;
    }
    // Initialize the region attributes.
    qurt_mem_region_attr_init (&region_ddr_attr);
    // Set the region cache properties. The DDR region must be set to be non-cacheable as otherwise the DMA will read stale values.
    // The TCM regions are left with default permissions.
    qurt_mem_region_attr_set_cache_mode(&region_ddr_attr, QURT_MEM_CACHE_WRITETHROUGH_NONL2CACHEABLE);
    // Set the region permissions to allow reads and writes.
    int perms = QURT_PERM_WRITE | QURT_PERM_READ;
    qurt_mem_region_attr_set_perms(&region_ddr_attr, perms);
    // Create the mappings. This maps a virtual region of the provided size to a contiguous physical block.
    nRet = qurt_mem_region_create(&region_ddr, region_ddr_size, pool_ddr, &region_ddr_attr);
    if(nRet !=QURT_EOK){
        return nRet;
    }

    printf("region_ddr_size %d\n", region_ddr_size);
    // Get the virtual addresses to use for the TCM and DDR regions.
    qurt_addr_t ddr_buf_vaddr;
    qurt_mem_region_attr_get(region_ddr, &region_ddr_attr);
    qurt_mem_region_attr_get_virtaddr(&region_ddr_attr, &ddr_buf_vaddr);

    uint8_t *input_memory = (uint8_t*)ddr_buf_vaddr;
    uint8_t *output_memory = (uint8_t*)ddr_buf_vaddr + frameSize; 

    int h = (height*3)/2;

    int fp;
    if((fp = open("adb.bin", O_RDONLY)) < 0 )
    {
      printf("Error: Cannot open %s for input\n", "adb.bin");
      return 1;
    }

    for(int i=0; i<h; i++)
    {
         if(read(fp, &input_memory[i*width],  sizeof(unsigned char)*width)!=width) {
         close(fp);
         return 1;
         }
    }
    close(fp);

    Halide::Runtime::Buffer<uint8_t> input(nullptr, width, (3*height) / 2);

    void *dma_engine = nullptr;
    int ret = halide_hexagon_dma_allocate_engine(nullptr, &dma_engine);
    if (ret != 0) {
        printf("halide_hexagon_dma_allocate_engine\n");
    }

    Halide::Runtime::Buffer<uint8_t> input_y = input.cropped(1, 0, height);    // Luma plane only
    Halide::Runtime::Buffer<uint8_t> input_uv = input.cropped(1, height, height / 2);  // Chroma plane only, with reduced height

    input_uv.embed(2, 0);
    input_uv.raw_buffer()->dim[2].extent = 2;
    input_uv.raw_buffer()->dim[2].stride = 1;
    input_uv.raw_buffer()->dim[0].stride = 2;
    input_uv.raw_buffer()->dim[0].extent = width / 2;
   

    input_uv.device_wrap_native(halide_hexagon_dma_device_interface(),
                             reinterpret_cast<uint64_t>(input_memory));

    halide_hexagon_dma_prepare_for_copy_to_host(nullptr, input_uv, dma_engine, false, halide_hexagon_fmt_NV12_UV);

    input_y.device_wrap_native(halide_hexagon_dma_device_interface(),
                             reinterpret_cast<uint64_t>(input_memory));

    halide_hexagon_dma_prepare_for_copy_to_host(nullptr, input_y, dma_engine, false, halide_hexagon_fmt_NV12_Y);

    input_y.set_device_dirty();
    input_uv.set_device_dirty();
    
    Halide::Runtime::Buffer<uint8_t> output(width, (height * 1.5));
    Halide::Runtime::Buffer<uint8_t> output_y = output.cropped(1, 0, height);    // Luma plane only
    Halide::Runtime::Buffer<uint8_t> output_uv = output.cropped(1, height, (height / 2));  // Chroma plane only, with reduced height

    output_uv.embed(2, 0);
    output_uv.raw_buffer()->dimensions = 3;
    output_uv.raw_buffer()->dim[2].extent = 2;
    output_uv.raw_buffer()->dim[2].stride = 1;
    output_uv.raw_buffer()->dim[0].stride = 2;
    output_uv.raw_buffer()->dim[0].extent = width / 2;

    output_y.device_wrap_native(halide_hexagon_dma_device_interface(), reinterpret_cast<uint64_t>(output_memory));
    output_uv.device_wrap_native(halide_hexagon_dma_device_interface(), reinterpret_cast<uint64_t>(output_memory));
    output_y.set_device_dirty();
    output_uv.set_device_dirty();
    halide_hexagon_dma_prepare_for_copy_to_device(nullptr, output_y, dma_engine, false, halide_hexagon_fmt_NV12_Y);
    halide_hexagon_dma_prepare_for_copy_to_device(nullptr, output_uv, dma_engine, false, halide_hexagon_fmt_NV12_UV);

    printf("before pipeline\n");
    long long cycles;

    cycles = benchmark([&]() {
        int error = dmatest_i(input_y, input_uv, output_y, output_uv);
        if (error != 0) {
          printf("pipeline failed! %d\n", error);
        }
     });
    double cycles_per_pixel = (double)cycles/width/h;
    // TestReport tr("dmatest", cycles_per_pixel, "cyc/pix", Mode::Simulator_Standalone);
    // tr.print();

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t correct = input_memory[x + y * width] * 2;
            if (correct != output_memory[x + y * width] ) {
                static int cnt = 0;
                printf("Mismatch at x=%d y=%d : %d != %d\n", x, y, correct, output_memory[x + y * width]);
                if (++cnt > 20) { 
                      fprintf(stdout, "Test_Info: {\n");
                      fprintf(stdout, "\tName:%s\n", "dmatest");
                      fprintf(stdout, "\tResult:%s\n", "Fail");
                      fprintf(stdout, "\tPerf:%f\n", cycles_per_pixel);
                      fprintf(stdout, "\tUnits:%s\n", "cyc/pix");
                      fprintf(stdout, "\tMode:%s\n", "simulator-standalone");
                      fprintf(stdout, "}\n");
                  abort();
                }
            }
        }
    }

    fprintf(stdout, "Test_Info: {\n");
    fprintf(stdout, "\tName:%s\n", "dmatest");
    fprintf(stdout, "\tResult:%s\n", "Pass");
    fprintf(stdout, "\tPerf:%f\n", cycles_per_pixel);
    fprintf(stdout, "\tUnits:%s\n", "cyc/pix");
    fprintf(stdout, "\tMode:%s\n", "simulator-standalone");
    fprintf(stdout, "}\n");

    halide_hexagon_dma_unprepare(nullptr, input_y);
    halide_hexagon_dma_unprepare(nullptr, input_uv);
    halide_hexagon_dma_unprepare(nullptr, output_y);
    halide_hexagon_dma_unprepare(nullptr, output_uv);

    printf("after pipeline\n");

    // We're done with the DMA engine, release it
    halide_hexagon_dma_deallocate_engine(nullptr, dma_engine);

    printf("after deallocation\n");
    qurt_mem_region_delete(region_ddr); 

    printf("Success!\n");
    return 0;
}
