#ifndef _ADSP_HVX_CALLBACK_H
#define _ADSP_HVX_CALLBACK_H
#include "AEEStdDef.h"
#include "adsp_hvx.h"
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
enum hvx_error_type_t {
   HVX_OK,
   HVX_ERROR_BUS_UNDER_RUN,
   HVX_ERROR_BUS_RCV_OVER_RUN,
   HVX_ERROR_BUS_OVERFLOW,
   HVX_ERROR_MEM_REGION_CREATE_FAILURE,
   HVX_ERROR_L2_LINE_LOCK_FAILURE,
   HVX_ERROR_NOT_ENOUGH_STACK_MEM,
   HVX_ERROR_NOT_ENOUGH_L2_MEM,
   HVX_ERROR_THREAD_CREATE_FAILURE,
   HVX_ERROR_FIRMWARE,
   HVX_ERROR_NO_CONTEXT_CONTROLLER,
   HVX_ERROR_ADSP_CLOCK_REQ_FAILURE,
   HVX_ERROR_HVX_POWER_REQ_FAILURE,
   HVX_ERROR_HVX_POWER_REL_FAILURE,
   HVX_ERROR_INPUT_STRIDE_NOT_ALIGNED,
   _32BIT_PLACEHOLDER_hvx_error_type_t = 0x7fffffff
};
typedef enum hvx_error_type_t hvx_error_type_t;
__QAIC_HEADER_EXPORT int __QAIC_HEADER(adsp_hvx_callback_data)(int handle, hvx_vfe_type_t vfe_type, unsigned char buf_label) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT int __QAIC_HEADER(adsp_hvx_callback_error)(int handle, hvx_vfe_type_t vfe_type, int frame_id, hvx_error_type_t error, const char* error_msg, int error_msgLen) __QAIC_HEADER_ATTRIBUTE;
#ifdef __cplusplus
}
#endif
#endif //_ADSP_HVX_CALLBACK_H
