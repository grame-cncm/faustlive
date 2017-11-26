
============================== FAUSTLIVE =========================

The goal of this project is to build an application that embeds Faust and LLVM compilers to provide dynamical compilation of the Faust programming language.

----------------MINGW PORT------------------------

MinGW port by Albert Graef 2014-10-16 <aggraef@gmail.com>

This is basically the Linux build, with some adjustments to make it work with MinGW (4.4 has been tested). Besides MinGW itself (available from http://www.mingw.org/), you'll need Qt (Windows binaries available from http://qt-project.org/, Qt 4.8 has been tested) and a bunch of additional libraries as detailed below, which you'll either have to compile from source or find on the web. This build uses PortAudio as the default audio architecture, but Jack should work as well if you have it installed. The build procedure is the same as on Linux (MSys shell required), although you may run into the usual Windows-specific quirks and peculiarities.

----------------LINKED LIBRARIES------------------

In order to compile FaustLive, the following libraries are needed:

Available in package manager : 
- llvm (3.1 and higher)
- clang (same version than llvm)
- qt (4.8 and higher)
- libqrencode
- libmicrohttpd
- libjack
- libjacknet

Available on sourceforge.net:
- libfaust (from branch faust2)
- libHTTPDFaust (make httpd in faust distribution)


For Remote Processing Features :
- libfaustremote (make remote in faust distribution)
- libcurl
- liblo (0.28 and higher)


Make sure to download the current development trunk of these libraries.

----------------PROJECT COMPILATION---------------

cd projetDIR

make
sudo make install

Linux default audio driver = Jack

To add NetJack to compilation
make NETJACK=1 

To enable remote processing option
make REMOTE=1

-----------------WARNINGS

1) If you want to recompile with new compilation options, you will have to execute 
	"make clean" before doing "make"
	& "make uninstall" before "make install"

2) If you use the remote control through httpd and you want to use a port < 100, you will have to start FaustLive in sudo mode.

3) To use remote processing through wi-fi, set network parameters MTU on standard 1500.

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


