### Platform name
arch := $(shell uname -s)

all:
	$(MAKE) -C Build/$(arch)
math_lib : 
	$(MAKE) -C Build/$(arch) math_lib 
deploy: 
	$(MAKE) -C Build/$(arch) deploy
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
