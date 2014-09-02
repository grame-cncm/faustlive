### Defining some variables
qm4 := $(shell which qmake-qt4)
qm := $(if $(qm4),$(qm4),qmake)

SPEC := ""
PREFIX := /usr
QM-DEFS := "JVAR=1"

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
ifeq ($(PORTAUDIO), 1)
     QM-DEFS += "PAVAR=1" 
endif
ifeq ($(STATIC), 1)
     QM-DEFS += "static=1" 
endif

####### Targets

all : Makefile.qt4
	./buildversion
	mkdir -p src/objectsFolder
	make -f Makefile.qt4

###### Creates LLVM Library containing math float functions like "powf" (not existing on windows)
math_lib : 
	$(shell $(shell llvm-config --prefix)/bin/clang -emit-llvm ../../src/Utilities/llvm_math.c -c -S -o ../../Resources/Libs/llvm_math.ll)

####### Packages

# make a source distribution .zip file
dist:
	make clean
	make NETJACK=1 STATIC=1
	cp FaustLive Distribution_Linux/FaustLive
	zip -r FaustLive_Linux.zip Distribution_Linux
	#./distversion

# make a distribution .zip file for FaustLive sources
dist-sources :
	git archive --format=tar.gz -o FaustLive-sources.tgz --prefix=FaustLive-sources/ HEAD

####### Install
install:
	install FaustLive $(PREFIX)/bin
	install FaustLive.desktop $(PREFIX)/share/applications/
	install ../../Resources/Images/Faustlive.xpm $(PREFIX)/share/pixmaps/
	install ../../Resources/Images/Faustlive.png $(PREFIX)/share/icons/hicolor/32x32/apps/
	install ../../Resources/Images/Faustlive.svg $(PREFIX)/share/icons/hicolor/scalable/apps/

uninstall:
	rm -f $(PREFIX)/bin/FaustLive
	rm -f $(PREFIX)/share/applications/FaustLive.desktop 
	rm -f $(PREFIX)/share/pixmaps/Faustlive.xpm 
	rm -f $(PREFIX)/share/icons/hicolor/32x32/apps/Faustlive.png
	rm -f $(PREFIX)/share/icons/hicolor/scalable/apps/Faustlive.svg

####### MAKE MAKEFILE.QT4

clean : 
	make -f Makefile.qt4 clean
	rm -f FaustLive.pro.user
	rm -rf FaustLive.app
	rm -f Makefile.qt4

Makefile.qt4: 
	$(qm) $(SPEC) -o Makefile.qt4 $(QM-DEFS)
