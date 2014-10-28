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

math_lib : 
	$(MAKE) -C Build/$(arch) math_lib 
deploy: 
	$(MAKE) -C Build/$(arch) deploy
	cp -R Build/$(arch)/FaustLive$(EXT) FaustLive$(EXT)
dist:
	$(MAKE) -C Build/$(arch) dist
dist-sources :
	$(MAKE) -C Build/$(arch) dist-sources
install:
	$(MAKE) -C Build/$(arch) install
uninstall: 
	$(MAKE) -C Build/$(arch) uninstall
clean : 
	$(MAKE) -C Build/$(arch) clean
	rm -rf FaustLive$(EXT)

