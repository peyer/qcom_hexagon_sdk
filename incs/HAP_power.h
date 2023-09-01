/*==============================================================================
Copyright (c) 2015,2019 Qualcomm Technologies, Inc.
All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#ifndef _HAP_POWER_H
#define _HAP_POWER_H

#include "AEEStdErr.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

//Add a weak reference so shared objects do not throw link error
#pragma weak HAP_power_destroy_client

/**
* Possible error codes returned
*/
typedef enum {
	HAP_POWER_ERR_UNKNOWN           = -1,
	HAP_POWER_ERR_INVALID_PARAM     = -2,
	HAP_POWER_ERR_UNSUPPORTED_API   = -3
} HAP_power_error_codes;

/**
* Payload for HAP_power_set_mips_bw

* @param set_mips				-		Set to TRUE to requst MIPS
* @param mipsPerThread			-		mips requested per thread, to establish a minimal clock frequency per HW thread
* @param mipsTotal				-		Total mips requested, to establish total number of MIPS required across all HW threads
* @param set_bus_bw				-		Set to TRUE to request bus_bw
* @param bwBytePerSec			-		Max bus BW requested (bytes per second)
* @param busbwUsagePercentage	-		Percentage of time during which bwBytesPerSec BW is required from the bus (0..100)
* @param set_latency			-		Set to TRUE to set latency
* @param latency				-		maximum hardware wakeup latency in microseconds.  The higher the value,
* 										the deeper state of sleep that can be entered but the longer it may take
* 										to awaken. Only values > 0 are supported (1 microsecond is the smallest valid value)
*/
typedef struct {
	boolean set_mips;
	unsigned int mipsPerThread;
	unsigned int mipsTotal;
	boolean set_bus_bw;
	uint64 bwBytePerSec;
	unsigned short busbwUsagePercentage;
	boolean set_latency;
	int latency;
} HAP_power_mips_bw_payload;

/**
* Payload for HAP_power_set_vapss/HAP_power_set_vapss_v2

* @param set_clk				-		Set to TRUE to requst clock frequency
* @param set_dma_clk			-		Set to TRUE to requst DMA clock frequency
* @param set_hcp_clk			-		Set to TRUE to requst HCP clock frequency
* @param dmaClkFreqHz			-		DMA Clock frequency in Hz
* @param hcpClkFreqHz			-		HCP Clock frequency in Hz
* @param freqMatch				-		Clock frequency match
* @param set_bus_bw				-		Set to TRUE to request bus_bw
* @param bwBytePerSec			-		Max bus BW requested (bytes per second)
* @param busbwUsagePercentage	-		Percentage of time during which bwBytesPerSec BW is required from the bus (0..100)
*/
typedef enum {
	HAP_FREQ_AT_LEAST,
	HAP_FREQ_AT_MOST,
	HAP_FREQ_CLOSEST,
	HAP_FREQ_EXACT,
	HAP_FREQ_MAX_COUNT
} HAP_freq_match_type;

typedef struct {
	boolean set_bus_bw;
	uint64 bwBytePerSec;
	unsigned short busbwUsagePercentage;
} HAP_power_bus_bw;

typedef struct {
	boolean set_clk;
	unsigned int clkFreqHz;
	HAP_freq_match_type freqMatch;
	HAP_power_bus_bw dma_ext;
	HAP_power_bus_bw hcp_ext;
	HAP_power_bus_bw dma_int;
	HAP_power_bus_bw hcp_int;
} HAP_power_vapss_payload;

typedef struct {
	boolean set_dma_clk;
	boolean set_hcp_clk;
	unsigned int dmaClkFreqHz;
	unsigned int hcpClkFreqHz;
	HAP_freq_match_type freqMatch;
	HAP_power_bus_bw dma_ext;
	HAP_power_bus_bw hcp_ext;
	HAP_power_bus_bw dma_int;
	HAP_power_bus_bw hcp_int;
} HAP_power_vapss_payload_v2;

/**
* Payload for HAP_power_set_HVX

* @param power_up	-	Set to TRUE to turn on HVX, and FALSE to turn off.
*/
typedef struct {
	boolean power_up;
} HAP_power_hvx_payload;

/**
* Payload for HAP_power_set_apptype

* @param power_up	-	Set to TRUE to turn on HVX, and FALSE to turn off.
*/
typedef enum {
	HAP_POWER_UNKNOWN_CLIENT_CLASS			= 0x00,		/**< Unknown client class */
	HAP_POWER_AUDIO_CLIENT_CLASS			= 0x01,		/**< Audio client class */
	HAP_POWER_VOICE_CLIENT_CLASS			= 0x02,		/**< Voice client class */
	HAP_POWER_COMPUTE_CLIENT_CLASS			= 0x04,		/**< Compute client class */
	HAP_POWER_STREAMING_1HVX_CLIENT_CLASS	= 0x08,		/**< Camera streaming with 1 HVX client class */
	HAP_POWER_STREAMING_2HVX_CLIENT_CLASS = 0x10,		/**< Camera streaming with 2 HVX client class */
} HAP_power_app_type_payload;

/**
* Payload for HAP_power_set_linelock

* @param startAddress		-		Start address of the memory region to be locked.
* @param size				-		Size (bytes) of the memory region to be locked. Set size to
*									to 0 to unlock memory.
*
* These throttling parameters; applicable only when enabling line locking.
* Please note that only ONE throttled linelock call is supported at this time.
* You can linelock additional regions (without throttling) using HAP_power_set_linelock_nothrottle
*
* @param throttleBlockSize	-		Block size for throttling, in bytes;
* 									0 for no throttling.  The region to be locked will be divided into
*									blocks of this size for throttling purposes.
* @param throttlePauseUs	-		Pause to be applied between locking each block, in microseconds.
*/
typedef struct {
	void* startAddress;
	uint32 size;
	uint32 throttleBlockSize;
	uint32 throttlePauseUs;
} HAP_power_linelock_payload;

/**
* Payload for HAP_power_set_linelock_nothrottle

* @param startAddress		-		Start address of the memory region to be locked.
* @param size				-		Size (bytes) of the memory region to be locked. Set size to 0
*									to unlock memory
*/
typedef struct {
	void* startAddress;
	uint32 size;
} HAP_power_linelock_nothrottle_payload;

/**
* Option for dcvs payload
**/
typedef enum {
	HAP_DCVS_ADJUST_UP_DOWN =   0x1,
	HAP_DCVS_ADJUST_ONLY_UP =   0x2,
} HAP_power_dcvs_payload_option;

/**
* Payload for HAP_power_set_DCVS

* @param dvcs_enable		-		Set to TRUE to participate in DCVS, and FALSE otherwise.
* @param dvcs_option		-		Set to one of
*		HAP_DCVS_ADJUST_UP_DOWN  - Allows for DCVS to adjust up and down.
*		HAP_DCVS_ADJUST_ONLY_UP  - Allows for DCVS to adjust up only.
*/
typedef struct {
	boolean dcvs_enable;
	HAP_power_dcvs_payload_option dcvs_option;
} HAP_power_dcvs_payload;

/**
* Voltage corners for HAP DCVS V2 interface
*/
typedef enum {
	HAP_DCVS_VCORNER_DISABLE,
	HAP_DCVS_VCORNER_SVS2,
	HAP_DCVS_VCORNER_SVS,
	HAP_DCVS_VCORNER_SVS_PLUS,
	HAP_DCVS_VCORNER_NOM,
	HAP_DCVS_VCORNER_NOM_PLUS,
	HAP_DCVS_VCORNER_TURBO,
	HAP_DCVS_VCORNER_TURBO_PLUS,
	HAP_DCVS_VCORNER_MAX = 255,
} HAP_dcvs_voltage_corner_t;

#define HAP_DCVS_VCORNER_SVSPLUS HAP_DCVS_VCORNER_SVS_PLUS
#define HAP_DCVS_VCORNER_NOMPLUS HAP_DCVS_VCORNER_NOM_PLUS

/**
* DCVS parameters for HAP_power_dcvs_v2_payload
* @param target_corner	-	target voltage corner
* @param min_corner		-	minimum voltage corner
* @param max_corner		-	maximum voltage corner
* @param param1			-	reserved
* @param param2			-	reserved
* @param param3			-	reserved
*/
typedef struct {
	HAP_dcvs_voltage_corner_t target_corner;
	HAP_dcvs_voltage_corner_t min_corner;
	HAP_dcvs_voltage_corner_t max_corner;
	uint32 param1;
	uint32 param2;
	uint32 param3;
} HAP_dcvs_params_t;

/**
* Core clock parameters for HAP_power_dcvs_v3_payload
* @param target_corner	-	target voltage corner
* @param min_corner		-	minimum voltage corner
* @param max_corner		-	maximum voltage corner
* @param param1			-	reserved
* @param param2			-	reserved
* @param param3			-	reserved
*/
typedef struct {
	HAP_dcvs_voltage_corner_t target_corner;
	HAP_dcvs_voltage_corner_t min_corner;
	HAP_dcvs_voltage_corner_t max_corner;
	uint32 param1;
	uint32 param2;
	uint32 param3;
} HAP_core_params_t;

/**
* Bus clock parameters for HAP_power_dcvs_v3_payload
* @param target_corner	-	target voltage corner
* @param min_corner		-	minimum voltage corner
* @param max_corner		-	maximum voltage corner
* @param param1			-	reserved
* @param param2			-	reserved
* @param param3			-	reserved
*/
typedef struct {
	HAP_dcvs_voltage_corner_t target_corner;
	HAP_dcvs_voltage_corner_t min_corner;
	HAP_dcvs_voltage_corner_t max_corner;
	uint32 param1;
	uint32 param2;
	uint32 param3;
} HAP_bus_params_t;

/**
* DCVS v3 parameters for HAP_power_dcvs_v3_payload
* @param param1	-	reserved
* @param param2	-	reserved
* @param param3	-	reserved
* @param param4	-	reserved
* @param param5	-	reserved
* @param param6	-	reserved
*/
typedef struct {
	uint32 param1;
	uint32 param2;
	uint32 param3;
	uint32 param4;
	uint32 param5;
	uint32 param6;
} HAP_dcvs_v3_params_t;

/**
* option for dcvs_v2 payload
**/
typedef enum {
	HAP_DCVS_V2_ADJUST_UP_DOWN =   0x1,
	HAP_DCVS_V2_ADJUST_ONLY_UP =   0x2,
	HAP_DCVS_V2_POWER_SAVER_MODE = 0x4,
	HAP_DCVS_V2_POWER_SAVER_AGGRESSIVE_MODE = 0x8,
	HAP_DCVS_V2_PERFORMANCE_MODE = 0x10,
	HAP_DCVS_V2_DUTY_CYCLE_MODE = 0x20,
} HAP_power_dcvs_v2_payload_option;

/**
* Payload for HAP_power_set_DCVS_v2

* @param dvcs_enable		-	Set to TRUE to participate in DCVS, and FALSE otherwise.
* @param dvcs_option		-	Set to one of
*		HAP_DCVS_ADJUST_UP_DOWN					-	Allows for DCVS to adjust up and down.
*		HAP_DCVS_ADJUST_ONLY_UP					-	Allows for DCVS to adjust up only.
*		HAP_DCVS_POWER_SAVER_MODE				-	Higher thresholds for power efficiency.
*		HAP_DCVS_POWER_SAVER_AGGRESSIVE_MODE	-	Higher thresholds for power efficiency with faster ramp down.
*		HAP_DCVS_PERFORMANCE_MODE				-	Lower thresholds for maximum performance
*		HAP_DCVS_DUTY_CYCLE_MODE				-	only for HVX based clients.
*													For streaming class clients:
*														> detects periodicity based on HVX usage
*														> lowers clocks in the no HVX activity region of each period.
*													For compute class clients:
*														> Lowers clocks on no HVX activity detects and brings clocks up on detecting HVX activity again.
*														> Latency involved in bringing up the clock with be at max 1 to 2 ms.
* @param set_latency		-	TRUE to set latency parameter, otherwise FALSE
* @param latency			-	sleep latency
* @param set_dcvs_params	-	TRUE to set DCVS params, otherwise FALSE
* @param dcvs_params		-	DCVS parameters
*/
typedef struct {
	boolean dcvs_enable;
	HAP_power_dcvs_v2_payload_option dcvs_option;
	boolean set_latency;
	uint32 latency;
	boolean set_dcvs_params;
	HAP_dcvs_params_t dcvs_params;
} HAP_power_dcvs_v2_payload;


/**
* Payload for HAP_power_set_DCVS_v3

* @param set_dvcs_enable    -	TRUE to consider DCVS enable/disable and option parameters, otherwise FALSE
* @param dvcs_enable		-	Set to TRUE to participate in DCVS, and FALSE otherwise.
* @param dvcs_option		-	Set to one of
*	HAP_DCVS_ADJUST_UP_DOWN					-	Allows for DCVS to adjust up and down.
*	HAP_DCVS_ADJUST_ONLY_UP					-	Allows for DCVS to adjust up only.
*	HAP_DCVS_POWER_SAVER_MODE				-	Higher thresholds for power efficiency.
*	HAP_DCVS_POWER_SAVER_AGGRESSIVE_MODE	-	Higher thresholds for power efficiency with faster ramp down.
*	HAP_DCVS_PERFORMANCE_MODE				-	Lower thresholds for maximum performance
*	HAP_DCVS_DUTY_CYCLE_MODE				-	only for HVX based clients.
*												For streaming class clients:
*													> detects periodicity based on HVX usage
*													> lowers clocks in the no HVX activity region of each period.
*												For compute class clients:
*													> Lowers clocks on no HVX activity detects and brings clocks up on detecting HVX activity again.
*													> Latency involved in bringing up the clock with be at max 1 to 2 ms.
* @param set_latency		-	TRUE to consider latency parameter, otherwise FALSE
* @param latency			-	sleep latency
* @param set_core_params	-	TRUE to consider core clock params, otherwise FALSE
* @param core_params		-	Core clock parameters
* @param set_bus_params	    -	TRUE to consider bus clock params, otherwise FALSE
* @param bus_params		    -	Bus clock parameters
* @param set_dcvs_v3_params	-	TRUE to consider DCVS v3 params, otherwise FALSE
* @param dcvs_v3_params		-	DCVS v3 parameters
* @param set_sleep_disable	-	TRUE to consider sleep disable/enable parameter, otherwise FALSE
* @param sleep_disable	    -	TRUE to disable sleep/LPM modes, FALSE to enable
*/
typedef struct {
	boolean set_dcvs_enable;
	boolean dcvs_enable;
	HAP_power_dcvs_v2_payload_option dcvs_option;
	boolean set_latency;
	uint32 latency;
	boolean set_core_params;
	HAP_core_params_t core_params;
	boolean set_bus_params;
	HAP_bus_params_t bus_params;
	boolean set_dcvs_v3_params;
	HAP_dcvs_v3_params_t dcvs_v3_params;
	boolean set_sleep_disable;
	boolean sleep_disable;
} HAP_power_dcvs_v3_payload;


/** @enum for dcvs update request type */
typedef enum {
	HAP_POWER_UPDATE_DCVS = 1,
	HAP_POWER_UPDATE_SLEEP_LATENCY,
	HAP_POWER_UPDATE_DCVS_PARAMS,
} HAP_power_update_type_t;

/* Payload for DCVS update */
typedef struct {
	boolean dcvs_enable;							/**< TRUE for DCVS enable and FALSE for DCVS disable */
	HAP_power_dcvs_v2_payload_option dcvs_option;	/**< Requested DCVS policy in case DCVS enable is TRUE */
} HAP_power_update_dcvs_t;

/* Payload for latency update */
typedef struct {
	boolean set_latency;	/**< TRUE if sleep latency request has to be considered */
	unsigned int latency;	/**< Sleep latency request in micro seconds */
} HAP_power_update_latency_t;

/* Payload for DCVS params update */
typedef struct {
    boolean set_dcvs_params;		/**< Flag to mark DCVS params structure validity, TRUE for valid DCVS
										params request and FALSE otherwise */
    HAP_dcvs_params_t dcvs_params;	/**< Intended DCVS params if set_dcvs_params is set to TRUE */
} HAP_power_update_dcvs_params_t;

/**
* Payload for HAP_power_set_DCVS_v2

* @param update_param		-	Type for which param to update
* @param update payload		-	Update payload for DCVS, latency or DCVS params
*/
typedef struct {
	HAP_power_update_type_t update_param;
	union {
		HAP_power_update_dcvs_t dcvs_payload;
		HAP_power_update_latency_t latency_payload;
		HAP_power_update_dcvs_params_t dcvs_params_payload;
	};
} HAP_power_dcvs_v2_update_payload;

/**
* Payload for HAP_power_set_streamer

* @param set_streamer0_clk		-	Set streamer 0 clock
* @param set_streamer1_clk		-	Set streamer 1 clock
* @param streamer0_clkFreqHz	-	Streamer 0 clock frequency
* @param streamer1_clkFreqHz	-	Streamer 1 clock frequency
* @param freqMatch				-	Clock frequency match
* @param paramX					-	Reserved for future streamer parameters
*/
typedef struct {
	boolean set_streamer0_clk;
	boolean set_streamer1_clk;
	unsigned int streamer0_clkFreqHz;
	unsigned int streamer1_clkFreqHz;
	HAP_freq_match_type freqMatch;
	uint32 param1;
	uint32 param2;
	uint32 param3;
} HAP_power_streamer_payload;

/**
* Identifies the HAP power request type
* @param HAP_power_set_mips_bw		-	Requests for MIPS. Provides
*										fine-grained control to set MIPS values.
*										Payload is set to HAP_power_payload
* @param HAP_power_set_HVX			-	Requests to enable / disable HVX
*										Payload is set to HAP_power_hvx_payload
* @param HAP_power_set_apptype		-	Sets the app_type
*										Payload is set to HAP_power_app_type_payload
* @param HAP_power_set_linelock		-	Sets the throttled L2 cache line locking parameters.
*										Only one throttled call is supported at this time. Additional
										un-throttled line-locks can be performed using HAP_power_set_linelock_nothrottle
*										Payload is set to HAP_power_linelock_payload
* @param HAP_power_set_DCVS			-	Requests to participate / stop participating in DCVS
*										Payload is set to HAP_power_dcvs_payload
* @param HAP_power_set_linelock_nothrottle	-	Sets the L2 cache line locking parameters (non-throttled).
*												Payload is set to HAP_power_linelock_nothrottle_payload
* @param HAP_power_set_vapss		-	Sets the VAPSS core clock and DDR/IPNOC bandwidth
*										Payload is set to HAP_power_vapss_payload
* @param HAP_power_set_vapss_v2		-	Sets the VAPSS core DMA/HCP clocks and DDR/IPNOC bandwidths
*										Payload is set to HAP_power_vapss_payload_v2
* @param HAP_power_set_dcvs_v2_update	-	Updates DCVS params
*											Payload is set to HAP_power_dcvs_v2_update_payload
* @param HAP_power_set_streamer		-	Sets the streamer core clocks
*										Payload is set to HAP_power_streamer_payload
*/
typedef enum {
	HAP_power_set_mips_bw = 1,
	HAP_power_set_HVX,
	HAP_power_set_apptype,
	HAP_power_set_linelock,
	HAP_power_set_DCVS,
	HAP_power_set_linelock_nothrottle,
	HAP_power_set_DCVS_v2,
	HAP_power_set_vapss,
	HAP_power_set_vapss_v2,
	HAP_power_set_dcvs_v2_update,
	HAP_power_set_streamer,
	HAP_power_set_DCVS_v3,
} HAP_Power_request_type;

/**
* Data type to change power values on the ADSP
* @param type					-	Identifies the request type.
* @param mips_bw				-	Requests for performance level.
* @param hvx					-	Requests to enable / disable HVX
* @param apptype				-	Sets the app_type
* @param linelock				-	Sets the throttled L2 cache linelock parameters. Only one
*									throttled linelock is permitted at this time. Additional
*									un-throttled linelocks can be performed using linelock_nothrottle.
* @param hvx					-	Requests to participate / stop participating in DCVS
* @param linelock_nothrottle	-	Sets the un-throttled L2 cache linelock parameters.
* @param vapss					-	Sets the VAPSS core clock and DDR/IPNOC bandwidth.
* @param vapss_v2				-	Sets the VAPSS core DMA/HCP clocks and DDR/IPNOC bandwidths.
* @param streamer				-	Sets the streamer core clocks.
* @param dcvs_v2_update			-	Updates DCVS params
*/
typedef struct {
	HAP_Power_request_type type;
	union{
		HAP_power_mips_bw_payload mips_bw;
		HAP_power_vapss_payload vapss;
		HAP_power_vapss_payload_v2 vapss_v2;
		HAP_power_streamer_payload streamer;
		HAP_power_hvx_payload hvx;
		HAP_power_app_type_payload apptype;
		HAP_power_linelock_payload linelock;
		HAP_power_dcvs_payload dcvs;
		HAP_power_dcvs_v2_payload dcvs_v2;
		HAP_power_dcvs_v2_update_payload dcvs_v2_update;
		HAP_power_linelock_nothrottle_payload linelock_nothrottle;
		HAP_power_dcvs_v3_payload dcvs_v3;
	};
} HAP_power_request_t;

/**
* Method to set power values from the ADSP
* @param context	-	To identify the power client
* @param request	-	Request params.
* @retval 0 on success, AEE_EMMPMREGISTER on MMPM client register request failure, -1 on unknown error
*/
int HAP_power_set(void* context, HAP_power_request_t* request);

/**
* Identifies the HAP power response type
* @param HAP_power_get_max_mips				-	Returns the max mips supported (max_mips)
* @param HAP_power_get_max_bus_bw			-	Returns the max bus bandwidth supported (max_bus_bw)
* @param HAP_power_get_client_class			-	Returns the client class (client_class)
* @param HAP_power_get_clk_Freq				-	Returns the core clock frequency (clkFreqHz)
* @param HAP_power_get_aggregateAVSMpps		-	Returns the aggregate Mpps used by audio and voice (clkFreqHz)
* @param HAP_power_get_dcvsEnabled			-	Returns the dcvs status (enabled / disabled).
* @param HAP_power_get_vapss_core_clk_Freq	-	Returns the VAPSS core clock frequency (clkFreqHz)
* @param HAP_power_get_dma_core_clk_Freq	-	Returns the DMA core clock frequency (clkFreqHz)
* @param HAP_power_get_hcp_core_clk_Freq	-	Returns the HCP core clock frequency (clkFreqHz)
* @param HAP_power_get_streamer0_core_clk_Freq	-	Returns the streamer 0 core clock frequency (clkFreqHz)
* @param HAP_power_get_streamer1_core_clk_Freq	-	Returns the streamer 1 core clock frequency (clkFreqHz)
*/
typedef enum {
	HAP_power_get_max_mips = 1,
	HAP_power_get_max_bus_bw,
	HAP_power_get_client_class,
	HAP_power_get_clk_Freq,
	HAP_power_get_aggregateAVSMpps,
	HAP_power_get_dcvsEnabled,
	HAP_power_get_vapss_core_clk_Freq,
	HAP_power_get_dma_core_clk_Freq,
	HAP_power_get_hcp_core_clk_Freq,
	HAP_power_get_streamer0_core_clk_Freq,
	HAP_power_get_streamer1_core_clk_Freq,
} HAP_Power_response_type;

/**
* Data type to retrieve power values from the ADSP
* @param type				-	Identifies the type to retrieve.
* @param max_mips			-	Max mips supported
* @param max_bus_bw			-	Max bus bw supported
* @param client_class		-	Current client class
* @param clkFreqHz			-	Current core CPU frequency
* @param aggregateAVSMpps	-	Aggregate AVS Mpps used by audio and voice
* @param dcvsEnabled		-	Indicates if dcvs is enabled / disabled.
*/
typedef struct {
	HAP_Power_response_type type;
	union{
		unsigned int max_mips;
		uint64 max_bus_bw;
		unsigned int client_class;
		unsigned int clkFreqHz;
		unsigned int aggregateAVSMpps;
		boolean dcvsEnabled;
	};
} HAP_power_response_t;

/**
* Method to retrieve power values from the ADSP
* @param context	-	Ignored
* @param request	-	Response.
*/
int HAP_power_get(void* context, HAP_power_response_t* response);

/**
* Method to initialize dcvs v3 structure in request param. It enables
*		flags and resets params for all fields in dcvs v3. So, this
*		can also be used to remove applied dcvs v3 params and restore
*		defaults.
* @param request	-	Pointer to request params.
*/
static inline void HAP_power_set_dcvs_v3_init(HAP_power_request_t* request) {
	memset(request, 0, sizeof(HAP_power_request_t) );
	request->type = HAP_power_set_DCVS_v3;
	request->dcvs_v3.set_dcvs_enable = TRUE;
	request->dcvs_v3.dcvs_enable = TRUE;
	request->dcvs_v3.dcvs_option = HAP_DCVS_V2_POWER_SAVER_MODE;
	request->dcvs_v3.set_latency = TRUE;
	request->dcvs_v3.latency = 65535;
	request->dcvs_v3.set_core_params = TRUE;
	request->dcvs_v3.set_bus_params = TRUE;
	request->dcvs_v3.set_dcvs_v3_params = TRUE;
	request->dcvs_v3.set_sleep_disable = TRUE;
	return;
}

/**
* Method to enable/disable dcvs and set particular dcvs policy.
* @param context		-	User context.
* @param dcvs_enable	-	TRUE to enable dcvs, FALSE to disable dcvs.
* @param dcvs_option	-	To set particular dcvs policy. In case of dcvs disable
*                           request, this param will be ignored.
* @returns	-	0 on success
*/
static inline int HAP_power_set_dcvs_option(void* context, boolean dcvs_enable,
		HAP_power_dcvs_v2_payload_option dcvs_option) {
	HAP_power_request_t request;
	memset(&request, 0, sizeof(HAP_power_request_t) );
	request.type = HAP_power_set_DCVS_v3;
	request.dcvs_v3.set_dcvs_enable = TRUE;
	request.dcvs_v3.dcvs_enable = dcvs_enable;
	if(dcvs_enable)
		request.dcvs_v3.dcvs_option = dcvs_option;
	return HAP_power_set(context, &request);
}

/**
* Method to set/reset sleep latency.
* @param context	-	User context.
* @param latency	-	Sleep latency value in microseconds, should be > 1.
*						Use 65535 max value to reset it to default.
* @returns	-	0 on success
*/
static inline int HAP_power_set_sleep_latency(void* context, uint32 latency) {
	HAP_power_request_t request;
	memset(&request, 0, sizeof(HAP_power_request_t) );
	request.type = HAP_power_set_DCVS_v3;
	request.dcvs_v3.set_latency = TRUE;
	request.dcvs_v3.latency = latency;
	return HAP_power_set(context, &request);
}

/**
* Method to set/reset DSP core clock voltage corners.
* @param context		-	User context.
* @param target_corner	-	Target voltage corner.
* @param min_corner		-	Minimum voltage corner.
* @param max_corner		-	Maximum voltage corner.
* @returns	-	0 on success
*/
static inline int HAP_power_set_core_corner(void* context, uint32 target_corner,
		uint32 min_corner, uint32 max_corner) {
	HAP_power_request_t request;
	memset(&request, 0, sizeof(HAP_power_request_t) );
	request.type = HAP_power_set_DCVS_v3;
	request.dcvs_v3.set_core_params = TRUE;
	request.dcvs_v3.core_params.min_corner = (HAP_dcvs_voltage_corner_t) (min_corner);
	request.dcvs_v3.core_params.max_corner = (HAP_dcvs_voltage_corner_t) (max_corner);
	request.dcvs_v3.core_params.target_corner = (HAP_dcvs_voltage_corner_t) (target_corner);
	return HAP_power_set(context, &request);
}

/**
* Method to set/reset bus clock voltage corners.
* @param context		-	User context.
* @param target_corner	-	Target voltage corner.
* @param min_corner		-	Minimum voltage corner.
* @param max_corner		-	Maximum voltage corner.
* @returns	-	0 on success
*/
static inline int HAP_power_set_bus_corner(void* context, uint32 target_corner,
		uint32 min_corner, uint32 max_corner) {
    HAP_power_request_t request;
	memset(&request, 0, sizeof(HAP_power_request_t) );
	request.type = HAP_power_set_DCVS_v3;
	request.dcvs_v3.set_bus_params = TRUE;
	request.dcvs_v3.bus_params.min_corner = (HAP_dcvs_voltage_corner_t) (min_corner);
	request.dcvs_v3.bus_params.max_corner = (HAP_dcvs_voltage_corner_t) (max_corner);
	request.dcvs_v3.bus_params.target_corner = (HAP_dcvs_voltage_corner_t) (target_corner);
	return HAP_power_set(context, &request);
}

/**
* Method to disable/enable all low power modes.
* @param context		-	User context.
* @param sleep_disable	-	TRUE to disable all low power modes.
*							FALSE to re-enable all low power modes.
* @returns	-	0 on success
*/
static inline int HAP_power_set_sleep_mode(void* context, boolean sleep_disable) {
	HAP_power_request_t request;
	memset(&request, 0, sizeof(HAP_power_request_t) );
	request.type = HAP_power_set_DCVS_v3;
	request.dcvs_v3.set_sleep_disable = TRUE;
	request.dcvs_v3.sleep_disable = sleep_disable;
	return HAP_power_set(context, &request);
}

/***************************************************************************

These APIs are deprecated and might generate undesired results.
Please use the HAP_power_get() and HAP_power_set() APIs instead.

****************************************************************************/
/**
* Requests a performance level by percentage for clock speed
* and bus speed.  Passing 0 for any parameter results in no
* request being issued for that particular attribute.
* @param clock		-	percentage of target's maximum clock speed
* @param bus		-	percentage of target's maximum bus speed
* @param latency	-	maximum hardware wake up latency in microseconds.  The
*						higher the value the deeper state of sleep
*						that can be entered but the longer it may
*						take to awaken.
* @retval 0 on success
* @par Comments	:	Performance metrics vary from target to target so the
*					intent of this API is to allow callers to set a relative
*					performance level to achieve the desired balance between
*					performance and power saving.
*/
int HAP_power_request(int clock, int bus, int latency);

/**
* Requests a performance level by absolute values.  Passing 0
* for any parameter results in no request being issued for that
* particular attribute.
* @param clock		-	speed in MHz
* @param bus		-	bus speed in MHz
* @param latency	-	maximum hardware wakeup latency in microseconds.  The
*						higher the value the deeper state of
*						sleep that can be entered but the
*						longer it may take to awaken.
* @retval 0 on success
* @par Comments	:	This API allows callers who are aware of their target
*					specific capabilities to set them explicitly.
*/
int HAP_power_request_abs(int clock, int bus, int latency);

/**
* queries the target for its clock and bus speed capabilities
* @param clock_max	-	maximum clock speed supported in MHz
* @param bus_max	-	maximum bus speed supported in MHz
* @retval 0 on success
*/
int HAP_power_get_max_speed(int* clock_max, int* bus_max);

/**
* Upvote for HVX power
* @retval 0 on success
*/
int HVX_power_request(void);

/**
* Downvote for HVX power
* @retval 0 on success
*/
int HVX_power_release(void);

/**
* Method to destroy clients created through HAP_power_set
* @param context	-	To uniquely identify the client
* @retval 0 on success, AEE_ENOSUCHCLIENT on Invalid context, -1 on unknown error
* DO NOT call this API directly, use HAP_power_destroy instead.
*/
int HAP_power_destroy_client(void *context);

//Wrapper to HAP_power_destroy_client API
static inline int HAP_power_destroy(void *client){
	if(0 != HAP_power_destroy_client)
		return HAP_power_destroy_client(client);
	return AEE_EUNSUPPORTEDAPI;
}

#ifdef __cplusplus
}
#endif
#endif //_HAP_POWER_H

