// Headers
#include "rpcmem.h"
#include "ion_allocation.h"
#include <stdlib.h>
#include <stdio.h>
#include "buffer.h"
#include <algorithm>
#include <inttypes.h>
#include "dma_raw_rw.h"
#include "test_report.h"

using namespace std;
// Verification function
bool verify(buffer_2d<uint8_t> &input, buffer_2d<uint8_t> &output, int w, int h) {
    for (int i = 0; i < w*h ; i++) {
        uint8_t correct = input.get(i) * 2;
        if (correct != output.get(i)) {
            printf("Mismatch at %d : %d != %d\n", i, correct, output.get(i));
            return false;
        }
    }
    return true;
}

int main(int argc, char *argv[]) {

    if (argc != 8){
        printf("usage: %s <width> <height> <stride> <format NV12 : 1 > <is_ubwc> <input_filename> <validate_filename> \n", argv[0]);
        return 1;
    }
    // Initialize the ion allocator.
    // Use Ion Memory for Input and OutPut Frames ( DMA Needs Contigious Memory and Ion provides that)
    alloc_init();
    // Frame Parameters
    int width  = atoi(argv[1]);
    int height = atoi(argv[2]);
    int stride = atoi(argv[3]);
    int format = atoi(argv[4]);
    int is_ubwc = atoi(argv[5]);
    char *input_filename = argv[6];
    char *validate_filename = argv[7];
    // Set HVX to Turbo mode for Peak Performance
    int set_perf_mode_turbo = dma_raw_rw_set_hvx_perf_mode_turbo();
    if (set_perf_mode_turbo != 0) {
        printf ("Error: Couldn't set perf mode to turbo: %d\n", set_perf_mode_turbo);
    }
    // Power on HVX
    int power_on = dma_raw_rw_power_on_hvx();
    if (power_on != 0) {
        printf("Error: Couldn't power on hvx: %d\n", power_on);
        abort();
    }

    int h = height*4;
    printf("width and height = %d, %d \n",width, h );
    buffer_2d<uint8_t> input(width,h);            // Input frame ION Allocation
    buffer_2d<uint8_t> input_validate(width,h);   // Validation Buffer Ion Memory Allocation
    buffer_2d<uint8_t> output(width,h);           // Output Frame Ion Allocation

    uint8_t* input_memory = input.get_buffer();                 // Pixel Buffer of Input Frame
    uint8_t* memory_to_validate = input_validate.get_buffer();  // Pixel Buffer of Validation Frame

    int fp;
    if((fp = open(input_filename, O_RDONLY)) < 0 )
    {
        printf("Error: Cannot open %s for input\n", input_filename);
        return 1;
    }
    // Read the Input Frame to Input Frame Buffer
    for(int i = 0; i < h; i++)
    {
        if(read(fp, &input_memory[i*width],  sizeof(unsigned char)*width)!=width)
        {
            printf("Error, Unable to read from %s\n", input_filename);
            close(fp);
            return 1;
        }
    }
    close(fp);
    // Read the Validation frame to Validation Frame Buffer
    if((fp = open(validate_filename, O_RDONLY)) < 0 )
    {
        printf("Error: Cannot open %s for input\n", validate_filename);
        return 1;
    }

    for(int i = 0; i < h; i++)
    {
        if(read(fp, &memory_to_validate[i*width],  sizeof(unsigned char)*width)!=width)
        {
            printf("Error, Unable to read from %s\n", validate_filename);
            close(fp);
            return 1;
        }
    }
    close(fp);

    int iterations = 10;
    printf("Running pipeline... \n");
    if (format == 1)
        printf(" for format NV12");
    else if (format == 4)
        printf(" for format P010");
    else if (format == 0)
        printf(" for format Raw Data");

    printf (" ubwc %d \n", is_ubwc);
    uint64 avg_time = 0;
    // Invoke the Host and Hexagon Interfacing Function to Perform DMA RD + WR
    printf("Running pipeline\n");
    int result = dma_raw_rw_run(input.get_buffer(), input.len(), width, height, stride, format, is_ubwc, output.get_buffer(), output.len(), iterations, &avg_time); 
    if (result) {
        printf ("Error: HVX pipeline failed with error %d\n", result);
        exit(1);
    }
    // Power of HVX as the processing is done
    dma_raw_rw_power_off_hvx();

    if (!verify(input_validate, output, width, h)) {
        TestReport tr("dma_raw_rw", avg_time, "microseconds", Mode::Device_Standalone, Result::Fail);
        tr.print(); 
        printf("Failed\n");
        exit(1);
    } else {
        TestReport tr("dma_raw_rw", avg_time, "microseconds", Mode::Device_Standalone, Result::Pass);
        tr.print();
    } 
    // Free the ION Buffers
    input.free_buff();
    input_validate.free_buff();
    output.free_buff();
    alloc_finalize();
    printf ("Success\n");
    return 0;
}
