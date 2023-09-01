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
#include <jni.h>
#include <string.h>

#include "Demosaic.h"


using namespace Halide::Runtime;
using namespace Halide::Tools;

#ifdef HALIDE_NO_PNG
#define IMG_EIN ".pgm"
#define IMG_EXT ".ppm"
#else
#define IMG_EIN ".png"
#define IMG_EXT ".png"
#endif


extern "C"
JNIEXPORT jdouble JNICALL
Java_com_hexagon_camerapipe_CameraPipe_processImageOnHVX(
        JNIEnv* env,
        jobject this_obj/* this */, jstring jinput, jstring jcolor_temp, jstring jgamma, jstring jcontract, jstring jtiming_iteration, jstring joutput, jstring CpuOrHVX) {
        jboolean bcp;
        const char* input_name = env->GetStringUTFChars(jinput, &bcp ) ;
        const char* scolor_temp = env->GetStringUTFChars(jcolor_temp, &bcp ) ;
        const char* sgamma = env->GetStringUTFChars(jgamma, &bcp ) ;
        const char* scontract = env->GetStringUTFChars(jcontract, &bcp ) ;
        const char* stiming_iteration = env->GetStringUTFChars(jtiming_iteration, &bcp ) ;
        const char* output_name = env->GetStringUTFChars(joutput, &bcp ) ;
        const char* cpuhvx_name = env->GetStringUTFChars(CpuOrHVX, &bcp ) ;

#ifdef HL_MEMINFO
    halide_enable_malloc_trace();
#endif

    LOGD("input: %s\n", input_name);
    Buffer<uint16_t> input = load_image(input_name);
    LOGD("       %d %d\n", input.width(), input.height());
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

    float color_temp = atof(scolor_temp);
    float gamma = atof(sgamma);
    float contrast = atof(scontract);
    int timing_iterations = atoi(stiming_iteration);
    int blackLevel = 25;
    int whiteLevel = 1023;

    double best, best_copy;
    LOGD("Parameter details: color_temp = %f gamma = %f contrast = %f timing_iterations = %d blackLevel = %d whiteLevel =%d\n", color_temp, gamma, contrast, timing_iterations, blackLevel, whiteLevel);
#ifdef HL_HEXAGON_DEVICE
    best = benchmark(1, 1, [&]() {
        input.copy_to_device(halide_hexagon_device_interface());
        output.copy_to_device(halide_hexagon_device_interface());
        matrix_3200.copy_to_device(halide_hexagon_device_interface());
        matrix_7000.copy_to_device(halide_hexagon_device_interface());
      });
    LOGD( "Copy to device %gus\n", best * 1e6);

    char *pow = getenv("HALIDE_HEXAGON_POWER");
    // To avoid the cost of powering HVX on in each call of the
    // pipeline, power it on once now.
    if (!pow) {
        // Set turbo power mode if not otherwise specified
        LOGD( "halide_hexagon_set_performance_mode(NULL, halide_hvx_power_turbo)\n");
        halide_hexagon_set_performance_mode(NULL, halide_hvx_power_turbo);
    } else if (strcmp(pow, "low") == 0) {
        LOGD( "halide_hexagon_set_performance_mode(NULL, halide_hvx_power_low)\n");
        halide_hexagon_set_performance_mode(NULL, halide_hvx_power_low);
    } else if (strcmp(pow, "nominal") == 0) {
        LOGD( "halide_hexagon_set_performance_mode(NULL, halide_hvx_power_nominal)\n");
        halide_hexagon_set_performance_mode(NULL, halide_hvx_power_nominal);
    } else if (strcmp(pow, "turbo") == 0) {
        LOGD( "halide_hexagon_set_performance_mode(NULL, halide_hvx_power_turbo)\n");
        halide_hexagon_set_performance_mode(NULL, halide_hvx_power_turbo);
    } else if (strcmp(pow, "default") == 0) {
        LOGD( "using default power mode\n");
    } else if (strcmp(pow, "perf") == 0) {
        // Power on with specified performance parameters
        // (see HalideRuntimeHexagonHost.h for details)
        LOGD( "halide_hexagon_power_hvx_on_perf(NULL, &perf)\n");
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
        LOGD( "Error: Unknown power mode: %s\n", pow);
        LOGD( "       Available modes: low, nominal, turbo, default, perf\n");
        exit(1);
    }
    halide_hexagon_power_hvx_on(NULL);
#endif

    int (*pipeline) (struct halide_buffer_t *_input_buffer, struct halide_buffer_t *_matrix_3200_buffer, struct halide_buffer_t *_matrix_7000_buffer, float _color_temp, float _gamma, float _contrast, int32_t _blackLevel, int32_t _whiteLevel, struct halide_buffer_t *_curved_buffer);

    if (strcmp(cpuhvx_name , "cpu") == 0) {
        pipeline = camera_pipe_cpu;
    } else {
        pipeline = camera_pipe;
    }
    best = benchmark(timing_iterations, 1, [&]() {
        pipeline(input, matrix_3200, matrix_7000,
                    color_temp, gamma, contrast, blackLevel, whiteLevel,
                    output);
    }, true);

    LOGD( "Halide:\t%gus\n", best * 1e6);

#ifdef HL_HEXAGON_DEVICE
    // We're done with HVX, power it off.
    halide_hexagon_power_hvx_off(NULL);

    best_copy = benchmark(1, 1, [&]() {
        output.copy_to_host();
      });
    LOGD( "Copy to host %gus\n", best_copy * 1e6);

#endif

    LOGD( "output: %s\n", output_name);
    save_image(output, output_name);
    //LOGD( "Size of output %ul\n", output.size_in_bytes());
    LOGD( "        %d %d\n", output.width(), output.height());

    return best * 1e6;
}

