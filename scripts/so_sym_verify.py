#!/usr/bin/env python

import sys
import re
import os
from subprocess import Popen, PIPE, STDOUT
import argparse
import inspect

VERBOSE = 0
DBG = 0
IS_ROOTED = 0

parser = argparse.ArgumentParser(prog='so_sym_verify.py')
parser.add_argument("-d", action="store", dest="dsp", 
                    nargs='*', help="DSP Image location in local directory. If no DSP image is specified, symbols are verified against the DSP on target") 
parser.add_argument("-f", action="store", dest="soFile", 
                    nargs=1, help="Shared Object file name") 
parser.add_argument("-v", action="store_true", dest="verbose", 
                    help="prints verbose information") 
parser.add_argument("-i", action="store_true", dest="debug_info", 
                    default=False, help="enable debugging info") 
options = parser.parse_args()

def vprint(msg):
   if VERBOSE:
      print msg

def dprint(msg):
   if DBG:
      # TODO stip newlines
      print('{} = {}'.format(inspect.stack()[1][3], msg))

def output_filter_verify(outputLine):
   outputFilter = {
                     'not executable: magic 7F45':'ERROR ==> Attempting to run a 64 bit app !',
                     'No such file or directory':'ERROR ==> Test directory not found !',
                     'device not found':'Check Device Connection !',
                     'not found':'ERROR ==> Application not found !',
                     'only position independent executables':'ERROR ==> Attempting to run a 32 bit app in 64 bit build !',
                     "'nm' is not recognized as an internal":'ERROR ==> nm not found. Do you have cygwin installed? ',
                  }

   if None ==  outputFilter:
      return True
   if None ==  outputLine:
      return True

   for k in outputFilter:
      if k in outputLine:
         print '{} - {}'.format(outputLine.rstrip('\n\r'), outputFilter[k])
         return False

   return True

def adb_cmd(cmd):
   
   dprint('cmd is ' + cmd)
   proc = Popen(cmd, shell=True, stdout=PIPE, stderr=STDOUT)
   while True:
      nextline = proc.stdout.readline()
      if nextline == '' and proc.poll() != None:
         break

      if not output_filter_verify(nextline):
         sys.exit(1)

      dprint(nextline)
   
def FileExists(fileName):
   dprint(fileName)
   if os.path.isfile(fileName):
      dprint('File Exists')
      return 1
   else:
      print('\n\nFile does NOT exist !!!\n\t{}'. format(fileName))
      return 0
   
def GetSymbols(fileName):

   REGEX_UNDEF_SYM = '\s+[UwW] \w+'
   symbols = []
   if not FileExists(fileName):
      sys.exit()

   
   cmd = nm + ' -D {}'.format(fileName)
   dprint('cmd is ' + cmd)
   proc = Popen(cmd, shell=True, stdout=PIPE, stderr=STDOUT)

   vprint('Looking for undefined symbols in object...')
   while True:
      line = proc.stdout.readline()
      if not line:
         return list(symbols)

      if not output_filter_verify(line):
         sys.exit(1)

      dprint (line)
      m = re.match(REGEX_UNDEF_SYM, line)
      if m is not None:
         l =  m.group()
         l = l.strip()
         _, sym = l.split(' ')
         symbols.append(sym)
         vprint('found - ' + sym)

def QueryTarget(symbolsList):
   REGEX_FOUND_SYM = '\s+ {} : Yes'
   removeList = list(symbolsList)

   adb_root()

   vprint('Checking target for symbols ...')
   for sym in symbolsList:
      cmd = 'adb shell export LD_LIBRARY_PATH=/data/tests ; /data/tests/adsp_info check_sym {}'.format(sym)
      dprint('Checking symbol ' + sym)
      dprint('Check cmd is ' + cmd)
      # why Popen here and run_cmd elsewhere, combine these into one, ideally
      # run_cmd should be passed an operation function that is called with the
      # output of Popen, if don't care what the output is (shouldn't ever be the case)
      # then empty operation function
      proc = Popen(cmd, shell=True, stdout=PIPE, stderr=STDOUT)
      while True:
         nextline = proc.stdout.readline()
         dprint('Output line : ' + nextline)
         if nextline == '' and proc.poll() != None:
            break

         if 0 == len(nextline):
            continue

         if not output_filter_verify(nextline):
            sys.exit(1)
         
         m = re.match(REGEX_FOUND_SYM.format(sym), nextline)
         if m is not None:
            removeList.remove(sym)
            vprint('found - ' + sym)
            break
   if list(removeList) :
      print "\nSymbols that are not defined on the Target:\n"
      for symbol in removeList:
         print symbol
   # If all symbols are found, the returned list should be empty
   return list(removeList)

# TODO untested
def QueryImage(symbolsList, dspLocation):

   cmd = nm + ' -D {}'.format(dspLocation)
   dprint('cmd is ' + cmd)
   removeList = list(symbolsList)

   if not FileExists(dspLocation):
      sys.exit()

   vprint('Checking image for symbols...')
   proc = Popen(cmd, shell=True, stdout=PIPE, stderr=STDOUT)
   while True:
      line = proc.stdout.readline()
      if not line:
         break

      if not output_filter_verify(line):
         sys.exit(1)

      #dprint (line)
      for sym in removeList:
         dprint('Checking symbols ' + sym)
         dprint('For line ' + line)
         # is this the only check?
         if sym in line:
            removeList.remove(sym)
            vprint('found - ' + sym)
   if list(removeList) :
      print "\nSymbols that are not defined in the Image:\n"
      for symbol in removeList:
         print symbol
   # If all symbols are found, the returned list should be empty
   return list(removeList)

def QueryDepSo(symbolsList, fileName):
   DtNeededSoList = []
   cmd = readelf + ' -a {}'.format(fileName)			
   removeList = list(symbolsList)
  #Reading dynamic section to get Dt_needed shared objects
   proc = Popen(cmd, shell=True, stdout=PIPE, stderr=STDOUT)
   while True:
      line = proc.stdout.readline()
      if not line:
         break

      if not output_filter_verify(line):
         sys.exit(1)

      if "NEEDED" in line:
            #get the Shared library name
            so = line.partition("Shared library: [")[2].replace("]","")
            DtNeededSoList.append(so.strip())
   print "\nChecking Dependant Shared objects for above undefined symbols..........\n"
   print "\nDependant Shared objects are : \n"
   for so in DtNeededSoList :
       print so
   
   
#check for symbols in Dt needed shared objects
   for so in DtNeededSoList:
      #Assuming that these Shared objests are located either in parent Directory of passed shared object(ex:libdspCV_skel.so) or in HexagonTools directory (ex: libc.so and libgcc.so)
      soPath = fileName.rsplit('\\',1)[0]+"\\"+so
      if sys.platform == "linux" or sys.platform == "linux2":
         toolsDir = "$HOME/Qualcomm/HEXAGON_Tools/7.2.12/Tools/target/hexagon/lib/v61/G0/pic/"+so
      else :
         toolsDir = "C:/Qualcomm/HEXAGON_Tools/7.2.12/Tools/target/hexagon/lib/v61/G0/pic/"+so
      #Assuming that these Shared objests are located in parent Directory of passed shared object
      if os.path.isfile(soPath):
         cmd = nm + ' --defined-only {}'.format(soPath)
      #Assuming that these Shared objests are located in C:/Qualcomm/HEXAGON_Tools/7.2.12/Tools/target/hexagon/lib(ex: libc.so and libgcc.so)  
      else:
         if os.path.isfile(toolsDir):
             cmd = nm + ' --defined-only {}'.format(toolsDir)
         #check LIBRARY_PATH    
         else :
            libPath = os.environ.get('LIBRARY_PATH')
            if libPath is None :
               print "\nCould not locate dependant shared object {}".format(so)+"\n\nPlease set LIBRARY_PATH to look for {}".format(so)+" and rerun the script"
               sys.exit()
            # Loop through the paths in LIBRARY_PATH to check whether so exists or not
            else :
              #print "\nLIBRARY_PATH : " + libPath+"\n"
               checker = None
               for lib in libPath.split(";"):
                  lib = lib.strip()
                  if os.path.isfile(lib) and so in lib:
                     checker =True
                     cmd = nm + ' --defined-only {}'.format(lib)
                  elif os.path.isfile(lib+"/"+so):
                     checker =True
                     cmd = nm + ' --defined-only {}'.format(lib+"/"+so)
                  else :
                     continue
               #Could not locate shared object in LIBRARY_PATH  
               if checker is None :
                  print "\nCould not locate dependant shared object {}".format(so)+"\n\nPlease set LIBRARY_PATH to look for {}".format(so)+" and rerun the script"
                  sys.exit()
      # Get all the defined symbols of dependant shared objects 
      proc = Popen(cmd, shell=True, stdout=PIPE, stderr=STDOUT)
      while True:
          line = proc.stdout.readline()
          if not line:
             break
          if not output_filter_verify(line):
             sys.exit(1)
          for sym in removeList:
             if sym in line:
                removeList.remove(sym)
                vprint('found - ' + sym)
                
   return list(removeList)

def CheckSymbols(fileName, dspLocation):
   if not FileExists(fileName):
      sys.exit()

   if dspLocation is not None and not FileExists(dspLocation):
      sys.exit()

   symTable = GetSymbols(fileName)
   if not symTable:
      print 'No undefined symbols found in: \n\t{}'.format(fileName)
      sys.exit()

   dummy = list(symTable)
        
   if None == dspLocation:
      device_setup()
      symTableResult = QueryTarget(symTable)
   else:
      symTableResult = QueryImage(symTable, dspLocation)
   #if symbols are not found in image, check against dependent shared objects 
   if symTableResult:
      symTableResult = QueryDepSo(symTableResult, fileName)

   print '\n==============================================================================='
   if not symTableResult:
      if dspLocation:
         print 'All symbols are defined'
      else:
         print 'All symbols are defined'
      for sym in dummy:
         vprint('- ' + sym)
   else:
      if dspLocation:
         print 'Symbols NOT defined'
      else:
         print 'Symbols NOT defined on target'
      for sym in symTableResult:
         print('- ' + sym)
   print '==============================================================================='

def adb_root():
   global IS_ROOTED
   if IS_ROOTED:
      dprint ('Device already rooted!')
      return
   else:
      adb_cmd('adb root')
      adb_cmd('adb wait-for-device')
      IS_ROOTED = 1

def get_env(env_name):
   env = os.environ.get(env_name)
   if not env:
      dprint('{} is NOT set'.format(env_name))
      return None

   if not os.path.isdir(env):
      dprint ('Directory does not exist - {}'.format(env))
      return None 

   return env

def device_setup():
   adb_root()
   adb_cmd('adb remount')
   adb_cmd('adb shell mount -o remount,rw /system')
   adb_cmd('adb shell mkdir /data/tests')
   vprint('Pushing adsp_info executable ...')
   adb_cmd('adb push {}/lib/common/adsp_info/ship/android_ReleaseG/adsp_info /data/tests'.format(get_env('HEXAGON_SDK_ROOT'))) 
   adb_cmd('adb shell chmod 777 /data/tests/adsp_info')
   vprint('Pushing libadsp_info_skel.so ...')
   adb_cmd('adb push {}/lib/common/adsp_info/ship/hexagon_Debug_dynamic /system/lib/rfsa/adsp'.format(get_env('HEXAGON_SDK_ROOT')))

if __name__ == '__main__':

   dsp_root = get_env('HEXAGON_SDK_ROOT')
   if not dsp_root:
      print 'Please run setup_sdk_env.cmd'
      sys.exit(1)

   #print parser.parse_args()
   if len(sys.argv) == 1:
      print 'print usage'
      parser.print_help()
      sys.exit()

   VERBOSE = options.verbose or options.debug_info
   DBG = options.debug_info

   if not options.soFile:
      print 'Shared Object file name not provided!'
      sys.exit()

   if not FileExists(options.soFile[0]):
      sys.exit()

   if sys.platform == "linux" or sys.platform == "linux2":
      readelf = '$HOME/Qualcomm/HEXAGON_Tools/7.2.12/Tools/bin/hexagon-readelf'
      nm = '$HOME/Qualcomm/HEXAGON_Tools/7.2.12/Tools/bin/hexagon-nm'
   else:
      readelf = 'C:/Qualcomm/HEXAGON_Tools/7.2.12/Tools/bin/hexagon-readelf.exe'
      nm = 'C:/Qualcomm/HEXAGON_Tools/7.2.12/Tools/bin/hexagon-nm.exe'
                     
   if None == options.dsp:
      CheckSymbols(options.soFile[0], None)
   elif [] == options.dsp:
      print '\nNo local DSP image specified'
      sys.exit()
   else:
      CheckSymbols(options.soFile[0], options.dsp[0])

