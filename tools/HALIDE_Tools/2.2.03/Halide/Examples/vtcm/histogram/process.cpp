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
int checker(Buffer<DTYPE> &in,
            Buffer<HTYPE> &out) {
    int errcnt = 0, maxerr = 10;
    printf("Checking...\n");

    HTYPE hist[HSIZE];
    // Compute reference histogram
    for (int x = 0; x < HSIZE; x++) {
        hist[x] = 0;
    }
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            int idx = std::min(std::max((int)in(x, y), 0), HSIZE-1);
            hist[idx]++;
        }
    }
    for (int x = 0; x < HSIZE; x++) {
        if (out(x) != hist[x]) {
            errcnt++;
            if (errcnt <= maxerr) {
                printf("Mismatch at (%5d): %4d (Halide) == %5d (Expected)\n",
                        x, out(x), hist[x]);
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
    Buffer<HTYPE> out(nullptr, HSIZE);

    in.device_malloc(halide_hexagon_device_interface());
    out.device_malloc(halide_hexagon_device_interface());

    srand(0);
    // Fill the input image
    in.for_each_value([&](DTYPE &x) {
        x = static_cast<DTYPE>(rand() % HSIZE);
    });

    // To avoid the cost of powering HVX on in each call of the
    // pipeline, power it on once now. Also, set Hexagon performance to turbo.
    halide_hexagon_set_performance_mode(NULL, halide_hexagon_power_turbo);
    halide_hexagon_power_hvx_on(NULL);

    printf("Running pipeline...\n");
    printf("Image size: %dx%d pixels\n", W, H);
    printf("Image type: %d bits\n", (int) sizeof(DTYPE)*8);
    printf("Histogram size: %d bins\n", HSIZE);
    printf("Histogram type: %d bits\n", (int) sizeof(HTYPE)*8);

    double time = benchmark(iterations, 10, [&]() {
        int result = pipeline(in, out);
        if (result != 0) {
            printf("pipeline failed! %d\n", result);
        }
    });
    out.copy_to_host();

    // We're done with HVX, power it off, and reset the performance mode
    // to default to save power.
    halide_hexagon_power_hvx_off(NULL);
    halide_hexagon_set_performance_mode(NULL, halide_hexagon_power_default);

    if (checker(in, out) != 0) {
        TestReport tr("vtcm/histogram", (W*H)/(1024*1024*time), "MPixels/sec", Mode::Unknown_Mode, Result::Fail);
        tr.print();
        return 1;
    }

    // Everything worked!
    TestReport tr("vtcm/histogram", (W*H)/(1024*1024*time), "MPixels/sec", Mode::Unknown_Mode, Result::Pass);
    tr.print();

    printf("Success!\n");

    return 0;
}
