#include <AEEStdErr.h>
#include <HAP_perf.h>
#include <HAP_power.h>
#include <string.h>
#include <HAP_farf.h>

/* Clock configuration settings*/

AEEResult image_dspq_test_set_clocks(void) {

    int err;

    HAP_setFARFRuntimeLoggingParams(0x1f, NULL, 0);
    // Set client class
    HAP_power_request_t request;
    memset(&request, 0, sizeof(HAP_power_request_t));
    request.type = HAP_power_set_apptype;
    request.apptype = HAP_POWER_COMPUTE_CLIENT_CLASS;
    if ( (err = HAP_power_set(NULL, &request)) != 0 ) {
        return err;
    }

    // Set to turbo and disable DCVS
    memset(&request, 0, sizeof(HAP_power_request_t));
    request.type = HAP_power_set_DCVS_v2;
    request.dcvs_v2.dcvs_enable = FALSE;
    request.dcvs_v2.set_dcvs_params = TRUE;
    request.dcvs_v2.dcvs_params.min_corner = HAP_DCVS_VCORNER_DISABLE;
    request.dcvs_v2.dcvs_params.max_corner = HAP_DCVS_VCORNER_DISABLE;
    request.dcvs_v2.dcvs_params.target_corner = HAP_DCVS_VCORNER_TURBO;
    request.dcvs_v2.set_latency = TRUE;
    request.dcvs_v2.latency = 100;
    if ( (err = HAP_power_set(NULL, &request)) != 0 ) {
        return err;
    }

    // Vote for HVX power
    memset(&request, 0, sizeof(HAP_power_request_t));
    request.type = HAP_power_set_HVX;
    request.hvx.power_up = TRUE;
    return HAP_power_set(NULL, &request);
}
