### Defining some variables
qm := $(shell which qmake)
##qm := $(if $(qm4),$(qm4),qmake)

SPEC := ""
PREFIX := /usr
DESTDIR := 
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

clean : 
	make -f Makefile-qt clean
	rm -f FaustLive.pro.user
	rm -rf FaustLive.app
	rm -f Makefile-qt

Makefile-qt: 
	$(qm) $(SPEC) -o Makefile-qt $(QM-DEFS)

