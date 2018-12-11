
VERSION := $(shell cat ../version.txt)
FLDIR := ../Build/FaustLive
NAME  := FaustLive-$(VERSION)

help:
	@echo "-- FaustLive packaging --"
	@echo "Available targets are:"
	@echo "  macos"
	@echo "  windows"
	
macos: FLIBS  := $(otool -L build/$(NAME)/FaustLive.app/Contents/MacOS/FaustLive | grep lib | grep -i faust | sed -e 's/dylib..*/dylib/' | sed -e 's/.rpath.//')
macos: build
	macdeployqt $(FLDIR)/FaustLive.app
	cd build && cmake ..
	cd build && make install
	hdiutil create $(NAME).dmg -srcfolder build/$(NAME) -ov

windows:
	@echo Run the 
	
build:
	mkdir build

clean:
	rm -rf build FaustLive-$(VERSION).dmg
