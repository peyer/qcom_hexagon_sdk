# Runs a directory of preprocessed .dat's on graph_app
#
# Usage:
# 		See command output using --help argument.
#
#		-i, --input_dir: The input directory which contains your preprocessed .dat files
#		-es, --element_size: The element size for the .dat input files. This should be 1 for uint8-based 
#		input and 4 for float-based input.
#		-it, --iterations: The number of iterations each image will be run for.
#		-l, --labels_filename: The file which contains human-readable output for each output index
#		-p, --perfdump: Specified if using perfdump on graph_app. Provides per-node profiling output.
#
# Example:
#		The following runs all .dat files inside of the `animals` directory on graph_app. 
#		The options assume your .dat files are uint8-based and that your human-readable labels 
#		are located in vgg_labels.txt:
#
#		python run_graph.py -i animals -es 1 -l vgg_labels.txt
#
# Notes:
#		Currently, this script assumes that you have already pushed graph_app, libhexagon_nn_skel.so,
#		and your labels file to your target device. The assumed location of these files and the location that
#		the images will be pushed to is /data. Storage size limitations may prevent a successful test elsewhere.
#

import os
import signal
import sys
import subprocess
import argparse

pid = os.getpid()

def signal_usr1(signum, frame):	# signum is signal used to call handler 'signal_usr1', frame is current stack frame
	print "Exiting..."			# print appropriate message
	sys.stdout.flush()			# flush output to command prompt
	sys.exit(0)

def print_and_run_cmd(cmd):
	print cmd
	return os.system(cmd)

def run_graph(input_dir_filepath, labels_filename, element_size, num_iters, perfdump):
	print  "\n---- root/remount device ----"
	print_and_run_cmd('adb wait-for-device root')		# restart adb as root
	print_and_run_cmd('adb wait-for-device remount')	# wait for device to remount device

	files = list()
	print  "\n---- pushing files to device ----"
	for filename in os.listdir(input_dir_filepath):
		if filename.endswith('.dat'):
			files.append(filename)
			filename_filepath = os.path.join(input_dir_filepath, filename)
			print_and_run_cmd('adb push %s /data/' % filename_filepath)

	print  "\n---- running on network ----"
	command = 'adb shell /data/graph_app --iters %d --elementsize %d ' % (num_iters, element_size)
	command += '--labels_filename "/data/%s" ' % labels_filename if labels_filename else ''
	command += '--perfdump 1 ' if perfdump else '' 
	for filename in files:
		command += '/data/%s ' % filename

	sys.stdout.flush()
	print command
	test_output = subprocess.check_output(command, shell=True)
	test_lines = iter(test_output.splitlines())

	for line in test_lines:
		temp = line.replace("\n", "")
		if line.startswith('Using'):
			print ''
			print line
		elif line != "":
			print line

	print 'Done'

# main entry point for child process
if __name__ == '__main__':
	signal.signal(signal.SIGINT, signal_usr1)				# register signal handler 'signal.SIGINT' to function handler 'signal_usr1'
	
	# parse arguments passed on command-line
	parser = argparse.ArgumentParser(description='Run a directory of images on graph_app', formatter_class=argparse.ArgumentDefaultsHelpFormatter)
	parser.add_argument('-i', '--input_dir', help='Directory that contans input images', required=True)
	parser.add_argument('-es', '--element_size', help='Input image element size (default is uint8=1)', type=int, default=1)
	parser.add_argument('-it', '--iterations', help='Number of iterations for graph_app', type=int, default=1)
	parser.add_argument('-l', '--labels_filename', help='Filename where labels are specified, expected to be in /vendor/etc/', type=str)
	parser.add_argument('-p', '--perfdump', help="Provides useful performance info", action='store_true')
	args = parser.parse_args()

	# capture to arguments
	input_dir_filepath = os.path.abspath(args.input_dir)
	labels_filename = args.labels_filename
	num_iters = args.iterations
	element_size = args.element_size
	perfdump = args.perfdump

	if not os.path.isdir(input_dir_filepath):
		raise RuntimeError('Path given is not directory')

	# run graph tests
	run_graph(input_dir_filepath, labels_filename, element_size, num_iters, perfdump)
	sys.stdout.flush()