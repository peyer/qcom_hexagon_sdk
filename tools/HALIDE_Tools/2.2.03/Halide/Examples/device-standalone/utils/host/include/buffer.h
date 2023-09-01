#ifndef BUFFER_H
#define BUFFER_H
#ifdef __hexagon__
void *alloc_ion(size_t size) {
  return memalign(128, size * sizeof(uint8_t));
}

void alloc_ion_free(void *ptr) {
  free(ptr);
}
#else
#include "ion_allocation.h"
#endif
#ifdef _HEXAGON_
#include <hexagon_standalone.h>
#endif
#include <string.h>

enum host_t {
  arm = 1,
  hexagon
};
template<typename T>
class buffer {
  T *data;
  long dataLen;
  int width;
  int height;
  enum host_t host;
public:
 buffer(enum host_t host_) : data(0), dataLen(0), host(host_) {}

  void alloc(int width, int height) {
    int size = width * height;
    dataLen = (size*sizeof(T)) + 128;
    if (host == host_t::arm) {
      data = (T *) alloc_ion(dataLen);
    } else {
      data = (T*) memalign(128, dataLen);
    }
    this->width = width;
    this->height = height;
  }

  void set() {
    int x, y;
    for(y=0; y<height; ++y) {
      int row_offset = y*width;
      for(x=0; x<width; ++x) {
        int offset = row_offset+ x;
        *(data + offset) = static_cast<T>(rand());
      }
    }
  }
  /* void for_eac_element(std::function<void (int, int)> &f) */
  void zero() {
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
  void free_buff() {
    if (host == host_t::arm) {
      alloc_ion_free((void *)data);
    } else {
      free(data);
    }
  }
  T* get_buffer() {
    return data;
  }
  int len() { return dataLen; }

};

typedef buffer<unsigned char> u8_buffer;
typedef buffer<char> i8_buffer;
#endif
