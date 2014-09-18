### Defining some variables
qm4 := $(shell which qmake-qt4)
qm := $(if $(qm4),$(qm4),qmake)

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

###### Creates LLVM Library containing math float functions like "powf" (not existing on windows)
math_lib : 
	$(shell $(shell llvm-config --prefix)/bin/clang -emit-llvm ../../src/Utilities/llvm_math.c -c -S -o ../../Resources/Libs/llvm_math.ll)

####### Packages

# Solve FaustLive's dependencies
deploy: 
	macdeployqt FaustLive.app

# make a binary distribution .dmg file for OSX
dist: 	
	rm -f FaustLive.dmg
	rm -rf FaustLive.app
	make
	cp /usr/local/bin/sound2faust FaustLive.app/Contents/MacOs
	macdeployqt FaustLive.app
	cp -R FaustLive.app Distribution_OSX/FaustLive
	hdiutil create -volname FaustLive.dmg -srcfolder Distribution_OSX -ov -format UDZO FaustLive.dmg
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
