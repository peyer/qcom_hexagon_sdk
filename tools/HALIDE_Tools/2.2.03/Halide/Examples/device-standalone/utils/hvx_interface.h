#ifndef HVX_INTERFACE_H
#define HVX_INTERFACE_H
#ifndef SIMULATOR
#include "HAP_power.h"
#endif
enum hvx_perf_mode {
  low = 0,
  nominal = 1,
  turbo = 2
};
typedef enum hvx_perf_mode hvx_perf_mode_t;
#ifdef SIMULATOR
int qurt_hvx_lock(qurt_hvx_mode_t lock_mode) {
  SIM_ACQUIRE_HVX;
  if (lock_mode == 0) {
    SIM_CLEAR_HVX_DOUBLE_MODE;
  } else {
    SIM_SET_HVX_DOUBLE_MODE;
  }
  return 0;
}
int qurt_hvx_unlock() {
    SIM_RELEASE_HVX;
    return 0;
}
int power_on_hvx() {
  return 0;
}
int power_off_hvx() {
  return 0;
}
int set_hvx_perf_mode(hvx_perf_mode_t mode) {
  return 0;
}
int set_hvx_perf_mode_low() {
  return 0;
}
int set_hvx_perf_mode_nominal() {
  return 0;
}
int set_hvx_perf_mode_turbo() {
  return 0;
}
#else

int power_on_hvx() {
  HAP_power_request_t request;
  request.type = HAP_power_set_HVX;
  request.hvx.power_up = TRUE;
  int result = HAP_power_set(NULL, &request);
  return result;
}
int power_off_hvx() {
  HAP_power_request_t request;
  request.type = HAP_power_set_HVX;
  request.hvx.power_up = FALSE;
  int result = HAP_power_set(NULL, &request);
  return result;
}
int set_hvx_perf(int set_mips,
                 unsigned int mipsPerThread,
                 unsigned int mipsTotal,
                 int set_bus_bw,
                 unsigned int bwMegabytesPerSec,
                 unsigned int busbwUsagePercentage,
                 int set_latency,
                 int latency) {
  HAP_power_request_t request;

  request.type = HAP_power_set_apptype;
  request.apptype = HAP_POWER_COMPUTE_CLIENT_CLASS;
  int retval = HAP_power_set(NULL, &request);
  if (0 != retval) {
    FARF(LOW, "HAP_power_set(HAP_power_set_apptype) failed (%d)\n", retval);
    return -1;
  }

  request.type = HAP_power_set_mips_bw;
  request.mips_bw.set_mips        = set_mips;
  request.mips_bw.mipsPerThread   = mipsPerThread;
  request.mips_bw.mipsTotal       = mipsTotal;
  request.mips_bw.set_bus_bw      = set_bus_bw;
  request.mips_bw.bwBytePerSec    = ((uint64_t) bwMegabytesPerSec) << 20;
  request.mips_bw.busbwUsagePercentage = busbwUsagePercentage;
  request.mips_bw.set_latency     = set_latency;
  request.mips_bw.latency         = latency;
  retval = HAP_power_set(NULL, &request);
  if (0 != retval) {
    FARF(LOW, "HAP_power_set(HAP_power_set_mips_bw) failed (%d)\n", retval);
    return -1;
  }
  return 0;
}
int set_hvx_perf_mode(hvx_perf_mode_t mode) {
  int set_mips = 0;
  unsigned int mipsPerThread = 0;
  unsigned int mipsTotal = 0;
  int set_bus_bw = 0;
  uint64_t bwBytePerSec = 0;
  unsigned int bwMegabytesPerSec = 0;
  unsigned int busbwUsagePercentage = 0;
  int set_latency = 0;
  int latency = 0;

  HAP_power_response_t power_info;
  unsigned int max_mips = 0;
  uint64 max_bus_bw = 0;

  power_info.type = HAP_power_get_max_mips;
  int retval = HAP_power_get(NULL, &power_info);
  if (0 != retval) {
    FARF(LOW, "HAP_power_get(HAP_power_get_max_mips) failed (%d)\n", retval);
    return -1;
  }
  max_mips = power_info.max_mips;

  // Make sure max_mips is at least sanity_mips
  const unsigned int sanity_mips = 500;
  if (max_mips < sanity_mips) {
    max_mips = sanity_mips;
  }

  power_info.type = HAP_power_get_max_bus_bw;
  retval = HAP_power_get(NULL, &power_info);
  if (0 != retval) {
    FARF(LOW, "HAP_power_get(HAP_power_get_max_bus_bw) failed (%d)\n", retval);
    return -1;
  }
  max_bus_bw = power_info.max_bus_bw;

  // The above API under-reports the max bus bw. If we use it as
  // reported, performance is bad. Experimentally, this only
  // needs to be ~10x.
  // Make sure max_bus_bw is at least sanity_bw
  const uint64 sanity_bw = 1000000000ULL;
  if (max_bus_bw < sanity_bw) {
    if (max_bus_bw == 0) {
      max_bus_bw = sanity_bw;
    }
    while (max_bus_bw < sanity_bw) {
      max_bus_bw <<= 3;  // Increase value while preserving bits
    }
  }

  set_mips    = TRUE;
  set_bus_bw  = TRUE;
  set_latency = TRUE;

  switch (mode) {
  case low:
    mipsPerThread          = max_mips / 4;
    bwBytePerSec           = max_bus_bw / 2;
    busbwUsagePercentage   = 25;
    latency                = 1000;
    break;
  case nominal:
    mipsPerThread          = (3 * max_mips) / 8;
    bwBytePerSec           = max_bus_bw;
    busbwUsagePercentage   = 50;
    latency                = 100;
    break;
  case turbo:
  default:
    mipsPerThread          = max_mips;
    bwBytePerSec           = max_bus_bw * 4;
    busbwUsagePercentage   = 100;
    latency                = 10;
    break;
  }
  mipsTotal = mipsPerThread * 2;

  bwMegabytesPerSec = bwBytePerSec >> 20;
  return set_hvx_perf(set_mips,
                      mipsPerThread,
                      mipsTotal,
                      set_bus_bw,
                      bwMegabytesPerSec,
                      busbwUsagePercentage,
                      set_latency,
                      latency);
}
int set_hvx_perf_mode_low() {
  return set_hvx_perf_mode(low);
}
int set_hvx_perf_mode_nominal() {
  return set_hvx_perf_mode(nominal);
}
int set_hvx_perf_mode_turbo() {
  return set_hvx_perf_mode(turbo);
}
#endif //SIMULATOR


#endif
