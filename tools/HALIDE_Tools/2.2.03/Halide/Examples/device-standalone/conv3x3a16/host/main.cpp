#ifndef __hexagon__
#include "rpcmem.h"
#include "ion_allocation.h"
#endif
#include "conv3x3a16.h"
#include <stdlib.h>
#include <stdio.h>
#include "buffer.h"
#include <algorithm>
#include "benchmark.h"

using namespace std;

template <typename T>
static T clamp(T val, T min, T max) {
  if (val < min) val = min;
  if (val > max) val = max;
  return val;
}

bool verify(u8_buffer &input, i8_buffer &mask, u8_buffer &output, int w, int h) {
#ifdef VERIFY
  auto u8_in = [&](int x_, int y_) {return input(clamp(x_, 0, w-1), clamp(y_, 0, h-1));};
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      int16_t sum = 0;
      for (int ry = -1; ry <= 1; ry++) {
        for (int rx = -1; rx <= 1; rx++) {
          sum += static_cast<int16_t>(u8_in(x+rx, y+ry))
            * static_cast<int16_t>(mask(rx+1, ry+1));
        }
      }
      sum = sum >> 4;
      if (sum > 255) {
        sum = 255;
      } else if (sum < 0) {
        sum = 0;
      }
      uint8_t u8_sum = static_cast<uint8_t>(sum);
      uint8_t out_val = output(x, y);
      if (out_val != u8_sum) {
        printf ("mismatch at (x = %d, y= %d). Expected: %d, got %d\n", x, y, u8_sum, out_val);
        for (int i = -1; i <= 1; ++i) {
          int16_t s=0;
          for (int j = -1; j <= 1; ++j) {
            s += static_cast<int16_t>(u8_in(x+j, y+i)) * static_cast<int16_t>(mask(j+1, i+1));
            printf ("(%d*%d)\t", u8_in(x+j, y+i), mask(j+1, i+1));
            if (j < 1) printf (" + ");
          }
          printf("s (row %d) = %d\n", i+1, s);
        }
        return false;
      }
    }
  }
#endif
  return true;
}
int main(int argc, char *argv[]) {
#ifdef __hexagon__
  u8_buffer input(host_t::hexagon);
  i8_buffer mask(host_t::hexagon);
  u8_buffer output(host_t::hexagon);
#else
  // Initialize the ion allocator.
  alloc_init();
  u8_buffer input(host_t::arm);
  i8_buffer mask(host_t::arm);
  u8_buffer output(host_t::arm);
#endif

  const int width = atoi(argv[1]);
  const int height = atoi(argv[2]);
  const int vlen = atoi(argv[3]);
  const int iterations = atoi(argv[4]);

  int stride = (width + vlen-1) & ~(vlen-1);

  // Allocate memory
  input.alloc(stride, height);
  input.set();

  mask.alloc(3, 3);
  mask(0, 0) = 1; mask(1, 0) = -4; mask(2, 0) = 7;
  mask(0, 1) = 2; mask(1, 1) = -5; mask(2, 1) = 8;
  mask(0, 2) = 3; mask(1, 2) = -6; mask(2, 2) = 9;

  output.alloc(stride, height);

  int set_perf_mode_turbo = conv3x3a16_set_hvx_perf_mode_turbo();
  if (set_perf_mode_turbo != 0) {
    printf ("Error: Couldn't set perf mode to turbo: %d\n", set_perf_mode_turbo);
  }

  int power_on = conv3x3a16_power_on_hvx();
  if (power_on != 0) {
    printf("Error: Couldn't power on hvx: %d\n", power_on);
    abort();
  }

  printf("Running pipeline... \n");
  double time = benchmark(iterations, 10, [&]() {
      int result = conv3x3a16_run(input.get_buffer(), input.len(), mask.get_buffer(), mask.len(),
                                  stride, height, width, vlen, output.get_buffer(), output.len());
      if (result) {
        printf ("Error: HVX pipeline failed with error %d\n", result);
        exit(1);
      }
    });
  printf("Done, time (conv3x3a16): %g s (%d byte mode)\n", time, vlen);
  conv3x3a16_power_off_hvx();

  if (!verify(input, mask, output, width, height)) {
    printf("Failed\n");
    exit(1);
  }
  input.free_buff();
  mask.free_buff();
  output.free_buff();
  printf ("Success\n");
}
