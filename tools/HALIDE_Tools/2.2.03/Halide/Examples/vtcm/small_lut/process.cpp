#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <stdlib.h>
#include "pipeline.h"
#include "HalideBuffer.h"
#include "test_report.h"
#include "process.h"
#include "halide_benchmark.h"
#include "HalideRuntimeHexagonHost.h"
using namespace Halide::Runtime;
using namespace Halide::Tools;

// Verify result for the halide pipeline
int checker(Buffer<DTYPE> &in, Buffer<DTYPE> &lut, Buffer<DTYPE> &out) {
    int errcnt = 0, maxerr = 10;
    printf("Checking...\n");

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            DTYPE idx = std::min(std::max((int)in(x, y), 0), LUT_SIZE-1);
            DTYPE expected = lut(idx);
            if (out(x, y) != expected) {
                errcnt++;
                if (errcnt <= maxerr) {
                    printf("Mismatch at (%d, %d): %ld (Halide) == %ld (Expected)\n",
                            x, y, (long)out(x, y), (long)expected);
                }
            }
        }
    }
    if (errcnt > maxerr) {
        printf("...\n");
    }
    if (errcnt > 0) {
        printf("Mismatch at %d places\n", errcnt);
    }
    return errcnt;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s (iterations)\n", argv[0]);
        return 0;
    }
    assert(W % 128 == 0);

    int iterations = atoi(argv[1]);

    Buffer<DTYPE>  in(nullptr, W, H);
    Buffer<DTYPE> lut(nullptr, LUT_SIZE);
    Buffer<DTYPE> out(nullptr, W, H);

    in.device_malloc(halide_hexagon_device_interface());
    lut.device_malloc(halide_hexagon_device_interface());
    out.device_malloc(halide_hexagon_device_interface());
    
    srand(0);
    // Fill the input image
    in.for_each_value([&](DTYPE &x) {
        x = static_cast<DTYPE>(rand());
    });
    // Fill the lookup table
    lut.for_each_value([&](DTYPE &x) {
        x = static_cast<DTYPE>(rand());
    });

    // To avoid the cost of powering HVX on in each call of the
    // pipeline, power it on once now. Also, set Hexagon performance to turbo.
    halide_hexagon_set_performance_mode(NULL, halide_hexagon_power_turbo);
    halide_hexagon_power_hvx_on(NULL);

    printf("Running pipeline...\n");
    printf("Image size: %dx%d pixels\n", W, H);
    printf("Image type: %d bits\n", (int) sizeof(DTYPE)*8);
    printf("Table size: %d elements\n", LUT_SIZE);
 
    double time = benchmark(iterations, 1, [&]() {
        int result = pipeline(in, lut, out);
        if (result != 0) {
            printf("pipeline failed! %d\n", result);
        }
    });
    out.copy_to_host();

    // We're done with HVX, power it off, and reset the performance mode
    // to default to save power.
    halide_hexagon_power_hvx_off(NULL);
    halide_hexagon_set_performance_mode(NULL, halide_hexagon_power_default);

    printf ("Time: %lfms\n", time*1000);

    if ((checker(in, lut, out) != 0)) {
        TestReport tr("vtcm/small_lut", (W*H)/(1024*1024*time), "MPixels/sec", Mode::Unknown_Mode, Result::Fail);
        tr.print();
        return 1;
    }

    // Everything worked!
    TestReport tr("vtcm/small_lut", (W*H)/(1024*1024*time), "MPixels/sec", Mode::Unknown_Mode, Result::Pass);
    tr.print();
    printf("Success!\n");
    return 0;
}
