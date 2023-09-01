#!/usr/bin/env python

import os
import sys
import shutil
import subprocess

# function to clone the src_folder to an existing destination folder with all files with "src_base" replaced by "dest_base"
def clone_folder (spath, dpath, src_base, dest_base):
    for file_name in os.listdir(spath):
        d_filename = file_name;
        if src_base in file_name:
            d_filename = file_name.replace(src_base, dest_base);
        #print "d_filename :"+d_filename;
        s_file = os.path.join(spath,file_name);
        d_file = os.path.join(dpath,d_filename);
        if(os.path.isfile(s_file)):
            with open(s_file, "rt") as fin:
                with open(d_file, "wt") as fout:
                    for line in fin:
                        fout.write(line.replace(src_base, dest_base));
        else:
            if "android_" not in s_file and "hexagon_" not in s_file and "UbuntuARM_" not in s_file:
                #print "Copying directory :<" + s_file + "> into directory: <"+d_file+">";
                os.makedirs(d_file);
                clone_folder(s_file,d_file,src_base,dest_base);


if not os.getenv('SDK_SETUP_ENV') :
    sys.exit("SDK Environment Not Setup, please run setup_sdk_env.cmd from SDK's root directory")

if len(sys.argv) != 3 :
    sys.exit('Usage: ' + os.path.basename(sys.argv[0]) + ' <path of project to clone> <name of new project>')

spath=os.path.normpath(sys.argv[1])
src_base=os.path.basename(spath)
dname=sys.argv[2]
joined_folder_name ="";

print 'src path is :' + sys.argv[1];

src_abs=os.path.abspath(sys.argv[1]);
#print 'src path abs is :' + src_abs;

#print 'dest path is :' + dname ;
#print 'dest dir is :' + os.path.dirname(dname);

dest_base = os.path.basename(dname);
#print 'dest path basename is :' + dest_base;

if os.path.dirname(dname) == "" :
    #print 'Destination is in the src folder'; 
    joined_folder_name = os.path.join(os.path.dirname(src_abs),dest_base);
    print 'Destination name is :' + joined_folder_name ;
else:
    #print 'Destination provided is a folder name';
    joined_folder_name = os.path.abspath(dname);
    print 'Destination name is :' + joined_folder_name ;

dpath=joined_folder_name;

if not os.path.exists(spath) :
	sys.exit(spath + ' does not exist!')

if os.path.exists(dpath) :
	sys.exit(dpath + ' already exists!')

print 'Cloning ' + spath + ' to ' + dpath + '...'

os.makedirs(dpath);

clone_folder(spath,dpath,src_base,dest_base);

print 'Cloning done'

