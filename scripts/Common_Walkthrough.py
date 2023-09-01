import Device_configuration
from Device_configuration import *
Lparser=""

# List of supported targets
def target_list():
	target_info = ["sdm835","sdm820","sdm660","sdm845","sdm670", "sdm710", "qcs605", "sm8150","sm6150","qcs405","sxr1130","sm7150","sm6125","sm8250","rennell","saipan","bitra"]
	return target_info

def check_for_user_supplied_hex_variant(hex_variant_input):
	regex = 'v\d+_v\d+'
	m = re.match(regex, hex_variant_input)
	if m is None:
		print "Error! unknown Hexgon version please give like below \nexample : v82_v65"
		sys.exit()
	hex_variant="hexagon_Debug_dynamic_tool"+hex_variant_input
	return hex_variant

def call_parser(parser):
	global Lparser
	parser.add_argument("-L", dest="linux_env", action="store_true", help="Build If your device HLOS is Linux embedded (e.g APQ8096)  Don't choose this option if your device HLOS is Linux Android") 
	parser.add_argument('-T', dest="target", help="specify target name  <sdm835, sdm820, sdm660, sdm845, sdm670, sdm710, qcs605, sm8150, sm6150, qcs405, sxr1130, sm7150, sm6125, sm8250, rennell, saipan, bitra>")
	parser.add_argument('-D', dest="dsp_image", default="True", help="specify subsystem to run on <cdsp, adsp, mdsp, slpi>")
	parser.add_argument('-V', dest="hex_version", default="True", help="specify hexagon version example : v83_v65")
	parser.add_argument('-M', dest="no_rebuild", action="store_true", help="does not recompile hexagon and android variants")
	parser.add_argument("-N", dest="no_signing", action="store_true", help="skips test signature installation")
	parser.add_argument('-32', dest="thirty_two_bit", action="store_true", help="specify only if the target is 32 bit")
	Lparser = parser
	return parser
	

class get_config:
	hex_variant=""
	default_DSP_flag=""
	LocalParser=""
	variant="android_Debug_aarch64"
	global Lparser
	slpi_supported = ["sdm820","sdm835","sdm845","sm8150","sm8250"]
	cdsp_supported = ["sdm660","sdm670","sdm710","sxr1130","sdm845","sm6150","sm8150","sm7150","qcs405","sm6125","sm8250","rennell","saipan","bitra"]
	def __init__(self):
		LocalParser = Lparser.parse_args()
		if LocalParser.target == "sdm660":
			self.hex_variant="hexagon_Debug_dynamic_toolv83_v60"
			self.default_DSP_flag="CDSP_FLAG=1"
		elif LocalParser.target == "sdm845" or LocalParser.target == "sdm670" or LocalParser.target == "sdm710" or LocalParser.target == "qcs605" or LocalParser.target == "sxr1130" or LocalParser.target == "sm7150" :
			self.hex_variant="hexagon_Debug_dynamic_toolv83_v65"
			self.default_DSP_flag="CDSP_FLAG=1"
		elif LocalParser.target == "sdm835":
			self.hex_variant="hexagon_Debug_dynamic_toolv83_v62"
			self.default_DSP_flag="ADSP_FLAG=1"
		elif LocalParser.target == "sdm820":
			self.hex_variant="hexagon_Debug_dynamic_toolv83_v60"
			self.default_DSP_flag="ADSP_FLAG=1"
		elif LocalParser.target == "sm8150" or LocalParser.target == "sm6150" or LocalParser.target == "qcs405" or LocalParser.target == "sm6125" or LocalParser.target == "sm8250" or LocalParser.target == "rennell" or LocalParser.target == "saipan" or LocalParser.target == "bitra":
			self.hex_variant="hexagon_Debug_dynamic_toolv83_v66"
			self.default_DSP_flag="CDSP_FLAG=1"
		if "True" != LocalParser.dsp_image:
			if LocalParser.dsp_image == "adsp":
				self.default_DSP_flag="ADSP_FLAG=1"
			elif LocalParser.dsp_image == "cdsp":
				self.default_DSP_flag="CDSP_FLAG=1"
				if LocalParser.target not in self.cdsp_supported:
					print LocalParser.target +" target won't support CDSP"
					sys.exit()
			elif LocalParser.dsp_image == "slpi":
				self.default_DSP_flag = "SLPI_FLAG=1"
				if LocalParser.target not in self.slpi_supported:
					print LocalParser.target +" target won't support SLPI"
					sys.exit()
			elif LocalParser.dsp_image == "mdsp":
				self.default_DSP_flag = "MDSP_FLAG=1"
				if LocalParser.target == "sdm820":
					self.hex_variant="hexagon_Debug_dynamic_toolv83_v55"
				if LocalParser.thirty_two_bit:
					self.variant="android_Debug"
			else:
				print "Error! unknown -D <argument> \nsupport arg: \n\tadsp, mdsp, slpi, cdsp\n"
				sys.exit()
		if LocalParser.linux_env:			#If -L specified, UbuntuARM_Debug_aarch64
			self.variant="UbuntuARM_Debug_aarch64"
			if LocalParser.thirty_two_bit :
				self.variant="UbuntuARM_Debug"
	def get_parameters(self):
		return self.hex_variant, self.default_DSP_flag, self.variant
	def get_slpi_supported(self):
		return self.slpi_supported

def get_DST_PARAMS(HEXAGON_SDK_ROOT):
	global Lparser
	LocalParser = Lparser.parse_args()
	prior_sm8250_targets = ["sdm835","sdm820","sdm660","sdm845","sdm670", "sdm710", "qcs605", "sm8150","sm6150","qcs405","sxr1130","sm7150","sm6125","saipan"]
	devdict=create_dict()
	if devdict=={}:
		print("Error! No devices connected!")
		sys.exit()
		
	if len(devdict.keys())==1:
		call_test_sig='python '+ HEXAGON_SDK_ROOT+'/scripts/testsig.py '
	else:
		if LocalParser.target:
			if LocalParser.target not in devdict.keys():
				print("Device with target name ",LocalParser.target," not connected")
				sys.exit()
			device_list=devdict[LocalParser.target]
			
			if len(device_list)>1:
				print("More than one device is connected with same target name!")
				print("target name: ",LocalParser.target)
				print("devices connected: ",device_list)
				sys.exit()
			call_test_sig='python '+ HEXAGON_SDK_ROOT+'/scripts/testsig.py -T '+LocalParser.target
		else:
			print("More than one devices are connected please specify target name using -T flag")
			sys.exit()
	DSP_LIB_SEARCH_PATH="DSP_LIBRARY_PATH"
	if LocalParser.target in prior_sm8250_targets:
		DSP_LIB_SEARCH_PATH = "ADSP_LIBRARY_PATH"
	APPS_DST='/vendor/bin'
	DSP_DST='/vendor/lib/rfsa/dsp/sdk/'
	if LocalParser.thirty_two_bit:
		LIB_DST='/vendor/lib/'
	else:
		LIB_DST='/vendor/lib64/'

	DSP_LIB_PATH="\"/vendor/lib/rfsa/dsp/sdk\;/vendor/lib/rfsa/dsp/testsig;\""
	if LocalParser.linux_env:			#If -L specified, UbuntuARM_Debug_aarch64
		if len(devdict.keys())==1:
			call_test_sig='python '+ HEXAGON_SDK_ROOT+'/scripts/testsig.py -LE'
		else:
			if LocalParser.target:
				if LocalParser.target not in devdict.keys():
					print("Device with target name ",LocalParser.target," not connected")
					sys.exit()
				device_list=devdict[LocalParser.target]
				
				if len(device_list)>1:
					print("More than one device is connected with same target name!")
					print("target name: ",LocalParser.target)
					print("devices connected: ",device_list)
					sys.exit()
					call_test_sig='python '+ HEXAGON_SDK_ROOT+'/scripts/testsig.py -LE -T '+LocalParser.target
			else:
				print("More than one devices are connected please specify target name using -T flag")
				sys.exit()
		APPS_DST='/usr/bin'
		DSP_DST='/usr/lib/rfsa/dsp/sdk/'
		if LocalParser.thirty_two_bit:
			LIB_DST='/usr/lib/'
		else:
			LIB_DST='/usr/lib64/'
		DSP_LIB_PATH="\"/usr/lib/rfsa/dsp/sdk\;/usr/lib/rfsa/dsp/testsig;\"";
		if LocalParser.target == "qcs405":
			APPS_DST='/data/bin'
			DSP_DST='/data/lib/rfsa/dsp/sdk/'
			if LocalParser.thirty_two_bit:
				LIB_DST='/data/lib/'
			else:
				LIB_DST='/data/lib64/'
			DSP_LIB_PATH="\"/data/lib/rfsa/dsp/sdk\;/data/lib/rfsa/dsp/testsig;\"";
	return call_test_sig, APPS_DST, DSP_DST, LIB_DST, DSP_LIB_PATH, DSP_LIB_SEARCH_PATH
