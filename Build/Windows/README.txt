
============================== FAUSTLIVE =========================

The goal of this project is to build an application that embeds Faust and LLVM compiler to provide dynamical compilation of the Faust programming language.

----------------LINKED LIBRARIES------------------

In order to compile FaustLive, the following libraries are needed:

dlls are included for Win32/Debug architecture
- llvm (3.1 and higher)
- clang (same version than llvm)
- qt (4.8 and higher)
- qrencode
- libmicrohttpd

Other Packages
- jackosx 	http://www.jackosx.com/
- faust		https://sourceforge.net/projects/faudiostream/files/
	Copy template.props and name it faust.props
	Modify faust.props with the paths to your dependencies
	Build the Visual Studio project


Make sure to download the current development trunk of these libraries.

----------------PROJECT COMPILATION---------------

cd projetDIR

In FaustLive.pro, change paths to your libs and includes.

qmake -spec win32-msvc2012 (or your version of Visual Studio)

In Visual Studio : create a new empty solution and add the created project in it.
Once it is built, add the dlls in executable folder and execute folder.


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
