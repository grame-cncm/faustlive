### Platform name
arch ?= $(shell uname -s)
VERSION = $(shell cat Resources/distVersion.txt)

system	:= $(shell uname -s)
OUTDIR = fldir

system := $(shell echo $(system) | grep MINGW > /dev/null && echo MINGW || echo $(system))
ifeq ($(system), Darwin)
	TARGET ?= unix
	SPEC ?= -spec macx-clang
else
ifeq ($(system), MINGW)
	TARGET ?= windows
else
ifeq ($(system), Linux)
	TARGET ?= unix
else
	TARGET = undefined
endif
endif
endif

all :
	make $(TARGET)

undefined:
	$(error System is undefined, not target available)

unix: $(OUTDIR)/Makefile
	make -C $(OUTDIR) -j 4

$(OUTDIR)/Makefile : $(OUTDIR) FaustLive.pro
	cd $(OUTDIR) && qmake .. $(SPEC)
	
$(OUTDIR) :
	mkdir $(OUTDIR)


help:
	@echo "Usage : 'make; sudo make install'"
	@echo "To enable Jack or NetJack driver : 'make JACK=1 NETJACK=1'"
	@echo "To enable remote processing : 'make REMOTE=1'"
	@echo "make or make all : compile FaustLive"
	@echo "make clean : remove all object files"
	@echo "make install : install FaustLive and its resources in Applications"
	@echo "make uninstall : undo what install did"
	@echo "make dist : make a FaustLive distribution as a .dmg file"

math_lib :
	$(MAKE) -C Build/$(arch) math_lib
deploy:
	$(MAKE) -C Build/$(arch) deploy
	cp -R Build/$(arch)/FaustLive$(EXT) FaustLive$(EXT)
dist:
	$(MAKE) -C Build/$(arch) dist

# make a distribution .zip file for FaustLive sources
dist-sources :
	git archive --format=tar.gz -o FaustLive-sources-$(VERSION).tgz --prefix=FaustLive-sources-$(VERSION)/ HEAD
install:
	$(MAKE) -C Build/$(arch) install
uninstall:
	$(MAKE) -C Build/$(arch) uninstall
clean :
	$(MAKE) -C Build/$(arch) clean
	rm -rf FaustLive$(EXT)
