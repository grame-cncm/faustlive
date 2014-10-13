### Defining some variables
qm := $(shell which qmake)
##qm := $(if $(qm4),$(qm4),qmake)

SPEC := -spec macx-g++
DST  := "FaustLive.app/Contents/MacOs"
QM-DEFS := "CAVAR=1"

# The following FAUSTDIR and LLVM_CONFIG variables also override defaults set
# in the project file.

# We need to know where Faust lives. This will be detected automatically if
# the faust executable is on the PATH, otherwise you may have to set this
# manually.
FAUSTDIR = $(patsubst %/bin/faust,%,$(shell which faust 2>/dev/null))
ifeq ($(strip $(FAUSTDIR)),)
# Fall back to /usr/local.
FAUSTDIR = /usr/local
endif

# We also need the name of the llvm-config command for the LLVM version that
# you want to use. By default, we assume just llvm-config, which must be on
# the system PATH. Otherwise you'll have to specify the full pathname of the
# llvm-config executable that you want to use.
LLVM_CONFIG = llvm-config

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

ifneq ($(FAUSTDIR),)
     QM-DEFS += "FAUSTDIR=$(FAUSTDIR)"
endif
ifneq ($(LLVM_CONFIG),)
     QM-DEFS += "LLVM_CONFIG=$(LLVM_CONFIG)"
endif
####### Targets

all : Makefile-qt
	./buildversion
	make -f Makefile-qt

help : 
	@echo "Usage : 'make; sudo make install'"
	@echo "To enable Jack or NetJack driver : 'make JACK=1 NETJACK=1'"
	@echo "To enable remote processing : 'make REMOTE=1'"
	@echo "make or make all : compile FaustLive"
	@echo "make clean : remove all object files"
	@echo "make install : install FaustLive and its resources in Applications"
	@echo "make uninstall : undo what install did"
	@echo "make dist : make a FaustLive distribution as a .dmg file"

###### Creates LLVM Library containing math float functions like "powf" (not existing on windows)
math_lib : 
	$(shell $(shell llvm-config --prefix)/bin/clang -emit-llvm ../../src/Utilities/llvm_math.c -c -S -o ../../Resources/Libs/llvm_math.ll)

####### Packages

# Solve FaustLive's dependencies
deploy: 
	macdeployqt FaustLive.app

# make a binary distribution .dmg file for OSX
dist: 	
	rm -rf FaustLive.app
	make
	cp $(FAUSTDIR)/bin/sound2faust FaustLive.app/Contents/MacOs
	macdeployqt FaustLive.app
	./distribution
	./distversion

# make a distribution .zip file for FaustLive sources
dist-sources :
	git archive --format=tar.gz -o FaustLive-sources.tgz --prefix=FaustLive-sources/ HEAD

####### Install

# The OSX application usually gets installed under /Applications, but MacPorts
# mandates an installation in /Applications/MacPorts, so we make the
# installation directory configurable here. Also, DESTDIR is supported for
# staging purposes. - ag
APPDIR = /Applications

install: 
	cp $(FAUSTDIR)/bin/sound2faust FaustLive.app/Contents/MacOs
	cp -r FaustLive.app $(DESTDIR)$(APPDIR)

uninstall: 
	rm -rf /Applications/FaustLive.app 

####### MAKE MAKEFILE-qt

clean : Makefile-qt
	make -f Makefile-qt clean
	rm -f FaustLive.pro.user
	rm -rf FaustLive.app
	rm -f Makefile-qt
	rm -f ../../Resources/styles.qrc

../../Resources/styles.qrc: ../../Resources/styles.qrc.in
	sed -e 's?@FAUSTDIR@?$(FAUSTDIR)?g' < $< > $@

Makefile-qt: ../../Resources/styles.qrc
	$(qm) $(SPEC) -o Makefile-qt $(QM-DEFS)
