# ===============================================================================
#
#  Copyright (c) 2013-2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#
# ===============================================================================

"""Provides a command line interface to the opendsp services

.. data:: TOOL_NAME

    Name of the tool

.. data:: TOOL_VERSION

    Version of the tool
"""


import os
import sys
import traceback
import shutil
import time

SECTOOLS_DIR = os.path.dirname(os.path.realpath(__file__))

from sectools.features.isc.cfgparser import ConfigParser

from sectools.common.utils.c_logging import logger
from sectools.common.utils import c_path
from sectools.common.utils.c_base import CoreOptionParser
from sectools.common.utils.c_process import CoreSubprocess
from sectools.common.utils.c_attribute import Attribute
from sectools.features.isc.imageinfo import StatusInfo
from sectools.common.utils import c_misc
from sectools.features.isc import secimage

from optparse import SUPPRESS_HELP

TOOL_NAME = 'Elfsigner'
TOOL_VERSION = '1.5'

__version__ = TOOL_NAME + ' ' + TOOL_VERSION

# Path definitions
DEF_TOOL_OUTPUT_DIR_NAME = 'output'
#DEF_TOOL_OUTPUT_DIR_PATH = c_path.join(dynamicToolStatus.toolDir, DEF_TOOL_OUTPUT_DIR_NAME)
DEF_TOOL_OUTPUT_DIR_PATH = c_path.join(".", DEF_TOOL_OUTPUT_DIR_NAME)

CHIPSET = 'tcg'
DEFAULT_SRC_CONFIG = os.path.join(SECTOOLS_DIR, "config", CHIPSET, "{0}_secimage.xml".format(CHIPSET))
TESTSIG_FILENAME = 'testsigbase.so'
TESTSIG_PATH = os.path.join(SECTOOLS_DIR, 'resources', "opendsp", TESTSIG_FILENAME)
SECIMAGE_OUTPUTDIR = "secimage_output"
DEBUG_DIR = "debug"

TESTSIG_PREFIX = "testsig-"

GENERATED_CONFIG = "generated_config.xml"

STR_OUTPUT_SIGNED_INVALID = "signature is not valid"
STR_OUTPUT_UNSIGNED_VALID = "is not signed"

OUTPUT_DELIMINATOR = "------------------------------------------------------------"

class ToolParser(CoreOptionParser):
    """Parser for command line arguments supported by the tool."""

    @property
    def c_usage(self):
        """(str) Returns the usage of the program.
        """
        return ('\n' +
                self.c_prog + ' INFILE [Options]\n' +
                self.c_prog + ' -t SERIALNUM, --testsig=SERIALNUM [Options]'
                )

    @property
    def c_prog(self):
        """(str) Returns the name of the program. By default this is the name
        of the python file being executed.
        """
        return os.path.basename(sys.argv[0])

    @property
    def c_description(self):
        """(str) Returns the description of the program."""
        #return 'This program signs ELF file with TCG and testsig serial number.'
        return \
            'This program signs ELF file with TCG (Trusted Code Group) or generates testsig file with serial number.\n'

    def format_description(self, formatter):
        """This method is implemented to override the OptionParser's formatting
        of the epilog"""
        return self.description

    @property
    def c_version(self):
        """(str) Returns the version of the program."""
        return __version__

    @property
    def c_epilog(self):
        """(str) Returns the epilog for the program."""
        return ('\nExamples:\n' +
                '   {0} -i input/libtest.so\n'.format(self.c_prog) +
                '       Signs input/libtest.so and saves signed file to directory: ./output (default)\n\n' +
                '   {0} -i input/libtest.so -o signed\n'.format(self.c_prog) +
                '       Signs input/libtest.so and saves signed file to directory: ./signed\n\n' +
                '   {0} -i libtest.so -o .\n'.format(self.c_prog) +
                '       Signs libtest.so and saves signed file to current directory, named: libtest.so\n\n' +
                '   {0} -t 0x1234 -o testsigs/\n'.format(self.c_prog) +
                '       Generates testsig-0x1234.so and saves it in directory: ./testsigs' +
                '\n'
                )

    def format_epilog(self, formatter):
        """This method is implemented to override the OptionParser's formatting
        of the epilog"""
        return self.epilog

    def c_add_options(self):
        """Adds the command line args supported by the tool."""

        # Signing ELF image
        elf_group = self.add_option_group('Signing ELF file')
        elf_group.add_option('-i', '--image_file', metavar='<file>',
                             default=None,
                             help='path to the image file.')
        elf_group.add_option('-r', '--input_dir', metavar='<dir>',
                              help='The directory containing multiple image files to sign.')

        # Signing testsig file
        testsig_group = self.add_option_group('Generating testsig file with serial number')
        testsig_group.add_option('-t', '--testsig', metavar='<serialnum>',
                             help='32-bit device serial number such as 0xabcd0123\n',
                             default=None)

        # Common options
        common_group = self.add_option_group('Common options')
        common_group.add_option('--no_disclaimer', action="store_false", dest="disclaimer", default=True, help=SUPPRESS_HELP)
        common_group.add_option('-o', '--output_dir', metavar='<dir>',
                                help='directory to store output files. DEFAULT: "./' + DEF_TOOL_OUTPUT_DIR_NAME + '"',
                                default=DEF_TOOL_OUTPUT_DIR_PATH)
        common_group.add_option("-c", "--cass", action="store_true", default=False,
                          help="Use CASS server to sign with production root/key\n(Requires CASS access)")
        common_group.add_option('-m', '--tcg_min', metavar='<tcg>',
                             default=None,
                             help='minimum Trusted Code Group. Override config file value if present.')
        common_group.add_option('-x', '--tcg_max', metavar='<tcg>',
                             default=None,
                             help='maximum Trusted Code Group. Override config file value if present.')
        common_group.add_option('-f', '--tcg_fix', metavar='<tcg>',
                             default=None,
                             help='fixed Trusted Code Group. Override tcg min and tcg max with the fixed value.')
        common_group.add_option('-p', '--capability', metavar='<cass capability>',
                             default=None,
                             help='CASS capability to select key for signing. Override config file value if present.')
        common_group.add_option('-a', '--validate', action='store_true',
                                    default=False, help='validate the image. Validation does not support directory (-r).')
        common_group.add_option("--config", metavar='<file>',
                          default=None,
                          help=SUPPRESS_HELP)


    def c_validate(self):
        """Validates the command line args provided by the user.

        :raises: RuntimeError if any error occurs.
        """
        args = self.parsed_args
        err = []

        config_params_32bits = {
                      "tcg min": args.tcg_min,
                      "tcg max": args.tcg_max,
                      'testsig': args.testsig,
                      "tcg fix": args.tcg_fix,
                      }

        #import pdb; pdb.set_trace()
        if (args.input_dir or args.image_file) and args.testsig:
            err.append('Generate testsig (-t) and sign with image_file (-i or -r) cannot be used together')

        if args.capability and (args.cass is False):
            err.append('Capability (-p) is only supported when CASS (-c) is used')

        if not (args.image_file or args.input_dir) and not args.testsig:
            err.append('Either generate testsig (-t) or sign with image_file (-i or -r) must be specified')

        if args.validate and (args.testsig or args.input_dir):
            err.append('Validate (-a) should be used with image_file (-i). An input signed image should be referenced by -i.')

        if args.validate is False and args.image_file:
            path, filename = os.path.split(args.image_file)
            if filename.startswith(TESTSIG_PREFIX):
                err.append('Cannot sign testsig file {0}. Please use -t to generate testsig instead.'.format(args.image_file))

        if args.tcg_fix:
            for key in ["tcg min", "tcg max"]:
                if config_params_32bits[key]:
                    err.append("{0} should not be specified when tcg_fix is specified".format(key))

        for key in config_params_32bits:
            if Attribute.validate(num_bits=32, string=config_params_32bits[key]) is False:
                err.append('{0}:{1} is not a valid 32 bit integer'.format(key, config_params_32bits[key]))

        # Check and sanitize any paths for read access
        for path in ['image_file', 'config']:
            path_val = getattr(args, path, None)
            if path_val:
                path_val = c_path.normalize(path_val)
                if not c_path.validate_file(path_val):
                    err.append('Cannot access ' + path + ' at: ' + path_val)
                setattr(args, path, path_val)

        for path in ['input_dir']:
            path_val = getattr(args, path, None)
            if path_val:
                path_val = c_path.normalize(path_val)
                if not c_path.validate_dir(path_val):
                    err.append('Cannot access ' + path + ' at: ' + path_val)
                setattr(args, path, path_val)

        # Check and sanitize paths for write access
        for path in ['output_dir']:
            path_val = getattr(args, path, None)
            if path_val:
                path_val = c_path.normalize(path_val)
                try:
                    c_path.create_dir(path_val)
                except Exception as e:
                    err.append('Cannot write at: ' + path_val + '\n'
                               '    ' + 'Error: ' + str(e))
                setattr(args, path, path_val)

        # Raise error if any
        if err:
            if len(err) > 1:
                err = [('  ' + str(idx + 1) + '. ' + error) for idx, error in enumerate(err)]
                err = 'Please check the command line args:\n\n' + '\n'.join(err)
            else:
                err = err[0]
            raise RuntimeError(err)

class CustomCoreSubprocess(CoreSubprocess):
    def printFinish(self, cmd):
        pass

class ElfSignerCore(object):
    FILE_FILTER = '(?!{0})(.*)(so|mbn|elf)'.format(TESTSIG_PREFIX)

    ERR_FILE_NOT_SIGNED = "Input file is not signed."
    ERR_FILE_INVALID = "Input file is invalid!"
    ERR_FILE_SIG_INVALID = "Input file's " + STR_OUTPUT_SIGNED_INVALID

    def __init__(self, args):
        self.args = args
        self.secimage_dir = os.path.join(self.args.output_dir, SECIMAGE_OUTPUTDIR)

        self.serialnum = self.get_testsig_from_args(args)

    def get_testsig_from_args(self, args):
        serial_num_str = None
        if args.validate:
            testsig_index = args.image_file.find(TESTSIG_PREFIX)
            if testsig_index >= 0:
                filename, ext = os.path.splitext(args.image_file)
                serial_num_str = filename[testsig_index+len(TESTSIG_PREFIX):]
        elif args.testsig:
            serial_num_str = args.testsig

        return Attribute.init(num_bits=32, string=serial_num_str)


    def _get_sign_id(self):
        if self.serialnum is not None:
            sign_id = "testsig_cass" if self.args.cass else "testsig_local"
        else:
            sign_id = "elf_cass" if self.args.cass else "elf_local"

        return sign_id

    def _get_server(self):
        """Provides an interface that can be overridden by the caller
           to connect to a different server.

           Return None will connect to default server.
        """
        return None

    def _get_user_identity(self):
        """Provides an interface that can be overridden by the caller
           to user a different user identity.

           Return None will user default identity.
        """
        return None

    def generate_config_file(self, src_config, dest_config, args):
        config = ConfigParser(src_config)

        if args.cass is True:
            config.root.general_properties.selected_signer = 'cass'

            if args.capability:
                config.root.general_properties.cass_capability = args.capability

            #Enable test interface
            server_config = self._get_server()
            if server_config is not None:
                config.root.signing.signer_attributes.\
                    cass_signer_attributes.server = server_config
            user_identity = self._get_user_identity()
            if user_identity is not None:
                config.root.signing.signer_attributes.\
                    cass_signer_attributes.user_identity = user_identity

        sign_id = self._get_sign_id()

        image_list = config._get_image_list()
        for image in image_list:
            if image.sign_id == sign_id:
                break
        else:
            raise RuntimeError('sign_id: ' + sign_id + ' not found in config file')

        image.general_properties_overrides.testsig_serialnum = self.serialnum.str if self.serialnum is not None else None
        if args.tcg_min:
            image.general_properties_overrides.object_id.min = args.tcg_min
        if args.tcg_max:
            image.general_properties_overrides.object_id.max = args.tcg_max
        if args.tcg_fix:
            image.general_properties_overrides.object_id.min = args.tcg_fix
            image.general_properties_overrides.object_id.max = args.tcg_fix

        config.generate(dest_config)

    def invokeSecImage(self, args, input_filename, dest_config):
        sign_id = self._get_sign_id()
        cmds = [
                "secimage",
                "-g", sign_id,
                "-i", input_filename,
                "-c", dest_config,
                "-o", os.path.join(args.output_dir, SECIMAGE_OUTPUTDIR)]

        if args.verbose:
            cmds.append("-v")

        if args.debug:
            cmds.append("-d")

        if args.validate:
            cmds.append("-a")
        else:
            cmds.append("-sa")

        logger.debug(
 """\nExecute command:
     python sectools.py {0}\n""".format(' '.join(cmds)))
        return self._executeCmds(cmds, args.verbose or args.debug)

    def getmsecs (self):
        return int(round(time.clock() * 1000))

    def suppress_output(self, x):
        newtime = self.getmsecs()
        if (newtime - self.start_time) > 1:
            sys.stdout.write('.')
            self.start_time = newtime

    def update_secimage_verbosity(self, level):
        """Updates the secimage logger's verbosity

        :param int level: Level to increase the verbosity to. Accepted values
            are - 0: ERROR, 1: INFO, 2+: DEBUG
        :raises: RuntimeError if the given level value is not supported.
        """
        backup_versity = logger.verbosity

        if level == 0:
            logger.verbosity = logger.ERROR
        elif level == 1:
            logger.verbosity = logger.INFO
        elif level == 2:
            logger.verbosity = logger.DEBUG
        elif level >= 3:
            logger.verbosity = logger.DEBUG2
        else:
            raise RuntimeError('Given verbosity level: "' + str(level) + '" is invalid')

        return backup_versity

    def _executeCmds(self, cmds, verbose):
        image_info_list = []
        from sectools.features.isc.parsegen.elf import NON_HASHABLE_SEGMENTS, \
            NON_ENCAP_SEGMENTS, PT_PHDR, set_non_hashable_segments, \
            set_non_encap_segments
        try:
            parsed_args = secimage.parse_args(cmds)
            #Suppress secimage log level
            backup_verbosity = self.update_secimage_verbosity(self.args.verbose)

            #Include phdr for hashing
            backup_non_hashable_segments = NON_HASHABLE_SEGMENTS
            set_non_hashable_segments([])

            #Incluse phdr as non-encap segment
            backup_non_encap_segments = NON_ENCAP_SEGMENTS
            set_non_encap_segments(backup_non_encap_segments + [PT_PHDR])

            # Call sectools
            image_info_list = secimage.main(parsed_args)

        except Exception:
            logger.debug(traceback.format_exc())
            logger.error(sys.exc_info()[1])

        finally:
            set_non_encap_segments(backup_non_encap_segments)
            set_non_hashable_segments(backup_non_hashable_segments)
            logger.verbosity = backup_verbosity

        return image_info_list

    def _copyfile(self, source, destination):
        logger.debug("Copying signed image from {0} to {1}".format(source, destination))
        if os.path.isfile(source):
            shutil.copyfile(source, destination)
            logger.info("Signing complete! Output saved at {0}\n".format(destination))
        else:
            raise RuntimeError("Signed image {0} does not exist!".format(destination))

    def postProcess(self, signed_filename, output_filename, generated_config):
        if self.args.validate is False:
            # Copy signed file
            signed_file = os.path.join(self.secimage_dir,
                                       CHIPSET,
                                       self._get_sign_id(),
                                       signed_filename)
            dest_file = os.path.join(self.args.output_dir, output_filename)
            self._copyfile(signed_file, dest_file)

        #Clean up
        #import pdb; pdb.set_trace()
        if self.args.debug:
            debug_dir = os.path.join(self.secimage_dir,
                                   CHIPSET,
                                   self._get_sign_id(),
                                   DEBUG_DIR)
            filename, ext = os.path.splitext(output_filename)
            destination_debug_dir = os.path.join(self.args.output_dir,
                                   filename + "_" + DEBUG_DIR)
            if os.path.exists(destination_debug_dir):
                shutil.rmtree(destination_debug_dir, ignore_errors=True)
            logger.debug("Copying debug directories from {0} to {1}".format(debug_dir, destination_debug_dir))
            shutil.copytree(debug_dir, destination_debug_dir)
        else:
            logger.debug("Removing generated config {0}".format(generated_config))
            os.remove(self.generated_config)

        self.clean_secimage_dir()

    def clean_secimage_dir(self):
        if os.path.exists(self.secimage_dir):
            logger.debug("Removing directory {0}".format(self.secimage_dir))
            shutil.rmtree(self.secimage_dir)

    def log_contains(self, string):
        isPresent = False
        log_path = os.path.join(self.args.output_dir, TOOL_NAME + "_log.txt")
        if c_path.validate_file(log_path):
            log_data = c_misc.load_data_from_file(log_path)
            if log_data.find(string)>0:
                isPresent = True

        return isPresent

    def process_file(self, input_file=None):
        if self.args.validate or self.serialnum is None:
            input_filename = input_file if input_file is not None else self.args.image_file
            path, filename = os.path.split(input_filename)
            signed_filename = filename
            if path != self.args.output_dir or self.args.validate is True:
                output_filename = signed_filename
            else:
                name, ext = os.path.splitext(signed_filename)
                output_filename = name + "_signed" + ext
        else:
            input_filename = TESTSIG_PATH
            output_filename = "{0}{1}.so".format(TESTSIG_PREFIX, self.serialnum.unpadded_str)
            signed_filename = TESTSIG_FILENAME

        c_path.create_dir(self.args.output_dir)

        output_filename_base, ext = os.path.splitext(output_filename)
        self.generated_config = os.path.join(self.args.output_dir,
                                        output_filename_base + "_" + GENERATED_CONFIG)
        src_config = self.args.config if self.args.config else DEFAULT_SRC_CONFIG
        logger.debug("Source config file: {0}".format(src_config))
        self.generate_config_file(src_config, self.generated_config, self.args)

        image_info_list = self.invokeSecImage(self.args, input_filename, self.generated_config)

        #Post processing
        retValue = False if len(image_info_list)==0 else True
        for image_info in image_info_list:
            if self.args.validate:
                if image_info.status.validate_sign.state == StatusInfo.SUCCESS:
                    logger.info("Input file is signed and valid!")
                elif image_info.status.validate_sign.state == StatusInfo.ERROR:
                    if self.log_contains(STR_OUTPUT_UNSIGNED_VALID):
                        logger.info(self.ERR_FILE_NOT_SIGNED)
                    elif self.log_contains(STR_OUTPUT_SIGNED_INVALID):
                        logger.info(self.ERR_FILE_SIG_INVALID)
                    else:
                        logger.error(self.ERR_FILE_INVALID)
                    retValue = False
            else:
                if image_info.status.sign.state == StatusInfo.SUCCESS and \
                    image_info.status.validate_sign.state == StatusInfo.SUCCESS:
                    self.postProcess(signed_filename, output_filename, self.generated_config)
                else:
                    logger.error("Signing failed!")
                    retValue = False

        return retValue


    def process(self):
        if self.args.disclaimer:
            self.disclaimer_approval()

        #disable secimage logging
        secimage.log_to_file = lambda x:x

        logger.info("Signing a file may take up to 3 minutes due to network connectivity. Please wait patiently.")

        retValue = True
        if self.args.input_dir:
            input_files_list = c_path.find_files(self.args.input_dir,
                                                       self.FILE_FILTER,
                                                       combine=False)
            if input_files_list == []:
                logger.info("No files to be signed in the input directory: {0}".format(self.args.input_dir))
                retValue = False

            failure_list = []
            for index, input_file in enumerate(input_files_list):
                input_file_path = c_path.join(self.args.input_dir, input_file)
                logger.info(OUTPUT_DELIMINATOR)
                logger.info("Processing {0}/{1}: {2}".format((index+1), len(input_files_list), input_file_path))
                ret = self.process_file(input_file_path)
                if ret is False:
                    failure_list.append(index)
                retValue = retValue and ret

            num_success = len(input_files_list) - len(failure_list)
            logger.info(OUTPUT_DELIMINATOR)
            logger.info("{0}/{1} succeeded.".format(num_success, len(input_files_list)))
            if len(failure_list)>0:
                logger.info("Failures:")
                for index in failure_list:
                    logger.info("{0}.{1}".format(index+1,
                                                 c_path.join(self.args.input_dir, input_files_list[index])))

        else:
            logger.info(OUTPUT_DELIMINATOR)
            retValue = self.process_file()

        return retValue

    def disclaimer_approval(self):
        disclaimer = """
        Attention:
        Use of this tool is conditioned upon your compliance with Qualcomm
        Technologies'(and its affiliates') license terms and conditions;
        including, without limitations, such terms and conditions addressing
        the use of such tools with open source software.

        Agree? [y/n]:
        """

        while True:
            sys.stdout.write(disclaimer)
            choice = raw_input().lower()
            if choice in ['yes', 'no', 'y', 'n']:
                if choice in ['no', 'n']:
                    sys.exit(2)
                else:
                    return

def log_to_file(folder):
    """Configures the logger to log to filesystem

    :param str folder: Directory to generate the logs in.
    """
    folder = c_path.normalize(folder)
    try:
        c_path.create_dir(folder)
    except Exception as e:
        raise RuntimeError('Unable to create directory for logging: ' + folder + '\n'
                           '    ' + 'Error: ' + str(e))
    logger.enable_file_logging(TOOL_NAME, num_logs=1, log_dir=folder)

def main(args):
    """Parses the command line arguments, performs any basic operations based on
    the parsed arguments and starts processing using the isc module.
    """
    # Log to file
    log_to_file(args.output_dir)

    # Print the tool's launch command
    logger.debug('\n\n    Tool launched as: "' + ' '.join(sys.argv) + '"\n')

    core = ElfSignerCore(args)
    return core.process()


def parse_args(argv):
    return ToolParser(argv).parsed_args

if __name__ == '__main__':
    try:
        ret = main(parse_args(sys.argv))

    except Exception:
        logger.debug(traceback.format_exc())
        logger.error(sys.exc_info()[1])
        sys.exit(1)

    except KeyboardInterrupt:
        print
        logger.error('Keyboard Interrupt Received. Exiting!')
        sys.exit(1)

    exitCode = 0 if ret is True else 1
    sys.exit(exitCode)
