
============================== FAUSTLIVE =========================

The goal of this project is to build an application that embeds Faust and LLVM compiler 
to provide dynamical compilation of the Faust langage.

----------------LINKED LIBRARIES------------------

In order to compile FaustLive, the following libraries are needed:

Through MacPorts :
- llvm (3.1 and higher)
- clang (same version than llvm)
- qt (4.8 and higher)
- libqrencode
- libmicrohttpd

Other Packages
- jackosx 	http://www.jackosx.com/
- faust		https://sourceforge.net/projects/faudiostream/files/
	cd FAUSTDIRECTORY
	make
	sudo make install

Make sure to download the current development trunk of these libraries.

----------------PROJECT COMPILATION---------------

cd projetDIR

//YOU CAN CHOSE THE AUDIO ARCHITECTURE SUPPORTED (CoreAudio, Jack, NetJack)
qmake -spec macx-g++ "CAVAR=1" "JVAR=1" "NJVAR=1"
make

To create a self-contained qt application, you can deploy the application with :
	 macdeployqt NAMEofAppBUNDLE

-----------------WARNINGS

1) If you want to recompile with new compilation options, you will have to execute 
	"make clean" before doing the qmake and make

2) LIBS/scheduler.ll depends on the version of llvm. This file has to be re-generated 
	if the version changes. For now llvm 3.1 is used

---------------TO ADD A NEW AUDIO ARCHITECTURE

1) Implement the classes : 
	- _audioFader
	- _audioFactory
	- _audioManager
	- _audioSettings

2) Modify AudioCreator to add the architecture in
	- enum audioArchitecture
	- add item in fAudioArchitecture
	- add case in createFactory
	- add case in read/write Settings


