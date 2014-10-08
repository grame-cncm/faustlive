### Defining some variables
qm := $(shell which qmake)
##qm := $(if $(qm4),$(qm4),qmake)

SPEC := ""
PREFIX := /usr
DESTDIR := 
QM-DEFS := "JVAR=1"

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
ifeq ($(PORTAUDIO), 1)
     QM-DEFS += "PAVAR=1" 
endif
ifeq ($(STATIC), 1)
     QM-DEFS += "static=1" 
endif
ifneq ($(FAUSTDIR),)
     QM-DEFS += "FAUSTDIR=$(FAUSTDIR)"
endif
ifneq ($(LLVM_CONFIG),)
     QM-DEFS += "LLVM_CONFIG=$(LLVM_CONFIG)"
endif
ifneq ($(CURL_CONFIG),)
     QM-DEFS += "CURL_CONFIG=$(CURL_CONFIG)"
endif

####### Targets

all : Makefile-qt
	./buildversion
	mkdir -p src/objectsFolder
	make -f Makefile-qt

####### List of targets

help : 
	@echo "Usage : 'make; sudo make install'"
	@echo "To enable NetJack driver : 'make NETJACK=1'"
	@echo "To enable remote processing : 'make REMOTE=1'"
	@echo "make or make all : compile FaustLive"
	@echo "make clean : remove all object files"
	@echo "make install : install FaustLive and its resources in $(DESTDIR)$(PREFIX)"
	@echo "make uninstall : undo what install did"
	@echo "make dist & ./distribution {platform}: make a FaustLive distribution as a .zip file"

###### Creates LLVM Library containing math float functions like "powf" (not existing on windows)
math_lib : 
	$(shell $(shell llvm-config --prefix)/bin/clang -emit-llvm ../../src/Utilities/llvm_math.c -c -S -o ../../Resources/Libs/llvm_math.ll)

####### Packages

# make a source distribution .zip file
dist:
	make clean
	make NETJACK=1 STATIC=1
	#./distribution ${platform}
	#./distversion

# make a distribution .zip file for FaustLive sources
dist-sources :
	git archive --format=tar.gz -o FaustLive-sources.tgz --prefix=FaustLive-sources/ HEAD

####### Install
installdirs = $(PREFIX)/bin $(PREFIX)/share $(PREFIX)/share/applications $(PREFIX)/share/pixmaps $(PREFIX)/share/icons/hicolor/32x32/apps $(PREFIX)/share/icons/hicolor/scalable/apps

install:
	install -d $(addprefix $(DESTDIR), $(installdirs))
	install FaustLive $(DESTDIR)$(PREFIX)/bin
	install FaustLive.desktop $(DESTDIR)$(PREFIX)/share/applications/
	install ../../Resources/Images/Faustlive.xpm $(DESTDIR)$(PREFIX)/share/pixmaps/
	install ../../Resources/Images/Faustlive.png $(DESTDIR)$(PREFIX)/share/icons/hicolor/32x32/apps/
	install ../../Resources/Images/Faustlive.svg $(DESTDIR)$(PREFIX)/share/icons/hicolor/scalable/apps/

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/FaustLive
	rm -f $(DESTDIR)$(PREFIX)/share/applications/FaustLive.desktop
	rm -f $(DESTDIR)$(PREFIX)/share/pixmaps/Faustlive.xpm
	rm -f $(DESTDIR)$(PREFIX)/share/icons/hicolor/32x32/apps/Faustlive.png
	rm -f $(DESTDIR)$(PREFIX)/share/icons/hicolor/scalable/apps/Faustlive.svg

####### MAKE MAKEFILE-QT

clean : Makefile-qt
	make -f Makefile-qt clean
	rm -f FaustLive.pro.user
	rm -f FaustLive
	rm -f Makefile-qt
	rm -f ../../Resources/styles.qrc

../../Resources/styles.qrc: ../../Resources/styles.qrc.in
	sed -e 's?@FAUSTDIR@?$(FAUSTDIR)?g' < $< > $@

Makefile-qt: ../../Resources/styles.qrc
	$(qm) $(SPEC) -o Makefile-qt $(QM-DEFS)

