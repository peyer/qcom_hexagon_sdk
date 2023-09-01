#!/usr/bin/env python
import urllib2
import os
import argparse
import sys
from optparse import OptionParser
import subprocess
import tempfile
import shutil
import os.path


def isLinux():
    if sys.platform == "linux" or sys.platform == "linux2":
        return True
    else :
        return False;

def parse_input():
    global Download_Min_Android
    global Download_Full_Android
    global Download_Eclipse
    global Download_Gow
    global Download_Devcon
    global donotInstall
    global Install7z
    donotInstall=False
    Install7z=True
    parser=OptionParser(usage="usage:  %prog [-E ] [--MA] [--FA] [-G] [-D]\n\npython install_dependencies.py -> installs all the components Minimal Android NDK, Eclipse, gow and devcon \n\npython install_dependencies.py -E -> installs only Eclipse",) 
                          
    parser.add_option("--MA",
                      action="store_true", dest="Download_Min_Android",
                      help="Install Minimal Android NDK",)
    parser.add_option("--FA",
                      action="store_true", dest="Download_Full_Android",
                      help="Install Full Android NDK",)
    parser.add_option("-E",
                      action="store_true", dest="Download_Eclipse",
                      help="Install Eclipse",)
    parser.add_option("-G",
                      action="store_true", dest="Download_Gow",
                      help="Install Gow",)
    
    parser.add_option("-D",
                      action="store_true", dest="Download_Devcon",
                      help="Install devcon",)
    parser.add_option("--donotInstall",
                      action="store_true", dest="donotInstall",
                      help="Just download and dont install",)
    parser.add_option("--7zExe",
                      action="store",
                      dest="exe_location",
                      type="string",
                      help="Specify 7z exe location",)

    (options, args) = parser.parse_args()
    
    Download_Min_Android=options.Download_Min_Android
    Download_Full_Android=options.Download_Full_Android
    Download_Eclipse=options.Download_Eclipse
    Download_Gow=options.Download_Gow
    Download_Devcon=options.Download_Devcon
    donotInstall=options.donotInstall
    UserZipExe=options.exe_location
    if UserZipExe:
        if not os.path.isfile(UserZipExe):
            debugMsg("Invalid location. Cannot find 7z.exe at "+UserZipExe)
            exit(1)
        else:
            Install7z=False
            zipExe=UserZipExe
        debugMsg("Using 7z from this location " +UserZipExe +"\n")


def debugMsg(debugMsg):
    print(debugMsg)


def installPlugins():
    if isLinux():
        pluginScript = HEXAGON_SDK_ROOT + "/scripts/manage_plugins.py";
        eclipseLoc = HEXAGON_SDK_ROOT + "/tools/eclipse";
        pluginFile = HEXAGON_SDK_ROOT + "/tools/hexagon_ide/ide_plugins/juno/IDE.zip";
        jreLoc = HEXAGON_SDK_ROOT + "/tools/hexagon_ide/jre/bin";
        pythonExe = "python";
        cmd="mv " +eclipseLoc+"/* " + HEXAGON_SDK_ROOT + "/tools/hexagon_ide/" 
    else:
        pluginScript = HEXAGON_SDK_ROOT + "\\scripts\\manage_plugins.py";
        eclipseLoc = HEXAGON_SDK_ROOT + "\\tools\\eclipse";
        pluginFile = HEXAGON_SDK_ROOT + "\\tools\\hexagon_ide\\ide_plugins\\juno\\IDE.zip";
        jreLoc = HEXAGON_SDK_ROOT + "\\tools\\hexagon_ide\\jre\\bin";
        pythonExe = "C:\\python27\\python.exe";
        cmd="robocopy /move /e "+eclipseLoc + " " +  HEXAGON_SDK_ROOT + "/tools/hexagon_ide/ " +"/njh /njs /ndl /nc /ns /nfl" 
    debugMsg("\nInstalling Eclipse plugins");
    os.system(pythonExe + " " + pluginScript + " -E " + eclipseLoc+ " -Z " + pluginFile + " -V " + jreLoc);
    os.system(cmd);
    debugMsg("\nInstallation of Eclipse plugins is Completed\n");

def unzipOrInstall(url,target):
    gowLocation = HEXAGON_SDK_ROOT +"\\tools\\utils"
    toolsDest = HEXAGON_SDK_ROOT +"\\tools"
    debugDirectory = HEXAGON_SDK_ROOT +"\\tools\\debug"
    ndkLocation = toolsDest +"\\android-ndk-r19c";    
    if url == unzip and Install7z==True:
        print("\nInstalling 7zip\n")
        os.system("msiexec /a " + target +  " /quiet TARGETDIR="+temp + "\\7zip")
        if os.path.isfile(zipExe):
            debugMsg("\nInstallation of 7zip is completed\n")
        else:
            debugMsg("\nInstallation of 7zip is Failed, Please re-run the script by providing 7zip exe location using --zipExe option\n " 
            +"for e.g : python install_dependencies.py --zipExe=\"C:\Program Files (x86)\7-Zip\7z.exe\n\" Use quotes in the path only if path has spaces")
            exit(1)
		
    else :
        if os.path.isfile(zipExe): 
            if url == minandroid:
                print("\nInstalling Minimal Android NDK\n")
                os.system(zipExe + " x -y -o\"" + toolsDest + "\" " + target);
                print("\nInstallation of Minimal  Android NDK is completed\n")
            elif url == fullandroid:
                print("\nInstalling Android NDK\n")
                os.system(zipExe + " x -y -o\"" + toolsDest + "\" " + target);
                os.system("python " + ndkLocation + "\\build\\tools\\make_standalone_toolchain.py --arch=arm64 --api 21 --install-dir "+ndkLocation+"\\install\\android-21\\arch-arm64");
                os.system("python " + ndkLocation + "\\build\\tools\\make_standalone_toolchain.py --arch=arm --api 21 --install-dir "+ndkLocation+"\\install\\android-21\\arch-arm");
                print("\nInstallation of Android NDK is completed\n")
            elif url == gow :
                debugMsg("\nInstalling gow\n");
                subprocess.call(zipExe + " x -y -o\"" + gowLocation + "\" " + target)
                subprocess.call("cmd.exe /C copy /Y "+gowLocation+"\\gow-0.8.0\\bin\\gfind.exe " +gowLocation+"\\gow-0.8.0\\bin\\find.exe")
                subprocess.call("cmd.exe /C mkdir "+gowLocation+"\\gow-0.8.0\\etc")
                debugMsg("\nInstallation of gow  is completed\n");
            elif url == devcon:
                debugMsg("\nInstalling devcon\n");
                subprocess.call(zipExe + " x -y -o\"" + debugDirectory + "\\devcon\\" + "\" " + target + "\"")
                debugMsg("\nInstallation of devcon is completed\n");
            elif url == eclipse:
                debugMsg("\nInstalling eclipse\n");
                os.system(zipExe + " x -y -o\"" +toolsDest +"\" " +"\""+target+"\"")
                installPlugins();
           
        else:
            debugMsg("7 Zip executable not found at " + zipExe + " so can't proceed to unzip " + target)
            return

def unzipOrInstall_Linux(url,target):
    toolsDest = HEXAGON_SDK_ROOT +"/tools"
    hexIDELocation = toolsDest + "/hexagon_ide"
    ndkLocation = toolsDest +"/android-ndk-r19c";
    print url
    if url == minandroid_linux or url == fullandroid_linux :
        debugMsg("\nInstalling Android NDK\n")
        os.system("unzip" + " -o -d " + toolsDest + " " + target);
        debugMsg("\nInstallation of Android NDK is completed\n")
    elif url == fullandroid_linux :
        debugMsg("\nInstalling Android NDK\n")
        os.system("unzip" + " -o -d " + toolsDest + " " + target);
        os.system(ndkLocation + "/build/tools/make-standalone-toolchain.sh --arch=arm64 --platform=android-21 --install-dir="+ndkLocation+"/install/android-21/arch-arm64");
        os.system(ndkLocation + "/build/tools/make-standalone-toolchain.sh --arch=arm --platform=android-21 --install-dir="+ndkLocation+"/install/android-21/arch-arm");
        debugMsg("\nInstallation of Android NDK is completed\n")

    elif url == eclipse_linux :
        debugMsg("\nInstalling eclipse\n");
        os.system("tar -C" + toolsDest + " -xzf " + target +" --no-same-owner");
        installPlugins();
    elif url == libusb_linux :
        libusb = toolsDest+"/libusb";
        if not os.path.exists(libusb):
            os.makedirs(libusb)
        else :
            print("libusb folder already exists")
        os.system("cp "+target+" " +libusb);
def getSDKROOT():
    global HEXAGON_SDK_ROOT
    try:
        os.environ["HEXAGON_SDK_ROOT"]
        HEXAGON_SDK_ROOT = os.environ.get('HEXAGON_SDK_ROOT', None)
    except KeyError:
        print "Please run setup_sdk_env.cmd from Hexagon SDK"
        sys.exit(1)

def downLoadAndExtract(url):
        file_name = url.split('/')[-1]
        if isLinux():
            target =setups + "/" + file_name
        else :
            target =setups + "\\" + file_name
        hdr = {'User-Agent':'Mozilla/5.0'}
        req = urllib2.Request(url,headers=hdr)
        u = urllib2.urlopen(req)
        f = open(target, 'wb')
        meta = u.info()
        fl_size = int(meta.getheaders("Content-Length")[0])
        print "Downloading: %s Bytes: %s\n" % (target, fl_size)

        downloaded_size = 0
        block_sz = 8192
        while True:
            buffer = u.read(block_sz)
            if not buffer:
                break
        
            downloaded_size += len(buffer)
            f.write(buffer)
            status = r"%12d [%3.2f%%]" % (downloaded_size, downloaded_size * 100. / fl_size)
            print target+status+"\n"
    
        f.close()
        print "Downloading: %s Completed\n" % (target)
        if donotInstall == True:
            print("donotInstall flag is set,so not proceeding to install...");
        else:
            if isLinux():
                unzipOrInstall_Linux(url,target)
            else:
                unzipOrInstall(url, target)

				
				
def installUnzipExe():

    if (((Download_Min_Android == True) or (Download_Full_Android == True) or (Download_Eclipse==True) or (Download_Gow==True) or (Download_Devcon==True) or (Download_All == True)) and (isLinux()==False) and (Install7z == True)):
        downLoadAndExtract(unzip)

def main(): 

    installUnzipExe()
    urlList=[]
    if Download_All == True:
        if isLinux():
            urlList=[minandroid_linux,eclipse_linux]
        else:
            urlList=[minandroid,eclipse,devcon,gow]
  
    if Download_Min_Android == True:
        if isLinux():
            urlList.append(minandroid_linux)
        else:
            urlList.append(minandroid)
    if Download_Full_Android == True:
        if isLinux():
            urlList.append(fullandroid_linux)
        else:
            urlList.append(fullandroid)

    if Download_Eclipse == True:
        if isLinux():
            urlList.append(eclipse_linux)
        else:
            urlList.append(eclipse)
        
    if Download_Gow == True:
        if isLinux():
            debugMsg("gow is supposed to be installed on windows not on linux")
            sys.exit(1)
        else:
            urlList.append(gow)
    if Download_Devcon == True:
        if isLinux():
            debugMsg("Devcon is supposed to be installed on windows not on linux")
            sys.exit(1)
        else:
            urlList.append(devcon)
					
    if not urlList:
        print("Nothing to Install, Exiting...")
        sys.exit(1)
    else:
        for url in urlList:
            downLoadAndExtract(url)
	
   

if __name__ == '__main__':
    temp= tempfile.gettempdir();
    if isLinux():
        setups=temp+"/setups"
    else :
        setups=temp+"\\setups"
    if not os.path.exists(setups):
        os.makedirs(setups)
    zipExe=temp + "\\7zip\\Files\\7-Zip\\7z.exe";
    unzip="https://www.intrinsyc.com/hexagonsdk/windows/7z922.msi";
    gow="https://www.intrinsyc.com/hexagonsdk/windows/gow-0.8.0.zip"
    minandroid = "https://www.intrinsyc.com/hexagonsdk/windows/minimal_android-ndk-r19c-windows-x86_64.zip";
    fullandroid = "https://www.intrinsyc.com/hexagonsdk/windows/android-ndk-r19c-windows-x86_64.zip";
    devcon = "https://www.intrinsyc.com/hexagonsdk/windows/devcon.exe";
    eclipse = "https://www.intrinsyc.com/hexagonsdk/windows/eclipse-cpp-photon-R-win32-x86_64.zip";
    minandroid_linux = "https://www.intrinsyc.com/hexagonsdk/linux/minimal_android-ndk-r19c-linux-x86_64.zip";
    fullandroid_linux = "https://www.intrinsyc.com/hexagonsdk/linux/android-ndk-r19c-linux-x86_64.zip";
    eclipse_linux = "https://www.intrinsyc.com/hexagonsdk/linux/eclipse-cpp-photon-R-linux-gtk-x86_64.tar.gz";
    libusb_linux="https://www.intrinsyc.com/hexagonsdk/linux/libusb-1.0.so";
    parse_input()
    Download_All=True
    if len(sys.argv) > 1:
        Download_All=False
    getSDKROOT()
    main()
    shutil.rmtree(setups,ignore_errors=True)
        


