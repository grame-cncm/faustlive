
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
FAUSTLIBS := $(wildcard ../Resources/Libs/*.lib)


all : ../Resources/application.qrc
	$(MAKE) $(TARGET)

../Resources/application.qrc : $(FAUSTLIBS)
	@echo "Faust libraries changed: check that Resources/application.qrc is uptodate." && true


unix: $(OUTDIR)/Makefile
	$(MAKE) -C $(OUTDIR) -j 4

help:
	@echo "----- FaustLive Makefile -----"
	@echo "Available target are:"
	@echo "  'all'  (default) : compiles FaustLive."
	@echo "  'deploy'         : call $(DEPLOY)."
	@echo "  'clean'          : remove the $(OUTDIR) and FaustLive folders."
	@echo "  'update'         : git update, including submodule."
	@echo "  'mathlib'        : generate or update llvm_math.ll."

update: 
	git submodule update --init
	git pull --rebase

windows: 
	@echo Compiling on Windows MSYS might work but has not been tested.
	@echo Proceed manually.

qmake:
	cd $(OUTDIR) && qmake .. $(SPEC) QMAKE_CFLAGS_ISYSTEM=-I

$(OUTDIR)/Makefile : $(OUTDIR) FaustLive.pro
	cd $(OUTDIR) && qmake .. $(SPEC) QMAKE_CFLAGS_ISYSTEM=-I
	
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

