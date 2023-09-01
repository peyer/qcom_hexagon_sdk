#ifndef SIMULATOR_BENCHMARK_H
#define SIMULATOR_BENCHMARK_H
#include "io.h"
template<typename F>
long long benchmark(F op) {
  RESET_PMU();
  long long start_time = READ_PCYCLES();

  op();

  long long total_cycles = READ_PCYCLES() - start_time;
  DUMP_PMU();
  return total_cycles;
}
#endif
