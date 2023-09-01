ifeq ($(OS),Windows_NT)
SHELL:=$(ComSpec)
else
SHELL=/bin/bash
endif

current_dir = $(shell pwd)
release = $(shell cat /etc/os-release )
words = $(wordlist 1,9, $(release))
versionid = $(foreach  ver, $(words), $(if $(findstring VERSION, $(ver)),$(ver)))
version = $(lastword $(subst ", ,$(versionid)))

ifeq ($(OS),Windows_NT)
bin/qaic.exe: 
	@"mkdir" -p bin
	@$(current_dir)/WinNT/qaic.exe -v && cp -rf WinNT/* bin/
else
bin/qaic: Ubuntu18/qaic Ubuntu16/qaic Ubuntu14/qaic
	@mkdir -p bin
	@if [[ $(version) > 18 ]]; \
	then cp -rf Ubuntu18/* bin/;\
	 elif [[ $(version) > 16 ]]; \
	 then cp -rf Ubuntu16/* bin/;\
	 elif [[ $(version) > 14 ]]; \
	 then cp -rf Ubuntu14/* bin/;\
	 else echo "qaic: couldn't find a working linux binary for this platform";\
	 exit -1;\
	fi
endif
	
clean:
	@rm -rf bin


