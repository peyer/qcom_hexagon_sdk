# ===============================================================================
#
#  Copyright (c) 2013-2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#
# ===============================================================================

'''
Created on Nov 7, 2017

@author: rkonduri
'''

import binascii

from sectools.common.utils.c_logging import logger
from sectools.features.isc.signer.cass.signing_request_base import BaseCASSSigningRequest
from sectools.features.isc.signer.signerutils.certificate import Certificate
from sectools.features.isc.defines import SECBOOT_VERSION_3_0


class SecureBoot3SigningRequest(BaseCASSSigningRequest):

    DEBUG_TYPE = "DEBUG_TYPE"
    JTAG_ID = "JTAG_ID"
    ROT_ENABLE = "ROT_ENABLE"
    SOC_HW_VERSION_ENABLE = "SOC_HW_VERSION_ENABLE"
    SERIAL_NUMBER_IN_SIGNING_ENABLE = "SERIAL_NUMBER_IN_SIGNING_ENABLE"
    ROOT_REVOKE_ACTIVATION = "ROOT_REVOKE_ACTIVATION"
    UIE_KEY_SWITCH_ACTIVATION = "UIE_KEY_SWITCH_ACTIVATION"
    MULTI_SERIAL_NUMBERS = "MULTI_SERIAL_NUMBERS"
    IN_USE_SOC_HW_VERSION = "IN_USE_SOC_HW_VERSION"
    ANTI_ROLLBACK_VERSION = "ANTI_ROLLBACK_VERSION"
    ROOT_CERT_SEL = "ROOT_CERT_SEL"
    HASH_SEGMENT_HEADER = "HASH_SEGMENT_HEADER"
    HASH_SEGMENT_BODY = "HASH_SEGMENT_BODY"
    HASH_TABLE_HASH_ALGORITHM = "HASH_TABLE_HASH_ALGORITHM"
    METADATA_MAJOR_VERSION = "METADATA_MAJOR_VERSION"
    METADATA_MINOR_VERSION = "METADATA_MINOR_VERSION"
    SIGNING_CONTEXT = "SIGNING_CONTEXT"
    signing_cntxt = {
        ('sha1', 'rsa', 'pkcs'):    'SHA1-RSA',
        ('sha256', 'rsa', 'pkcs'):  'SHA256-RSA',
        ('sha384', 'rsa', 'pkcs'):  'SHA384-RSA',
        ('sha1', 'rsa', 'pss'):     'SHA1-RSAPSS-MGF1',
        ('sha256', 'rsa', 'pss'):   'SHA256-RSAPSS-MGF1',
        ('sha384', 'rsa', 'pss'):   'SHA384-RSAPSS-MGF1',
    }

    def __init__(self, hash_to_sign, data_to_sign, description, signing_attributes):
        logger.debug("Creating CASS Secure Boot 3 signing request...")
        self.signing_attributes = signing_attributes
        if data_to_sign is None:
            raise RuntimeError("Hash Segment is required for CASS Secure Boot 3 signing request")
        self.data_to_sign = data_to_sign
        BaseCASSSigningRequest.__init__(self, hash_to_sign, data_to_sign, description, signing_attributes)

    @classmethod
    def is_plugin(cls):
        return True

    @classmethod
    def get_plugin_id(cls):
        return "secure_boot_3_cass_signing_request"

    @classmethod
    def get_supported_capabilities(cls):
        return [
                "xblsec_prod_v2_sdx24_release",
                "xblsec_prod_v2_presilicon",
                "qtee_prod_v2_sdx24m_debugpolicy_release",
                "qtee_prod_v2_qcs6200_debugpolicy_release",
                "qtee_prod_v2_presilicon",
                "qtee_test_v2_qtee_release",
                "qtee_test_v2_presilicon"
                ]

    @classmethod
    def get_secboot_version(cls):
        return SECBOOT_VERSION_3_0

    def _create_attributes_to_add(self):
        additions = {}
        sa = self.signing_attributes

        # required attribute
        # JTAG_ID
        additions.update({SecureBoot3SigningRequest.JTAG_ID: sa.msm_part})
        logger.debug("Using msm_part for JTAG_ID value.")

        # Note: General Rules while handling Optional Attributes:
        # If the optional attribute is BOOL, if it is set to 1, it is set to TRUE, else (including when it is None) FALSE
        # If the optional attribute is non-BOOL, if it is None, it is set to 0x0, else value provided provided the value is acceptable
        # If the optional attribute is None, it is sent as part of Signing Request - CASS server follows the rules stated as above

        # optional attribute
        # SOC_VERS
        if sa.soc_vers is not None:
            soc_vers = [soc_ver[2:] for soc_ver in sa.soc_vers]
            additions.update({Certificate.SIGNATTR_SOC_VERS: ",".join(soc_vers)})

        # optional attribute
        # SOC_HW_VERSION_ENABLE
        if sa.in_use_soc_hw_version == 1:
            additions.update({SecureBoot3SigningRequest.SOC_HW_VERSION_ENABLE: "TRUE"})
        else:
            additions.update({SecureBoot3SigningRequest.SOC_HW_VERSION_ENABLE: "FALSE"})

        # optional attribute
        # SERIAL_NUMBER_IN_SIGNING_ENABLE
        if sa.use_serial_number_in_signing == 1:
            additions.update({SecureBoot3SigningRequest.SERIAL_NUMBER_IN_SIGNING_ENABLE: "TRUE"})
        else:
            additions.update({SecureBoot3SigningRequest.SERIAL_NUMBER_IN_SIGNING_ENABLE: "FALSE"})

        # optional attribute
        # ROT_ENABLE
        if sa.rot_en == 1:
            additions.update({SecureBoot3SigningRequest.ROT_ENABLE: "TRUE"})
        else:
            additions.update({SecureBoot3SigningRequest.ROT_ENABLE: "FALSE"})

        # optional attribute
        # OEM_ID_INDEPENDENT
        if sa.oem_id_independent == 1:
            additions.update({Certificate.SIGNATTR_OEM_ID_INDEPENDENT: "TRUE"})
        else:
            additions.update({Certificate.SIGNATTR_OEM_ID_INDEPENDENT: "FALSE"})

        # optional attribute
        # MULTI_SERIAL_NUMBERS
        if sa.multi_serial_numbers is not None and len(sa.multi_serial_numbers.serial) > 0:
            serials = [serial[2:] for serial in sa.multi_serial_numbers.serial]
            additions.update({SecureBoot3SigningRequest.MULTI_SERIAL_NUMBERS: ",".join(serials)})

        # optional attribute
        # DEBUG_TYPE
        if sa.debug is not None: # no need to check valid values as it has been done earlier
            additions.update({SecureBoot3SigningRequest.DEBUG_TYPE: "0x" + sa.debug[-1]})

        # optional attribute
        # ROOT_REVOKE_ACTIVATION
        if sa.root_revoke_activate_enable is not None:
            additions.update({SecureBoot3SigningRequest.ROOT_REVOKE_ACTIVATION: sa.root_revoke_activate_enable})

        # optional attribute
        # UIE_KEY_SWITCH_ACTIVATION
        if sa.uie_key_switch_enable is not None:
            additions.update({SecureBoot3SigningRequest.UIE_KEY_SWITCH_ACTIVATION: sa.uie_key_switch_enable})

        # optional attribute
        # ANTI_ROLLBACK_VERSION
        if sa.anti_rollback_version is not None:
            additions.update({SecureBoot3SigningRequest.ANTI_ROLLBACK_VERSION: sa.anti_rollback_version})

        # optional attribute
        # ROOT_CERT_SEL
        # this needs to be converted to hex as CASS expects in the same format
        if sa.mrc_index is not None:
            additions.update({SecureBoot3SigningRequest.ROOT_CERT_SEL: hex(sa.mrc_index)})

        # required attribute
        # HASH_SEGMENT_HEADER & HASH_SEGMENT_BODY
        hash_segment_str = binascii.hexlify(self.data_to_sign)
        if hash_segment_str >= 96:
            additions.update({SecureBoot3SigningRequest.HASH_SEGMENT_HEADER: hash_segment_str[:96]})
            additions.update({SecureBoot3SigningRequest.HASH_SEGMENT_BODY: hash_segment_str[96:]})
        else:
            raise RuntimeError("Size of Hash Segment should not be less than 48 bytes")

        # required attribute
        # HASH_TABLE_HASH_ALGORITHM
        if sa.segment_hash_algorithm is not None:
            additions.update({SecureBoot3SigningRequest.HASH_TABLE_HASH_ALGORITHM: sa.segment_hash_algorithm.upper()})

        # required attributes
        # METADATA_MAJOR_VERSION & METADATA_MINOR_VERSION
        # only supported values as of now are 0x0 & 0x0
        additions.update({SecureBoot3SigningRequest.METADATA_MAJOR_VERSION: '0x0'})
        additions.update({SecureBoot3SigningRequest.METADATA_MINOR_VERSION: '0x0'})

        # required attribute
        # SIGNING_CONTEXT
        # Note: SHA256 + PSS supported for Chiron and Hana
        if (sa.hash_algorithm, sa.dsa_type, sa.rsa_padding) not in SecureBoot3SigningRequest.signing_cntxt.keys():
            raise RuntimeError("CASS Secure Boot 3 signing request only support RSA signing.")
        else:
            additions.update({SecureBoot3SigningRequest.SIGNING_CONTEXT: SecureBoot3SigningRequest.signing_cntxt[(sa.hash_algorithm, sa.dsa_type, sa.rsa_padding)]})

        return additions

    def _create_attributes_to_override(self):
        pass

    def _create_attributes_to_remove(self):
        removals = list()

        # HW_ID
        removals.append(Certificate.SIGNATTR_HW_ID)
        # CRASH_DUMP
        if self.signing_attributes.crash_dump is not None:
            removals.append(Certificate.SIGNATTR_CRASH_DUMP)
        # SHA256
        removals.append(Certificate.SIGNATTR_SHA256)
        # SW_SIZE
        removals.append(Certificate.SIGNATTR_SW_SIZE)

        return removals
