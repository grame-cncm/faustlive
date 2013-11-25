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
ifeq ($(NETJACK), 1)
     QM-DEFS += "NJVAR=1" 
endif 
ifeq ($(CAJACK), 1)
     QM-DEFS += "CAVAR=1" 
endif

####### Targets

all : Makefile.qt4
	make -f Makefile.qt4

install : install-$(system)
uninstall : uninstall-$(system)

####### Install

install-Darwin: 
	cp -r FaustLive.app /Applications
	cp -r Resources/Libs FaustLive.app/Contents/Resources
	
uninstall-Darwin: rm -rf /Applications/FaustLive.app 

install-Linux :
	install FaustLive $(PREFIX)/bin
	install FaustLive.desktop $(PREFIX)/share/applications/
	install Resources/Images/faustlive.xpm $(PREFIX)/share/pixmaps/
	install Resources/Images/faustlive.png $(PREFIX)/share/icons/hicolor/32x32/apps/
	install Resources/Images/faustlive.svg $(PREFIX)/share/icons/hicolor/scalable/apps/

uninstall-Linux :
	rm -f $(PREFIX)/bin/FaustLive
	rm -f $(PREFIX)/share/applications/FaustLive.desktop 
	rm -f $(PREFIX)/share/pixmaps/faustlive.xpm 
	rm -f $(PREFIX)/share/icons/hicolor/32x32/apps/faustlive.png
	rm -f $(PREFIX)/share/icons/hicolor/scalable/apps/faustlive.svg

####### MAKE MAKEFILE.QT4

clean : Makefile.qt4
	make -f Makefile.qt4 clean
	rm -f FaustLive.pro.user
	rm -rf FaustLive.app
	rm -f Makefile.qt4

Makefile.qt4: 
	$(qm) $(SPEC) -o Makefile.qt4 $(QM-DEFS)
