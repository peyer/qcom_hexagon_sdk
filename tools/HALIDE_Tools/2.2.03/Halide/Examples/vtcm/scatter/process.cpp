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

void swap(Buffer<DTYPE> &buf, int idx1, int idx2) {
    DTYPE tmp = buf(idx1);
    buf(idx1) = buf(idx2);
    buf(idx2) = tmp;
}

// Verify result for the halide pipeline
int checker(Buffer<DTYPE> &x_idx,
            Buffer<DTYPE> &y_idx,
            Buffer<DTYPE> &out) {
    int errcnt = 0, maxerr = 10;
    printf("Checking...\n");

    DTYPE ref[H][W];

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            ref[y][x] = (DTYPE)19;
        }
    }
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            int xCoord = std::min(std::max((int)x_idx(x), 0), W-1);
            int yCoord = std::min(std::max((int)y_idx(y), 0), H-1);
            ref[yCoord][xCoord] = x_idx(x) + x + y_idx(y) + y;
        }
    }
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            if (out(x, y) != ref[y][x]) {
                errcnt++;
                if (errcnt <= maxerr) {
                    printf("Mismatch at (%d, %d): %ld (Halide) == %ld (Expected)\n",
                            x, y, (long)out(x, y), (long)ref[y][x]);
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

    Buffer<DTYPE> x_idx(nullptr, W);
    Buffer<DTYPE> y_idx(nullptr, H);
    Buffer<DTYPE> out(nullptr, W, H);

    x_idx.device_malloc(halide_hexagon_device_interface());
    y_idx.device_malloc(halide_hexagon_device_interface());
    out.device_malloc(halide_hexagon_device_interface());

    srand(0);
    // Fill the input buffers.
    for (int x = 0; x < W; x++) {
        x_idx(x) = (uint8_t) x;
    }
    for (int x = 0; x < H; x++) {
        y_idx(x) = (uint8_t) x;
    }
    // Create a random permutation for x_idx and y_idx by randomly shuffling
    // elements. All indices should be unique for scatters to avoid race
    // conditions.
    for(int i = 0; i < 1000; i++) {
        swap(x_idx, rand() % W, rand() % W);
        swap(y_idx, rand() % H, rand() % H);
    }

    // To avoid the cost of powering HVX on in each call of the
    // pipeline, power it on once now. Also, set Hexagon performance to turbo.
    halide_hexagon_set_performance_mode(NULL, halide_hexagon_power_turbo);
    halide_hexagon_power_hvx_on(NULL);

    printf("Running pipeline...\n");
    printf("Output size: %dx%d pixels\n", W, H);
    printf("Output type: %d bits\n", (int) sizeof(DTYPE)*8);

    double time = benchmark(iterations, 10, [&]() {
        int result = pipeline(x_idx, y_idx, out);
        if (result != 0) {
            printf("Scatter pipeline failed! %d\n", result);
        }
    });
    out.copy_to_host();

    // We're done with HVX, power it off, and reset the performance mode
    // to default to save power.
    halide_hexagon_power_hvx_off(NULL);
    halide_hexagon_set_performance_mode(NULL, halide_hexagon_power_default);

    if (checker(x_idx, y_idx, out) != 0) {
        TestReport tr("vtcm/scatter", (W*H)/(1024*1024*time), "MPixels/sec", Mode::Unknown_Mode, Result::Fail);
        tr.print();
        return 1;
    }

    // Everything worked!
    TestReport tr("vtcm/scatter", (W*H)/(1024*1024*time), "MPixels/sec", Mode::Unknown_Mode, Result::Pass);
    tr.print();

    printf("Success!\n");

    return 0;
}
