### Defining some variables
qm4 := $(shell which qmake-qt4)
qm := $(if $(qm4),$(qm4),qmake)

libOSC := $(shell ls /usr/local/lib/faust/libOSCFaust*.dylib)

SPEC := -spec macx-g++
DST  := "FaustLive.app/Contents/MacOs"
QM-DEFS := "CAVAR=1"

### Defining qmake variables
ifeq ($(JACK), 1)
    QM-DEFS += "JVAR=1" 
endif
ifeq ($(REMOTE), 1)
     QM-DEFS += "JVAR=1"
     QM-DEFS += "NJVAR=1" 
     QM-DEFS += "REMVAR=1" 
endif 
ifeq ($(NETJACK), 1)
     QM-DEFS += "NJVAR=1" 
endif 
ifeq ($(COREAUDIO), 1)
     QM-DEFS += "CAVAR=1" 
endif
ifeq ($(PORTAUDIO), 1)
     QM-DEFS += "PAVAR=1" 
endif
####### Targets

all : Makefile.qt4
	./buildversion
	make -f Makefile.qt4
	install_name_tool -change /usr/local/lib/libOSCFaust.dylib /usr/local/lib/faust/libOSCFaust.dylib FaustLive.app/Contents/MacOS/FaustLive
	
###### Creates LLVM Library containing math float functions like "powf" (not existing on windows)
math_lib : 
	$(shell $(shell llvm-config --prefix)/bin/clang -emit-llvm ../../src/Utilities/llvm_math.c -c -S -o ../../Resources/Libs/llvm_math.ll)

####### Packages

# Solve FaustLive's dependencies
deploy: 
	macdeployqt FaustLive.app
	cp $(wildcard /usr/local/lib/faust/*.dylib) FaustLive.app/Contents/Frameworks/

# make a binary distribution .dmg file for OSX
dist: 	
	rm -rf FaustLive.app
	make
	cp /usr/local/bin/sound2faust FaustLive.app/Contents/MacOs
	macdeployqt FaustLive.app
	cp /usr/local/lib/faust/libfaust.dylib FaustLive.app/Contents/Frameworks/libfaust.dylib
	install_name_tool -change /usr/local/lib/faust/libfaust.dylib @executable_path/../Frameworks/libfaust.dylib FaustLive.app/Contents/MacOS/FaustLive
	cp /usr/local/lib/faust/libOSCFaust.0.93.dylib FaustLive.app/Contents/Frameworks/libOSCFaust.0.93.dylib
	cp /usr/local/lib/faust/libOSCFaust.0.dylib FaustLive.app/Contents/Frameworks/libOSCFaust.1.dylib
	cp /usr/local/lib/faust/libOSCFaust.dylib FaustLive.app/Contents/Frameworks/libOSCFaust.dylib
	install_name_tool -change /usr/local/lib/faust/libOSCFaust.dylib @executable_path/../Frameworks/libOSCFaust.dylib FaustLive.app/Contents/MacOS/FaustLive
	./distribution
	./distversion

# make a distribution .zip file for FaustLive sources
dist-sources :
	git archive --format=tar.gz -o FaustLive-sources.tgz --prefix=FaustLive-sources/ HEAD

####### Install

install: 
	cp /usr/local/bin/sound2faust FaustLive.app/Contents/MacOs
	cp -r FaustLive.app /Applications
	
uninstall: 
	rm -rf /Applications/FaustLive.app 

####### MAKE MAKEFILE.QT4

clean : 
	make -f Makefile.qt4 clean
	rm -f FaustLive.pro.user
	rm -rf FaustLive.app
	rm -f Makefile.qt4

Makefile.qt4: 
	$(qm) $(SPEC) -o Makefile.qt4 $(QM-DEFS)
