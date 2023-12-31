# ===============================================================================
#
#  Copyright (c) 2013-2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#
# ===============================================================================

'''
Created on Jan 28, 2016

@author: coryf
'''

from sectools.common.utils.c_logging import logger
from sectools.features.isc.signer.cass.signing_request_base import BaseCASSSigningRequest
from sectools.features.isc.signer.signerutils.certificate import Certificate
from sectools.features.isc.defines import SECBOOT_VERSION_2_0


class SecureBoot2SigningRequest(BaseCASSSigningRequest):

    AUTH_SN = "AUTH_SN"
    DEBUG_SERIAL = "DEBUG_SERIAL"
    DEBUG_TYPE = "DEBUG_TYPE"
    HARDWARE_ID = "HARDWARE_ID"

    def __init__(self, hash_to_sign, data_to_sign, image_path, signing_attributes):
        logger.debug("Creating CASS Secure Boot 2 signing request...")
        self.signing_attributes = signing_attributes
        BaseCASSSigningRequest.__init__(self, hash_to_sign, data_to_sign, image_path, signing_attributes)

    @classmethod
    def is_plugin(cls):
        return True

    @classmethod
    def get_plugin_id(cls):
        return "secure_boot_2_cass_signing_request"

    @classmethod
    def get_supported_capabilities(cls):
        return ["qsee_production_debugpolicy",
                "qsee_production_nazgul",
                "qsee_test_nazgul",
                "qsee_test_starlord",
                "qsee_test_napali",
                "spss_test_nazgul",
                "xblsec_nazgul",
                "xblsec_presilicon",
                "qsee_test_presilicon",
                "qsee_prod_presilicon",
                "spss_test_presilicon",
                "spss_prod_nazgul",
                "spss_prod_nazgul_with_serial_number",
                "quartz_patch",
                "quartz_patch_with_serial",
                "secboot_sha2_pss_subca1"]

    @classmethod
    def get_secboot_version(cls):
        return SECBOOT_VERSION_2_0

    def _create_attributes_to_add(self):
        additions = {}
        sa = self.signing_attributes

        # Add required attributes

        # HARDWARE_ID
        if sa.in_use_soc_hw_version == 1:
            if sa.soc_hw_version is not None:
                additions.update({SecureBoot2SigningRequest.HARDWARE_ID: sa.soc_hw_version})
                logger.debug("Using soc_hw_version for HARDWARE_ID value.")
            else:
                raise RuntimeError("CASS Secure Boot 2 Signing Request requires a soc_hw_version value when in_use_soc_hw_version is set to 1.")
        else:
            if sa.msm_part is not None:
                additions.update({SecureBoot2SigningRequest.HARDWARE_ID: sa.msm_part})
                logger.debug("Using msm_part for HARDWARE_ID value.")
            else:
                raise RuntimeError("CASS Secure Boot 2 Signing Request requires an msm_part value when in_use_soc_hw_version is not set to 1.")

        # Add optional attributes

        # SOC_VERS
        if sa.soc_vers:
            soc_vers = [soc_ver[2:] for soc_ver in sa.soc_vers]
            additions.update({Certificate.SIGNATTR_SOC_VERS: ",".join(soc_vers)})

        # IN_USE_SOC_HW_VERSION
        if sa.in_use_soc_hw_version is not None:
            if sa.in_use_soc_hw_version == 1:
                additions.update({Certificate.SIGNATTR_IN_USE_SOC_HW_VERSION: "TRUE"})
            elif sa.in_use_soc_hw_version == 0:
                additions.update({Certificate.SIGNATTR_IN_USE_SOC_HW_VERSION: "FALSE"})
            else:
                raise RuntimeError("in_use_soc_hw_version value of {0} is invalid for CASS Secure Boot 2 Signing Request.".format(sa.in_use_soc_hw_version))
        else:
            additions.update({Certificate.SIGNATTR_IN_USE_SOC_HW_VERSION: "FALSE"})

        # USE_SERIAL_NUMBER_IN_SIGNING
        if sa.use_serial_number_in_signing is not None:
            if sa.use_serial_number_in_signing == 1:
                additions.update({Certificate.SIGNATTR_USE_SERIAL_NUMBER_IN_SIGNING: "TRUE"})
            elif sa.use_serial_number_in_signing == 0:
                additions.update({Certificate.SIGNATTR_USE_SERIAL_NUMBER_IN_SIGNING: "FALSE"})
            else:
                raise RuntimeError("use_serial_number_in_signing value of {0} is invalid for CASS Secure Boot 2 Signing Request.".format(sa.use_serial_number_in_signing))
        else:
            additions.update({Certificate.SIGNATTR_USE_SERIAL_NUMBER_IN_SIGNING: "FALSE"})

        # OEM_ID_INDEPENDENT
        if sa.oem_id_independent is not None:
            if sa.oem_id_independent == 1:
                additions.update({Certificate.SIGNATTR_OEM_ID_INDEPENDENT: "TRUE"})
            elif sa.oem_id_independent == 0:
                additions.update({Certificate.SIGNATTR_OEM_ID_INDEPENDENT: "FALSE"})
            else:
                raise RuntimeError("oem_id_independent value of {0} is invalid for CASS Secure Boot 2 Signing Request.".format(sa.oem_id_independent))

        # AUTH_SN
        if sa.multi_serial_numbers is not None and len(sa.multi_serial_numbers.serial) > 0:
            serials = [serial[2:] for serial in sa.multi_serial_numbers.serial]
            additions.update({SecureBoot2SigningRequest.AUTH_SN: ",".join(serials)})

        # DEBUG_SERIAL
        # DEBUG_TYPE
        if sa.debug is not None:
            additions.update({SecureBoot2SigningRequest.DEBUG_SERIAL: sa.debug[0:10]})
            additions.update({SecureBoot2SigningRequest.DEBUG_TYPE: "0x" + sa.debug[10:18]})

        # This values is fixed in server profile and should not be sent as part of the request
        # EXPONENT
        # if sa.exponent is not None:
        #     additions.update({BaseCASSSigningRequest.EXPONENT: hex(sa.exponent)})

        # UIE_KEY_SWITCH_ENABLE
        if sa.uie_key_switch_enable is not None:
            additions.update({Certificate.SIGNATTR_UIE_KEY_SWITCH_ENABLE: sa.uie_key_switch_enable})

        return additions

    def _create_attributes_to_override(self):
        overrides = dict()

        # Split serial_number between OEM_ID AND MODEL_ID
        # This is required because CASS API does not allow serial_number to be sent to the server
        if self.signing_attributes.use_serial_number_in_signing == 1:
            serial = "{0:#0{1}x}".format(int(self.signing_attributes.serial_number, 16), 10)
            # OEM_ID
            overrides[Certificate.SIGNATTR_OEM_ID] = {Certificate.SIGNATTR_OEM_ID: serial[:6]}
            # MODEL_ID
            overrides[Certificate.SIGNATTR_MODEL_ID] = {Certificate.SIGNATTR_MODEL_ID: "0x" + serial[6:]}

        return overrides

    def _create_attributes_to_remove(self):
        removals = list()

        # HW_ID
        removals.append(Certificate.SIGNATTR_HW_ID)

        return removals
