
VERSION := $(shell cat ../version.txt)
FLDIR := ../Build/FaustLive
NAME  := FaustLive-$(VERSION)

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
	hdiutil create $(NAME).dmg -srcfolder build/$(NAME) -ov

windows: README.html
	@echo Run the makepgk.bat script

README.html: ../README.md
	echo "<!DOCTYPE html><html><xmp>" > README.html
	cat ../README.md >> README.html
	echo "</xmp>" >> README.html
	echo "<script src=http://strapdownjs.com/v/0.2/strapdown.js></script>" >> README.html
	echo "</html>" >> README.html

build:
	mkdir build

clean:
	rm -rf build FaustLive-$(VERSION).dmg
