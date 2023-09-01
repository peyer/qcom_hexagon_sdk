import sys
import os
import subprocess
import re
import platform

sys.path.append('../../scripts')
HEXAGON_NN = os.getenv('HEXAGON_NN')
HEXAGON_SDK_ROOT=os.getenv('HEXAGON_SDK_ROOT')

if HEXAGON_SDK_ROOT == None:
    sys.exit("Hexagon SDK env is not setup. Please run setup_sdk_env from <SDK_ROOT>")
else:
    print ("HEXAGON_SDK_ROOT is " + HEXAGON_SDK_ROOT)


if HEXAGON_NN == None:
    sys.exit("Hexagon NN env is not setup. Please run setup_hexagon_nn.cmd from <SDK_ROOT>\examples\hexagon_nn")
else:
    print ("HEXAGON_NN is " + HEXAGON_NN)
    
TEST_LOG_FILE = "test_log.txt"
WIN_NEED_PYTHON_VERSION = "Python 3.6.7"
LNX_NEED_PYTHON_VERSION = "Python 3.6.0"
OS_SYSTEM = platform.system()
    

if OS_SYSTEM == "Windows":
    if (sys.version_info < (3, 6, 7)):
        sys.exit("\n " + WIN_NEED_PYTHON_VERSION + " is not present. Please install " + WIN_NEED_PYTHON_VERSION + " 64 bit before proceeding further." + "\n After installing python 3.6.7, run the following commands to set the temporary path of python to python 3.6.7. \n This is required to override the hexagon SDK default python path of python 2.7.x" + "\n For example," + " \n \n " + "set PATH=C:\\Users\\myusername\\AppData\\Local\\Programs\\Python\\Python36;%PATH% " + "\n cmd \n python tutorial_008_walkthrough.py" )
    import urllib.request
    import ssl
    ssl._create_default_https_context = ssl._create_unverified_context
elif OS_SYSTEM == "Linux":
    if (sys.version_info < (3, 6)):
        sys.exit("\n " + LNX_NEED_PYTHON_VERSION + " is not present. Please install " + LNX_NEED_PYTHON_VERSION + " before proceeding further")
    import urllib.request
    import ssl   
    ssl._create_default_https_context = ssl._create_unverified_context

import shutil
import tempfile

def retrieve_file_from_web(web_url, local_file_name):
    if OS_SYSTEM == "Windows":
        with urllib.request.urlopen(web_url) as response:
            with open(local_file_name, 'wb') as local_file:
                shutil.copyfileobj(response, local_file)
    elif OS_SYSTEM == "Linux":
        with urllib.request.urlopen(web_url) as response:
            with open(local_file_name, 'wb') as local_file:
                shutil.copyfileobj(response, local_file)



        
    
def print_and_run_cmd(cmd):
        print (cmd)
        if os.system(cmd) != 0 : sys.exit(2) # in error stop execution and exit

        
def print_and_run_cmd_in_subproc(cmd, TEST_LOG_FILE):
    with open(TEST_LOG_FILE, 'w') as test_log:
        print(os.getcwd())
        p = subprocess.Popen(cmd, stdout=test_log, stderr=subprocess.STDOUT, shell=True)
        out, err = p.communicate()

        if p.returncode != 0:
            raise RuntimeError('{0} failed, check {1} for details.'.format(
                               cmd, TEST_LOG_FILE))

                               
URL_PROTO = "http://gist.githubusercontent.com/ksimonyan/211839e770f7b538e2d8/raw/ded9363bd93ec0c770134f4e387d8aaaaa2407ce/VGG_ILSVRC_16_layers_deploy.prototxt"
URL_MODEL = "http://www.robots.ox.ac.uk/~vgg/software/very_deep/caffe/VGG_ILSVRC_16_layers.caffemodel"

print ("Downloading proto txt and model (will take a few minutes)")
retrieve_file_from_web(URL_MODEL, "caffemodel")
retrieve_file_from_web(URL_PROTO,"prototxt")


if OS_SYSTEM == "Windows":
    cmd_seq = [
    "echo Collecting the log @ " + TEST_LOG_FILE,
    
    "echo VIRTUALENV SET UP  ",
    "echo del_env.bat",
    "del_env.bat",
    "echo pip install virtualenv",
    "pip install virtualenv",
    "echo virtualenv -p python.exe env3",
    "virtualenv -p python.exe env3",
    "echo env3\\Scripts\\activate",
    "env3\\Scripts\\activate",
    "echo pip install -U pip",
    "pip install -U pip",
    "echo pip3 install -r " + HEXAGON_SDK_ROOT + "\\tools\\python_venv\\environments\\req3.txt",
    "pip3 install -r " + HEXAGON_SDK_ROOT + "\\tools\\python_venv\\environments\\req3.txt",

    "echo GENERATING HEXAGON NN IMPLEMENTATION ",
    "echo python ../../scripts/img_to_dat.py --root vgg --input " + HEXAGON_NN + "/test/animals --size 224 --bgr --byte",
    "python ../../scripts/img_to_dat.py --root vgg --input " + HEXAGON_NN + "/test/animals --size 224 --bgr --byte",
    "echo python ../../scripts/caffe_to_hexagon_nn.py prototxt caffemodel vgg --min_input -114.2 --max_input 140.8",
    "python ../../scripts/caffe_to_hexagon_nn.py prototxt caffemodel vgg --min_input -114.2 --max_input 140.8",
    "echo robocopy .  " + HEXAGON_NN + "  vgg.c vgg_data.c vgg_data_32b.c vgg.h ",
    "robocopy .  " + HEXAGON_NN + "  vgg.c vgg_data.c vgg_data_32b.c vgg.h ",
    "echo make -C " + HEXAGON_NN + " tree VERBOSE=1 V66=1 V=hexagon_ReleaseG_dynamic_toolv83_v66",
    "make -C " + HEXAGON_NN + " tree VERBOSE=1 V66=1 V=hexagon_ReleaseG_dynamic_toolv83_v66",
    "echo make -C " + HEXAGON_NN + " tree VERBOSE=1 V=android_ReleaseG CDSP_FLAG=1 V66=1 GRAPHINIT=\"vgg.c vgg_data.c vgg_data_32b.c\"",
    "make -C " + HEXAGON_NN + " tree VERBOSE=1 V=android_ReleaseG CDSP_FLAG=1 V66=1 GRAPHINIT=\"vgg.c vgg_data.c vgg_data_32b.c\"",
    ]

elif OS_SYSTEM == "Linux":
        
    cmd_seq = [
    "echo Collecting the log @ " + TEST_LOG_FILE,
    
    "echo VIRTUALENV SET UP  ", 
    "echo rm -rf env3",
    "rm -rf env3",  
    "echo pip install virtualenv",
    "pip install virtualenv",   
    "echo virtualenv -p python3 env3",
    "virtualenv -p python3 env3",   
    "echo source env3/bin/activate",
    "source env3/bin/activate", 
    "echo pip install -U pip",
    "pip install -U pip",   
#    "echo pip install --upgrade distribute",
#    "pip install --upgrade distribute", 
    "echo pip3 install -r " + HEXAGON_SDK_ROOT + "/tools/python_venv/environments/req3.txt",
    "pip3 install -r " + HEXAGON_SDK_ROOT + "/tools/python_venv/environments/req3.txt",  

    "echo GENERATING HEXAGON NN IMPLEMENTATION ",
    "echo python ../../scripts/img_to_dat.py --root vgg --input " + HEXAGON_NN + "/test/animals --size 224 --bgr --byte",
    "python ../../scripts/img_to_dat.py --root vgg --input " + HEXAGON_NN + "/test/animals --size 224 --bgr --byte",
    "echo python ../../scripts/caffe_to_hexagon_nn.py prototxt caffemodel vgg --min_input -114.2 --max_input 140.8",
    "python ../../scripts/caffe_to_hexagon_nn.py prototxt caffemodel vgg --min_input -114.2 --max_input 140.8",
    "echo cp -u vgg.c vgg_data.c vgg_data_32b.c vgg.h " + HEXAGON_NN,
    "cp -u vgg.c vgg_data.c vgg_data_32b.c vgg.h " + HEXAGON_NN,
    "echo make -C " + HEXAGON_NN + " tree VERBOSE=1 V66=1 V=hexagon_ReleaseG_dynamic_toolv83_v66",
    "make -C " + HEXAGON_NN + " tree VERBOSE=1 V66=1 V=hexagon_ReleaseG_dynamic_toolv83_v66",   
    "echo make -C " + HEXAGON_NN + " tree VERBOSE=1 V=android_ReleaseG CDSP_FLAG=1 V66=1 GRAPHINIT='vgg.c vgg_data.c vgg_data_32b.c'",
    "make -C " + HEXAGON_NN + " tree VERBOSE=1 V=android_ReleaseG CDSP_FLAG=1 V66=1 GRAPHINIT='vgg.c vgg_data.c vgg_data_32b.c'",
    ]
    

call_test_sig='python '+ HEXAGON_SDK_ROOT+'/scripts/testsig.py'
#os.system(call_test_sig)    # test_sig.py is not 3.6 ready yet. 
    
cmd_seq_string = "  &&  ".join(cmd_seq)
#print(cmd_seq_string)

if OS_SYSTEM == "Windows":
    with open("del_env.bat", 'w') as env_file:
        env_file.write("if exist env3 (rmdir /S /Q env3) else (echo)")

print_and_run_cmd_in_subproc(cmd_seq_string,TEST_LOG_FILE)

print ("Full log at @ " + TEST_LOG_FILE);


print ("\nPUSH THE BINARIES TO THE DEVICE ")    
print_and_run_cmd("adb root")
print_and_run_cmd("adb wait-for-device remount")

print_and_run_cmd("adb push " + HEXAGON_NN + "/test/animals_vgg_b/panda_224x224_vgg_b.dat /vendor/etc")
print_and_run_cmd("adb push " + HEXAGON_NN + "/test/vgg_labels.txt /vendor/etc")
print_and_run_cmd("adb push " + HEXAGON_NN + "/android_ReleaseG/ship/graph_app /data/")
print_and_run_cmd("adb shell chmod 777 /data/graph_app")
print_and_run_cmd("adb push " + HEXAGON_NN + "/hexagon_ReleaseG_dynamic_toolv83_v66/ship/libhexagon_nn_skel.so /vendor/lib/rfsa/adsp")
print_and_run_cmd("adb shell /data/graph_app --iters 1 /vendor/etc/panda_224x224_vgg_b.dat --labels_filename \"/vendor/etc/vgg_labels.txt\"")

#with open(TEST_LOG_FILE, 'r') as fin:
#    print( fin.read())
