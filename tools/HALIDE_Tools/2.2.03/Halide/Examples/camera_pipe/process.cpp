#include "fcam/Demosaic.h"
#include "fcam/Demosaic_ARM.h"

#include "benchmark.h"
#include "camera_pipe.h"
#include "camera_pipe_cpu.h"
#include "HalideBuffer.h"
#include "halide_image_io.h"
#ifdef HL_MEMINFO
#include "halide_image_info.h"
#include "halide_malloc_trace.h"
#endif

#ifdef HL_HEXAGON_DEVICE
#include "HalideRuntimeHexagonHost.h"
#endif

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cassert>

using namespace Halide::Runtime;
using namespace Halide::Tools;

#ifdef HALIDE_NO_PNG
#define IMG_EIN ".pgm"
#define IMG_EXT ".ppm"
#else
#define IMG_EIN ".png"
#define IMG_EXT ".png"
#endif

int main(int argc, char **argv) {
    if (argc < 7) {
        printf("Usage: ./process raw.png color_temp gamma contrast timing_iterations output.png\n"
               "e.g. ./process raw.png 3200 2 50 5 output.png [fcam_c.png] [fcam_arm.png]");
        return 0;
    }

#ifdef HL_MEMINFO
    halide_enable_malloc_trace();
#endif

    fprintf(stderr, "input: %s\n", argv[1]);
    Buffer<uint16_t> input = load_image(argv[1]);
    fprintf(stderr, "       %d %d\n", input.width(), input.height());
    Buffer<uint8_t> output(((input.width() - 32)/32)*32, ((input.height() - 24)/32)*32, 3);

#ifdef HL_MEMINFO
    info(input, "input");
    stats(input, "input");
    // dump(input, "input");
#endif

    // These color matrices are for the sensor in the Nokia N900 and are
    // taken from the FCam source.
    float _matrix_3200[][4] = {{ 1.6697f, -0.2693f, -0.4004f, -42.4346f},
                                {-0.3576f,  1.0615f,  1.5949f, -37.1158f},
                                {-0.2175f, -1.8751f,  6.9640f, -26.6970f}};

    float _matrix_7000[][4] = {{ 2.2997f, -0.4478f,  0.1706f, -39.0923f},
                                {-0.3826f,  1.5906f, -0.2080f, -25.4311f},
                                {-0.0888f, -0.7344f,  2.2832f, -20.0826f}};
    Buffer<float> matrix_3200(4, 3), matrix_7000(4, 3);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            matrix_3200(j, i) = _matrix_3200[i][j];
            matrix_7000(j, i) = _matrix_7000[i][j];
        }
    }

    float color_temp = atof(argv[2]);
    float gamma = atof(argv[3]);
    float contrast = atof(argv[4]);
    int timing_iterations = atoi(argv[5]);
    int blackLevel = 25;
    int whiteLevel = 1023;

    double best;

#ifdef HL_HEXAGON_DEVICE
    best = benchmark(1, 1, [&]() {
        input.copy_to_device(halide_hexagon_device_interface());
        output.copy_to_device(halide_hexagon_device_interface());
        matrix_3200.copy_to_device(halide_hexagon_device_interface());
        matrix_7000.copy_to_device(halide_hexagon_device_interface());
      });
    fprintf(stderr, "Copy to device %gus\n", best * 1e6);

#ifndef NO_SET_PRIO
    // Increase the default priority of Halide Hexagon threads
    halide_hexagon_set_thread_priority(NULL, 50);
#endif
    char *pow = getenv("HALIDE_HEXAGON_POWER");
    // To avoid the cost of powering HVX on in each call of the
    // pipeline, power it on once now.
    if (!pow) {
        // Set turbo power mode if not otherwise specified
        fprintf(stderr, "halide_hexagon_set_performance_mode(NULL, halide_hvx_power_turbo)\n");
        halide_hexagon_set_performance_mode(NULL, halide_hvx_power_turbo);
    } else if (strcmp(pow, "low") == 0) {
        fprintf(stderr, "halide_hexagon_set_performance_mode(NULL, halide_hvx_power_low)\n");
        halide_hexagon_set_performance_mode(NULL, halide_hvx_power_low);
    } else if (strcmp(pow, "nominal") == 0) {
        fprintf(stderr, "halide_hexagon_set_performance_mode(NULL, halide_hvx_power_nominal)\n");
        halide_hexagon_set_performance_mode(NULL, halide_hvx_power_nominal);
    } else if (strcmp(pow, "turbo") == 0) {
        fprintf(stderr, "halide_hexagon_set_performance_mode(NULL, halide_hvx_power_turbo)\n");
        halide_hexagon_set_performance_mode(NULL, halide_hvx_power_turbo);
    } else if (strcmp(pow, "default") == 0) {
        fprintf(stderr, "using default power mode\n");
    } else if (strcmp(pow, "perf") == 0) {
        // Power on with specified performance parameters
        // (see HalideRuntimeHexagonHost.h for details)
        fprintf(stderr, "halide_hexagon_power_hvx_on_perf(NULL, &perf)\n");
        halide_hvx_power_perf_t perf;
        perf.set_mips               = 1;
        perf.mipsPerThread          = 200;
        perf.mipsTotal              = 400;
        perf.set_bus_bw             = 1;
        perf.bwMegabytesPerSec      = 4000;
        perf.busbwUsagePercentage   = 50;
        perf.set_latency            = 1;
        perf.latency                = 100;
        halide_hexagon_set_performance(NULL, &perf);
    } else {
        fprintf(stderr, "Error: Unknown power mode: %s\n", pow);
        fprintf(stderr, "       Available modes: low, nominal, turbo, default, perf\n");
        exit(1);
    }
    halide_hexagon_power_hvx_on(NULL);
#endif

    int (*pipeline) (struct halide_buffer_t *_input_buffer, struct halide_buffer_t *_matrix_3200_buffer, struct halide_buffer_t *_matrix_7000_buffer, float _color_temp, float _gamma, float _contrast, int32_t _blackLevel, int32_t _whiteLevel, struct halide_buffer_t *_curved_buffer);

    if (strcmp(argv[7], "cpu") == 0) {
        pipeline = camera_pipe_cpu;
    } else {
        pipeline = camera_pipe;
    }
    best = benchmark(timing_iterations, 1, [&]() {
        pipeline(input, matrix_3200, matrix_7000,
                    color_temp, gamma, contrast, blackLevel, whiteLevel,
                    output);
    }, true);
    fprintf(stderr, "Halide:\t%gus\n", best * 1e6);

#ifdef HL_HEXAGON_DEVICE
    // We're done with HVX, power it off.
    halide_hexagon_power_hvx_off(NULL);

    best = benchmark(1, 1, [&]() {
        output.copy_to_host();
      });
    fprintf(stderr, "Copy to host %gus\n", best * 1e6);
#endif

    fprintf(stderr, "output: %s\n", argv[6]);
    save_image(output, argv[6]);
    fprintf(stderr, "        %d %d\n", output.width(), output.height());

    Buffer<uint8_t> output_c(output.width(), output.height(), output.channels());
    best = benchmark(timing_iterations, 1, [&]() {
        FCam::demosaic(input, output_c, color_temp, contrast, true, blackLevel, whiteLevel, gamma);
    });
#if 0
    fprintf(stderr, "C++:\t%gus\n", best * 1e6);
    if (argc > 7) {
        fprintf(stderr, "output_c: %s\n", argv[7]);
        save_image(output_c, argv[7]);
    }
    fprintf(stderr, "        %d %d\n", output_c.width(), output_c.height());

    Buffer<uint8_t> output_asm(output.width(), output.height(), output.channels());
    best = benchmark(timing_iterations, 1, [&]() {
        FCam::demosaic_ARM(input, output_asm, color_temp, contrast, true, blackLevel, whiteLevel, gamma);
    });
    fprintf(stderr, "ASM:\t%gus\n", best * 1e6);
    if (argc > 8) {
        fprintf(stderr, "output_asm: %s\n", argv[8]);
        save_image(output_asm, argv[8]);
    }
    fprintf(stderr, "        %d %d\n", output_asm.width(), output_asm.height());

    // Timings on N900 as of SIGGRAPH 2012 camera ready are (best of 10)
    // Halide: 722ms, FCam: 741ms
#endif
    return 0;
}
