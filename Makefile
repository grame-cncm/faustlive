### Defining some variables

system	:= $(shell uname -s)
qm4 := $(shell which qmake-qt4)
qm := $(if $(qm4),$(qm4),qmake)

ifeq ($(system), Darwin)
	SPEC := -spec macx-g++
	DST  := "FaustLive.app/Contents/MacOs"
	QM-DEFS := "CAVAR=1"
else
	SPEC := ""
	PREFIX := /usr
	QM-DEFS := "JVAR=1"
endif

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
	make -f Makefile.qt4

install : install-$(system)

uninstall : uninstall-$(system)

###### Creates LLVM Library containing math float functions like "powf" (not existing on windows)
math_lib : $(shell $(shell llvm-config --prefix)/bin/clang -emit-llvm src/llvm_math.c -c -S -o Resources/Libs/llvm_math.ll)

####### Packages

# Solve FaustLive's dependencies
deploy: 
	macdeployqt FaustLive.app

# make a binary distribution .dmg file for OSX
dmg : 
	cp -R FaustLive.app Distribution_OSX
	hdiutil create -volname FaustLive.dmg -srcfolder Distribution_OSX -ov -format UDZO FaustLive.dmg

	
# make a source distribution .zip file
dist :
	git archive -o FaustLive.zip --prefix=FaustLive/ HEAD


####### Install

install-Darwin: 
	cp -r FaustLive.app /Applications
	
uninstall-Darwin: 
	rm -rf /Applications/FaustLive.app 

install-Linux :
	install FaustLive $(PREFIX)/bin
	install FaustLive.desktop $(PREFIX)/share/applications/
	install Resources/Images/Faustlive.xpm $(PREFIX)/share/pixmaps/
	install Resources/Images/Faustlive.png $(PREFIX)/share/icons/hicolor/32x32/apps/
	install Resources/Images/Faustlive.svg $(PREFIX)/share/icons/hicolor/scalable/apps/

uninstall-Linux :
	rm -f $(PREFIX)/bin/FaustLive
	rm -f $(PREFIX)/share/applications/FaustLive.desktop 
	rm -f $(PREFIX)/share/pixmaps/Faustlive.xpm 
	rm -f $(PREFIX)/share/icons/hicolor/32x32/apps/Faustlive.png
	rm -f $(PREFIX)/share/icons/hicolor/scalable/apps/Faustlive.svg

####### MAKE MAKEFILE.QT4

clean : Makefile.qt4
	make -f Makefile.qt4 clean
	rm -f FaustLive.pro.user
	rm -rf FaustLive.app
	rm -f Makefile.qt4

Makefile.qt4: 
	$(qm) $(SPEC) -o Makefile.qt4 $(QM-DEFS)

