
system := $(shell uname -s)
system := $(shell echo $(system) | grep MINGW > /dev/null && echo MINGW || echo $(system))
ifeq ($(system), Darwin)
	TARGET ?= unix
	SPEC ?= -spec macx-clang
	DEPLOY ?= macdeployqt
else
ifeq ($(system), MINGW)
	TARGET ?= windows
	DEPLOY ?= windeployqt
else
ifeq ($(system), Linux)
	TARGET ?= unix
	DEPLOY ?= echo no deployment on Linux for 
else
	TARGET = undefined
endif
endif
endif

OUTDIR = fldir
MAKE   ?= make
FAUST  ?= faust
DEST   ?= .
FAUSTLIBS := $(wildcard ../Resources/Libs/*.lib)


all : ../Resources/application.qrc
	$(MAKE) $(TARGET)

../Resources/application.qrc : $(FAUSTLIBS)
	make -C ../Resources


unix: $(OUTDIR)/Makefile
	$(MAKE) -C $(OUTDIR)

help:
	@echo "----- FaustLive Makefile -----"
	@echo "Available target are:"
	@echo "  'all'  (default) : compiles FaustLive."
	@echo "  'deploy'         : call $(DEPLOY)."
	@echo "  'clean'          : remove the $(OUTDIR) and FaustLive folders."
	@echo "  'update'         : git update, including submodule."
	@echo "  'mathlib'        : generate or update llvm_math.ll."
	@echo "  'rsrc'           : re-generate application.qrc."

rsrc:
	git submodule update --init
	make -C ../Resources
	
update: 
	git submodule update --init
	git pull --rebase

package: 
	$(MAKE) rsrc
	$(MAKE) -C ../package

dist: 
	$(MAKE) -C ../package dist

release: 
	git pull --rebase
	$(MAKE) clean
	$(MAKE)
	$(MAKE) package DEST=$(DEST)
	$(MAKE) dist DISTDEST=$(DEST)

windows: 
	@echo Compiling on Windows MSYS might work but has not been tested.
	@echo Proceed manually.

qmake:
	cd $(OUTDIR) && qmake .. FAUST=$(FAUST) $(SPEC) QMAKE_CFLAGS_ISYSTEM=-I

$(OUTDIR)/Makefile : $(OUTDIR) FaustLive.pro
	cd $(OUTDIR) && qmake .. FAUST=$(FAUST) $(SPEC) QMAKE_CFLAGS_ISYSTEM=-I
	
$(OUTDIR) :
	mkdir $(OUTDIR)

mathlib : ../Resources/Libs/llvm_math.ll	

../Resources/Libs/llvm_math.ll : ../src/Utilities/llvm_math.c 
	clang -emit-llvm ../src/Utilities/llvm_math.c -c -S -o ../Resources/Libs/llvm_math.ll

deploy:
	$(DEPLOY) FaustLive/FaustLive.???

clean :
	rm -rf $(OUTDIR) FaustLive

undefined:
	$(error System is undefined, not target available)

