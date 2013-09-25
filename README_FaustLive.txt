			FAUSTLIVE
			=========

The goal of this project is to build an application that embeds Faust and LLVM compiler to provide dynamical compilation of the Faust langage.

LINKED LIBRARIES
----------------

In order to compile FaustLive, the following libraries are needed:

Through MacPorts :
- llvm (3.1 and higher)
- qt (4.8 and higher)
- libqrencode
- libmicrohttpd

Other Packages
- jackosx 	http://www.jackosx.com/
- faust		https://sourceforge.net/projects/faudiostream/files/
	cd FAUSTDIRECTORY
	make
	sudo make install


PROJECT COMPILATION
-------------------

cd projetDIR
qmake -spec macx-g++
make
