# ===============================================================================
#
#  Copyright (c) 2013-2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#
# ===============================================================================

import binascii
import math
import os
import struct
import subprocess

from sectools.common import crypto
from sectools.common.utils.c_logging import logger
from sectools.common.utils.c_misc import create_mismatch_table
from sectools.common.utils.c_misc import hexdump
from sectools.common.utils.c_misc import store_debug_data_to_file
from sectools.features.isc import defines


class EncryptionParamsSectionBody(object):

    RSVD_BYTE_LEN_BYTES = 1
    ENCRYPTED_KEY_PAYLOAD_LEN_BYTES = 16
    MAC_LEN_BYTES = 16
    BASE_IV_LEN_BYTES = 16

    class B0block(object):

        B0_BLOCK_SIZE_BYTES = 16

        L1_B0_FLG_FLD_LEN_BYTES = 1
        L1_IV_FLD_LEN_BYTES = 13
        L1_B0_Q_FLD_LEN_BYTES = 2

        L1_B0_FLAGS_VAL = 0x79
        L1_B0_Q_VAL = 0x10

        @classmethod
        def get_spec_size(self):
            return EncryptionParamsSectionBody.B0block.B0_BLOCK_SIZE_BYTES

        @classmethod
        def get_iv_val(cls):
            return os.urandom(EncryptionParamsSectionBody.B0block.L1_IV_FLD_LEN_BYTES)
            # return "N1N1N1N1N1N1N" #os.urandom(EncryptionParamsSectionBody.B0block.L1_IV_FLD_LEN_BYTES)

        def __init__(self, binary_blob=None):
            if binary_blob == None:
                self.flag_fld = EncryptionParamsSectionBody.B0block.L1_B0_FLAGS_VAL
                self.nonce_fld = EncryptionParamsSectionBody.B0block.get_iv_val()
                self.q_fld = EncryptionParamsSectionBody.B0block.L1_B0_Q_VAL
            else:
                self._decode_binary_header_blob(binary_blob)


        def _generate_binary_blob(self):
            binary_blob = struct.pack("=B", self.flag_fld)
            binary_blob += self.nonce_fld
            binary_blob += struct.pack(">H", self.q_fld)
            return binary_blob

        def get_binary_blob(self):
            return self._generate_binary_blob()

        def _decode_binary_header_blob(self, binary_blob):
            if len(binary_blob) != EncryptionParamsSectionBody.B0block.B0_BLOCK_SIZE_BYTES:
                raise RuntimeError("B0 block blob is of the wrong size")

            string_offset = 0
            string_end = EncryptionParamsSectionBody.B0block.L1_B0_FLG_FLD_LEN_BYTES
            self.flag_fld, = struct.unpack("B", binary_blob[string_offset:string_end])

            string_offset = string_end
            string_end += EncryptionParamsSectionBody.B0block.L1_IV_FLD_LEN_BYTES
            self.nonce_fld = binary_blob[string_offset:string_end]

            string_offset = string_end
            string_end += EncryptionParamsSectionBody.B0block.L1_B0_Q_FLD_LEN_BYTES
            self.q_fld, = struct.unpack(">H", binary_blob[string_offset:string_end])

    class L2AssociatedData(object):

        IMAGE_ID = "image_id"

        L2_ASSOCIATED_DATA_SIZE_FLD_LEN_BYTES = 2
        MAJOR_VERSION_FLD_LEN_BYTES = 1
        MINOR_VERSION_FLD_LEN_BYTES = 1
        KEY_LADDER_LEN_FLD_LEN_BYTES = 1
        IMAGE_ID_BITMAP_FLD_VERSION_1_0_LEN_BYTES = 4
        IMAGE_ID_BITMAP_FLD_VERSION_1_1_LEN_BYTES = 16

        SPEC_SIZE_BYTES = 32
        L2_ASSOCIATED_DATA_SIZE_VAL = 32
        RSVD_FLD_VAL = 0
        MAJOR_VERSION_FLD_VAL_1 = 1
        MINOR_VERSION_FLD_VAL_0 = 0
        MINOR_VERSION_FLD_VAL_1 = 1
        KEY_LADDER_LEN_FLD_VAL = 3
        IMAGE_ID_BITMAP_FLD_VAL = 0x1

        @classmethod
        def get_spec_size(cls):
            return EncryptionParamsSectionBody.L2AssociatedData.SPEC_SIZE_BYTES

        def __init__(self, major_version, minor_version, image_id, binary_blob=None, validating=False):
            self.major_version = major_version
            self.minor_version = minor_version
            self.image_id = image_id
            logger.debug("Configured Encryption Parameters L2 Associated Data version is {0}.{1}".format(self.major_version, self.minor_version))
            if binary_blob is None:
                self.l2_associated_data_size = EncryptionParamsSectionBody.L2AssociatedData.L2_ASSOCIATED_DATA_SIZE_VAL
                self.key_ladder_length = EncryptionParamsSectionBody.L2AssociatedData.KEY_LADDER_LEN_FLD_VAL
            else:
                self._decode_binary_blob(binary_blob, validating)

        def _generate_binary_blob(self):
            binary_blob = struct.pack(">H", self.l2_associated_data_size)
            binary_blob += struct.pack("=BBBBBB",
                                       EncryptionParamsSectionBody.L2AssociatedData.RSVD_FLD_VAL,
                                       EncryptionParamsSectionBody.L2AssociatedData.RSVD_FLD_VAL,
                                       self.major_version,
                                       self.minor_version,
                                       self.key_ladder_length,
                                       EncryptionParamsSectionBody.L2AssociatedData.RSVD_FLD_VAL)

            if (self.major_version, self.minor_version) == (EncryptionParamsSectionBody.L2AssociatedData.MAJOR_VERSION_FLD_VAL_1, EncryptionParamsSectionBody.L2AssociatedData.MINOR_VERSION_FLD_VAL_0):
                if self.image_id > 31:
                    raise RuntimeError("SW_ID is too large. Max value allowed is 0x1F. It is set to 0x%X" % self.image_id)
                binary_blob += struct.pack("=IQQL",
                                           2 ** self.image_id,
                                           EncryptionParamsSectionBody.L2AssociatedData.RSVD_FLD_VAL,
                                           EncryptionParamsSectionBody.L2AssociatedData.RSVD_FLD_VAL,
                                           EncryptionParamsSectionBody.L2AssociatedData.RSVD_FLD_VAL)
            elif (self.major_version, self.minor_version) == (EncryptionParamsSectionBody.L2AssociatedData.MAJOR_VERSION_FLD_VAL_1, EncryptionParamsSectionBody.L2AssociatedData.MINOR_VERSION_FLD_VAL_1):
                if self.image_id > 127:
                    raise RuntimeError("SW_ID is too large. Max value allowed is 0x7F. It is set to 0x%X" % self.image_id)
                image_id_bitmap = 2 ** self.image_id

                # Little endian encoding
                upper_image_id_bitmap = image_id_bitmap % (2 ** 64)
                lower_image_id_bitmap = image_id_bitmap / (2 ** 64)
                binary_blob += struct.pack("=QQQ",
                                           upper_image_id_bitmap if upper_image_id_bitmap != 0 else upper_image_id_bitmap,
                                           lower_image_id_bitmap if lower_image_id_bitmap != 0 else lower_image_id_bitmap,
                                           EncryptionParamsSectionBody.L2AssociatedData.RSVD_FLD_VAL)
            else:
                raise RuntimeError("Encryption Parameters L2 Associated Data version \"{0}.{1}\" is invalid.".format(self.major_version, self.minor_version))

            return binary_blob

        def _decode_binary_blob(self, binary_blob, validating):
            if len(binary_blob) != EncryptionParamsSectionBody.L2AssociatedData.SPEC_SIZE_BYTES:
                raise RuntimeError("L2 Associated Data blob is of the wrong size")

            string_offset = 0
            string_end = EncryptionParamsSectionBody.L2AssociatedData.L2_ASSOCIATED_DATA_SIZE_FLD_LEN_BYTES
            self.l2_associated_data_size, = struct.unpack(">H", binary_blob[string_offset:string_end])

            string_offset = string_end + EncryptionParamsSectionBody.RSVD_BYTE_LEN_BYTES * 2
            string_end = string_offset + EncryptionParamsSectionBody.L2AssociatedData.MAJOR_VERSION_FLD_LEN_BYTES + \
                         EncryptionParamsSectionBody.L2AssociatedData.MINOR_VERSION_FLD_LEN_BYTES + \
                         EncryptionParamsSectionBody.L2AssociatedData.KEY_LADDER_LEN_FLD_LEN_BYTES + \
                         EncryptionParamsSectionBody.RSVD_BYTE_LEN_BYTES

            major_version, minor_version, self.key_ladder_length, tmp = struct.unpack("=BBBB", binary_blob[string_offset:string_end])
            string_offset = string_end

            if (major_version, minor_version) != (self.major_version, self.minor_version):
                raise RuntimeError(("Encryption Parameters L2 Associated Data version \"{0}.{1}\" does not match expected version \"{2}.{3}\""
                                    "\n       Ensure that the correct encryptor value is set.").format(major_version, minor_version, self.major_version, self.minor_version))

            if (major_version, minor_version) == (EncryptionParamsSectionBody.L2AssociatedData.MAJOR_VERSION_FLD_VAL_1, EncryptionParamsSectionBody.L2AssociatedData.MINOR_VERSION_FLD_VAL_0):
                string_end = string_offset + EncryptionParamsSectionBody.L2AssociatedData.IMAGE_ID_BITMAP_FLD_VERSION_1_0_LEN_BYTES
                image_id_bitmap, = struct.unpack("=I", binary_blob[string_offset:string_end])
            elif (major_version, minor_version) == (EncryptionParamsSectionBody.L2AssociatedData.MAJOR_VERSION_FLD_VAL_1, EncryptionParamsSectionBody.L2AssociatedData.MINOR_VERSION_FLD_VAL_1):
                string_end = string_offset + EncryptionParamsSectionBody.L2AssociatedData.IMAGE_ID_BITMAP_FLD_VERSION_1_1_LEN_BYTES
                image_id_bitmap_upper, image_id_bitmap_lower = struct.unpack("=QQ", binary_blob[string_offset:string_end])
                image_id_bitmap = image_id_bitmap_lower * (2 ** 64) + image_id_bitmap_upper
            else:
                raise RuntimeError("Configured Encryption Parameters L2 Associated Data version \"{0}.{1}\" is invalid.".format(self.major_version, self.minor_version))

            image_id = int(math.log(image_id_bitmap, 2))
            if image_id != self.image_id:
                if validating:
                    errstr = list()
                    mismatches = list()
                    mismatches.append((EncryptionParamsSectionBody.L2AssociatedData.IMAGE_ID, "0x%X" % image_id, "0x%X" % self.image_id))
                    create_mismatch_table(mismatches, errstr, operation="encryption", data_type_to_compare="Attribute", image_region="Encryption Parameters")
                    logger.error('Following validations failed for the image:\n       ' +
                                 '\n       '.join([(str(i + 1) + '. ' + e) for i, e in enumerate(errstr)]))
                else:
                    logger.warning(("Extracted Encryption Parameters " + EncryptionParamsSectionBody.L2AssociatedData.IMAGE_ID + " value \"{0}\" does not match config value \"{1}\""
                                   "\n\t Encryption Parameters " + EncryptionParamsSectionBody.L2AssociatedData.IMAGE_ID + " value will be updated with value \"{1}\"").format(hex(image_id), hex(self.image_id)))


        def get_binary_blob(self):
            return self._generate_binary_blob()

    class L3AssociatedData(object):

        L3_ASSOCIATED_DATA_SIZE_FLD_LEN_BYTES = 2
        IMAGE_ENC_ALGO_FLD_LEN_BYTES = 1
        IMAGE_ENC_MODE_FLD_LEN_BYTES = 1
        ENCRYPT_ALL_SEGMENTS_FLD_LEN_BYTES = 1
        NUMBER_OF_SEGMENTS_FLD_LEN_BYTES = 1
        SEGMENT_BITMAP_31_0_FLD_LEN_BYTES = 4
        SEGMENT_BITMAP_63_32_FLD_LEN_BYTES = 4
        SEGMENT_BITMAP_95_64_FLD_LEN_BYTES = 4
        SEGMENT_BITMAP_127_96_FLD_LEN_BYTES = 4

        L3_ASSOCIATED_DATA_SIZE_VAL = 32
        RSVD_FLD_VAL = 0
        IMAGE_ENC_ALGO_FLD_VAL = 1
        IMAGE_ENC_MODE_FLD_VAL = 1
        ENCRYPT_ALL_SEGMENTS_FLD_VAL = 1
        NUMBER_OF_SEGMENTS_FLD_VAL = 0
        SEGMENT_BITMAP_31_0_FLD_VAL = 0
        SEGMENT_BITMAP_63_32_FLD_VAL = 0
        SEGMENT_BITMAP_95_64_FLD_VAL = 0
        SEGMENT_BITMAP_127_96_FLD_VAL = 0

        SPEC_SIZE_BYTES = 32

        @classmethod
        def get_spec_size(self):
            return EncryptionParamsSectionBody.L3AssociatedData.SPEC_SIZE_BYTES

        def __init__(self, binary_blob=None, encrypted_segments_indices=None):
            if binary_blob is None:
                self.l3_associated_data_size = EncryptionParamsSectionBody.L3AssociatedData.L3_ASSOCIATED_DATA_SIZE_VAL
                self.image_enc_algo = EncryptionParamsSectionBody.L3AssociatedData.IMAGE_ENC_ALGO_FLD_VAL
                self.image_enc_mode = EncryptionParamsSectionBody.L3AssociatedData.IMAGE_ENC_MODE_FLD_VAL
                self.encrypted_segments_indices = encrypted_segments_indices
                if self.encrypted_segments_indices is not None:
                        self._update_encrypted_segment_bitmap()
                else:
                    self.encrypt_all_segments = EncryptionParamsSectionBody.L3AssociatedData.ENCRYPT_ALL_SEGMENTS_FLD_VAL
                    self.number_of_segments = EncryptionParamsSectionBody.L3AssociatedData.NUMBER_OF_SEGMENTS_FLD_VAL
                    self.segment_bitmap_31_0 = EncryptionParamsSectionBody.L3AssociatedData.SEGMENT_BITMAP_31_0_FLD_VAL
                    self.segment_bitmap_63_32 = EncryptionParamsSectionBody.L3AssociatedData.SEGMENT_BITMAP_63_32_FLD_VAL
                    self.segment_bitmap_95_64 = EncryptionParamsSectionBody.L3AssociatedData.SEGMENT_BITMAP_95_64_FLD_VAL
                    self.segment_bitmap_127_96 = EncryptionParamsSectionBody.L3AssociatedData.SEGMENT_BITMAP_127_96_FLD_VAL
            else:
                self._decode_binary_header_blob(binary_blob)

        def _update_encrypted_segment_bitmap(self):
            self.encrypt_all_segments = 0
            self.number_of_segments = len(self.encrypted_segments_indices)
            self.segment_bitmap_31_0 = 0
            self.segment_bitmap_63_32 = 0
            self.segment_bitmap_95_64 = 0
            self.segment_bitmap_127_96 = 0
            for seg_idx in self.encrypted_segments_indices:
                if seg_idx <= 31:
                    self.segment_bitmap_31_0 = self.segment_bitmap_31_0 | (1 << (seg_idx % 32))
                elif seg_idx >= 32 and seg_idx <= 63:
                    self.segment_bitmap_63_32 = self.segment_bitmap_63_32 | (1 << (seg_idx % 32))
                elif seg_idx >= 64 and seg_idx <= 95:
                    self.segment_bitmap_95_64 = self.segment_bitmap_63_32 | (1 << (seg_idx % 32))
                elif seg_idx >= 96 and seg_idx <= 127:
                    self.segment_bitmap_127_96 = self.segment_bitmap_127_96 | (1 << (seg_idx % 32))
                else:
                    raise RuntimeError("Cannot encrypt ELF images containing more than 128 segments")


        def _generate_binary_blob(self):
            binary_blob = struct.pack(">H", self.l3_associated_data_size)
            binary_blob += struct.pack("=BBBBBBIBBBBIIII",
                                       EncryptionParamsSectionBody.L3AssociatedData.RSVD_FLD_VAL,
                                       EncryptionParamsSectionBody.L3AssociatedData.RSVD_FLD_VAL,
                                       self.image_enc_algo,
                                       self.image_enc_mode,
                                       EncryptionParamsSectionBody.L3AssociatedData.RSVD_FLD_VAL,
                                       EncryptionParamsSectionBody.L3AssociatedData.RSVD_FLD_VAL,
                                       EncryptionParamsSectionBody.L3AssociatedData.RSVD_FLD_VAL,
                                       self.encrypt_all_segments,
                                       self.number_of_segments,
                                       EncryptionParamsSectionBody.L3AssociatedData.RSVD_FLD_VAL,
                                       EncryptionParamsSectionBody.L3AssociatedData.RSVD_FLD_VAL,
                                       self.segment_bitmap_31_0,
                                       self.segment_bitmap_63_32,
                                       self.segment_bitmap_95_64,
                                       self.segment_bitmap_127_96,
                                       )
            return binary_blob

        def get_binary_blob(self):
            return self._generate_binary_blob()

        def _decode_binary_header_blob(self, binary_blob):
            if len(binary_blob) != EncryptionParamsSectionBody.L3AssociatedData.SPEC_SIZE_BYTES:
                raise RuntimeError("L3AssociatedData block blob is of the wrong size")

            string_offset = 0
            string_end = EncryptionParamsSectionBody.L3AssociatedData.L3_ASSOCIATED_DATA_SIZE_FLD_LEN_BYTES
            self.l3_associated_data_size, = struct.unpack('>H', binary_blob[string_offset:string_end])

            string_offset = string_end
            string_end = string_offset + \
                         EncryptionParamsSectionBody.RSVD_BYTE_LEN_BYTES * 2 + \
                         EncryptionParamsSectionBody.L3AssociatedData.IMAGE_ENC_ALGO_FLD_LEN_BYTES + \
                         EncryptionParamsSectionBody.L3AssociatedData.IMAGE_ENC_MODE_FLD_LEN_BYTES + \
                         EncryptionParamsSectionBody.RSVD_BYTE_LEN_BYTES * 2 + \
                         EncryptionParamsSectionBody.RSVD_BYTE_LEN_BYTES * 4 + \
                         EncryptionParamsSectionBody.L3AssociatedData.ENCRYPT_ALL_SEGMENTS_FLD_LEN_BYTES + \
                         EncryptionParamsSectionBody.L3AssociatedData.NUMBER_OF_SEGMENTS_FLD_LEN_BYTES + \
                         EncryptionParamsSectionBody.RSVD_BYTE_LEN_BYTES * 2 + \
                         EncryptionParamsSectionBody.L3AssociatedData.SEGMENT_BITMAP_31_0_FLD_LEN_BYTES + \
                         EncryptionParamsSectionBody.L3AssociatedData.SEGMENT_BITMAP_63_32_FLD_LEN_BYTES + \
                         EncryptionParamsSectionBody.L3AssociatedData.SEGMENT_BITMAP_95_64_FLD_LEN_BYTES + \
                         EncryptionParamsSectionBody.L3AssociatedData.SEGMENT_BITMAP_127_96_FLD_LEN_BYTES
            tmp, tmp, self.image_enc_algo, self.image_enc_mode, tmp, tmp, tmp, self.encrypt_all_segments, self.number_of_segments, tmp, tmp, self.segment_bitmap_31_0, self.segment_bitmap_63_32, self.segment_bitmap_95_64, self.segment_bitmap_127_96, = struct.unpack("=BBBBBBIBBBBIIII", binary_blob[string_offset:string_end])

    def __init__(self,
                 L2_associated_data_major_version,
                 L2_associated_data_minor_version,
                 image_id,
                 l1_key=None,
                 l2_key=None,
                 l3_key=None,
                 enc_l2_key=None,
                 enc_l3_key=None,
                 enc_key_prov_is_qti=False,
                 enc_param_section_body_blob=None,
                 validating=False,
                 debug_dir=None,
                 encrypted_segments_indices=None):

        self.L2_associated_data_major_version = L2_associated_data_major_version
        self.L2_associated_data_minor_version = L2_associated_data_minor_version
        self.image_id = image_id
        self.enc_key_prov_is_qti = enc_key_prov_is_qti
        if enc_param_section_body_blob is None:
            self._create_new(l1_key, l2_key, l3_key, enc_l2_key, enc_l3_key, encrypted_segments_indices)
        else:
            self._decode_binary_blob(enc_param_section_body_blob, validating, l1_key, debug_dir)

    def _create_new(self, l1_key, l2_key, l3_key, enc_l2_key, enc_l3_key, encrypted_segments_indices):
        self.L2B0block = EncryptionParamsSectionBody.B0block()
        self.L2_IV = self.L2B0block.nonce_fld
        self.L2_associated_data = EncryptionParamsSectionBody.L2AssociatedData(self.L2_associated_data_major_version, self.L2_associated_data_minor_version, self.image_id).get_binary_blob()
        self.L3B0block = EncryptionParamsSectionBody.B0block()
        self.L3_IV = self.L3B0block.nonce_fld
        self.L3_associated_data = EncryptionParamsSectionBody.L3AssociatedData(encrypted_segments_indices=encrypted_segments_indices).get_binary_blob()
        self.l3_key = l3_key
        self.base_iv = os.urandom(EncryptionParamsSectionBody.BASE_IV_LEN_BYTES)

        # determine if encrypted l2 and l3 keys are provided or need to be generated
        if None not in (l1_key, l2_key, l3_key):
            self.l1_key = l1_key
            self.l2_key = l2_key
            # generate encrypted keys
            self.L1_key_payload, self.L2_MAC = self._get_l1_encrypted_l2_key()
            self.L2_key_payload, self.L3_MAC, = self._get_l2_encrypted_l3_key()
        else:
            # get lengths of encrypted keys
            len_of_enc_l2_key = len(enc_l2_key)
            len_of_enc_l3_key = len(enc_l3_key)

            if self.enc_key_prov_is_qti:
                if len_of_enc_l2_key == 80 and len_of_enc_l3_key == 80:
                    self.L2_wrapped = enc_l2_key
                    self.L3_wrapped = enc_l3_key
                else:
                    raise RuntimeError("Wrapped L2 key and wrapped L3 key received from encrypted key provider are of \n"
                                       "unsupported lengths {0} bytes and {1} bytes. They must each be 80 bytes in length."
                                       .format(len_of_enc_l2_key, len_of_enc_l3_key))
            else:
                if len_of_enc_l2_key == 32 and len_of_enc_l3_key == 32:
                    # dissect encrypted keys
                    self.L1_key_payload, self.L2_MAC = enc_l2_key[0:16], enc_l2_key[16:32]
                    self.L2_key_payload, self.L3_MAC = enc_l3_key[0:16], enc_l3_key[16:32]
                else:
                    raise RuntimeError("Encrypted L2 key and encrypted L3 key received from encrypted key provider are of \n"
                                       "unsupported lengths {0} bytes and {1} bytes. They must each be 32 bytes in length."
                                       .format(len_of_enc_l2_key, len_of_enc_l3_key))

    def get_binary_blob(self):
        return self._generate_binary_blob()

    def _generate_binary_blob(self):
        binary_blob = ""
        if self.enc_key_prov_is_qti:
            binary_blob += self.L2_wrapped
            binary_blob += self.L3_wrapped
            binary_blob += self.base_iv
        else:
            binary_blob += self.L2B0block.get_binary_blob()
            binary_blob += self.L2_associated_data
            binary_blob += self.L1_key_payload
            binary_blob += self.L2_MAC
            binary_blob += self.L3B0block.get_binary_blob()
            binary_blob += self.L3_associated_data
            binary_blob += self.L2_key_payload
            binary_blob += self.L3_MAC
            binary_blob += self.base_iv
        return binary_blob

    def _decode_binary_blob(self, binary_blob, validating, l1_key, debug_dir):
        string_offset = 0
        string_end = EncryptionParamsSectionBody.B0block.get_spec_size()

        self.L2B0block = EncryptionParamsSectionBody.B0block(binary_blob[string_offset:string_end])
        self.L2_IV = self.L2B0block.nonce_fld
        logger.debug("L2 IV: \n" + hexdump(self.L2_IV))

        string_offset = string_end
        string_end += EncryptionParamsSectionBody.L2AssociatedData.get_spec_size()
        self.L2_associated_data = binary_blob[string_offset:string_end]
        EncryptionParamsSectionBody.L2AssociatedData(self.L2_associated_data_major_version,
                                                     self.L2_associated_data_minor_version,
                                                     self.image_id,
                                                     binary_blob=self.L2_associated_data,
                                                     validating=validating)

        string_offset = string_end
        string_end += EncryptionParamsSectionBody.ENCRYPTED_KEY_PAYLOAD_LEN_BYTES
        self.L1_key_payload = binary_blob[string_offset:string_end]

        string_offset = string_end
        string_end += EncryptionParamsSectionBody.MAC_LEN_BYTES
        self.L2_MAC = binary_blob[string_offset:string_end]

        store_debug_data_to_file(defines.DEST_DEBUG_FILE_ENCRYPTED_L2_KEY, self.L1_key_payload + self.L2_MAC, debug_dir)
        store_debug_data_to_file(defines.DEST_DEBUG_FILE_L2_IMAGE_IV, self.L2_IV, debug_dir)
        store_debug_data_to_file(defines.DEST_DEBUG_FILE_L2_ADD, self.L2_associated_data, debug_dir)

        try:
            self.l2_key = crypto.aes_ccm.decrypt(self.L1_key_payload + self.L2_MAC,
                                                 binascii.hexlify(l1_key),
                                                 binascii.hexlify(self.L2_IV),
                                                 binascii.hexlify(self.L2_associated_data))
            logger.debug("L2 Key extracted from image: \n" + hexdump(self.l2_key))
            logger.debug("Any previously generated L2 keys will be ignored.")
        except subprocess.CalledProcessError:
            raise RuntimeError("Extraction of L2 key from image failed. This can be caused by the use of an invalid L1 key.")

        string_offset = string_end
        string_end += EncryptionParamsSectionBody.B0block.get_spec_size()
        self.L3B0block = EncryptionParamsSectionBody.B0block(binary_blob[string_offset:string_end])
        self.L3_IV = self.L3B0block.nonce_fld
        logger.debug("L3 IV: \n" + hexdump(self.L3_IV))

        string_offset = string_end
        string_end += EncryptionParamsSectionBody.L3AssociatedData.get_spec_size()
        self.L3_associated_data = binary_blob[string_offset:string_end]

        string_offset = string_end
        string_end += EncryptionParamsSectionBody.ENCRYPTED_KEY_PAYLOAD_LEN_BYTES
        self.L2_key_payload = binary_blob[string_offset:string_end]

        string_offset = string_end
        string_end += EncryptionParamsSectionBody.MAC_LEN_BYTES
        self.L3_MAC = binary_blob[string_offset:string_end]

        store_debug_data_to_file(defines.DEST_DEBUG_FILE_ENCRYPTED_L3_KEY, self.L2_key_payload + self.L3_MAC, debug_dir)
        store_debug_data_to_file(defines.DEST_DEBUG_FILE_L3_IMAGE_IV, self.L3_IV, debug_dir)
        store_debug_data_to_file(defines.DEST_DEBUG_FILE_L3_ADD, self.L3_associated_data, debug_dir)

        try:
            self.l3_key = crypto.aes_ccm.decrypt(self.L2_key_payload + self.L3_MAC,
                                                 binascii.hexlify(self.l2_key),
                                                 binascii.hexlify(self.L3_IV),
                                                 binascii.hexlify(self.L3_associated_data))
            logger.debug("L3 Key extracted from image: \n" + hexdump(self.l3_key))
            logger.debug("Any previously generated L3 keys will be ignored.")
        except subprocess.CalledProcessError:
            raise RuntimeError("Extraction of L3 key from image failed. This can be caused by the use of an invalid L1 or L2 key.")

        string_offset = string_end
        string_end += EncryptionParamsSectionBody.BASE_IV_LEN_BYTES
        self.base_iv = binary_blob[string_offset:string_end]

    def _get_l1_encrypted_l2_key(self):
        pt = self.l2_key
        key = binascii.hexlify(self.l1_key)
        iv = binascii.hexlify(self.L2_IV)
        aad = binascii.hexlify(self.L2_associated_data)
        logger.debug("L1 encrypted L2 key:")
        logger.debug("PT = \n" + hexdump(pt))
        logger.debug("KEY = \n" + hexdump(key))
        logger.debug("IV = \n" + hexdump(iv))
        logger.debug("AAD = \n" + hexdump(aad))

        enc_l2_key = crypto.aes_ccm.encrypt(pt, key, iv, aad)
        return enc_l2_key[0:16], enc_l2_key[16:32]

    def _get_l2_encrypted_l3_key(self):
        pt = self.l3_key
        key = binascii.hexlify(self.l2_key)
        iv = binascii.hexlify(self.L3_IV)
        aad = binascii.hexlify(self.L3_associated_data)
        logger.debug("L2 encrypted L3 key:")
        logger.debug("PT = \n" + hexdump(pt))
        logger.debug("KEY = \n" + hexdump(key))
        logger.debug("IV = \n" + hexdump(iv))
        logger.debug("AAD = \n" + hexdump(aad))

        enc_l3_key = crypto.aes_ccm.encrypt(pt, key, iv, aad)
        return enc_l3_key[0:16], enc_l3_key[16:32]

    def get_l3_key(self):
        return self.l3_key

    def get_image_iv(self):
        return self.base_iv
