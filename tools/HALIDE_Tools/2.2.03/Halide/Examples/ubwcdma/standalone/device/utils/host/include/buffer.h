/* Header file for managing memory.
*  This is meant to be used in device-standalone mode. */

#ifndef BUFFER_H
#define BUFFER_H

#include "ion_allocation.h"
#include <string.h>

/* #ifdef _HEXAGON_ */
/* #include <hexagon_standalone.h> */
/* #endif */

template<typename T>
class buffer_2d {
  T *data;
  long dataLen;
  int width;
  int height;

public:
  buffer_2d() : data(0), dataLen(0) {}

  buffer_2d(int width, int height) {
    int size = width * height;
    dataLen = size * sizeof(T) + 128;   // add vector length padding
    data = (T *) alloc_ion(dataLen);
    if (!data) {
      printf("Could not allocate\n");
    }
    this->width = width;
    this->height = height;
  }

  void set_all(T v) {
    int x, y;
    for(y=0; y<height; ++y) {
      int row_offset = y*width;
      for(x=0; x<width; ++x) {
        int offset = row_offset+ x;
        *(data + offset) = v;
      }
    }
  }

  /* void for_eac_element(std::function<void (int, int)> &f) */
  void set_zero() {
    memset(data, 0, dataLen);
  }

  T& operator()(int x, int y) {
    int offset = (y*width) + x;
    T *ptr = data + offset;
    return *ptr;
  }

  T get(int x) {
    if (x < dataLen) {
      return static_cast<T>(*(data + x));
    } else {
      printf("Error: Accessing beyond allocated memory\n");
      exit(1);
    }
  }
  void free_buff() { alloc_ion_free(data); }

  T* get_buffer() { return data; }

  int len() { return (dataLen)/sizeof(T); }

};

#endif
