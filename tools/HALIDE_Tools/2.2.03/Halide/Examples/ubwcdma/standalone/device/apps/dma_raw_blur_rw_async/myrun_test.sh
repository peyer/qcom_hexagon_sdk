#!/bin/bash
export HL_NUM_THREADS=16
adb shell  HL_NUM_THREADS=16 ADSP_LIBRARY_PATH=/data/ /data/main-dma_raw_blur_rw_async.out 512 512 512 0 0 "/data/adb_raw.bin" "/data/adb_raw.bin"
