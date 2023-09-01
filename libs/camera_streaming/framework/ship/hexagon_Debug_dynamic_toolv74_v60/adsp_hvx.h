#ifndef _ADSP_HVX_H
#define _ADSP_HVX_H
#include "AEEStdDef.h"
#ifndef __QAIC_HEADER
#define __QAIC_HEADER(ff) ff
#endif //__QAIC_HEADER

#ifndef __QAIC_HEADER_EXPORT
#define __QAIC_HEADER_EXPORT
#endif // __QAIC_HEADER_EXPORT

#ifndef __QAIC_HEADER_ATTRIBUTE
#define __QAIC_HEADER_ATTRIBUTE
#endif // __QAIC_HEADER_ATTRIBUTE

#ifndef __QAIC_IMPL
#define __QAIC_IMPL(ff) ff
#endif //__QAIC_IMPL

#ifndef __QAIC_IMPL_EXPORT
#define __QAIC_IMPL_EXPORT
#endif // __QAIC_IMPL_EXPORT

#ifndef __QAIC_IMPL_ATTRIBUTE
#define __QAIC_IMPL_ATTRIBUTE
#endif // __QAIC_IMPL_ATTRIBUTE
#ifdef __cplusplus
extern "C" {
#endif
enum hvx_event_type_t {
   HVX_EVENT_OPEN,
   HVX_EVENT_STATIC_CONFIG,
   HVX_EVENT_DYNAMIC_CONFIG,
   HVX_EVENT_START,
   HVX_EVENT_RESET,
   HVX_EVENT_STOP,
   HVX_EVENT_CLOSE,
   HVX_EVENT_ENABLE_STATS,
   HVX_EVENT_DISABLE_STATS,
   HVX_EVENT_ENABLE_DUMP,
   HVX_EVENT_DISABLE_DUMP,
   HVX_EVENT_MAX,
   _32BIT_PLACEHOLDER_hvx_event_type_t = 0x7fffffff
};
typedef enum hvx_event_type_t hvx_event_type_t;
enum hvx_vfe_type_t {
   HVX_VFE_NULL,
   HVX_VFE0,
   HVX_VFE1,
   HVX_VFE_BOTH,
   HVX_VFE_MAX,
   _32BIT_PLACEHOLDER_hvx_vfe_type_t = 0x7fffffff
};
typedef enum hvx_vfe_type_t hvx_vfe_type_t;
enum hvx_pixel_format_t {
   HVX_BAYER_RGGB,
   HVX_BAYER_BGGR,
   HVX_BAYER_GRBG,
   HVX_BAYER_GBRG,
   HVX_PIXEL_UYVY,
   HVX_PIXEL_VYUY,
   HVX_PIXEL_YUYV,
   HVX_PIXEL_YVYU,
   HVX_PIXEL_MAX,
   _32BIT_PLACEHOLDER_hvx_pixel_format_t = 0x7fffffff
};
typedef enum hvx_pixel_format_t hvx_pixel_format_t;
enum hvx_vector_mode_t {
   HVX_VECTOR_NULL,
   HVX_VECTOR_32,
   HVX_VECTOR_64,
   HVX_VECTOR_INVALID,
   HVX_VECTOR_128,
   HVX_VECTOR_MAX,
   _32BIT_PLACEHOLDER_hvx_vector_mode_t = 0x7fffffff
};
typedef enum hvx_vector_mode_t hvx_vector_mode_t;
enum hvx_request_buffer_type_t {
   HVX_STATS,
   HVX_FRAME_DUMP,
   HVX_REQUEST_BUFFER_TYPE_MAX,
   _32BIT_PLACEHOLDER_hvx_request_buffer_type_t = 0x7fffffff
};
typedef enum hvx_request_buffer_type_t hvx_request_buffer_type_t;
typedef struct hvx_query_caps_t hvx_query_caps_t;
struct hvx_query_caps_t {
   hvx_vector_mode_t hvx_vector_mode;
   int max_hvx_unit;
   int cx_buf_size;
};
typedef struct hvx_open_t hvx_open_t;
struct hvx_open_t {
   char name[32];
   hvx_vfe_type_t vfe_id;
   int dsp_clock;
   int bus_clock;
   int dsp_latency;
};
typedef struct hvx_static_config_t hvx_static_config_t;
struct hvx_static_config_t {
   hvx_vfe_type_t vfe_id;
   int hvx_unit_no[2];
   hvx_vector_mode_t hvx_vector_mode;
   int width;
   int height;
   int image_overlap;
   int right_image_offset;
   hvx_pixel_format_t pixel_format;
   int bits_per_pixel;
   int rx_lines[2];
   int rx_line_width[2];
   int rx_line_stride[2];
   int tx_lines[2];
   int tx_line_width[2];
   int tx_line_stride[2];
   unsigned int vfe_clk_freq[2];
};
/**
       * Start fatsrpc thread, Called only once for every camera session
       */
__QAIC_HEADER_EXPORT int __QAIC_HEADER(adsp_hvx_query_caps)(char* data, int dataLen) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT int __QAIC_HEADER(adsp_hvx_event)(int handle, hvx_event_type_t type, const char* data, int dataLen) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT int __QAIC_HEADER(adsp_hvx_request_buffers)(int handle, hvx_request_buffer_type_t type, int num_buffers, int buffer_size, uint64* buffer_addrs, int buffer_addrsLen) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT int __QAIC_HEADER(adsp_hvx_send_buffer)(int handle, hvx_request_buffer_type_t type, hvx_vfe_type_t vfe_type, unsigned char buffer_label_0, char* addr_0, int addr_0Len, unsigned char buffer_label_1, char* addr_1, int addr_1Len) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT int __QAIC_HEADER(adsp_hvx_status_dump)(int handle, char* status_buf, int status_bufLen) __QAIC_HEADER_ATTRIBUTE;
#ifdef __cplusplus
}
#endif
#endif //_ADSP_HVX_H
