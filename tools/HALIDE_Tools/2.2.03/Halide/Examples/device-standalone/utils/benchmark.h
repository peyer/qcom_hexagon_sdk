#ifndef BENCHMARK_H
#define BENCHMARK_H
#ifdef __hexagon__
template <typename F>
double benchmark(int samples, int iterations, F op) {
  /* For now this does nothing more than just call F */
  op();
  return 0;
}

#else
#include <chrono>

template <typename F>
double benchmark(int samples, int iterations, F op) {
    double best = std::numeric_limits<double>::infinity();
    for (int i = 0; i < samples; i++) {
        auto t1 = std::chrono::high_resolution_clock::now();
        for (int j = 0; j < iterations; j++) {
            op();
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        double dt = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1e6;
        if (dt < best) best = dt;
    }
    return best / iterations;
}

#endif
#endif
