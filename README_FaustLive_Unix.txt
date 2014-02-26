
============================== FAUSTLIVE =========================

The goal of this project is to build an application that embeds Faust and LLVM compiler 
to provide dynamical compilation of the Faust langage.

----------------LINKED LIBRARIES------------------

In order to compile FaustLive, the following libraries are needed:

Through MacPorts on OSX:
- llvm (3.1 and higher)
- clang (same version than llvm)
- qt (4.8 and higher)
- qrencode
- libmicrohttpd

Other Packages
- jackosx 	http://www.jackosx.com/
- faust		https://sourceforge.net/projects/faudiostream/files/
	cd FAUSTDIRECTORY
	make
	make httpd
	sudo make install
	(  if you want to use remote processing
	make remote
	sudo make install  )

Make sure to download the current development trunk of these libraries.

----------------PROJECT COMPILATION---------------

cd projetDIR

make
sudo make install

OSX default audio = CoreAudio
Linux default audio = JackAudio

To add Jack/NetJack or PortAudio to compilation
make JACK=1 NETJACK=1 PORTAUDIO=1

To enable remote processing option
make REMOTE=1

-----------------WARNINGS

1) If you want to recompile with new compilation options, you will have to execute 
	"make clean" before doing "make"
	& "make uninstall" before "make install"

2) LIBS/scheduler.ll depends on the version of llvm. This file has to be re-generated 
	if the version changes. For now llvm 3.1 is used

3) If you use the remote control through httpd and you want to use a port < 100, you will have to start FaustLive in sudo mode.

4) To use remote processing through wi-fi, set network parameters MTU on standard 1500.

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

3) Modify FaustLive.pro to add the libraries through conditional Compilation

----------------TO ADD FAUST LIBRAIRIES

1) Add the library in "Resources" folder
2) Add the library in application.qrc
