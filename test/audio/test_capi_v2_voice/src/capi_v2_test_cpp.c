/*==============================================================================
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//#include "IEC61937.h"

#include <hexagon_protos.h>
#include <hexagon_sim_timer.h>

#include "capi_v2_test.h"
#include "Elite_pcm_ch_defs.h"

#include "capi_v2_library_factory.h"

#include "HAP_farf.h"
#include "HAP_mem.h"
#include "AEEstd.h"


#define NUM_CHANNELS 2

#define STD_MIN(a,b)   ((a)<(b)?(a):(b))
#define TST_STD_MEDIA_FMT_V2_MIN_SIZE   (sizeof(capi_v2_data_format_header_t) + sizeof(capi_v2_standard_data_format_v2_t))
#define ELITE_ALIGN_BY_4(x)             ((((uint32_t)(x) + 3) >> 2) << 2)

// TODO
#define QURT_ELITE_ASSERT(_x) \
  do { \
    if (!(_x)) { \
      return CAPI_V2_EFAILED; \
    } \
  } while(0)

boolean test_script = FALSE;

// TODO should be part of an included header
/* Payload format for stream parameter data. */
typedef struct asm_stream_param_data_v2_t asm_stream_param_data_v2_t;

/** @weakgroup weak_asm_stream_param_data_v2_t
@{ */
/* Payload of the stream parameter data of the
    ASM_STREAM_CMD_SET_PP_PARAMS_V2 command. */
/** Immediately following this structure are param_size bytes of parameter
    data, which must be aligned to 32 bytes. The structure and size depend on
    the module_id/param_id pair.
*/
struct asm_stream_param_data_v2_t
{
    uint32_t                  module_id;
    /**< Valid ID of the module to be configured (see Section
         @xref{hdr:AudioPostProcIDs}). */

    uint32_t                  param_id;
    /**< Valid ID of the parameter to be configured (see Section
         @xref{hdr:AudioPostProcIDs}). */

    uint16_t                  param_size;
    /**< Data size of the param_id/module_id combination. This is a multiple
         of four bytes.

         @values > 0 bytes */

    uint16_t                  reserved;
    /**< This field must be set to zero. */
};

// TODO should be part of an included header
/* Payload format for stream parameter data. */
typedef struct asm_stream_cmd_set_pp_params_v2_t asm_stream_cmd_set_pp_params_v2_t;

/** @weakgroup weak_asm_stream_cmd_set_pp_params_v2_t
@{ */
/* Payload of the ASM_STREAM_CMD_SET_PP_PARAMS_V2 command, which allows
    one or more parameters to be set on a stream. If data_payload_addr=NULL, a
    series of parameter data structures immediately follow, whose
    total size is data_payload_size bytes.
*/
struct asm_stream_cmd_set_pp_params_v2_t
{
    uint32_t                  data_payload_addr_lsw;
    /**< Lower 32 bits of the parameter data payload address.  */

    uint32_t                  data_payload_addr_msw;
    /**< Upper 32 bits of the parameter data payload address.

         The 64-bit number formed by data_payload_addr_lsw and
         data_payload_addr_msw must be set to zero for in-band data. */

    uint32_t                  mem_map_handle;
    /**< Unique identifier for an address. This memory map handle is returned
         by the aDSP through the #ASM_CMD_SHARED_MEM_MAP_REGIONS command.

         @values
         - NULL -- Parameter data payloads are within the message payload
           (in-band).
         - Non-NULL -- Parameter data payloads begin at the address specified
           in the data_payload_addr_lsw and data_payload_addr_msw fields
           (out-of-band).

         If the address is sent (non-NULL), the
         @xhyperref{hdr:AsmParamDataVariablePayload,Parameter data variable
         payload} begins at the specified data payload address. */

    uint32_t                  data_payload_size;
    /**< Actual size of the variable payload accompanying the message, or in
         shared memory. This field is used for parsing the parameter payload.

         @values > 0 bytes */
};

// TODO should be part of an included header
/* Payload format for stream parameter data. */
typedef struct asm_stream_cmd_get_pp_params_v2_t asm_stream_cmd_get_pp_params_v2_t;

/** @weakgroup weak_asm_stream_cmd_get_pp_params_v2_t
@{ */
/* Payload of the ASM_STREAM_CMD_GET_PP_PARAMS_V2 command, which allows
    a query for one pre/postprocessing parameter of a stream.
*/
struct asm_stream_cmd_get_pp_params_v2_t
{
    uint32_t                  data_payload_addr_lsw;
    /**< Lower 32 bits of the parameter data payload address. */

    uint32_t                  data_payload_addr_msw;
    /**< Upper 32 bits of the parameter data payload address.

         The size of the shared memory, if specified, must be large enough to
         contain the entire parameter data payload. For details, see the
         @xhyperref{hdr:AsmParamDataVariablePayload,Parameter data variable
         payload}.

         The 64-bit number formed by data_payload_addr_lsw and
         data_payload_addr_msw must be set to zero for in-band data.

         @subhd{For a 32-bit shared memory address} This field must be set
         to zero.

         @subhd{For a 36-bit shared memory address} Bits 31 to 4 of this
         field must be set to zero. */

    uint32_t                  mem_map_handle;
    /**< Unique identifier for an address.

         This memory map handle is returned by the aDSP through the
         #ASM_CMD_SHARED_MEM_MAP_REGIONS command and is used only for
         out-of-band messages. For in-band messages, this field must be set to
         zero.

         @values
         - NULL -- Parameter data payloads in the acknowledgment are within
           the message payload (in-band).
         - Non-NULL -- Parameter data payloads  in the acknowledgment begin at
           the address specified in the data_payload_addr_lsw and
           data_payload_addr_msw fields (out-of-band).

         If the address is sent (non-NULL), the
         @xhyperref{hdr:AsmParamDataVariablePayload,Parameter data variable
         payload} begins at the specified address. */

    uint32_t                  module_id;
    /**< Valid ID of the module to be configured (see Section
         @xref{hdr:AudioPostProcIDs}). */

    uint32_t                  param_id;
    /**< Valid ID of the parameter to be configured (see Section
         @xref{hdr:AudioPostProcIDs}). */

    uint16_t                  param_max_size;
    /**< Maximum data size of the module_id/param_id combination. This is a
         multiple of four bytes.

         @values > 0 bytes */

    uint16_t                  reserved;
    /**< Clients must set this field to zero. */
};



// This indicates the iteration in the processing
// at which steady state may have started
static const uint32_t begin_steady_state_iteration = 3;

typedef struct tst_standard_media_format tst_standard_media_format;
struct tst_standard_media_format {
  capi_v2_data_format_header_t h;
  capi_v2_standard_data_format_t f;
};

typedef struct tst_standard_media_format_v2 tst_standard_media_format_v2;
struct tst_standard_media_format_v2
{
   capi_v2_data_format_header_t h;
   capi_v2_standard_data_format_v2_t f;
   capi_v2_channel_type_t channel_type[CAPI_V2_MAX_CHANNELS_V2];
};

capi_v2_port_num_info_t num_port_info;
uint32_t frame_size_ms;
uint32_t num_channels_ref=0;
uint16_t channel_type_ref[CAPI_V2_MAX_CHANNELS];

static inline uint32_t bits_to_bytes(uint32_t bits_per_sample)
{
  return (bits_per_sample >> 3);
}


/*===========================================================================
    FUNCTION : copy_media_fmt_v2_to_v1
    DESCRIPTION: Function to copy v2 version of media format to v1 structure.
===========================================================================*/
static void copy_media_fmt_v2_to_v1(tst_standard_media_format *media_fmt_v1_dst, const tst_standard_media_format_v2 *const media_fmt_v2_src)
{

	media_fmt_v1_dst->h = media_fmt_v2_src->h;
	media_fmt_v1_dst->f.bitstream_format = media_fmt_v2_src->f.bitstream_format;
	media_fmt_v1_dst->f.num_channels = media_fmt_v2_src->f.num_channels;
	media_fmt_v1_dst->f.bits_per_sample = media_fmt_v2_src->f.bits_per_sample;
	media_fmt_v1_dst->f.q_factor = media_fmt_v2_src->f.q_factor;
	media_fmt_v1_dst->f.sampling_rate = media_fmt_v2_src->f.sampling_rate;
	media_fmt_v1_dst->f.data_is_signed = media_fmt_v2_src->f.data_is_signed;
	media_fmt_v1_dst->f.data_interleaving = media_fmt_v2_src->f.data_interleaving;

	std_memscpy(media_fmt_v1_dst->f.channel_type, sizeof(media_fmt_v1_dst->f.channel_type), \
		media_fmt_v2_src->channel_type, media_fmt_v2_src->f.num_channels * sizeof(media_fmt_v2_src->channel_type[0]));

	return;

}

/*===========================================================================
    FUNCTION : copy_media_fmt_v1_to_v2
    DESCRIPTION: Function to copy v2 version of media format to another v2 structure.
===========================================================================*/
static void copy_media_fmt_v1_to_v2( tst_standard_media_format_v2* media_fmt_v2_dst, const tst_standard_media_format *const media_fmt_v1_src)
{
  media_fmt_v2_dst->h = media_fmt_v1_src->h;
  media_fmt_v2_dst->f.minor_version = CAPI_V2_MEDIA_FORMAT_MINOR_VERSION;
  media_fmt_v2_dst->f.bits_per_sample = media_fmt_v1_src->f.bits_per_sample;
  media_fmt_v2_dst->f.bitstream_format = media_fmt_v1_src->f.bitstream_format;
  media_fmt_v2_dst->f.data_interleaving = media_fmt_v1_src->f.data_interleaving;
  media_fmt_v2_dst->f.data_is_signed = media_fmt_v1_src->f.data_is_signed;
  media_fmt_v2_dst->f.num_channels = media_fmt_v1_src->f.num_channels;
  media_fmt_v2_dst->f.q_factor = media_fmt_v1_src->f.q_factor;
  media_fmt_v2_dst->f.sampling_rate = media_fmt_v1_src->f.sampling_rate;

  std_memscpy(media_fmt_v2_dst->channel_type, sizeof(media_fmt_v2_dst->channel_type) ,
             media_fmt_v1_src->f.channel_type, media_fmt_v1_src->f.num_channels * sizeof(media_fmt_v1_src->f.channel_type[0]));

  return;
}


static bool_t IsCompressedData(const capi_v2_buf_t* format_ptr)
{
  QURT_ELITE_ASSERT(format_ptr->max_data_len >= sizeof(capi_v2_data_format_header_t));
  capi_v2_data_format_header_t* header = (capi_v2_data_format_header_t*)format_ptr->data_ptr;
  bool_t result = FALSE;

  switch(header->data_format) {
    case CAPI_V2_FIXED_POINT:
    case CAPI_V2_FLOATING_POINT:
    {
      result = FALSE;
      break;
    }
    case CAPI_V2_RAW_COMPRESSED:
    case CAPI_V2_IEC61937_PACKETIZED:
    {
      result = TRUE;
      break;
    }
    case CAPI_V2_MAX_FORMAT_TYPE:
    {
      QURT_ELITE_ASSERT(0);
      result = FALSE;
      break;
    }
    default:
    {
	break;
    }
  }
  return result;
}

uint32_t calculateNumBuffers(const capi_v2_buf_t* format)
{
  if (IsCompressedData(format)) {
    return 1;
  }

   QURT_ELITE_ASSERT(format->actual_data_len >= sizeof(tst_standard_media_format_v2));

   tst_standard_media_format_v2* tst_format = (tst_standard_media_format_v2*)format->data_ptr;

  if (CAPI_V2_DEINTERLEAVED_UNPACKED == tst_format->f.data_interleaving) {
    return tst_format->f.num_channels;
  } else {
    return 1;
  }
}


static capi_v2_err_t update_output_media_format(module_info_t* module, const capi_v2_buf_t* out_format, const capi_v2_event_id_t id)
{
  switch(id)
  {
  case CAPI_V2_EVENT_OUTPUT_MEDIA_FORMAT_UPDATED:
  {
   QURT_ELITE_ASSERT(out_format->actual_data_len <= sizeof(module->out_format_buf));

   tst_standard_media_format *out_format_buf_v1 = (tst_standard_media_format *) out_format->data_ptr;

   uint32_t channel_map_size = out_format_buf_v1->f.num_channels * sizeof(out_format_buf_v1->f.channel_type[0]);
   channel_map_size = ELITE_ALIGN_BY_4(channel_map_size);

  // Adjust output buffer sizes for new media formats
   module->out_format.actual_data_len = TST_STD_MEDIA_FMT_V2_MIN_SIZE + channel_map_size;
   module->out_format.max_data_len = sizeof(module->out_format_buf);
   module->out_format.data_ptr = module->out_format_buf;

   copy_media_fmt_v1_to_v2((tst_standard_media_format_v2 *)module->out_format.data_ptr, out_format_buf_v1);

   QURT_ELITE_ASSERT(out_format->actual_data_len >= sizeof(capi_v2_data_format_header_t));

   if (IsCompressedData(out_format)) {
     module->out_buffer_len = module->in_buffer_len;
   } else {
     QURT_ELITE_ASSERT(out_format->actual_data_len >= sizeof(tst_standard_media_format));
     tst_standard_media_format* format = (tst_standard_media_format*)out_format->data_ptr;

     module->out_buffer_len = (format->f.sampling_rate * frame_size_ms / 1000) * bits_to_bytes(format->f.bits_per_sample);
     FARF(HIGH, "CAPI V2 TEST: In function update_output_media_format: frame_size_ms=%d", frame_size_ms);
   }

	break;

	}
	case CAPI_V2_EVENT_OUTPUT_MEDIA_FORMAT_UPDATED_V2:
        {
	QURT_ELITE_ASSERT(out_format->actual_data_len <= sizeof(module->out_format_buf));

	// Adjust output buffer sizes for new media formats
	module->out_format.actual_data_len = out_format->actual_data_len;
	module->out_format.max_data_len = sizeof(module->out_format_buf);
	module->out_format.data_ptr = module->out_format_buf;
	std_memscpy(module->out_format.data_ptr, module->out_format.max_data_len, out_format->data_ptr, out_format->actual_data_len);

	QURT_ELITE_ASSERT(out_format->actual_data_len >= sizeof(capi_v2_data_format_header_t));

	if (IsCompressedData(out_format))
	{
		module->out_buffer_len = module->in_buffer_len;
	}
	else
	{
		QURT_ELITE_ASSERT(out_format->actual_data_len >= TST_STD_MEDIA_FMT_V2_MIN_SIZE);
		tst_standard_media_format_v2 *format = (tst_standard_media_format_v2*)(out_format->data_ptr);

		module->out_buffer_len = (format->f.sampling_rate * frame_size_ms / 1000) * bits_to_bytes(format->f.bits_per_sample);
		FARF(HIGH, "CAPI V2 TEST: In function update_output_media_format: frame_size_ms=%d", frame_size_ms);
	}
	break;

	}
	default:
	{
		FARF(HIGH, "CAPI V2 TEST: Error in updating output media format. Unsupported event id : %d", (uint32_t)(id));
		return CAPI_V2_EFAILED;

	}
	}

  return CAPI_V2_EOK;
}

capi_v2_err_t FillInputBuffer(capi_v2_stream_data_t* input_bufs_ptr, module_info_t* module, void* temp_in_buffer)
{
  FILE* in_file = module->finp;
  uint32_t num_bytes_to_read = input_bufs_ptr->bufs_num * input_bufs_ptr->buf_ptr[0].max_data_len;
  uint32_t num_bytes_read = fread(temp_in_buffer, sizeof(int8_t), num_bytes_to_read, in_file);
  uint32_t ch = 0 , j = 0;

  //FARF(HIGH, "CAPI V2 TEST: num_bytes_read=%d, num_bytes_to_read=%d", num_bytes_read, num_bytes_to_read);
  if (num_bytes_read < num_bytes_to_read) {
    FARF(ERROR, "CAPI V2 TEST: Input file seems to be complete");
  }

  // ------------------------
  // Deinterleave data if needed
  // ------------------------
  if (IsCompressedData(&module->in_format)) {
    memcpy(input_bufs_ptr->buf_ptr[0].data_ptr, temp_in_buffer, num_bytes_read);
    input_bufs_ptr->buf_ptr[0].actual_data_len = num_bytes_read;
  } else {
		QURT_ELITE_ASSERT(module->in_format.actual_data_len >= sizeof(tst_standard_media_format_v2));
		tst_standard_media_format_v2* format = (tst_standard_media_format_v2*)module->in_format.data_ptr;

    switch(format->f.data_interleaving) {
      case CAPI_V2_DEINTERLEAVED_PACKED:
      {
        const uint32_t num_channels = format->f.num_channels;
        const uint32_t bits_per_sample = format->f.bits_per_sample;
        capi_v2_buf_t* buf_ptr = &(input_bufs_ptr->buf_ptr[0]);
        if (bits_per_sample == 16) {
          int16_t* temp_in_buffer_16 = (int16_t*)temp_in_buffer;
          uint32_t num_samples_per_channel = num_bytes_read / sizeof(int16_t) / num_channels;
          int16_t* data_ptr = (int16_t*)(buf_ptr->data_ptr);

          for (ch = 0; ch < num_channels; ch++) {
            for (j = 0; j < num_samples_per_channel; j++) {
              data_ptr[ch * num_samples_per_channel + j] = temp_in_buffer_16[j * num_channels + ch];
            }
            buf_ptr->actual_data_len = num_samples_per_channel * sizeof(int16_t);
          }
        } else if (bits_per_sample == 32) {
          int32_t* temp_in_buffer_32 = (int32_t*)temp_in_buffer;
          uint32_t num_samples_per_channel = num_bytes_read / sizeof(int32_t) / num_channels;
          int32_t* data_ptr = (int32_t*)(buf_ptr->data_ptr);

          for (ch = 0; ch < num_channels; ch++) {

            for (j = 0; j < num_samples_per_channel; j++) {
              data_ptr[ch * num_samples_per_channel + j] = temp_in_buffer_32[j * num_channels + ch];
            }
            buf_ptr->actual_data_len = num_samples_per_channel * sizeof(int32_t);
          }
        } else {
          FARF(ERROR, "CAPI V2 TEST: bit-width not supported");
        }
        break;
      }
      case CAPI_V2_DEINTERLEAVED_UNPACKED:
      {
        const uint32_t num_channels = format->f.num_channels;
        const uint32_t bits_per_sample = format->f.bits_per_sample;
        if (bits_per_sample == 16) {
          int16_t* temp_in_buffer_16 = (int16_t*)temp_in_buffer;
          uint32_t num_samples_per_channel = num_bytes_read / sizeof(int16_t) / num_channels;

          for (ch = 0; ch < num_channels; ch++) {
            capi_v2_buf_t* buf_ptr = &(input_bufs_ptr->buf_ptr[ch]);
            int16_t* data_ptr = (int16_t*)(buf_ptr->data_ptr);

            for (j = 0; j < num_samples_per_channel; j++) {
              data_ptr[j] = temp_in_buffer_16[j * num_channels + ch];
            }
            buf_ptr->actual_data_len = num_samples_per_channel * sizeof(int16_t);
          }
        } else if (bits_per_sample == 32) {
          int32_t* temp_in_buffer_32 = (int32_t*)temp_in_buffer;
          uint32_t num_samples_per_channel = num_bytes_read / sizeof(int32_t) / num_channels;

          for (ch = 0; ch < num_channels; ch++) {
            capi_v2_buf_t* buf_ptr = &(input_bufs_ptr->buf_ptr[ch]);
            int32_t* data_ptr = (int32_t*)(buf_ptr->data_ptr);

            for (j = 0; j < num_samples_per_channel; j++) {
              data_ptr[j] = temp_in_buffer_32[j * num_channels + ch];
            }
            buf_ptr->actual_data_len = num_samples_per_channel * sizeof(int32_t);
          }
        } else {
          FARF(ERROR, "CAPI V2 TEST: bit-width not supported");

        }
        break;
      }
      case CAPI_V2_INTERLEAVED:
      case CAPI_V2_INVALID_INTERLEAVING:
      {
        memcpy(input_bufs_ptr->buf_ptr[0].data_ptr, temp_in_buffer, num_bytes_read);
        input_bufs_ptr->buf_ptr[0].actual_data_len = num_bytes_read;
        break;
      }
      default:
        // TODO ignore?
        break;
    }
  }
  return CAPI_V2_EOK;
}


static capi_v2_err_t FillInputBufferRef(capi_v2_stream_data_t* input_bufs_ptr, module_info_t* module, void* temp_in_buffer)
{
  FILE* in_file = module->finpref;
  uint32_t num_bytes_to_read = input_bufs_ptr->bufs_num * input_bufs_ptr->buf_ptr[0].max_data_len;
  uint32_t num_bytes_read = fread(temp_in_buffer, sizeof(int8_t), num_bytes_to_read, in_file);
  uint32_t ch = 0 , j = 0;

  //FARF(HIGH, "CAPI V2 TEST: num_bytes_read=%d, num_bytes_to_read=%d", num_bytes_read, num_bytes_to_read);
  if (num_bytes_read < num_bytes_to_read) {
    FARF(ERROR, "CAPI V2 TEST: Input file seems to be complete");
  }

  // ------------------------
  // Deinterleave data if needed
  // ------------------------
  if (IsCompressedData(&module->in_format)) {
    memcpy(input_bufs_ptr->buf_ptr[0].data_ptr, temp_in_buffer, num_bytes_read);
    input_bufs_ptr->buf_ptr[0].actual_data_len = num_bytes_read;
  } else {
		QURT_ELITE_ASSERT(module->in_format.actual_data_len >= sizeof(tst_standard_media_format_v2));
		tst_standard_media_format_v2* format = (tst_standard_media_format_v2*)module->in_format.data_ptr;

    switch(format->f.data_interleaving) {
      case CAPI_V2_DEINTERLEAVED_PACKED:
      {
        const uint32_t num_channels = format->f.num_channels;
        const uint32_t bits_per_sample = format->f.bits_per_sample;
        capi_v2_buf_t* buf_ptr = &(input_bufs_ptr->buf_ptr[0]);
        if (bits_per_sample == 16) {
          int16_t* temp_in_buffer_16 = (int16_t*)temp_in_buffer;
          uint32_t num_samples_per_channel = num_bytes_read / sizeof(int16_t) / num_channels;
          int16_t* data_ptr = (int16_t*)(buf_ptr->data_ptr);

          for (ch = 0; ch < num_channels; ch++) {
            for (j = 0; j < num_samples_per_channel; j++) {
              data_ptr[ch * num_samples_per_channel + j] = temp_in_buffer_16[j * num_channels + ch];
            }
            buf_ptr->actual_data_len = num_samples_per_channel * sizeof(int16_t);
          }
        } else if (bits_per_sample == 32) {
          int32_t* temp_in_buffer_32 = (int32_t*)temp_in_buffer;
          uint32_t num_samples_per_channel = num_bytes_read / sizeof(int32_t) / num_channels;
          int32_t* data_ptr = (int32_t*)(buf_ptr->data_ptr);

          for (ch = 0; ch < num_channels; ch++) {

            for (j = 0; j < num_samples_per_channel; j++) {
              data_ptr[ch * num_samples_per_channel + j] = temp_in_buffer_32[j * num_channels + ch];
            }
            buf_ptr->actual_data_len = num_samples_per_channel * sizeof(int32_t);
          }
        } else {
          FARF(ERROR, "CAPI V2 TEST: bit-width not supported");
        }
        break;
      }
      case CAPI_V2_DEINTERLEAVED_UNPACKED:
      {
        const uint32_t num_channels = format->f.num_channels;
        const uint32_t bits_per_sample = format->f.bits_per_sample;
        if (bits_per_sample == 16) {
          int16_t* temp_in_buffer_16 = (int16_t*)temp_in_buffer;
          uint32_t num_samples_per_channel = num_bytes_read / sizeof(int16_t) / num_channels;

          for (ch = 0; ch < num_channels; ch++) {
            capi_v2_buf_t* buf_ptr = &(input_bufs_ptr->buf_ptr[ch]);
            int16_t* data_ptr = (int16_t*)(buf_ptr->data_ptr);

            for (j = 0; j < num_samples_per_channel; j++) {
              data_ptr[j] = temp_in_buffer_16[j * num_channels + ch];
            }
            buf_ptr->actual_data_len = num_samples_per_channel * sizeof(int16_t);
          }
        } else if (bits_per_sample == 32) {
          int32_t* temp_in_buffer_32 = (int32_t*)temp_in_buffer;
          uint32_t num_samples_per_channel = num_bytes_read / sizeof(int32_t) / num_channels;

          for (ch = 0; ch < num_channels; ch++) {
            capi_v2_buf_t* buf_ptr = &(input_bufs_ptr->buf_ptr[ch]);
            int32_t* data_ptr = (int32_t*)(buf_ptr->data_ptr);

            for (j = 0; j < num_samples_per_channel; j++) {
              data_ptr[j] = temp_in_buffer_32[j * num_channels + ch];
            }
            buf_ptr->actual_data_len = num_samples_per_channel * sizeof(int32_t);
          }
        } else {
          FARF(ERROR, "CAPI V2 TEST: bit-width not supported");

        }
        break;
      }
      case CAPI_V2_INTERLEAVED:
      case CAPI_V2_INVALID_INTERLEAVING:
      {
        memcpy(input_bufs_ptr->buf_ptr[0].data_ptr, temp_in_buffer, num_bytes_read);
        input_bufs_ptr->buf_ptr[0].actual_data_len = num_bytes_read;
        break;
      }
      default:
        // TODO ignore?
        break;
    }
  }
  return CAPI_V2_EOK;
}


capi_v2_err_t CheckInputOutputSizes(capi_v2_stream_data_t* input[],
                                 uint32_t input_bytes_given[],
                                 capi_v2_stream_data_t* output[],
                                 bool_t requires_data_buffering,
                                 uint32_t num_input_ports,
                                 uint32_t num_output_ports)
{
  uint32_t i = 0 , j = 0;
  if (requires_data_buffering) {
    // Check if any input is empty
    for (i = 0; i < num_input_ports; i++) {
      bool_t is_empty = TRUE;
      for (j = 0; j < input[i]->bufs_num; j++) {
        if (input[i]->buf_ptr[j].actual_data_len != input_bytes_given[i]) {
          is_empty = FALSE;
          break;
        }
      }

      if (is_empty) {
        return CAPI_V2_EOK;
      }
    }

    // If none of the inputs are empty, at least one output must be full
    for (i = 0; i < num_output_ports; i++) {
      bool_t is_full = TRUE;
      for (j = 0; j < output[i]->bufs_num; j++) {
        if (output[i]->buf_ptr[j].actual_data_len != output[i]->buf_ptr[j].max_data_len) {
          is_full = FALSE;
          break;
        }
      }

      if (is_full) {
        return CAPI_V2_EOK;
      }
    }

    FARF(ERROR, "CAPI V2 TEST: Process did not empty any input or fill any output completely.");
    return CAPI_V2_EFAILED;
  } else {
    // All inputs should be empty
    for (i = 0; i < num_input_ports; i++) {
      bool_t is_empty = TRUE;
      for (j = 0; j < input[i]->bufs_num; j++) {
        if (input[i]->buf_ptr[j].actual_data_len != input_bytes_given[i]) {
          is_empty = FALSE;
          break;
        }
      }

      if (!is_empty) {
        FARF(ERROR, "CAPI V2 TEST: Process did not empty input %lu", i);
        return CAPI_V2_EFAILED;
      }
    }

    // All outputs must be filled with the same amount of data as the input
    for (i = 0; i < num_output_ports; i++) {
      bool_t is_full = TRUE;
      for (j = 0; j < output[i]->bufs_num; j++) {
        if (output[i]->buf_ptr[j].actual_data_len != input_bytes_given[i]) {
          is_full = FALSE;
          break;
        }
      }

      if (!is_full) {
        FARF(ERROR, "CAPI V2 TEST: Process did not fill output %lu", i);
        return CAPI_V2_EFAILED;
      }
    }

    return CAPI_V2_EOK;
  }
}

capi_v2_err_t DumpOutputToFile(capi_v2_stream_data_t* output_bufs_ptr, module_info_t* module, void* temp_out_buffer)
{
  FILE* out_file = module->fout;
  uint32_t num_bytes_to_write = output_bufs_ptr->bufs_num * output_bufs_ptr->buf_ptr[0].actual_data_len;
  uint32_t ch = 0, j = 0;

  int8_t* out_buf = NULL;
  // ------------------------
  // Interleave data if not already so.
  // ------------------------
  if (IsCompressedData(&module->out_format)) {
    out_buf = (int8_t*)output_bufs_ptr->buf_ptr[0].data_ptr;
  } else {
	QURT_ELITE_ASSERT(module->in_format.actual_data_len >= sizeof(tst_standard_media_format_v2));

	tst_standard_media_format_v2* format = (tst_standard_media_format_v2*)module->out_format.data_ptr;

    switch(format->f.data_interleaving) {
      case CAPI_V2_DEINTERLEAVED_PACKED:
      {
        const uint32_t num_channels = format->f.num_channels;
        const uint32_t bits_per_sample = format->f.bits_per_sample;
        out_buf = (int8_t*)temp_out_buffer;

        capi_v2_buf_t* buf_ptr = &(output_bufs_ptr->buf_ptr[0]);
        if (bits_per_sample == 16) {
          int16_t* data_ptr = (int16_t*)(buf_ptr->data_ptr);
          int16_t* temp_out_buffer_16 = (int16_t*)temp_out_buffer;
          uint32_t num_samples_per_channel = buf_ptr->actual_data_len / sizeof(int16_t);
          for (ch = 0; ch < num_channels; ch++) {
            for (j = 0; j < num_samples_per_channel; j++) {
              temp_out_buffer_16[j * num_channels + ch] = data_ptr[ch * num_samples_per_channel + j];
            }
            buf_ptr->actual_data_len = 0;
          }
        } else if (bits_per_sample == 32) {
          int32_t* temp_out_buffer_32 = (int32_t*)temp_out_buffer;
          int32_t* data_ptr = (int32_t*)(buf_ptr->data_ptr);
          uint32_t num_samples_per_channel = buf_ptr->actual_data_len / sizeof(int32_t);
          for (ch = 0; ch < num_channels; ch++) {
            for (j = 0; j < num_samples_per_channel; j++) {
              temp_out_buffer_32[j * num_channels + ch] = data_ptr[ch * num_samples_per_channel + j];
            }
            buf_ptr->actual_data_len = 0;
          }
        } else {
          FARF(ERROR, "CAPI V2 TEST: bit-width not supported");
          return CAPI_V2_EUNSUPPORTED;
        }
        break;
      }
      case CAPI_V2_DEINTERLEAVED_UNPACKED:
      {
        const uint32_t num_channels = format->f.num_channels;
        const uint32_t bits_per_sample = format->f.bits_per_sample;
        out_buf = (int8_t*)temp_out_buffer;

        if (bits_per_sample == 16) {
          int16_t* temp_out_buffer_16 = (int16_t*)temp_out_buffer;
          for (ch = 0; ch < num_channels; ch++) {
            capi_v2_buf_t* buf_ptr = &(output_bufs_ptr->buf_ptr[ch]);
            int16_t* data_ptr = (int16_t*)(buf_ptr->data_ptr);
            uint32_t num_samples_per_channel = buf_ptr->actual_data_len / sizeof(int16_t);

            for (j = 0; j < num_samples_per_channel; j++) {
              temp_out_buffer_16[j * num_channels + ch] = data_ptr[j];
            }
            buf_ptr->actual_data_len = 0;
          }
        } else if (bits_per_sample == 32) {
          int32_t* temp_out_buffer_32 = (int32_t*)temp_out_buffer;
          for (ch = 0; ch < num_channels; ch++) {
            capi_v2_buf_t* buf_ptr = &(output_bufs_ptr->buf_ptr[ch]);
            int32_t* data_ptr = (int32_t*)(buf_ptr->data_ptr);
            uint32_t num_samples_per_channel = buf_ptr->actual_data_len / sizeof(int32_t);

            for (j = 0; j < num_samples_per_channel; j++) {
              temp_out_buffer_32[j * num_channels + ch] = data_ptr[j];
            }
            buf_ptr->actual_data_len = 0;
          }
        } else {
          FARF(ERROR, "CAPI V2 TEST: bit-width not supported");
          return CAPI_V2_EUNSUPPORTED;
        }
        break;
      }
      case CAPI_V2_INTERLEAVED:
      case CAPI_V2_INVALID_INTERLEAVING:
        out_buf = (int8_t*)output_bufs_ptr->buf_ptr[0].data_ptr;
        break;
    default:
      // TODO ignore?
      break;
    }
  }

  // ------------------------
  // Write output
  // ------------------------
  uint32_t num_bytes_written = fwrite(out_buf,
                                      sizeof(int8_t),
                                      num_bytes_to_write,
                                      out_file);
///  FARF (HIGH, "CAPI V2 TEST: num_bytes_written = %d, num_bytes_to_write = %d", num_bytes_written, num_bytes_to_write);
  if (num_bytes_written != num_bytes_to_write) {
    FARF(ERROR, "CAPI V2 TEST: Failed to write output");
    return CAPI_V2_EFAILED;
  }

  return CAPI_V2_EOK;
}

// TODO GetWord, .... ReadChannelMapping should all be in a library common to
// both appi and capiv2 tests

/*------------------------------------------------------------------------
  Function name: GetWord
  Description- Gets one word from config file
 * -----------------------------------------------------------------------*/
capi_v2_err_t GetWord(FILE* fCfg, char word[])
{
  if (feof(fCfg)) {
    FARF(HIGH, "CAPI V2 TEST: End of file reached                                   ");
    return CAPI_V2_EFAILED;
  }

  char c;

  do {
    c = fgetc(fCfg);
    if ('#' == c) {
      // Go to next line
      char tmp[1024];
      fgets(tmp, 1024, fCfg);
    }
  }
  while ((isspace(c) || ('#' == c)) && !feof(fCfg));

  if (feof(fCfg)) {
    FARF(HIGH, "CAPI V2 TEST: End of file reached                                   ");
    return CAPI_V2_EFAILED;
  }

  // Read the word now
  fseek(fCfg, -1L, SEEK_CUR); // Since we read one character into c.
  fscanf(fCfg, "%s", word);

  return CAPI_V2_EOK;
}

/*------------------------------------------------------------------------
  Function name: GetUIntParameter
  Description- Gets value associated with parameter
 * -----------------------------------------------------------------------*/
capi_v2_err_t GetUIntParameter(FILE* fCfg, const char* param_name, uint32_t* pValues)
{
  int32_t value;

  GetIntParameter(fCfg, param_name, &value);
  if (value < 0) {
    value = 0;
  }

  *pValues = (uint32_t)value;

  return CAPI_V2_EOK;
}

/*------------------------------------------------------------------------
  Function name: GetIntParameter
  Description- Gets value associated with parameter
 * -----------------------------------------------------------------------*/
capi_v2_err_t GetIntParameter(FILE* fCfg, const char* param_name, int32_t* pValue)
{
  capi_v2_err_t result;
  char word[WORDSIZE];

  if (!test_script) {
    result = GetWord(fCfg, word);
    if (CAPI_V2_EOK != result) {
      return result;
    }

    if (0 != strncmp(word, param_name, WORDSIZE)) {
      FARF(ERROR, "CAPI V2 TEST: Did not find the %s parameter", param_name);
      return CAPI_V2_EFAILED;
    }
  }

  result = GetWord(fCfg, word);
  if (CAPI_V2_EOK != result) {
    FARF(ERROR, "CAPI V2 TEST: Failed to read %s parameter value number",
         param_name);
    return result;
  }

  int32_t value;
  value = strtoul(word, NULL, 0);

  *pValue = value;

  return CAPI_V2_EOK;
}

capi_v2_err_t ReadBufferContents(FILE* fCfg, const uint32_t payloadSize, uint8_t* pPayload)
{
  uint32_t i;
  int index = 0;

  // Read the payload contents
  for (i = 0; i < payloadSize;) {
    capi_v2_err_t result;
    char word[WORDSIZE];
    result = GetWord(fCfg, word);
    if (CAPI_V2_EOK != result) {
      FARF(ERROR, "CAPI V2 TEST: Failed to read payload contents");
      return result;
    }

    int32_t value = strtoul(word, NULL, 16);
    if (!test_script) {
      pPayload[i++] = (uint8_t)(value & 0xFF);
    } else {
      for (index = 0; index < (int)sizeof(uint32_t); index++) {
        if (i >= payloadSize) {
          break;
        }
        pPayload[i++] = (uint8_t)((value & (0x000000FF << (index * (2 * sizeof(uint32_t))))) >> (index * (2 * sizeof(uint32_t))));
      }
    }
  }

  return CAPI_V2_EOK;
}

capi_v2_err_t ReadChannelMapping(FILE *fCfg, const uint32_t num_channels, uint16_t channel_mapping[])
{
   uint32_t i;

   //FARF(HIGH, "CAPI V2 TEST: num_channels=%d", num_channels);
   for (i = 0; i < num_channels; i++)
   {
      capi_v2_err_t result;
      char word[WORDSIZE];
      result = GetWord(fCfg, word);
      if (CAPI_V2_EOK != result)
      {
         FARF(ERROR, "CAPI V2 TEST: Failed to read channel mapping.");
         return result;
      }

      int value;
      sscanf(word, "%x", &value);
      //FARF(HIGH, "CAPI V2 TEST: Channel Type: %x", value);

      channel_mapping[i] = (uint16_t) (value & 0xFFFF);
   }

   return CAPI_V2_EOK;
}

capi_v2_err_t Inputfile(module_info_t* module)
{

  capi_v2_err_t result = CAPI_V2_EOK;
  char filename_input[250] = { 0 };

  GetWord(module->fCfg, filename_input);
  FARF(HIGH, "Inputfile: %s                                                       ", filename_input);
  FARF(HIGH, "--------------------------------------------------------------------");
  if (filename_input[0] != NULL) {
    if ((module->finp = fopen(filename_input, "rb")) == NULL) {
      FARF(ERROR, "Cannot open input file                                          ");
      return CAPI_V2_EFAILED;
    }
  }

  ADD_PROFILE_ATTR(SESSION_PROCESS, "Input", filename_input);
  test_script = TRUE;

  return result;
}

capi_v2_err_t Outputfile(module_info_t* module)
{

  capi_v2_err_t result = CAPI_V2_EOK;
  char filename_output[250] = { 0 };

  GetWord(module->fCfg, filename_output);

  FARF(HIGH, "Outputfile: %s                                                      ", filename_output);
  FARF(HIGH, "--------------------------------------------------------------------");

  if (filename_output[0] != NULL) {
    if ((module->fout = fopen(filename_output, "wb")) == NULL) {
      FARF(ERROR, "Cannot open output file                             ");
      return CAPI_V2_EFAILED;
    }
  }

  return result;
}

capi_v2_err_t ProcessDataWithOutRefPort(module_info_t* module)
{
  uint32_t ch = 0;
  FARF(HIGH, "CAPI V2 TEST: Executing Process Data command.");


  capi_v2_err_t result = CAPI_V2_EOK;
  uint32_t i;

  uint32_t numBuffers = 0;

  // ------------------------
  // Profiling attributes
  // ------------------------
#if ((defined __hexagon__) || (defined __qdsp6__))
  // Note floats being used only for profiling information
  uint64_t prof_cycles = 0;
  #if FARF_HIGH == 1
  uint64_t prof_max_iter = 0;
  #endif
  uint64_t prof_total_cycles = 0;
  uint64_t prof_num_samples = 0;
  float prof_peak_mips = 0.0;
  float prof_mips = 0.0;
  char buffer[20] = { 0 };
#endif // __qdsp6___

  // ------------------------
  // Buffer pointers
  // ------------------------

  int8_t* input_buffer = NULL;
  int8_t* temp_in_buffer = NULL;
  int8_t* output_buffer = NULL;
  int8_t* temp_out_buffer = NULL;

  // ------------------------
  // Buffer setup
  // ------------------------
  capi_v2_stream_data_t input_str_data[1], output_str_data[1];
  capi_v2_stream_data_t* input[] = { &input_str_data[0] }, *output[] = { &output_str_data[0] };
	capi_v2_buf_t in[CAPI_V2_MAX_CHANNELS_V2], out[CAPI_V2_MAX_CHANNELS_V2];
  input_str_data[0].buf_ptr = &in[0];
  output_str_data[0].buf_ptr = &out[0];
  input_str_data[0].bufs_num = calculateNumBuffers(&module->in_format);
  output_str_data[0].bufs_num = calculateNumBuffers(&module->out_format);

  uint32_t in_buf_size = module->in_buffer_len * input_str_data[0].bufs_num;
  uint32_t out_buf_size = module->out_buffer_len * output_str_data[0].bufs_num;

  // ------------------------
  // Get number of buffers to
  // process from config file
  // ------------------------
  result = GetUIntParameter(module->fCfg, "NumBuffers", &(numBuffers));
  if (CAPI_V2_EOK != result) {
    FARF(ERROR, "CAPI V2 TEST: Process Buffers command failed to read NumBuffers.");
    return CAPI_V2_EFAILED;
  }

  // Allocate input buffer

  HAP_malloc(in_buf_size, (void**)&input_buffer);
  if (NULL == input_buffer) {
    FARF(ERROR, "CAPI V2 TEST: Process Buffers command Memory allocation error for input buffer");
    result = CAPI_V2_ENOMEMORY;
    goto done;
  }
  HAP_malloc(in_buf_size, (void**)&temp_in_buffer);
  if (NULL == temp_in_buffer) {
    FARF(ERROR, "CAPI V2 TEST: Process Buffers command Memory allocation error for temp input buffer");
    result = CAPI_V2_ENOMEMORY;
    goto done;
  }
  if (out_buf_size) {
    HAP_malloc(out_buf_size, (void**)&output_buffer);
    if (NULL == output_buffer) {
      FARF(ERROR, "CAPI V2 TEST: Process Buffers command Memory allocation error for output_str_data buffer");
      result = CAPI_V2_ENOMEMORY;
      goto done;
    }
    HAP_malloc(out_buf_size, (void**)&temp_out_buffer);
    if (NULL == temp_out_buffer) {
      FARF(ERROR, "CAPI V2 TEST: Process Buffers command Memory allocation error for temp output_str_data buffer");
      result = CAPI_V2_ENOMEMORY;
      goto done;
    }
  }

  // ------------------------
  // Set input and output_str_data buffers
  // ------------------------

  {
    int8_t* ptr = NULL;
    uint32_t increment = 0;
    uint32_t ch = 0;

    ptr = input_buffer;
    increment = 0;

    for (ch = 0; ch < input_str_data[0].bufs_num; ch++) {
      in[ch].data_ptr = ptr + increment;
      in[ch].actual_data_len = 0;
      in[ch].max_data_len = module->in_buffer_len;
      increment += module->in_buffer_len;
    }

    if (output_buffer) {
      ptr = output_buffer;
      increment = 0;
      for (ch = 0; ch < output_str_data[0].bufs_num; ch++) {
        out[ch].data_ptr = ptr + increment;
        out[ch].actual_data_len = 0;
        out[ch].max_data_len = module->out_buffer_len;
        increment += module->out_buffer_len;
      }
    }
  }


  // ------------------------
  // Process buffers
  // ------------------------

  for (i = 0; i < numBuffers; i++) {
    // Fill input buffer
    // while (input buffer not consumed)
    // {
    //    process()
    //    dumpOutput()
    // }

    // ------------------------
    // Read input
    // ------------------------
    if (CAPI_V2_FAILED(result = FillInputBuffer(input_str_data, module, temp_in_buffer))) {
      FARF(ERROR, "CAPI V2 TEST: Process failed with error %d.", result);
      break;
    }
    bool_t is_file_over = (input_str_data[0].buf_ptr[0].actual_data_len < input_str_data[0].buf_ptr[0].max_data_len);
    if (is_file_over) {
      break;
    }

    while (input_str_data[0].buf_ptr[0].actual_data_len > 0) {
      result = CAPI_V2_EOK;
      uint32_t input_bytes_given[] = { input_str_data[0].buf_ptr[0].actual_data_len };

      // ------------------------
      // Call Processing function
      // ------------------------
      if (module->is_enabled) {
		  // ------------------------
          // Begin profiling
          // ------------------------
    	  START_PROCESS_PROFILE();
    	  prof_cycles = GET_PROFILE_CYCLES(SESSION_PROCESS);
          result = module->module_ptr->vtbl_ptr->process(module->module_ptr, input, output);

		  // ------------------------
          // Complete profiling
          // ------------------------
          // Diff Current Cycle State against previously acquired to
          // check Cycles elapsed
          if (prof_cycles > 0) {
        	  uint64_t curr_cycle = GET_PROFILE_CYCLES(SESSION_PROCESS);
              prof_cycles = curr_cycle - prof_cycles;
           } else {
        	   prof_cycles = GET_PROFILE_CYCLES(SESSION_PROCESS);
           }
		   // Number of samples produced per channel
           STOP_PROCESS_PROFILE();
      } else {
    	if (module->fout) {
          for (ch = 0; ch < input_str_data[0].bufs_num; ch++) {
            memcpy(output_str_data[0].buf_ptr[ch].data_ptr, input_str_data[0].buf_ptr[ch].data_ptr, input_str_data[0].buf_ptr[ch].actual_data_len);
            output_str_data[0].buf_ptr[ch].actual_data_len = input_str_data[0].buf_ptr[ch].actual_data_len;
          }
          result = CAPI_V2_EOK;
    	}
      }

      if ( module->fout != NULL &&  !IsCompressedData(&module->out_format)) {
				QURT_ELITE_ASSERT(module->out_format.actual_data_len >= TST_STD_MEDIA_FMT_V2_MIN_SIZE);
				tst_standard_media_format_v2* tst_format = (tst_standard_media_format_v2*)module->out_format.data_ptr;
        uint32_t SampleCnt = (output_str_data[0].buf_ptr[0].actual_data_len * output[0]->bufs_num / (bits_to_bytes(tst_format->f.bits_per_sample) * tst_format->f.num_channels));
        // Begin recording after a few iterations, for steady state
        if (i > begin_steady_state_iteration) {
          prof_num_samples += SampleCnt;
          prof_total_cycles += prof_cycles;
          if (SampleCnt > 0) {
            prof_mips = ((float)(((0.000001 * tst_format->f.sampling_rate))) / SampleCnt) * prof_cycles;
            if (prof_mips > prof_peak_mips) {
              prof_peak_mips = prof_mips;
              #if FARF_HIGH == 1
              prof_max_iter = i;
              #endif
            }
          }
        }
      }

      // Now check if the process function went through fine.
      if (CAPI_V2_FAILED(result)) {
        FARF(ERROR, "CAPI V2 TEST: Process failed with error %d.", result);
        break;
      }

      if ( module->fout != NULL) {
        result = CheckInputOutputSizes(input,
                                     input_bytes_given,
                                     output,
                                     module->requires_data_buffering,
                                     1lu,
                                     1lu);

        if (CAPI_V2_FAILED(result)) {
          break;
        }

        result = DumpOutputToFile(output[0], module, temp_out_buffer);
        if (CAPI_V2_FAILED(result)) {
          break;
        }
      }

      // Adjust the input buffers if they are not completely consumed.
      for (ch = 0; ch < input_str_data[0].bufs_num; ch++) {
        capi_v2_buf_t* buf_ptr = &(input_str_data[0].buf_ptr[ch]);
        uint32_t num_bytes_consumed = buf_ptr->actual_data_len;
        memmove(buf_ptr->data_ptr, buf_ptr->data_ptr + num_bytes_consumed, input_bytes_given[0] - num_bytes_consumed);
        buf_ptr->actual_data_len = input_bytes_given[0] - num_bytes_consumed;
      }
    }
    if (CAPI_V2_FAILED(result)) {
      break;
    }
  }

  // ------------------------
  // Display profiling information
  // ------------------------
#if ((defined __hexagon__) || (defined __qdsp6__))
  if (!IsCompressedData(&module->out_format)) {
    sprintf(buffer, "%d", (int)numBuffers);
    ADD_PROFILE_ATTR(SESSION_PROCESS, "Buffers", buffer);
    PRINT_PROFILE_RESULT(SESSION_PROCESS);
    RESET_PROFILE_RESULT(SESSION_PROCESS);
    ///QURT_ELITE_ASSERT(module->out_format.actual_data_len >= sizeof(tst_standard_media_format));

#if FARF_HIGH == 1
		tst_standard_media_format_v2* tst_format;
    if (module->fout != NULL) {
			tst_format = (tst_standard_media_format_v2*)module->out_format.data_ptr;
    } else {
			tst_format = (tst_standard_media_format_v2*)module->in_format.data_ptr;
    }
    #endif
    FARF(HIGH, "-------------------------------");
    FARF(HIGH, "CAPI V2 TEST: Profiling information");
    FARF(HIGH, "CAPI V2 TEST: Average MIPS: %.3f", ((float)(0.000001 * tst_format->f.sampling_rate) / prof_num_samples) * prof_total_cycles);
    FARF(HIGH, "CAPI V2 TEST: Peak MIPS: %.3f occurred at iteration# %lld", prof_peak_mips, prof_max_iter);
    FARF(HIGH, "-------------------------------");
  } else {
    FARF(HIGH, "-------------------------------");
    FARF(HIGH, "CAPI V2 TEST: Profiling information");
    FARF(HIGH, "CAPI V2 TEST: Output format is compressed. Profiling not supported.");
    FARF(HIGH, "-------------------------------");
  }
#endif // __qdsp6__

  // ------------------------
  // Clear processing buffers
  // ------------------------
done:
  if (NULL != input_buffer) {
    HAP_free(input_buffer);
    input_buffer = NULL;
  }
  if (NULL != temp_in_buffer) {
    HAP_free(temp_in_buffer);
    temp_in_buffer = NULL;
  }
  if (NULL != output_buffer) {
    HAP_free(output_buffer);
    output_buffer = NULL;
  }
  if (NULL != temp_out_buffer) {
    HAP_free(temp_out_buffer);
    temp_out_buffer = NULL;
  }

  return result;
}


capi_v2_err_t ProcessDataWithRefPort(module_info_t* module)
{
  uint32_t ch = 0;
  FARF(HIGH, "CAPI V2 TEST: Executing Process Data command.");


  capi_v2_err_t result = CAPI_V2_EOK;
  uint32_t i;

  uint32_t numBuffers = 0;

  // ------------------------
  // Profiling attributes
  // ------------------------
#if ((defined __hexagon__) || (defined __qdsp6__))
  // Note floats being used only for profiling information
  uint64_t prof_cycles = 0;
  #if FARF_HIGH == 1
  uint64_t prof_max_iter = 0;
  #endif
  uint64_t prof_total_cycles = 0;
  uint64_t prof_num_samples = 0;
  float prof_peak_mips = 0.0;
  float prof_mips = 0.0;
  char buffer[20] = { 0 };
#endif // __qdsp6___

  // ------------------------
  // Buffer pointers
  // ------------------------

  int8_t* input_buffer = NULL;
  int8_t* inputref_buffer = NULL;
  int8_t* temp_in_buffer = NULL;
  int8_t* temp_inref_buffer = NULL;
  int8_t* output_buffer = NULL;
  int8_t* temp_out_buffer = NULL;

  // ------------------------
  // Buffer setup
  // ------------------------
  capi_v2_stream_data_t input_str_data[2], output_str_data[1];
  capi_v2_stream_data_t* input[] = { &input_str_data[0], &input_str_data[1] }, *output[] = { &output_str_data[0] };
  capi_v2_buf_t in[CAPI_V2_MAX_CHANNELS], inref[CAPI_V2_MAX_CHANNELS], out[CAPI_V2_MAX_CHANNELS];
  input_str_data[0].buf_ptr = &in[0];
  input_str_data[1].buf_ptr = &inref[0];
  output_str_data[0].buf_ptr = &out[0];
  input_str_data[0].bufs_num = calculateNumBuffers(&module->in_format);
  input_str_data[1].bufs_num = calculateNumBuffers(&module->in_format);
  output_str_data[0].bufs_num = calculateNumBuffers(&module->out_format);

  uint32_t in_buf_size = module->in_buffer_len * input_str_data[0].bufs_num;
  uint32_t inref_buf_size = module->in_buffer_len * input_str_data[1].bufs_num;
  uint32_t out_buf_size = module->out_buffer_len * output_str_data[0].bufs_num;


  // ------------------------
  // Get number of buffers to
  // process from config file
  // ------------------------
  result = GetUIntParameter(module->fCfg, "NumBuffers", &(numBuffers));
  if (CAPI_V2_EOK != result) {
    FARF(ERROR, "CAPI V2 TEST: Process Buffers command failed to read NumBuffers.");
    return CAPI_V2_EFAILED;
  }

  // Allocate input buffer

  HAP_malloc(in_buf_size, (void**)&input_buffer);
  if (NULL == input_buffer) {
    FARF(ERROR, "CAPI V2 TEST: Process Buffers command Memory allocation error for input buffer");
    result = CAPI_V2_ENOMEMORY;
    goto done;
  }
  HAP_malloc(in_buf_size, (void**)&temp_in_buffer);
  if (NULL == temp_in_buffer) {
    FARF(ERROR, "CAPI V2 TEST: Process Buffers command Memory allocation error for temp input buffer");
    result = CAPI_V2_ENOMEMORY;
    goto done;
  }
  HAP_malloc(inref_buf_size, (void**)&inputref_buffer);
  if (NULL == inputref_buffer) {
    FARF(ERROR, "CAPI V2 TEST: Process Buffers command Memory allocation error for input ref buffer");
    result = CAPI_V2_ENOMEMORY;
    goto done;
  }
  HAP_malloc(inref_buf_size, (void**)&temp_inref_buffer);
  if (NULL == temp_inref_buffer) {
    FARF(ERROR, "CAPI V2 TEST: Process Buffers command Memory allocation error for temp input ref buffer");
    result = CAPI_V2_ENOMEMORY;
    goto done;
  }

  HAP_malloc(out_buf_size, (void**)&output_buffer);
  if (NULL == output_buffer) {
    FARF(ERROR, "CAPI V2 TEST: Process Buffers command Memory allocation error for output_str_data buffer");
    result = CAPI_V2_ENOMEMORY;
    goto done;
  }
  HAP_malloc(out_buf_size, (void**)&temp_out_buffer);
  if (NULL == temp_out_buffer) {
    FARF(ERROR, "CAPI V2 TEST: Process Buffers command Memory allocation error for temp output_str_data buffer");
    result = CAPI_V2_ENOMEMORY;
    goto done;
  }

  // ------------------------
  // Set input and output_str_data buffers
  // ------------------------

  {
    int8_t* ptr = NULL;
    uint32_t increment = 0;
    uint32_t ch = 0;

    ptr = input_buffer;
    increment = 0;
    for (ch = 0; ch < input_str_data[0].bufs_num; ch++) {
      in[ch].data_ptr = ptr + increment;
      in[ch].actual_data_len = 0;
      in[ch].max_data_len = module->in_buffer_len;
      increment += module->in_buffer_len;
    }

    ptr = inputref_buffer;
    increment = 0;
    for (ch = 0; ch < input_str_data[1].bufs_num; ch++) {
      inref[ch].data_ptr = ptr + increment;
      inref[ch].actual_data_len = 0;
      inref[ch].max_data_len = module->in_buffer_len;
      increment += module->in_buffer_len;
    }

    ptr = output_buffer;
    increment = 0;
    for (ch = 0; ch < output_str_data[0].bufs_num; ch++) {
      out[ch].data_ptr = ptr + increment;
      out[ch].actual_data_len = 0;
      out[ch].max_data_len = module->out_buffer_len;
      increment += module->out_buffer_len;
    }
  }


  // ------------------------
  // Process buffers
  // ------------------------

  for (i = 0; i < numBuffers; i++) {
	// Fill input buffer
    // while (input buffer not consumed)
    // {
    //    process()
    //    dumpOutput()
    // }

    // ------------------------
    // Read input
    // ------------------------
    if (CAPI_V2_FAILED(result = FillInputBuffer(input_str_data, module, temp_in_buffer))) {
      FARF(ERROR, "CAPI V2 TEST: Process failed with error %d.", result);
      break;
    }
    if (CAPI_V2_FAILED(result = FillInputBufferRef(&input_str_data[1], module, temp_inref_buffer))) {
      FARF(ERROR, "CAPI V2 TEST: Process failed with error %d.", result);
      break;
    }
    bool_t is_file_over = (input_str_data[0].buf_ptr[0].actual_data_len < input_str_data[0].buf_ptr[0].max_data_len) && (input_str_data[1].buf_ptr[0].actual_data_len < input_str_data[1].buf_ptr[0].max_data_len);
    if (is_file_over) {
      break;
    }

    while (input_str_data[0].buf_ptr[0].actual_data_len > 0 && input_str_data[1].buf_ptr[0].actual_data_len > 0) {
      result = CAPI_V2_EOK;
      uint32_t input_bytes_given[] = { input_str_data[0].buf_ptr[0].actual_data_len };
      uint32_t inputref_bytes_given[] = { input_str_data[1].buf_ptr[0].actual_data_len };

      // ------------------------
      // Call Processing function
      // ------------------------
      if (module->is_enabled) {
		  // ------------------------
          // Begin profiling
          // ------------------------
    	  START_PROCESS_PROFILE();
    	  prof_cycles = GET_PROFILE_CYCLES(SESSION_PROCESS);
          result = module->module_ptr->vtbl_ptr->process(module->module_ptr, input, output);


		  // ------------------------
          // Complete profiling
          // ------------------------
          // Diff Current Cycle State against previously acquired to
          // check Cycles elapsed
          if (prof_cycles > 0) {
        	  uint64_t curr_cycle = GET_PROFILE_CYCLES(SESSION_PROCESS);
              prof_cycles = curr_cycle - prof_cycles;
           } else {
        	   prof_cycles = GET_PROFILE_CYCLES(SESSION_PROCESS);
           }
		   // Number of samples produced per channel
           STOP_PROCESS_PROFILE();
      } else {
		 FARF(HIGH, "CAPI V2 TEST: Module not Enabled!!! That is, parsing else part of process call");
        for (ch = 0; ch < input_str_data[0].bufs_num; ch++) {
          memcpy(output_str_data[0].buf_ptr[ch].data_ptr, input_str_data[0].buf_ptr[ch].data_ptr, input_str_data[0].buf_ptr[ch].actual_data_len);
          output_str_data[0].buf_ptr[ch].actual_data_len = input_str_data[0].buf_ptr[ch].actual_data_len;
        }
        result = CAPI_V2_EOK;
      }

      if (!IsCompressedData(&module->out_format)) {
				QURT_ELITE_ASSERT(module->out_format.actual_data_len >= TST_STD_MEDIA_FMT_V2_MIN_SIZE);

				tst_standard_media_format_v2* tst_format = (tst_standard_media_format_v2*)module->out_format.data_ptr;
        uint32_t SampleCnt = (output_str_data[0].buf_ptr[0].actual_data_len * output[0]->bufs_num / (bits_to_bytes(tst_format->f.bits_per_sample) * tst_format->f.num_channels));
        // Begin recording after a few iterations, for steady state
        if (i > begin_steady_state_iteration) {
          prof_num_samples += SampleCnt;
          prof_total_cycles += prof_cycles;
          if (SampleCnt > 0) {
            prof_mips = ((float)(((0.000001 * tst_format->f.sampling_rate))) / SampleCnt) * prof_cycles;
            if (prof_mips > prof_peak_mips) {
              prof_peak_mips = prof_mips;
              #if FARF_HIGH == 1
              prof_max_iter = i;
              #endif
            }
          }
        }
      }

      // Now check if the process function went through fine.
      if (CAPI_V2_FAILED(result)) {
        FARF(ERROR, "CAPI V2 TEST: Process failed with error %d.", result);
        break;
      }
      result = CheckInputOutputSizes(input,
                                     input_bytes_given,
                                     output,
                                     module->requires_data_buffering,
                                     1lu,
                                     1lu);

      if (CAPI_V2_FAILED(result)) {
        break;
      }

      result = DumpOutputToFile(output[0], module, temp_out_buffer);
      if (CAPI_V2_FAILED(result)) {
        break;
      }

      // Adjust the input buffers if they are not completely consumed.
      for (ch = 0; ch < input_str_data[0].bufs_num; ch++) {
        capi_v2_buf_t* buf_ptr = &(input_str_data[0].buf_ptr[ch]);
        uint32_t num_bytes_consumed = buf_ptr->actual_data_len;
        memmove(buf_ptr->data_ptr, buf_ptr->data_ptr + num_bytes_consumed, input_bytes_given[0] - num_bytes_consumed);
        buf_ptr->actual_data_len = input_bytes_given[0] - num_bytes_consumed;
      }

      for (ch = 0; ch < input_str_data[1].bufs_num; ch++) {
        capi_v2_buf_t* bufref_ptr = &(input_str_data[1].buf_ptr[ch]);
        uint32_t ref_num_bytes_consumed = bufref_ptr->actual_data_len;
        memmove(bufref_ptr->data_ptr, bufref_ptr->data_ptr + ref_num_bytes_consumed, inputref_bytes_given[0] - ref_num_bytes_consumed);
        bufref_ptr->actual_data_len = input_bytes_given[0] - ref_num_bytes_consumed;
      }

    }
    if (CAPI_V2_FAILED(result)) {
      break;
    }
  }

  // ------------------------
  // Display profiling information
  // ------------------------
#if ((defined __hexagon__) || (defined __qdsp6__))
  if (!IsCompressedData(&module->out_format)) {
	sprintf(buffer, "%d", (int)numBuffers);
	ADD_PROFILE_ATTR(SESSION_PROCESS, "Buffers", buffer);
	PRINT_PROFILE_RESULT(SESSION_PROCESS);
	RESET_PROFILE_RESULT(SESSION_PROCESS);
		QURT_ELITE_ASSERT(module->out_format.actual_data_len >= TST_STD_MEDIA_FMT_V2_MIN_SIZE);

#if FARF_HIGH == 1
		tst_standard_media_format_v2* tst_format = (tst_standard_media_format_v2*)module->out_format.data_ptr;
#endif
    FARF(HIGH, "-------------------------------");
    FARF(HIGH, "CAPI V2 TEST: Profiling information");
    FARF(HIGH, "CAPI V2 TEST: Average MIPS: %.3f", ((float)(0.000001 * tst_format->f.sampling_rate) / prof_num_samples) * prof_total_cycles);
    FARF(HIGH, "CAPI V2 TEST: Peak MIPS: %.3f occurred at iteration# %lld", prof_peak_mips, prof_max_iter);
    FARF(HIGH, "-------------------------------");
  } else {
    FARF(HIGH, "-------------------------------");
    FARF(HIGH, "CAPI V2 TEST: Profiling information");
    FARF(HIGH, "CAPI V2 TEST: Output format is compressed. Profiling not supported.");
    FARF(HIGH, "-------------------------------");
  }
#endif // __qdsp6__

  // ------------------------
  // Clear processing buffers
  // ------------------------
done:
  if (NULL != input_buffer) {
    HAP_free(input_buffer);
    input_buffer = NULL;
  }
  if (NULL != temp_in_buffer) {
    HAP_free(temp_in_buffer);
    temp_in_buffer = NULL;
  }
  if (NULL != inputref_buffer) {
    HAP_free(inputref_buffer);
    inputref_buffer = NULL;
  }
  if (NULL != temp_inref_buffer) {
    HAP_free(temp_inref_buffer);
    temp_inref_buffer = NULL;
  }
  if (NULL != output_buffer) {
    HAP_free(output_buffer);
    output_buffer = NULL;
  }
  if (NULL != temp_out_buffer) {
    HAP_free(temp_out_buffer);
    temp_out_buffer = NULL;
  }

  return result;
}


capi_v2_err_t ProcessData(module_info_t* module)
{

   capi_v2_err_t result = CAPI_V2_EOK;

   if (num_port_info.num_input_ports > 1)
   {
      result = ProcessDataWithRefPort(module);
   }
   else
   {
	   result = ProcessDataWithOutRefPort(module);
   }

   return result;

}


capi_v2_err_t SetParamInband(module_info_t* module)
{
  FARF(HIGH, "CAPI V2 TEST: Executing Set Params command.");

  capi_v2_err_t result = CAPI_V2_EOK;

  uint8_t* pPacket = NULL;
  int8_t* ptr;
  uint32_t size;
  uint32_t packetSize = 0;
  asm_stream_param_data_v2_t* pHeader;


  result = GetUIntParameter(module->fCfg, "PayloadSizeInBytes", &(packetSize));

  if (CAPI_V2_EOK != result) {
    FARF(ERROR, "CAPI V2 TEST: SetParam command failed to read PayloadSizeInBytes.");
    goto done;
  }

  HAP_malloc(packetSize, (void**)&pPacket);
  if (NULL == pPacket) {
    FARF(ERROR, "CAPI V2 TEST: SetParam command failed to allocate packet.");
    result = CAPI_V2_ENOMEMORY;
    goto done;
  }

  result = ReadBufferContents(module->fCfg, packetSize, pPacket);
  if (CAPI_V2_EOK != result) {
    FARF(ERROR, "CAPI V2 TEST: SetParam command failed to read packet contents.");
    goto done;
  }

  ptr = (int8_t*)pPacket;
  size = packetSize;

  // Skip the pp set param packet header
  ptr += sizeof(asm_stream_cmd_set_pp_params_v2_t);
  size -= sizeof(asm_stream_cmd_set_pp_params_v2_t);

  pHeader = (asm_stream_param_data_v2_t*)(ptr);
  ptr += sizeof(asm_stream_param_data_v2_t);
  size -= sizeof(asm_stream_param_data_v2_t);

  {
    capi_v2_port_info_t port_info;
    port_info.is_valid = FALSE;
    // Send set params to module
    capi_v2_buf_t param_buf;
    param_buf.data_ptr = ptr;
    param_buf.actual_data_len = pHeader->param_size;
    param_buf.max_data_len = size;
    char paramIDStr[10] = { 0 };
    sprintf(paramIDStr, "0x%x", (unsigned int)pHeader->param_id);
    START_SETPARAM_PROFILE();
    ADD_PROFILE_ATTR(SESSION_SETPARAM, "ID", paramIDStr);
    result = module->module_ptr->vtbl_ptr->set_param(module->module_ptr, pHeader->param_id, &port_info, &param_buf);
    STOP_SETPARAM_PROFILE();
    if (CAPI_V2_EOK != result) {
      FARF(ERROR, "CAPI V2 TEST: Cannot set params, result=%d", result);
      goto done;
    }
  }
done:

  if (NULL != pPacket) {
    HAP_free(pPacket);
    pPacket = NULL;
  }

  return result;
}

capi_v2_err_t GetParamInband(module_info_t* module)
{
  FARF(HIGH, "CAPI V2 TEST: Executing Get Params command.");

  capi_v2_err_t result = CAPI_V2_EOK;
  uint32_t i;
  capi_v2_buf_t param_buf;

  int8_t* pPayload = NULL;

  uint32_t payloadSize = 0;
  uint32_t param_id = 0;
  uint32_t param_size = 0;


  result = GetUIntParameter(module->fCfg, "PayloadSizeInBytes", &payloadSize);
  if (CAPI_V2_EOK != result) {
    FARF(ERROR, "CAPI V2 TEST: GetParam command failed to read PayloadSizeInBytes");
    goto done;
  }

  // Read the module id and param id
  union {
    asm_stream_cmd_get_pp_params_v2_t h;
    uint8_t byte_stream[sizeof(asm_stream_cmd_get_pp_params_v2_t)];
  } header;

  QURT_ELITE_ASSERT(payloadSize == sizeof(header.byte_stream));
  for (i = 0; i < sizeof(header.byte_stream); i++) {
    char word[WORDSIZE];
    result = GetWord(module->fCfg, word);
    if (CAPI_V2_EOK != result) {
      FARF(ERROR, "CAPI V2 TEST: GetParam command failed to read payload contents");
      goto done;
    }

    int value;
    sscanf(word, "%x", &value);

    header.byte_stream[i] = (value & 0xFF);
  }

  param_id = header.h.param_id;
  param_size = header.h.param_max_size;

  HAP_malloc(param_size, (void**)&pPayload);
  if (NULL == pPayload) {
    FARF(ERROR, "CAPI V2 TEST: GetParam command failed to allocate packet");
    result = CAPI_V2_ENOMEMORY;
    goto done;
  }

  {
    // Send set params to module
    param_buf.data_ptr = pPayload;
    param_buf.actual_data_len = 0;
    param_buf.max_data_len = param_size;

    capi_v2_port_info_t port_info;
    port_info.is_valid = FALSE;
    char paramIDStr[10] = { 0 };
    sprintf(paramIDStr, "0x%x", (unsigned int)param_id);
    START_GETPARAM_PROFILE();
    ADD_PROFILE_ATTR(SESSION_GETPARAM, "ID", paramIDStr);
    result = module->module_ptr->vtbl_ptr->get_param(
       module->module_ptr,
       param_id,
       &port_info,
       &param_buf);

    STOP_GETPARAM_PROFILE();

    if (CAPI_V2_EOK != result) {
      FARF(ERROR, "CAPI V2 TEST: Cannot get params, result=%d", result);
      goto done;
    }

    FARF(HIGH, "CAPI V2 TEST: Done Get parameters for Param ID: 0x%lx", param_id);
  }

  // Compare the payload with reference
  result = GetUIntParameter(module->fCfg, "RefPayloadSizeInBytes", &payloadSize);
  if (CAPI_V2_EOK != result) {
    FARF(ERROR, "CAPI V2 TEST: GetParam command has no reference for comparison");
    result = CAPI_V2_EOK;
    goto done;
  }

  if (param_size != param_buf.actual_data_len) {
    FARF(ERROR, "CAPI V2 TEST: GetParam return packet size %lu does not match reference size %lu", param_buf.actual_data_len, param_size);
    result = CAPI_V2_EFAILED;
    goto done;
  }

  {
    for (i = 0; i < payloadSize; i++) {
      char word[WORDSIZE];
      result = GetWord(module->fCfg, word);
      if (CAPI_V2_EOK != result) {
        FARF(ERROR, "CAPI V2 TEST: GetParam command failed to read ref payload contents");
        goto done;
      }

      int value;
      sscanf(word, "%x", &value);

      int8_t refVal = (value & 0xFF);
      if (pPayload[i] != refVal) {
        FARF(ERROR, "CAPI V2 TEST: GetParam packet does not match reference in byte %lu", i);
        result = CAPI_V2_EFAILED;
        goto done;
      }
    }
  }

done:
  if (NULL != pPayload) {
    HAP_free(pPayload);
    pPayload = NULL;
  }

  return result;
}

capi_v2_err_t ParseMediaFormat(module_info_t* module, tst_standard_media_format_v2* format)
{
  capi_v2_err_t result = CAPI_V2_EOK;

  result = GetUIntParameter(module->fCfg, "SetBitstreamFormat", &(format->f.bitstream_format));
  if (CAPI_V2_EOK != result) {
    FARF(ERROR, "CAPI V2 TEST: Set Media format command failed to read bitstream format.");
    return CAPI_V2_EBADPARAM;
  }

  {
    uint32_t temp;
    result = GetUIntParameter(module->fCfg, "SetDataFormat", &(temp));
    if (CAPI_V2_EOK != result) {
      FARF(ERROR, "CAPI V2 TEST: Set Media format command failed to read data format.");
      return CAPI_V2_EBADPARAM;
    }
    format->h.data_format =(data_format_t)temp;
  }

  result = GetUIntParameter(module->fCfg, "SetNumChannelsAndMapping", &(format->f.num_channels));
  if (CAPI_V2_EOK != result) {
    FARF(ERROR, "CAPI V2 TEST: Set Media format command failed to read num in channels.");
    return CAPI_V2_EBADPARAM;
  }

  result = ReadChannelMapping(module->fCfg, format->f.num_channels, format->f.channel_type);
  if (CAPI_V2_EOK != result) {
    FARF(ERROR, "CAPI V2 TEST: Set Media format command failed to read channel mapping.");
    return CAPI_V2_EBADPARAM;
  }

  result = GetUIntParameter(module->fCfg, "SetBitsPerSample", &(format->f.bits_per_sample));
  if (CAPI_V2_EOK != result) {
    FARF(ERROR, "CAPI V2 TEST: Set Media format command failed to read bits per sample.");
    return CAPI_V2_EBADPARAM;
  }

  result = GetUIntParameter(module->fCfg, "QFactor", &(format->f.q_factor));
  if (CAPI_V2_EOK != result) {
    FARF(ERROR, "CAPI V2 TEST: Set Media format command failed to read q factor.");
    return CAPI_V2_EBADPARAM;
  }

  result = GetUIntParameter(module->fCfg, "SetSamplingRate", &(format->f.sampling_rate));
  if (CAPI_V2_EOK != result) {
    FARF(ERROR, "CAPI V2 TEST: Set Media format command failed to read sampling rate.");
    return CAPI_V2_EBADPARAM;
  }

  result = GetUIntParameter(module->fCfg, "SetIsSigned", &(format->f.data_is_signed));
  if (CAPI_V2_EOK != result) {
    FARF(ERROR, "CAPI V2 TEST: Set Media format command failed to read is signed.");
    return CAPI_V2_EBADPARAM;
  }

  {
    uint32_t temp;
    result = GetUIntParameter(module->fCfg, "SetInterleaving", &temp);
    if (CAPI_V2_EOK != result) {
      FARF(ERROR, "CAPI V2 TEST: Set Media format command failed to read interleaving.");
      return CAPI_V2_EBADPARAM;
    }
    format->f.data_interleaving = (capi_v2_interleaving_t)temp;
  }

  return result;
}

/* call setParam with data read from a file
 *
 *   Form of SetParamFromFile config file command is:
 *        4-byte Module Id
 *        4-byte Param Id
 *        n-byte string containing file path
 *   E.g.
 *   SetParamFromFile
 *        02 2D 01 00                # MODULE_ID 0x00012D02
 *        14 2C 01 00                # param_id to use for setParam call
 *        ..\..\data\sound_model.bin # name of file to be copied into mem
 *
 *   This function
 *       parses module and param ids
 *       parses file name
 *       gets size file
 *       allocates memory
 *       copies data from file into memory
 *       calls setParam with the following:
 *         00 00 00 00    # Data Payload address <msw>
 *         00 00 00 00    # Data Payload address <lsw>
 *         00 00 00 00    # mem_map_handle
 *         0c 00 00 00    # Data payload size - should this be 4 + 8
 *         mm mm mm mm    # module_id (ignored by module code)
 *         pp pp pp pp    # param_id parsed from cfg command
 *         ss ss ss ss    # 32-bit size of data stored at address below
 *                        # This is the numbers of bytes read from file
 *         yy yy yy yy    # address of memory <msw>
 *         yy yy yy yy    # address of memory <lsw>
 */
capi_v2_err_t SetParamFromFile(module_info_t* module)
{
	char filename_sound_model[128] = { 0 };
	capi_v2_err_t result = CAPI_V2_EOK;
    uint32_t num_bytes_to_read = 0;
	uint32_t num_bytes_read = 0;
	int8_t* p_sound_model_data;
	FILE* file_handle;
	uint8_t module_id_u8_array[4];
    uint32_t *module_id_u32;
	uint8_t param_id_u8_array[4];
    uint32_t *param_id_u32;
	uint8_t* pPacket = NULL;
	int pathIndx = 0;

	// get module id
	result = ReadBufferContents(module->fCfg, 4, module_id_u8_array);
	if (CAPI_V2_EOK != result) {
	    FARF(ERROR, "CAPI V2 TEST: SetParamFromFile failed to read module id.      ");
	    goto done;
	}
    module_id_u32 = (uint32_t *)module_id_u8_array;
	FARF(HIGH, "CAPI V2 TEST: SetParamFromFile moduleId = %lu              ",
			 *module_id_u32);
	// get parameter id
	result = ReadBufferContents(module->fCfg, 4, param_id_u8_array);
	if (CAPI_V2_EOK != result) {
	    FARF(ERROR, "CAPI V2 TEST: SetParamFromFile failed to read param id.      ");
	    goto done;
	}
    param_id_u32 = (uint32_t *)param_id_u8_array;
	FARF(HIGH, "CAPI V2 TEST: SetParamFromFile paramId = %lu              ",
			 *param_id_u32);

    // get file path string
#ifdef ENABLE_COMMAND_LINE_PARAMS
	strncpy(filename_sound_model, "..\\", 3);
	pathIndx = 3;
#else
	pathIndx = 0;
#endif
	result = GetWord(module->fCfg, &filename_sound_model[pathIndx]);
	if (CAPI_V2_EOK != result) {
		FARF(ERROR, "CAPI V2 TEST: SetParamFromFile failed for find file name.    ");
		return CAPI_V2_EBADPARAM;
	}

	FARF(HIGH, "CAPI V2 TEST: SetParamFromFile name = %s                          ",
			 filename_sound_model);

	if ((file_handle = fopen(filename_sound_model, "rb")) == NULL) {
	    FARF(ERROR, "Cannot open sound model file                             ");
	    return CAPI_V2_EFAILED;
	}

	// get file size
	if (fseek(file_handle, 0 , SEEK_END) != 0) {
		FARF(ERROR, "Sound model file seek to end failed                             ");
		return CAPI_V2_EFAILED;
	}
	num_bytes_to_read = ftell(file_handle);
	if (num_bytes_to_read == -1) {
		FARF(ERROR, "sound model file tell failed                             ");
		return CAPI_V2_EFAILED;
	}
	FARF(HIGH, "Sound model file size, number of bytes to read = %d          ",
			  num_bytes_to_read);
	if (fseek(file_handle, 0 , SEEK_SET) != 0) {
		FARF(ERROR, "Sound model file rewind to beginning failed                  ");
		return CAPI_V2_EFAILED;
	}

	HAP_malloc(num_bytes_to_read , (void**)&p_sound_model_data);
	if (NULL == p_sound_model_data) {
	    FARF(ERROR, "Memory allocation error for sound model block             ");
	    return CAPI_V2_ENOMEMORY;
	}
	FARF(HIGH, "ptr to Sound model data = %p                                ",
			  p_sound_model_data);
	num_bytes_read = fread(p_sound_model_data, sizeof(int8_t), num_bytes_to_read, file_handle);
	if (num_bytes_read != num_bytes_to_read) {
		FARF(ERROR, "only %d bytes read from soundmodel                       ", num_bytes_read);
		return CAPI_V2_EFAILED;
	}

	/** Manually Fill setParam command structure command passing
	 *  payload containing:
	 *    4-byte size of data stored at address below
	 *    8-byte address of memory
	 */
	uint32_t addrSize = sizeof(int8_t*);
	uint32_t packetSize = sizeof(asm_stream_cmd_set_pp_params_v2_t) +
			  sizeof(asm_stream_param_data_v2_t ) + addrSize;
	int8_t* ptr;
	uint32_t size;
	asm_stream_param_data_v2_t * pParamData;

	HAP_malloc(packetSize, (void**)&pPacket);
	if (NULL == pPacket) {
	    FARF(ERROR, "CAPI V2 TEST: SetParam command failed to allocate packet.");
	    result = CAPI_V2_ENOMEMORY;
	    goto done;
	}
	memset((void *)pPacket, 0, packetSize);  // Initialize structure with zeros

	ptr = (int8_t*)pPacket;
	size = packetSize;

	// Skip the set param packet header which will remain all zeros
	ptr += sizeof(asm_stream_cmd_set_pp_params_v2_t);
	size -= sizeof(asm_stream_cmd_set_pp_params_v2_t);

	pParamData = (asm_stream_param_data_v2_t *)(ptr);
	pParamData->param_id = *param_id_u32;
	pParamData->module_id = *module_id_u32;
	pParamData->param_size = 12; // size of data memory and size of memory address
	// place 32-bit size into 16-bit param_size + 16-bit padded fields
	memcpy((void *)&pParamData->param_size, (void *)&num_bytes_read, 4);

	ptr += sizeof(asm_stream_param_data_v2_t );
	size -= sizeof(asm_stream_param_data_v2_t );
	if (size < addrSize) {
		FARF(ERROR, "CAPI V2 TEST: remaining payload data (%d) isn't big enough for address (%d)",
		    		size, addrSize);
		result = CAPI_V2_ENOMEMORY;
		goto done;
	}
    memcpy(&ptr, &p_sound_model_data, addrSize); // copy address into payload
	capi_v2_port_info_t port_info;
	port_info.is_valid = FALSE;
	// Send set params to module
	capi_v2_buf_t param_buf;
	param_buf.data_ptr = ptr;
	FARF(HIGH, "param_buf.data_ptr = %p                    ", param_buf.data_ptr);
	param_buf.actual_data_len = pParamData->param_size;
	param_buf.max_data_len = size;

	char paramIDStr[10] = { 0 };
	sprintf(paramIDStr, "0x%x", (unsigned int)pParamData->param_id);
	START_SETPARAM_PROFILE();
	ADD_PROFILE_ATTR(SESSION_SETPARAM, "ID", paramIDStr);
	result = module->module_ptr->vtbl_ptr->set_param(module->module_ptr,
	    		pParamData->param_id, //  LSM_PARAM_ID_REGISTER_SND_MODEL
	    		&port_info, &param_buf);
	STOP_SETPARAM_PROFILE();

	if (CAPI_V2_EOK != result) {
	    FARF(ERROR, "CAPI V2 TEST: Cannot set params, result=%d", result);
	    goto done;
	}

  done:

	if (NULL != pPacket) {
	    HAP_free(pPacket);
	    pPacket = NULL;
	}
  return result;
}

capi_v2_err_t ParseMediaFormatGeneric(module_info_t* module)
{
capi_v2_err_t result = CAPI_V2_EOK;

  result = GetUIntParameter(module->fCfg, "InputNumberOfPorts", &(num_port_info.num_input_ports));
  if (CAPI_V2_EOK != result) {
    FARF(ERROR, "CAPI V2 TEST: Set Media format command failed to read num_input_ports.");
    return CAPI_V2_EBADPARAM;
  }

  result = GetUIntParameter(module->fCfg, "OutputNumberOfPorts", &(num_port_info.num_output_ports));
  if (CAPI_V2_EOK != result) {
    FARF(ERROR, "CAPI V2 TEST: Set Media format command failed to read num_output_ports.");
    return CAPI_V2_EBADPARAM;
  }


if (num_port_info.num_input_ports > 1)
{
  result = GetUIntParameter(module->fCfg, "SetNumChannelsAndMappingRef", &(num_channels_ref));
  if (CAPI_V2_EOK != result) {
    FARF(HIGH, "CAPI V2 TEST: Reference Port is not specified.");
    return CAPI_V2_EBADPARAM;
  }

  result = ReadChannelMapping(module->fCfg, num_channels_ref, channel_type_ref);
  if (CAPI_V2_EOK != result) {
    FARF(HIGH, "CAPI V2 TEST: Unable to read Number of Channels and Channel Mapping for Reference Port.");
    return CAPI_V2_EBADPARAM;
  }
}
  result = GetUIntParameter(module->fCfg, "FrameSize", &(frame_size_ms));
  if (CAPI_V2_EOK != result) {
    FARF(ERROR, "CAPI V2 TEST: Set Media format command failed to read frame_size_ms.");
    return CAPI_V2_EBADPARAM;
  }

  return result;
}

capi_v2_err_t SetOutputMediaFormat(module_info_t* module)
{
  FARF(HIGH, "CAPI V2 TEST: Executing Set Output Media format with channel mapping command.");

	tst_standard_media_format_v2 out_format = { };
  capi_v2_err_t result = ParseMediaFormat(module, &out_format);
  if (CAPI_V2_FAILED(result)) {
    FARF(ERROR, "CAPI V2 TEST: Failed to parse the output media format, result = %d", result);
    return result;
  }

  // Adjust input buffer sizes for new media formats
  QURT_ELITE_ASSERT(sizeof(out_format) <= sizeof(module->out_format_buf));
  QURT_ELITE_ASSERT(sizeof(out_format) <= sizeof(module->out_format_buf));
  memcpy(module->out_format_buf, &out_format, sizeof(out_format));
  module->out_format.actual_data_len = STD_MIN(sizeof(out_format), sizeof(module->out_format_buf));
  module->out_format.max_data_len = sizeof(module->out_format_buf);
  module->out_format.data_ptr = module->out_format_buf;

  if (IsCompressedData(&module->out_format)) {
    // TODO support comporessed data
    /*
    result = IEC61937_get_buffer_size(out_format.f.bitstream_format, &module->in_buffer_len);
    if (CAPI_V2_EOK != result) {
      FARF(ERROR, "CAPI V2 TEST: Unable to get compressed buffer size, result=%d", result);
      return result;
    }*/
    return CAPI_V2_EFAILED;
  } else {
	  FARF(HIGH, "CAPI V2 TEST: In function SetOutputMediaFormat: frame_size_ms=%d", frame_size_ms);
    module->out_buffer_len = (out_format.f.sampling_rate * frame_size_ms / 1000) * bits_to_bytes(out_format.f.bits_per_sample);
  }

  // ------------------------
  // Send media type information
  // ------------------------
  {
    capi_v2_prop_t prop = {
      CAPI_V2_OUTPUT_MEDIA_FORMAT_V2,
      module->out_format,
      {
        TRUE,
        TRUE,
        0
      }
    };
    capi_v2_proplist_t proplist = {
      1,
      &prop
    };

    START_SETPROP_PROFILE();
    result = module->module_ptr->vtbl_ptr->set_properties(
       module->module_ptr, &proplist);

		/* If the module does'nt support media format V2, then there are chances of failure,
				 * so we update the property with media format V1 and redo the set_property */
		if ((CAPI_V2_EOK != result) && (out_format.f.num_channels<=CAPI_V2_MAX_CHANNELS))
		{
			tst_standard_media_format temp_out_format;
			capi_v2_buf_t temp_module_out_format;
			copy_media_fmt_v2_to_v1(&temp_out_format, &out_format);

			// Adjust input buffer sizes for new media formats
			QURT_ELITE_ASSERT(sizeof(temp_out_format) <= sizeof(module->in_format_buf));
			temp_module_out_format.actual_data_len = sizeof(tst_standard_media_format);
			temp_module_out_format.max_data_len = temp_module_out_format.actual_data_len;
			temp_module_out_format.data_ptr = (int8_t *)(&temp_out_format);

			prop.id = CAPI_V2_OUTPUT_MEDIA_FORMAT;
			prop.payload = temp_module_out_format;

			result = module->module_ptr->vtbl_ptr->set_properties(
					module->module_ptr, &proplist);

		}
    STOP_SETPROP_PROFILE();
    if (CAPI_V2_EOK != result) {
      FARF(ERROR, "CAPI V2 TEST: Cannot set input media format, result=%d", result);
    }
  }

  return result;
}

capi_v2_err_t SetMediaFormat(module_info_t* module)
{

  FARF(HIGH, "CAPI V2 TEST: Executing Set Media format with channel mapping command.");

  tst_standard_media_format_v2 in_format = { };
  capi_v2_err_t result = ParseMediaFormat(module, &in_format);
  if (CAPI_V2_FAILED(result)) {
    FARF(ERROR, "CAPI V2 TEST: Failed to parse the output media format, result = %d", result);
    return result;
  }
  result = ParseMediaFormatGeneric(module);
  if (CAPI_V2_FAILED(result)) {
    FARF(ERROR, "CAPI V2 TEST: Failed to parse the Generic media format, result = %d", result);
    return result;
  }

  // Adjust input buffer sizes for new media formats
  QURT_ELITE_ASSERT(sizeof(in_format) <= sizeof(module->in_format_buf));
  memcpy(module->in_format_buf, &in_format, sizeof(in_format));
  module->in_format.actual_data_len = STD_MIN(sizeof(in_format), sizeof(module->in_format_buf));
  module->in_format.max_data_len = sizeof(module->in_format_buf);
  module->in_format.data_ptr = module->in_format_buf;

  //update ref number of channels and their channel types
  if (num_channels_ref > 0)
  {
	  uint32_t i;
	  in_format.f.num_channels = num_channels_ref;;
	  for (i = 0; i < num_channels_ref; i++)
	  {
			in_format.channel_type[i] = channel_type_ref[i];
	  }

	  //Now copy ref media format.
	  memcpy(module->in_format_buf+sizeof(in_format), &in_format, sizeof(in_format));
	  module->in_format.actual_data_len = STD_MIN(sizeof(in_format)*num_port_info.num_input_ports, sizeof(module->in_format_buf));
	  module->in_format.max_data_len = sizeof(module->in_format_buf);
	  module->in_format.data_ptr = module->in_format_buf;
  }

  if (IsCompressedData(&module->in_format)) {
    // TODO support comporessed data
    /*
    result = IEC61937_get_buffer_size(in_format.f.bitstream_format, &module->in_buffer_len);
    if (CAPI_V2_EOK != result) {
      FARF(ERROR, "CAPI V2 TEST: Unable to get compressed buffer size, result=%d", result);
      return result;
    }*/
    return CAPI_V2_EFAILED;
  } else {
	//FARF(HIGH, "CAPI V2 TEST: In function SetMediaFormat: frame_size_ms=%d", frame_size_ms);
    module->in_buffer_len = (in_format.f.sampling_rate * frame_size_ms / 1000) * bits_to_bytes(in_format.f.bits_per_sample);
  }

  // ----------------------------------------------------------------------------
  // Send input and output number of ports, before sending media type information
  // ----------------------------------------------------------------------------
  capi_v2_proplist_t proplist1;
  capi_v2_prop_t props[2];
  proplist1.props_num = 2;
  proplist1.prop_ptr = props;
  capi_v2_port_num_info_t port_num_info;
  props[0].id = CAPI_V2_PORT_NUM_INFO;
  port_num_info.num_input_ports = num_port_info.num_input_ports;
  port_num_info.num_output_ports = num_port_info.num_output_ports;
  props[0].payload.data_ptr = (int8_t*)&port_num_info;
  props[0].payload.actual_data_len = sizeof(port_num_info);
  props[0].payload.max_data_len = sizeof(port_num_info);

  // Set event callback information
  capi_v2_event_callback_info_t event_cb_info;
  event_cb_info = capi_v2_tst_get_cb_info(module);
  props[1].id = CAPI_V2_EVENT_CALLBACK_INFO;
  props[1].payload.data_ptr = (int8_t*)(&event_cb_info);
  props[1].payload.actual_data_len = sizeof(event_cb_info);
  props[1].payload.max_data_len = sizeof(event_cb_info);

      result = module->module_ptr->vtbl_ptr->set_properties(
       module->module_ptr, &proplist1);


  // ------------------------
  // Send media type information
  // ------------------------
  {
    capi_v2_prop_t prop = {
      CAPI_V2_INPUT_MEDIA_FORMAT_V2,
      module->in_format,
      {
        TRUE,
        TRUE,
        0
      }
    };
    capi_v2_proplist_t proplist = {
      1,
      &prop
    };
    START_SETPROP_PROFILE();
    result = module->module_ptr->vtbl_ptr->set_properties(
       module->module_ptr, &proplist);

		/* If the module does'nt support media format V2, then there are chances of failure,
		 * so we update the property with media format V1 and redo the set_property */
		if ((CAPI_V2_EOK != result) && (in_format.f.num_channels<=CAPI_V2_MAX_CHANNELS))
		{
			int8_t temp_in_format_buf[FORMAT_BUF_SIZE] = {0};
			tst_standard_media_format temp_in_format;
			capi_v2_buf_t temp_module_in_format;
			copy_media_fmt_v2_to_v1(&temp_in_format, &in_format);

			// Adjust input buffer sizes for new media formats
			QURT_ELITE_ASSERT(sizeof(temp_in_format) <= sizeof(module->in_format_buf));
			memcpy(temp_in_format_buf, &temp_in_format, sizeof(temp_in_format));

			temp_module_in_format.actual_data_len = sizeof(tst_standard_media_format);
			temp_module_in_format.max_data_len = temp_module_in_format.actual_data_len;
			temp_module_in_format.data_ptr = temp_in_format_buf;

			//update ref number of channels and their channel types
			if (num_channels_ref > 0)
			{
				uint32_t i;
				temp_in_format.f.num_channels = num_channels_ref;;
				for (i = 0; i < num_channels_ref; i++)
				{
					temp_in_format.f.channel_type[i] = channel_type_ref[i];
				}

				//Now copy ref media format.
				memcpy(temp_in_format_buf+sizeof(temp_in_format), &temp_in_format, sizeof(temp_in_format));
				temp_module_in_format.actual_data_len = sizeof(temp_in_format) * num_port_info.num_input_ports;
				temp_module_in_format.max_data_len = sizeof(module->in_format_buf);
				temp_module_in_format.data_ptr = temp_in_format_buf;
			}

			prop.id = CAPI_V2_INPUT_MEDIA_FORMAT;
			prop.payload = temp_module_in_format;

			result = module->module_ptr->vtbl_ptr->set_properties(
					module->module_ptr, &proplist);

		}

    STOP_SETPROP_PROFILE();
    if (CAPI_V2_EOK != result) {
      FARF(ERROR, "CAPI V2 TEST: Cannot set input media format, result=%d", result);
    }
  }

  return result;
}


capi_v2_err_t ExecuteOneCommand(const testCommand* pTable, const uint32_t tableLength, module_info_t* module, bool_t* pEOFReached)
{
  char word[WORDSIZE];
  bool_t commandFound = FALSE;
  uint32_t commandIndex = 0;
  uint32_t i = 0;
  *pEOFReached = FALSE;

  while (!commandFound) {
    capi_v2_err_t result;

    result = GetWord(module->fCfg, word);
    if (CAPI_V2_EOK != result) {
      FARF(HIGH, "CAPI V2 TEST: No more valid commands in file.");
      *pEOFReached = TRUE;
      return CAPI_V2_EOK;
    }

    // Search for the command
    for (i = 0; i < tableLength; i++) {
      if (0 == strncmp(word, pTable[i].opCode, WORDSIZE)) {
        commandFound = TRUE;
        commandIndex = i;
        break;
      }
    }
  }

  return ((pTable[commandIndex].pFunction)(module));
}

const testCommand steadyStateCommandSet[] = {
  { "ProcessData", &ProcessData },
  { "SetParamInband", &SetParamInband },
  { "GetParamInband", &GetParamInband },
  { "SetMediaFormat", &SetMediaFormat },
  { "SetOutputMediaFormat", &SetOutputMediaFormat },
  { "SetParamFromFile", &SetParamFromFile},
};

capi_v2_err_t RunTest(module_info_t* module, const testCommand* pExtendedCmdSet, const uint32_t extendedCmdSetSize)
{
  capi_v2_err_t result = CAPI_V2_EOK;
  static const uint32_t MAX_COMMANDS = 50;
  testCommand completeSet[MAX_COMMANDS];
  uint32_t numCommands = 0;

  // Create a complete command set
  uint32_t steadyStateSetSize = sizeof(steadyStateCommandSet) / sizeof(steadyStateCommandSet[0]);
  if (numCommands + steadyStateSetSize > MAX_COMMANDS) {
    FARF(ERROR, "CAPI V2 TEST: Too many commands in steady state");
    return CAPI_V2_ENOMEMORY;
  }
  memcpy(completeSet + numCommands, steadyStateCommandSet, sizeof(testCommand) * steadyStateSetSize);
  numCommands += steadyStateSetSize;

  if (numCommands + extendedCmdSetSize > MAX_COMMANDS) {
    FARF(ERROR, "CAPI V2 TEST: Too many commands in extended set");
    return CAPI_V2_ENOMEMORY;
  }
  memcpy(completeSet + numCommands, pExtendedCmdSet, sizeof(testCommand) * extendedCmdSetSize);
  numCommands += extendedCmdSetSize;

  FARF(HIGH, "CAPI V2 TEST: Executing commands now                                  ");

  while (!feof(module->fCfg)) {
    bool_t EOFReached = FALSE;
    result = ExecuteOneCommand(completeSet, numCommands, module, &EOFReached);
    if (CAPI_V2_EOK != result) {
      FARF(ERROR, "CAPI V2 TEST: Error in executing command. Exiting test");
      break;
    }

    if (EOFReached) {
      break;
    }
  }

  return result;
}

static capi_v2_err_t capi_v2_tst_cb_fn(void* context_ptr, capi_v2_event_id_t id, capi_v2_event_info_t* event_ptr)
{
  module_info_t* module = (module_info_t*)context_ptr;
  capi_v2_buf_t* payload = &event_ptr->payload;
  int i = 0;

  if (payload->actual_data_len > payload->max_data_len) {
    FARF(ERROR, "CAPI V2 TEST: Error in callback function. The actual size %lu is greater than the max size %lu for id %lu", payload->actual_data_len, payload->max_data_len, (uint32_t)id);
    return CAPI_V2_EBADPARAM;
  }

  switch(id) {
    case CAPI_V2_EVENT_KPPS:
    {
      if (payload->actual_data_len < sizeof(capi_v2_event_KPPS_t)) {
        FARF(ERROR, "CAPI V2 TEST: Error in callback function. The actual size %lu is less than the required size %zu for id %lu.", payload->actual_data_len, sizeof(capi_v2_event_KPPS_t), (uint32_t)id);
        return CAPI_V2_ENEEDMORE;
      }
	  //FARF (HIGH, "CAPI V2 TEST: Debug at CAPI_V2_EVENT_KPPS");
      module->kpps = ((capi_v2_event_KPPS_t*)(payload->data_ptr))->KPPS;
      FARF(HIGH, "CAPI V2 TEST: Module KPPS set to %lu", (uint32_t)module->kpps);

      return CAPI_V2_EOK;
    }

#if 0
    case CAPI_V2_EVENT_GET_LIBRARY_INSTANCE:
      {
         capi_v2_err_t result = CAPI_V2_EOK;
         capi_v2_event_get_library_instance_t* get_library_instance = (capi_v2_event_get_library_instance_t*)event_ptr->payload.data_ptr;;
         FARF(HIGH, "CAPI V2 TEST: get_library_instance->id %x", get_library_instance->id);
         result = capi_v2_library_factory_get_instance(get_library_instance->id,&get_library_instance->ptr);
         if(CAPI_V2_EOK != result)
         {
           FARF (HIGH, "CAPI V2 TEST: Failed to Get library instance with lib_id(0x%x), payload_ptr(0x%p)",get_library_instance->id, get_library_instance->ptr);
           return CAPI_V2_EFAILED;
         }
         else
         {
		    FARF (HIGH, "CAPI V2 TEST: Get library instance with lib_id(0x%x), payload_ptr(0x%p)",get_library_instance->id, get_library_instance->ptr);
		 }
         return CAPI_V2_EOK;
      }
#endif

    case CAPI_V2_EVENT_ALGORITHMIC_DELAY:
    {
      if (payload->actual_data_len < sizeof(capi_v2_event_algorithmic_delay_t)) {
        FARF(ERROR, "CAPI V2 TEST: Error in callback function. The actual size %lu is less than the required size %zu for id %lu.", payload->actual_data_len, sizeof(capi_v2_event_algorithmic_delay_t), (uint32_t)id);
        return CAPI_V2_ENEEDMORE;
      }
	  //FARF (HIGH, "CAPI V2 TEST: Debug at CAPI_V2_EVENT_ALGORITHMIC_DELAY");
      module->alg_delay = ((capi_v2_event_algorithmic_delay_t*)(payload->data_ptr))->delay_in_us;
      FARF(HIGH, "CAPI V2 TEST: Module delay set to %lu", (uint32_t)module->alg_delay);

      return CAPI_V2_EOK;
    }
    case CAPI_V2_EVENT_PROCESS_STATE:
    {
      if (payload->actual_data_len < sizeof(capi_v2_event_process_state_t)) {
        FARF(ERROR, "CAPI V2 TEST: Error in callback function. The actual size %lu is less than the required size %zu for id %lu.", payload->actual_data_len, sizeof(capi_v2_event_process_state_t), (uint32_t)id);
        return CAPI_V2_ENEEDMORE;
      }

      module->is_enabled = ((capi_v2_event_process_state_t*)(payload->data_ptr))->is_enabled;
      FARF(HIGH, "CAPI V2 TEST: Module process check set to %lu", (uint32_t)module->is_enabled);

      return CAPI_V2_EOK;
    }
    case CAPI_V2_EVENT_OUTPUT_MEDIA_FORMAT_UPDATED:
    case CAPI_V2_EVENT_OUTPUT_MEDIA_FORMAT_UPDATED_V2:
    {
      if (!event_ptr->port_info.is_valid) {
        FARF(ERROR, "CAPI V2 TEST: Error in callback function. Changing media type only supported for port 0. Invalid port id passed.");
        return CAPI_V2_EUNSUPPORTED;
      }

      if (0 != event_ptr->port_info.port_index) {
        FARF(ERROR, "CAPI V2 TEST: Error in callback function. Changing media type only supported for port 0. Port id %lu passed.", event_ptr->port_info.port_index);
        return CAPI_V2_EUNSUPPORTED;
      }

      if (payload->actual_data_len < sizeof(capi_v2_set_get_media_format_t)) {
        FARF(ERROR, "CAPI V2 TEST: Error in callback function. The actual size %lu is less than the required size %zu for id %lu.", payload->actual_data_len, sizeof(capi_v2_set_get_media_format_t), (uint32_t)id);
        return CAPI_V2_ENEEDMORE;
      }

        if (CAPI_V2_EOK != update_output_media_format(module, payload, id)) {
        return CAPI_V2_EFAILED;
      }
      FARF(HIGH, "CAPI V2 TEST: Module output format updated.");

      return CAPI_V2_EOK;
    }

    case CAPI_V2_EVENT_DATA_TO_DSP_SERVICE:
    {
        FARF(HIGH, "CAPI V2 TEST: Module DATA_TO_DSP received");
        capi_v2_event_data_to_dsp_service_t * data_to_dsp_event = (capi_v2_event_data_to_dsp_service_t*)payload->data_ptr;

        if (payload->actual_data_len < sizeof(capi_v2_event_data_to_dsp_service_t)) {
          FARF(ERROR, "CAPI V2 TEST cb_fun: The actual size %lu is less than the required size %zu for id CAPI_V2_EVENT_DATA_TO_DSP_SERVICE.",
        		  payload->actual_data_len, sizeof(capi_v2_event_data_to_dsp_service_t));
          return CAPI_V2_ENEEDMORE;
        }
        // print out generic payload
        FARF(HIGH, "CAPI V2 TEST cb_fun: CAPI_V2_EVENT_DATA_TO_DSP_SERVICE, detection paramId %d         ",
        		data_to_dsp_event->param_id );
        uint8_t* data_payload = (uint8_t *)data_to_dsp_event->payload.data_ptr;
        uint32_t actual_data_len = data_to_dsp_event->payload.actual_data_len;
        FARF(HIGH, "CAPI V2 TEST: CAPI_V2_EVENT_DATA_TO_DSP_SERVICE, detection payload data len 0x%x     ",
        		actual_data_len);
        if (data_payload != NULL) {
          for (i=0; i<actual_data_len; i++) {
              FARF(HIGH, "        payload[%d] = 0x%x                                                       ",
                     i, data_payload[i]);
          }
        }
        return CAPI_V2_EOK;
    }
    case CAPI_V2_EVENT_BANDWIDTH:
    {
        if (payload->actual_data_len < sizeof(capi_v2_event_bandwidth_t)) {
          FARF(ERROR, "CAPI V2 TEST: Error in callback function. The actual size %lu is less than the required size %zu for id %lu.",
        		  payload->actual_data_len, sizeof(capi_v2_event_bandwidth_t), (uint32_t)id);
          return CAPI_V2_ENEEDMORE;
        }

        #ifdef ERROR_HIGH
        {
          capi_v2_event_bandwidth_t* bandwidths;
          bandwidths = (capi_v2_event_bandwidth_t*)payload->data_ptr;
          // not stored in module
          FARF(HIGH, "CAPI V2 TEST: Module Bandwidth code, data set to %lu, %lu",
               bandwidths->code_bandwidth, bandwidths->data_bandwidth);
        }
        #endif
        return CAPI_V2_EOK;
    }

   default:
    {
      FARF(ERROR, "CAPI V2 TEST: Error in callback function. ID %lu not supported.", (uint32_t)id);
      return CAPI_V2_EUNSUPPORTED;
    }
  }

  return CAPI_V2_EOK;
}

capi_v2_event_callback_info_t capi_v2_tst_get_cb_info(module_info_t* module)
{
  capi_v2_event_callback_info_t cb_info;

  cb_info.event_cb = capi_v2_tst_cb_fn;
  cb_info.event_context = module;

  return cb_info;
}
