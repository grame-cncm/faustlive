#############################################
#          FaustLive Makefile
#############################################
VERSION := $(shell cat ../version.txt)

BUILDDIR ?= builddir
MAKE   ?= make
CMAKE  ?= cmake
DEST   ?= .
FAUSTLIBS := $(wildcard ../Resources/Libs/*.lib)


all :
	@git submodule update --init
	$(MAKE) qrc
	$(MAKE) cmake
	$(MAKE) faustlive


faustlive: $(BUILDDIR)
	$(CMAKE) --build $(BUILDDIR) --config Release

cmake: $(BUILDDIR)
	cd $(BUILDDIR) && $(CMAKE) .. $(CMAKEOPT) $(GENERATOR)

$(BUILDDIR):
	mkdir $(BUILDDIR)

help:
	@echo "----- FaustLive Makefile -----"
	@echo "Available target are:"
	@echo "  'all'  (default) : call the qrc, cmake and faustlive targets."
	@echo "  'qrc'            : re-generate application.qrc."
	@echo "  'cmake'          : re-generate the cmake project."
	@echo "  'faustlive'      : compiles FaustLive."
	@echo "Variables:"
	@echo "  'GENERATOR'      : a cmake generator passed to 'cmake' target"
	@echo "  'CMAKEOPT'       : cmake options passed to 'cmake' target"
	@echo "  'PREFIX'         : install prefix to be passed to 'cmake'"
	@echo "Packaging:"
	@echo "  'install'        : install the faustlive package"
	@echo "  'package'        : build the faustlive package [MacOS]"
	@echo "  'dist'           : make a faustlive distribution tarball."
	@echo "Utilities:"
	@echo "  'deploy'         : call $(DEPLOY)."
	@echo "  'clean'          : remove the $(BUILDDIR) and bin folders."
	@echo "  'mathlib'        : generate or update llvm_math.ll."

qrc: ../Resources/application.qrc 

../Resources/application.qrc : $(FAUSTLIBS)
	make -C ../Resources

install: 
	$(CMAKE) --build $(BUILDDIR) --target install

package: PACK:=FaustLive-$(VERSION)
package: 
	$(MAKE) cmake CMAKEOPT='-DPREFIX=../$(PACK)'
	$(MAKE) install
	hdiutil create $(PACK).dmg -fs HFS+ -srcfolder $(PACK) -format UDBZ -ov
	@echo "Warning: package target has changed the install PREFIX"

submodule = Resources/Libs
dist : DIST:= FaustLive-$(VERSION)
dist :
	rm -rf ../$(DIST)
	cd .. && git archive --format=tar --prefix=$(DIST)/ HEAD | tar xf -
	cd ../$(DIST) && git -C ../$(submodule) archive --format=tar --prefix=$(submodule) HEAD | tar xf -
	cd .. && tar cfz $(DIST).tar.gz $(DIST)
	mv ../$(DIST).tar.gz .
	rm -rf ../$(DIST)

release: 
	git pull --rebase
	$(MAKE) clean
	$(MAKE)
	$(MAKE) package DEST=$(DEST)
	$(MAKE) dist DISTDEST=$(DEST)

mathlib : ../Resources/Libs/llvm_math.ll	

../Resources/Libs/llvm_math.ll : ../src/Utilities/llvm_math.c 
	clang -emit-llvm ../src/Utilities/llvm_math.c -c -S -o ../Resources/Libs/llvm_math.ll

deploy:
	cd bin && macdeployqt FaustLive.app

clean :
	rm -rf $(BUILDDIR) bin

