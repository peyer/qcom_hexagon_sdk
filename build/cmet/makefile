####################################################################
# Deploy results of remote & local builds
####################################################################

EveryExe = pakman p4x ccov cmet smark
EveryPkg = pakman smark lua-5.1.4 makeutils
EveryOS  = Linux Darwin WinNT
EveryF   = Release
EveryV   = $(foreach s,$(EveryOS),$(foreach f,$(EveryF),$($s.tgt)_$f$($s.opts)))

# <exe>.pkg = package that contains the exe
<exe>.pkg     = $1
ccov.pkg      = lua
cmet.pkg      = lua

# <pkg>.uri = pakman URI for package
<pkg>.uri     = p4://qctp406/depot/users/bkelley/proto/$1/pak
lua-5.1.4.uri = p4://qctp406/depot/users/bkelley/opensource/lua/lua-5.1.4/build/pak
smark.uri     = p4://qctp406/brewery/apiOne/pakhub/exe/smarkSrc
pakman.uri    = p4://qctp406/brewery/apiOne/pakhub/exe/pakmanSrc

# <pkg>.Vs = all Vs to build for deploy_<pkg>
<pkg>.Vs     = $(EveryV)

# <os>.tgt = first part of $V for <os>
Linux.tgt  = Linux
Darwin.tgt = Darwin
WinNT.tgt  = WinNT

# <os>.opts = additional options for the V
WinNT.opts = _msvc8# VS10 not compatible with SSH

# <os>.exeV = default V for deploy_exe builds for OS
<os>.exeV  = $($1.tgt)_Release$($1.opts)

Linux.os   = Linux
Darwin.os  = Darwin
Win32.os   = WinNT
WinNT.os   = WinNT
<v>.os     = $($(word 1,$(subst _, ,$1)).os)

WinNT.exe = .exe

Linux.host  = ruffian
Darwin.host = bkelley1-mac
WinNT.host  = sedition

################################################################
## Rules
################################################################

_@ = @

# .1, .2, .3 : access fields from a target name
#
.1 = $(word 1,$(subst __, ,$*))
.2 = $(word 2,$(subst __, ,$*))
.3 = $(word 3,$(subst __, ,$(basename $*)))

# tarfile = construct tar file target name
#
tarfile = obj/$1__$2__$3.tar

# $(call .,CLASS,ATTR,NAME) -> get attribute of NAME: $(NAME.ATTR) or $(CLASS.ATTR)
. = $(if $(filter undefined,$(origin $3.$2)),$(call $1.$2,$3),$(call $3.$2,$1))

# CL = the changelist number (or other version spec) to build.  Default
#      to the most recent changelist in the entire repository, $(defaultCL),
#      but evaluate it once at most.
#      
CL = $(if $(XCL),,$(eval XCL=$(defaultCL))$(info CL=$(XCL)))$(XCL)

CLPaths = //depot/users/bkelley/... //brewery/apiOne/pakhub/...
defaultCL = $(word 2,$(shell p4 changes -m 1 $(CLPaths) | sort -r))

# $(call ssh,<os>,<cmd>) uses SSH unless the current host matches <os>
# Do not propagate MAKEFLAGS (e.g. --jobs) to remote host
ssh = $(info OS=$1)MAKEFLAGS="" $(if $(filter $(shell hostname -s),$($1.host)),sh -c "$2",ssh $($1.host) "$2")

# validate
$(foreach o,$(EveryOS),$(if $(call .,<os>,tgt,$o),,$(error Invalid OS in EveryOS: '$o')))

help:
	@echo ''
	@echo 'To build a package and tar its results:'
	@echo ''
	@echo '    make tar_<pkg> [EveryV=...] [CL=...]'
	@echo ''
	@echo 'To build & deploy (p4 edit/add/delete) package results into ./pkgs/<pkg>,'
	@echo 'overlaying tars for multiple Vs into the same pkgs/<pkg> directory:'
	@echo ''
	@echo '    make deploy_<pkg> [<pkg>.Vs=...]'
	@echo ''
	@echo 'To build & deploy release executables into ./<os>/<exe> :'
	@echo ''
	@echo '    make deploy_exe_<exe> [EveryOS=...]'
	@echo ''
	@echo 'Defaults:'
	@echo '   EveryPkg = $(EveryPkg)'
	@echo '   EveryExe = $(EveryExe)'
	@echo '   EveryOS  = $(EveryOS)'
	@echo '   EveryF   = $(EveryF)'
	@echo '   EveryV   = $(EveryV)'
	@echo ''
	@echo 'To make an arbitrary URI, choose a package name and assign <pkg>.uri, as in'
	@echo '    make tar_foo foo.uri="p4://server/x/y/x"'
	@echo ''

clean:
	rm -rf obj

# make '$(CL)' to see latest changelist
$$%: ; @# $(info $(call or,$$$*))

# do not use pattern rule (make deletes intermediaries)
obj/.exist:
	$(_@)if [ ! -d $(@D) ] ; then mkdir $(@D) ; fi
	$(_@)touch $@

.PHONY: FORCE help clean tars

fromhub:
	for o in Darwin Linux WinNT ; do \
	   for e in pakman smark ; do \
	      echo cp -f ~/p4/pakhub/exe/pakman/$$o/$$e* $$o ; \
	      cp -f ~/p4/pakhub/exe/pakman/$$o/$$e* $$o ; \
	   done ; \
	   p4x addremove --all --new $$o; \
	done

################################################################
## Deploy executables into ./<os>/<exe>
##
## External vars:  CL

deploy_exe_%:
	$(foreach p,$(call .,<exe>,pkg,$*),\
	  $(_@)$(MAKE) tars EveryPkg=$p EveryV='$(foreach s,$(EveryOS),$(call .,<os>,exeV,$s))' CL=$(CL) ;\
	  $(foreach s,$(EveryOS),\
	  $(foreach v,$(call .,<os>,exeV,$s),\
	  $(foreach x,$*$($s.exe),\
	  $(foreach o,$s/$x,\
	    echo 'Deploying $v/$x' ;\
	    mkdir -p obj/$v ;\
	    tar -x -f $(call tarfile,$p,$v,$(CL)) -C obj/$v $x && \
	    mv -f obj/$v/$x $o ; ))))) \
	p4x addremove --all --force $(EveryOS)

################################################################
## Make "composite" package binary (with binaries for multiple Vs)
##
##    ./pkgs/<name>/...
##
## 1) Create tars for all Vs in <pkg>.Vs
## 2) Untar all tars over pkgs/<name> 
##
## External vars:  EveryV, CL
##
## Notes on Perforce and text files: If your client's "Line ending" policy
## is not set to "shared", then checking in a file with CRLF sequences on a
## UNIX machine will result in CRCRLF sequences when a Windows machine
## checks the file out.  This causes problems for this deployment target,
## since it untar's what was built on Windows, potentially onto a UNIX box.

deploy_%:
	@echo 'Deploying $@'
	$(_@)$(MAKE) tars EveryPkg=$* EveryV='$(call .,<pkg>,Vs,$*)' CL=$(CL)
	$(_@)rm -rf $(foreach s,$(call .,<pkg>,Vs,$*),pkgs/$*/$s)
	$(_@)mkdir -p pkgs/$*
	$(_@)$(foreach s,$(call .,<pkg>,Vs,$*),mkdir -p pkgs/$*/$s ; tar -x -f $(call tarfile,$*,$s,$(CL)) -C pkgs/$*/$s;) echo Extracted $*
	$(_@)p4x addremove --all --new --force pkgs/$*

################################################################
## Make all tar files for $(EveryV) for package
##
## External vars:  EveryV, CL

tar_%:
	$(_@)$(MAKE) tars EveryPkg=$* CL=$(CL)

################################################################
## Make all tar files for $(EveryPkg) x $(EveryV)
##
## External vars:  EveryPkg, EveryV, CL

tars:
	@echo 'Making $@ EveryPkg="$(EveryPkg)" EveryV="$(EveryV)" CL=$(CL)'
	$(_@)$(MAKE) $(foreach p,$(EveryPkg),$(foreach v,$(EveryV),$(call tarfile,$p,$v,$(CL)))) -j 9

################################################################
## Make a tar file with build results:   obj/<pkg>.<V>.<CL>.tar
##
## External vars:  <v>.os, <pkg>.uri, <os>.host
##

obj/%.tar: obj/.exist
	@echo 'Building $@'
	$(_@)PAKMAKE_PAKMAN_OPTS=--mapshort $(call ssh,$(call .,<v>,os,$(.2)),pakmake '$(call .,<pkg>,uri,$(.1))@$(.3)?$(.2)' VERSION_TYPE='' 3>&1 1>&2) > $@ 2> $@.log || { rm $@ ; echo 'ERROR building $@' ; cat $@.log ; false ; }
	@echo 'Built $@'
