### Platform name
arch ?= $(shell uname -s)

ifeq ($(arch), Darwin)
EXT = .app
else
ifneq ($(findstring MINGW32, $(arch)),)
EXT = .exe
arch = MinGW32
else
EXT = 
endif
endif

all:
	$(MAKE) -C Build/$(arch)
	rm -rf FaustLive$(EXT)
	cp -R Build/$(arch)/FaustLive$(EXT) .

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
	git archive --format=tar.gz -o FaustLive-sources.tgz --prefix=FaustLive-sources/ HEAD
install:
	$(MAKE) -C Build/$(arch) install
uninstall: 
	$(MAKE) -C Build/$(arch) uninstall
clean : 
	$(MAKE) -C Build/$(arch) clean
	rm -rf FaustLive$(EXT)

