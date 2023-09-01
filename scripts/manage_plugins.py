#!/usr/bin/env python

from optparse import OptionParser
import os,sys
import urlparse
import subprocess
import time
import zipfile
import glob
import platform

import urllib2

global not_specified
not_specified = 'not_specified'

parser=OptionParser(usage="usage: %prog [-e --hexagonEclipse] [-Z --hexagonZip] [-V --vm]",)
                          # version="%prog 1.0")

def optional_arg(arg_default):
    def func(option,opt_str,value,parser):
        if parser.rargs and not parser.rargs[0].startswith('-'):
            val=parser.rargs[0]
            parser.rargs.pop(0)
        else:
            val=arg_default
        setattr(parser.values,option.dest,val)
    return func
    
def parse_input():

    parser.add_option("-E", "--hexagonEclipse",
                      action="store",
                      dest="eclipse_location",
                      type="string",
                      help="Specify eclipse location",)
    parser.add_option("-V", "--vm",
                      action="store",
                      dest="java_location",
                      type="string",
                      help="Specify JRE (vm) location",)
    parser.add_option("-Z", "--hexagonZip",
                      #type=int,
                      action="store",
                      dest="zip_install",
                      #default=not_specified,
                      type="string",
                      help="Specify URL or IDE.zip of Installer",)
    parser.add_option("-A", "--androidEclipse",
                      #type=int,
                      action="callback",
                      dest="android_location",
                      callback=optional_arg('empty'),
                      help="Specify location of eclipse to install android plugins",)
    parser.add_option("-L", "--luaEclipse",
                      #type=int,
                      action="callback",
                      dest="lua_location",
                      callback=optional_arg('empty'),
                      help="Specify location of eclipse to install lua debugger",)
    parser.add_option("-U", "--uninstallHexagon",
                      #type=int,
                      action="callback",
                      dest="uninstall",
                      callback=optional_arg('empty'),
                      help="Uninstall Hexagon plugins, Takes no arguments",)

    (options, args) = parser.parse_args()
    
    global eclipse_exe
    global java_exe
    global eclipse_loc
    global java_loc
    global android_sdk_location
    global lua_location
    global uninstall
    android_sdk_location = options.android_location
    eclipse_loc = options.eclipse_location
    java_loc = options.java_location
    uninstall = options.uninstall
            
    global installer
    installer = options.zip_install
    lua_location = options.lua_location
    
    #print options.zip_install

    return

#Reading from file
def file_read(file_path):
    f = open(file_path,'r')
    lines = f.readlines()
    f.close()
    return lines

def file_write(path, lines):
    dest_path = path
    f = open(dest_path,'w')
    for line in lines:
        f.write(line)
    f.close()

def display_stream(out , err):
    if out:
            print out
    if err:
        print err

def install_plugins(version):
    global eclipse_loc
    if not eclipse_loc:
        eclipse_loc = "../tools/hexagon_ide"
    if not uninstall:
        osType = platform.platform()
        if 'windows' in osType.lower():
            eclipse_exe = os.path.join(eclipse_loc, "eclipse.exe")
            java_exe = os.path.join(java_loc, "javaw.exe")
        else:
            eclipse_exe = os.path.join(eclipse_loc, "eclipse")
            java_exe = os.path.join(java_loc, "java")
        if(not os.path.exists(eclipse_exe)):
            sys.exit('Not a valid eclipse Location eclipse.exe not found at: '+os.path.abspath(eclipse_exe))
        final_installer=""
        global installer
        if not installer:
            if(version=="juno"):
                installer = os.path.abspath('../tools/hexagon_ide/ide_plugins/juno/IDE.zip')
            else:
                installer = os.path.abspath('../tools/hexagon_ide/ide_plugins/kepler/IDE.zip')
            if(not os.path.exists(installer)):
                installer = os.path.abspath('../tools/hexagon_ide/ide_plugins/IDE.zip')
        if(installer.startswith('http')):
            parts = urlparse.urlsplit(installer)
            if not parts.scheme or not parts.netloc:  
                sys.exit('Not an valid url')
            final_installer = installer
        else:
            if((not os.path.isfile(installer))):
                sys.exit('Not a valid install location: '+installer)
            elif(installer.split(".")[-1] != "zip"):
                sys.exit('Not a valid zip file')
            if 'windows' in osType.lower():
                final_installer = "jar:file:/" + installer + "!/"
            else:
                if installer.startswith('/'):
                    final_installer = "jar:file:" + installer + "!/"
                else:
                    final_installer = "jar:file:/" + installer + "!/"
        final_installer.replace('\\', '/')
        print 'Installing from ' + final_installer
        print "Installing Hexagon plugins..."
        print "Please wait..."
        #uninstall '
        uninstall_plugins(0)
        #now install
        proc=subprocess.Popen([eclipse_exe, '-vm', java_exe, '-application', 'org.eclipse.equinox.p2.director', '-repository', final_installer, '-installIU', 'com.qualcomm.feature.ide.feature.group,com.qualcomm.feature.ide.hexagon.feature.group,com.qualcomm.feature.ide.hexagon.hlos.interface.feature.group,com.qualcomm.feature.ide.hexagon.opendsp.feature.group' ,'-noSplash'],stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = proc.communicate()
        display_stream(out,err);
        print 'Hexagon plugin Installation Done...'
        if not (glob.glob(eclipse_loc+'/plugins/com.qualcomm.ide.core_[1-9]*.[4-9]*.*.jar') or glob.glob(eclipse_loc+'/plugins/com.qualcomm.ide.core_[2-9]*.*.*.jar')):
            print 'You are using the older version of Hexagon, please update the plugins.';
        else:
            modify_configs(1)
        
    return

def uninstall_plugins(display):
    global eclipse_loc
    if not eclipse_loc:
        eclipse_loc = "../tools/hexagon_ide"
    osType = platform.platform()
    if 'windows' in osType.lower():
        eclipse_exe = os.path.join(eclipse_loc, "eclipse.exe")
        java_exe = os.path.join(java_loc, "javaw.exe")
    else:
        eclipse_exe = os.path.join(eclipse_loc, "eclipse")
        java_exe = os.path.join(java_loc, "java")
    if(not os.path.exists(eclipse_exe)):
        sys.exit('Not a valid eclipse Location eclipse.exe not found at: '+os.path.abspath(eclipse_exe))
			
    proc=subprocess.Popen([eclipse_exe, '-vm', java_exe, '-application', 'org.eclipse.equinox.p2.director', '-uninstallIU', 'com.qualcomm.feature.ide.feature.group', '-noSplash'],stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = proc.communicate()
    if display==1:
        display_stream(out,err);		

    proc=subprocess.Popen([eclipse_exe,'-vm', java_exe, '-application', 'org.eclipse.equinox.p2.director', '-uninstallIU', 'com.qualcomm.feature.ide.hexagon.feature.group', '-noSplash'],stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = proc.communicate()
    if display==1:
        display_stream(out,err);	
            
    proc=subprocess.Popen([eclipse_exe,'-vm', java_exe, '-application', 'org.eclipse.equinox.p2.director', '-uninstallIU', 'com.qualcomm.feature.ide.hexagon.hlos.interface.feature.group', '-noSplash'],stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = proc.communicate()
    if display==1:
        display_stream(out,err);	
	
    proc=subprocess.Popen([eclipse_exe,'-vm', java_exe, '-application', 'org.eclipse.equinox.p2.director', '-uninstallIU', 'com.qualcomm.feature.ide.hexagon.opendsp.feature.group' ,'-noSplash'],stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = proc.communicate()
    if display==1:
        display_stream(out,err);	
		
    restore_configs(display)
    return

def restore_configs(display):
    global eclipse_loc
    if display==1:
        print "Restoring configuration files...."
    eclipse_ini = os.path.join(eclipse_loc, "eclipse.ini")
    config_ini = os.path.join(eclipse_loc, "configuration", "config.ini")
    
    lines = file_read(eclipse_ini)
    length = len(lines)
    i = 0
    while i < length:
        if(lines[i].strip()== "-product"):
            i=i+1
            while(lines[i].strip()==""):
                i=i+1
            if(not lines[i].strip().startswith("-")):
                lines[i] = "org.eclipse.epp.package.cpp.product\n"
                
        elif(lines[i].strip()== "-showsplash"):
            i=i+1
            while(lines[i].strip()==""):
                i=i+1
            if(not lines[i].strip().startswith("-")):
                lines[i] = "org.eclipse.platform\n"
        i = i + 1
        
    file_write(eclipse_ini, lines)
    
    config_lines =[]
    i=0
    d = {}
    with open(config_ini) as f:
        for line in f:
            if(not line.startswith("#")):
                (key, val) = line.split("=")
                if(key == "eclipse.product"):
                    config_lines.append(key + "=org.eclipse.platform.ide\n")
                elif(key == "osgi.splashPath"):
                    config_lines.append(key + "=platform\:/base/plugins/org.eclipse.platform\n")
                elif(key == "eclipse.application"):
                    config_lines.append(key +"=org.eclipse.ui.ide.workbench\n")
                else:
                    config_lines.append(line)
            else:
                config_lines.append(line)
    
    file_write(config_ini, config_lines)
    if display == 1:
        print 'Restoring configuration files done...'
        
def modify_configs(display):
    global eclipse_loc
    if display==1:
        print "Modifying configuration files...."
    eclipse_ini = os.path.join(eclipse_loc, "eclipse.ini")
    config_ini = os.path.join(eclipse_loc, "configuration", "config.ini")
    
    lines = file_read(eclipse_ini)
    length = len(lines)
    i = 0
    while i < length:
        if(lines[i].strip()== "-product"):
            i=i+1
            while(lines[i].strip()==""):
                i=i+1
            if(not lines[i].strip().startswith("-")):
                lines[i] = "com.qualcomm.ide.product\n"
                
        elif(lines[i].strip()== "-showsplash"):
            i=i+1
            while(lines[i].strip()==""):
                i=i+1
            if(not lines[i].strip().startswith("-")):
                lines[i] = "com.qualcomm.ide\n"
        elif(lines[i].strip()== "--launcher.XXMaxPermSize"):
            i=i+1
            while(lines[i].strip()==""):
                i=i+1
            if(not lines[i].strip().startswith("-")):
                lines[i] = "512m\n"
        i = i + 1
        
    file_write(eclipse_ini, lines)
    
    config_lines =[]
    i=0
    d = {}
    with open(config_ini) as f:
        for line in f:
            if(not line.startswith("#")):
                (key, val) = line.split("=")
                if(key == "eclipse.product"):
                    config_lines.append(key + "=com.qualcomm.ide\n")
                elif(key == "osgi.splashPath"):
                    config_lines.append(key + "=platform\:/base/plugins/com.qualcomm.ide\n")
                elif(key == "eclipse.application"):
                    config_lines.append(key +"=com.qualcomm.ide.application\n")
                else:
                    config_lines.append(line)
            else:
                config_lines.append(line)
    
    file_write(config_ini, config_lines)
    if display==1:
        print 'Modifying configuration files done...'

def extractFiles(zipFile , outputPath):
    if not isdir(outputPath):
        os.makedirs(outputPath)
    for eachFile in zipFile.nameList():
        print "Extracting " + os.path.basename(eachFile)
        if not eachFile.endswith('/'):
            root,name = spilt(eachFile)
            directoty = normpath(join(path,root))
            if not isdir(directory):
                os.makedirs(directory)
            file(join(directory, name),'wb').write(zip.read(each))

def install_android():
    androidEclipsePluginsURL="https://dl-ssl.google.com/android/eclipse/"
    androidURL="http://dl.google.com/android/adt/adt-bundle-windows-x86_64-20131030.zip"
    ndkURL="http://dl.google.com/android/adt/adt-bundle-windows-x86_64-20131030.zip"
    eclipse_location=""
    if not android_sdk_location:
        pass
        #do not install android
    elif android_sdk_location == "empty":
        eclipse_location = "../eclipse"
    else:
        eclipse_location = android_sdk_location + "/"
    
    if eclipse_location:
        fileName = "../android_sdk/" + androidURL.split("/")[-1]
        try:
            os.makedirs("../android_sdk")
        except OSError:
            pass
        is_android_downloaded=False;
        download_android=raw_input('SDK will be downloaded at: '+ os.path.abspath(fileName) + "\nDownload SDK? [Press Y to download, N to ignore it] ").lower()
        if download_android == "yes" or download_android == "y":
            is_android_downloaded=False
        else:
            is_android_downloaded=True
        if not is_android_downloaded:
            u = urllib2.urlopen(androidURL)
            fileHandle = open(fileName , 'wb')
            meta = u.info()
            file_size = int(meta.getheaders("Content-Length")[0])
            file_size_MB = file_size / 1024
            file_size_MB = file_size_MB / 1024
            print "Downloading at %s: bytes %s " % (os.path.abspath(fileName) , file_size)

            file_size_dl = 0
            block_sz = 8192
            while True:
                buffer = u.read(block_sz)
                if not buffer:
                    break
                file_size_dl +=len(buffer)
                fileHandle.write(buffer)
                if isinstance(file_size_dl , int):
                    status = "%.3fMB of %.3fMB Downloaded [%3.2f%% done]" % (file_size_dl / (1024.0 * 1024.0), file_size_MB, file_size_dl * 100. / file_size)
                    print status + '\r',
                    #time.sleep(2)
            fileHandle.close()
            print "Android SDK downloaded at " + os.path.abspath(fileName)
            print 'Extracting file...'
            z = zipfile.ZipFile(fileName)
            z.extractall('../android_sdk/')
            print 'Done extracting...'
            z.close()

        ndkURL=''
        download_ndk=raw_input('Download NDK? Choose a option:\n1.32bit NDK\t2.64bit NDK\t3.Don\'t download NDK\n')
        if download_ndk=='1':
            ndkURL='http://dl.google.com/android/ndk/android-ndk32-10-windows-x86.zip'
        elif download_ndk=='2':
            ndkURL = 'http://dl.google.com/android/ndk/android-ndk32-r10-windows-x86_64.zip'
        else:
            ndkURL=''
        if ndkURL:
            ndkFileName = "../android_sdk/"  + ndkURL.split("/")[-1]
            print 'Downloading from: ' , ndkURL
            u = urllib2.urlopen(ndkURL)
            ndkFileHandle = open(ndkFileName , 'wb')
            meta = u.info()
            file_size = int(meta.getheaders("Content-Length")[0])
            file_size_MB = file_size / 1024
            file_size_MB = file_size_MB / 1024
            print "Downloading at %s: bytes %s " % (os.path.abspath(fileName) , file_size)

            file_size_dl = 0
            block_sz = 8192
            while True:
                buffer = u.read(block_sz)
                if not buffer:
                    break
                file_size_dl +=len(buffer)
                ndkFileHandle.write(buffer)
                if isinstance(file_size_dl , int):
                    status = "%.3fMB of %.3fMB Downloaded [%3.2f%% done]" % (file_size_dl / (1024.0 * 1024.0), file_size_MB, file_size_dl * 100. / file_size)
                    print status + '\r',
                    #time.sleep(2)
            ndkFileHandle.close()
            print "Android NDK downloaded at " + os.path.abspath(ndkFileName)
            print 'Extracting file...'
            z = zipfile.ZipFile(ndkFileName)
            z.extractall('../android_sdk/')
            print 'Done extracting...'
            z.close()

        if os.path.exists(eclipse_location+"/eclipse.exe"):
            print "Installing Android plugins at: " , (eclipse_location)
            print "Installing Dependencies..."
            
            install_cmd = eclipse_location + "/eclipse.exe -application org.eclipse.equinox.p2.director -repository http://download.eclipse.org/releases/juno -installIU org.eclipse.jdt"
            subprocess.call(install_cmd)
            print "org.eclipse.jdt installed"
            
            install_cmd = eclipse_location + "/eclipse.exe -application org.eclipse.equinox.p2.director -repository http://download.eclipse.org/releases/juno -installIU org.eclipse.jdt.junit"
            subprocess.call(install_cmd)
            print "org.eclipse.jdt.junit installed"
            
            install_cmd = eclipse_location + "/eclipse.exe -application org.eclipse.equinox.p2.director -repository http://download.eclipse.org/releases/juno -installIU org.eclipse.wst.sse.core"
            subprocess.call(install_cmd)
            print "org.eclipse.wst.sse.core installed"

            install_cmd = eclipse_location + "/eclipse.exe -application org.eclipse.equinox.p2.director -repository http://download.eclipse.org/releases/juno -installIU org.eclipse.wst.sse.ui"
            subprocess.call(install_cmd)
            print "org.eclipse.wst.sse.ui installed"

            install_cmd = eclipse_location + "/eclipse.exe -application org.eclipse.equinox.p2.director -repository http://download.eclipse.org/releases/juno -installIU org.eclipse.wst.xml.core"
            subprocess.call(install_cmd)
            print "org.eclipse.wst.xml.core installed"

            install_cmd = eclipse_location + "/eclipse.exe -application org.eclipse.equinox.p2.director -repository http://download.eclipse.org/releases/juno -installIU org.eclipse.wst.xml.ui"
            subprocess.call(install_cmd)
            print "org.eclipse.wst.xml.ui installed"

            print "Installing android plugins..."
            install_cmd = eclipse_location + "/eclipse.exe -application org.eclipse.equinox.p2.director -repository " + androidEclipsePluginsURL + " -installIU com.android.ide.eclipse.ddms.feature.group -installIU com.android.ide.eclipse.adt.feature.group -installIU com.android.ide.eclipse.hierarchyviewer.feature.group -installIU com.android.ide.eclipse.traceview.feature.group -installIU com.android.ide.eclipse.gldebugger.feature.group -installIU com.android.ide.eclipse.ndk.feature.group"
            subprocess.call(install_cmd)
            print "Android plugins installed"
        else:
            print "Not a valid eclipse location to install android"

def install_lua():
    luaURL="http://download.eclipse.org/koneki/releases/stable"
    eclipse_location=""
    global lua_location
    if not lua_location:
        pass
        #do not install lua debugger
    elif lua_location == "empty":
        lua_location = "../eclipse"
    else:
        lua_location = lua_location + "/"
    if lua_location and os.path.exists(lua_location+"/eclipse.exe"):
        print "Installing LUA plugins at: " , (lua_location)
        print "Please wait..."
        install_cmd = lua_location + "/eclipse.exe -application org.eclipse.equinox.p2.director -repository "+ luaURL +" -installIU org.eclipse.koneki.protocols.omadm.feature.group -installIU org.eclipse.koneki.simulators.omadm.feature.group -installIU org.eclipse.koneki.commons.feature.group -installIU org.eclipse.koneki.dashboard.feature.group -installIU org.eclipse.koneki.examples.feature.group -installIU org.eclipse.koneki.tema.feature.group -installIU org.eclipse.koneki.ldt.feature.group -installIU org.eclipse.koneki.ldt.remote.feature.group"
        subprocess.call(install_cmd)
        print "Lua Debugger plugins installed"

def check_eclipse_version():
    global eclipse_loc
    if not eclipse_loc:
        eclipse_loc = "../tools/hexagon_ide"        
    if not glob.glob(eclipse_loc+'/plugins/org.eclipse.cdt.dsf.gdb_[4-9]*.[3-9]*.*.jar'):
        return "juno"
    else:
        return "kepler"
        
def main():
    parse_input()
    install_android()
    install_lua()
    
    install_plugins(check_eclipse_version())
    
    global uninstall
    uninstallFlag=False
    if not uninstall:
        uninstallFlag=False
    else:
        uninstallFlag = True
    if uninstallFlag:
        print "Uninstalling Hexagon plugins..."
        print "Please wait..."
        uninstall_plugins(1)
    
def data_parser(text, dic):
    for i, j in dic.iteritems():
        text = text.replace(i,j)
    return text

try:
	main()
except: 
	parser.print_help()
