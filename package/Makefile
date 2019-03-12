
# start to determine the current platform
system := $(shell uname -s)
# normalizes MINGW versions
system := $(shell echo $(system) | grep MINGW > /dev/null && echo MINGW || echo $(system))

ifeq ($(system),  MINGW)
	TARGET = windows
endif
ifeq ($(system),  Darwin)
	TARGET = macos
else
	TARGET = help
endif

VERSION := $(shell cat ../version.txt)
FLDIR   := ../Build/FaustLive
NAME    := FaustLive-$(VERSION)
CHGELOG := ../changelog.txt
DEST    ?= .

all:
	$(MAKE) $(TARGET)

help:
	@echo "-- FaustLive packaging --"
	@echo "Available targets are:"
	@echo "  macos"
	@echo "  windows"
	
macos: FLIBS  := $(otool -L build/$(NAME)/FaustLive.app/Contents/MacOS/FaustLive | grep lib | grep -i faust | sed -e 's/dylib..*/dylib/' | sed -e 's/.rpath.//')
macos: build README.html
	macdeployqt $(FLDIR)/FaustLive.app
	cd build && cmake ..
	cd build && make install
	hdiutil create $(DEST)/)$(NAME)-osx.dmg -fs HFS+ -srcfolder build/$(NAME) -format UDBZ -ov

windows: README.html
	@echo Run the makepgk.bat script

README.html: N := $(shell wc -l $(CHGELOG) | sed -e 's/ *\([0-9][0-9]*\).*/\1/')
README.html: N := $(shell expr $(N) - 7)
README.html: ../README.md ../changelog.txt
	@echo "<!DOCTYPE html><html><xmp>" > README.html
	cat ../README.md >> README.html
	@echo "<br /><br />" >> README.html
	@echo "###Change log" >> README.html
	tail -n $(N) $(CHGELOG) >> README.html
	@echo "</xmp>" >> README.html
	@echo "<script src=http://strapdownjs.com/v/0.2/strapdown.js></script>" >> README.html
	@echo "</html>" >> README.html

build:
	mkdir build


# make a faustlive distribution tarball
dist ?= $(NAME)
submodule = Resources/Libs
dist :
	rm -rf $(dist)
	git submodule update --init
	cd .. && git archive --format=tar --prefix=$(dist)/ HEAD | tar xf -
# Grab the submodules.
	cd ../$(dist) && rm -rf $(submodule) && git -C ../$(submodule) archive --format=tar --prefix=$(submodule) HEAD | tar xf -
# Create the source tarball.
	cd .. && tar cfz package/$(dist).tar.gz $(dist)
	rm -rf ../$(dist)


clean:
	rm -rf build FaustLive-$(VERSION).dmg
