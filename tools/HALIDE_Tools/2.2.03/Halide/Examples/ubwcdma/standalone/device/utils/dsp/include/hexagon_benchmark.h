/* Header file used to time a pipeline.
*  This is meant to be used in device-standalone mode. */

#ifndef HEXAGON_BENCHMARK_H
#define HEXAGON_BENCHMARK_H
#include "hexagon_types.h"
#include "HAP_perf.h"

// Returns the average time, in microseconds, taken to run
// op for the given number of iterations.
template<typename F>
uint64_t benchmark(int iterations, F op) {
  uint64_t start_time = HAP_perf_get_time_us();

  for (int i = 0; i < iterations; ++i) {
      op();
  }

  uint64_t end_time = HAP_perf_get_time_us();
  return (uint64_t) ((end_time - start_time) / iterations);
}
#endif


