### Platform name
arch ?= $(shell uname -s)
VERSION = $(shell cat Resources/distVersion.txt)

system := $(shell uname -s)
system := $(shell echo $(system) | grep MINGW > /dev/null && echo MINGW || echo $(system))
ifeq ($(system), Darwin)
	TARGET ?= unix
	SPEC ?= -spec macx-clang
else
ifeq ($(system), MINGW)
	TARGET ?= windows
	CONFIG +=portaudio
else
ifeq ($(system), Linux)
	TARGET ?= unix
	CONFIG +=
else
	TARGET = undefined
endif
endif
endif

OUTDIR = fldir
MAKE ?= make


all :
	$(MAKE) $(TARGET)

unix: $(OUTDIR)/Makefile
	$(MAKE) -C $(OUTDIR) -j 4

$(OUTDIR)/Makefile : $(OUTDIR) FaustLive.pro
	cd $(OUTDIR) && qmake .. $(SPEC)
	
$(OUTDIR) :
	mkdir $(OUTDIR)

help:
	@echo "----- FaustLive Makefile -----"
	@echo "in progress..."
# 	@echo "To enable Jack or NetJack driver : 'make JACK=1 NETJACK=1'"
# 	@echo "To enable remote processing : 'make REMOTE=1'"
# 	@echo "make or make all : compile FaustLive"
# 	@echo "make clean : remove all object files"
# 	@echo "make install : install FaustLive and its resources in Applications"
# 	@echo "make uninstall : undo what install did"
# 	@echo "make dist : make a FaustLive distribution as a .dmg file"

math_lib : ../Resources/Libs/llvm_math.ll	

../Resources/Libs/llvm_math.ll : ../src/Utilities/llvm_math.c 
	clang -emit-llvm ../src/Utilities/llvm_math.c -c -S -o ../Resources/Libs/llvm_math.ll

deploy:
	macdeployqt FaustLive/FaustLive.app

clean :
	$(MAKE) -C $(OUTDIR) clean
	rm -rf FaustLive

undefined:
	$(error System is undefined, not target available)

