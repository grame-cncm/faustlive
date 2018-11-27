######################################################################
# FAUSTLIVE.PRO
######################################################################
TARGET 	 = FaustLive
win32:!msys:!mingw {
	TEMPLATE = vcapp
}
else {
	TEMPLATE = app
}
ROOT 	 = $$PWD/..
BUILD 	 = $$PWD
SRC 	 = $$ROOT/src
DESTDIR  = $$PWD/FaustLive

LOCALLIB 	= $$ROOT/lib

isEmpty(FAUSTLIB) 		{ FAUSTLIB = "$$system(faust -libdir)" }
isEmpty(FAUSTINC) 		{ FAUSTINC = "$$system(faust -includedir)" }	

isEmpty(LLVM_CONFIG) 	{ LLVM_CONFIG = llvm-config }
## The LLVM version we are building against, for the Version popup.
isEmpty(LLVM_VERSION) 	{ LLVM_VERSION = $$system($$LLVM_CONFIG --version) }
LLVM_LIBS = $$system($$LLVM_CONFIG --libnames)

## Output settings
OBJECTS_DIR = tmp
MOC_DIR 	= tmp
RCC_DIR 	= tmp

CONFIG += exceptions rtti c++11

## QT libraries needed
QT += core gui widgets network
DEFINES += APP_VERSION=\\\"2.0\\\" LLVM_VERSION=\\\"$$LLVM_VERSION\\\"

## Images/Examples and other needed resources
RESOURCES        = $$ROOT/Resources/application.qrc
RESOURCES 	    += $$ROOT/Resources/styles.qrc
ICON             = $$ROOT/Resources/Images/FaustLiveIcon.icns
macx {
	QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10
	QMAKE_INFO_PLIST = rsrc/FaustLiveInfo.plist
}
win32 {
	RC_FILE = rsrc/FaustLive.rc
}

####### INCLUDES PATHS && LIBS PATHS
DEPENDPATH += $$FAUSTINC/faust/gui
INCLUDEPATH += . $$SRC/Audio $$SRC/MainStructure $$SRC/MenusAndDialogs $$SRC/Network $$SRC/Utilities

unix {
	LIBS += -L/usr/local/lib
	QMAKE_CXX_FLAGS +== -Wno-unused-parameter -Wno-unused-variable
	INCLUDEPATH += /usr/local/include
}

####### windows config
win32 {
	msys|mingw {
		DEFINES += USEWINMAIN 
		DEFINES += GCC
    	LIBS += $$FAUSTLIB/libfaust.a
		LIBS += $$FAUSTLIB/libHTTPDFaust.a
		LIBS += $$FAUSTLIB/libOSCFaust.a
		LIBS += $$system($$LLVM_CONFIG --ldflags) $$system($$LLVM_CONFIG --libs)
		LIBS += $$LOCALLIB/libmicrohttpd/x64/MSYS/libmicrohttpd.lib
 	  	LIBS += -lwinmm -lws2_32 
 	}
	else {
		DEFINES += _WIN32
		defined(LIBSNDFILE) {}
		else { LIBSNDFILE = "C:\Program Files\Mega-Nerd\libsndfile" }
		message ("Using libsndfile from $$LIBSNDFILE")
		CONSOLE { 
			CONFIG += console 
			message ("Generates FaustLive with console")
		}
		else {
			CONFIG -= console
			DEFINES += USEWINMAIN
		}
		LLVM_LD_FLAGS= "$$system($$LLVM_CONFIG --ldflags)"
		QMAKE_LFLAGS += "$$replace(LLVM_LD_FLAGS, "-", "/")"
    	LIBS += winmm.lib ws2_32.lib $$LIBSNDFILE/lib/libsndfile-1.lib
    	LIBS += $$FAUSTLIB/libfaust.lib
		LIBS += $$FAUSTLIB/libHTTPDFaust.lib
		LIBS += $$FAUSTLIB/libOSCFaust.lib
    	LIBS += $$LOCALLIB/libmicrohttpd/x64/libmicrohttpd.lib
    	LIBS += $$LLVM_LIBS
    	CONFIG += portaudio
	}
    INCLUDEPATH += $$FAUSTINC $$LIBSNDFILE/include $$LOCALLIB/libmicrohttpd
}
else {
 static {
	message("Uses static link for Faust libs")
	LIBS += $$FAUSTLIB/libfaust.a
	LIBS += $$FAUSTLIB/libHTTPDFaust.a
	LIBS += $$FAUSTLIB/libOSCFaust.a
 } else {
	message("Uses dynamic link for Faust libs")
	LIBS += -lHTTPDFaust -lOSCFaust -lfaust
 }
 LIBS += $$system($$LLVM_CONFIG --ldflags) $$system($$LLVM_CONFIG --libs)
 LIBS += -lmicrohttpd -lsndfile -lcurl -lz -ldl
}

DEFINES += HTTPCTRL
DEFINES += QRCODECTRL

############################## 
# common source and headers
############################## 
HEADERS +=  $$files($$SRC/Utilities/*.h)
HEADERS +=  $$files($$SRC/Audio/*.h)
HEADERS +=  $$files($$SRC/MenusAndDialogs/*.h)
HEADERS +=  $$files($$SRC/MainStructure/*.h)
HEADERS +=  $$SRC/Network/FLServerHttp.h \
			$$SRC/Network/HTTPWindow.h \
			$$FAUSTINC/faust/gui/faustqt.h

SOURCES += 	$$files($$SRC/Utilities/*.cpp)
SOURCES +=	$$files($$SRC/Audio/*.cpp) 
SOURCES +=	$$files($$SRC/MenusAndDialogs/*.cpp) 
SOURCES +=	$$files($$SRC/MainStructure/*.cpp) 
SOURCES +=	$$SRC/Network/FLServerHttp.cpp \
			$$SRC/Network/HTTPWindow.cpp

############################## 
# platform settings
############################## 
macx {
	message("MacOS CoreAudio driver")
	LIBS        += -framework CoreAudio -framework AudioUnit -framework CoreServices -framework CoreMIDI
	DEFINES     += COREAUDIO
	INCLUDEPATH += $$SRC/Audio/CA
	HEADERS     += $$files($$SRC/Audio/CA/*.h)
	SOURCES     += $$files($$SRC/Audio/CA/*.cpp)
	QMAKE_INFO_PLIST = $$BUILD/Darwin/FaustLiveInfo.plist
}

unix:!macx {
   	CONFIG += jack
	LIBS += -lasound -ltinfo
}


############################## 
# optional settings
############################## 
alsa {
	message("Linux Alsa audio driver")
	LIBS        += -lasound
	DEFINES     += ALSA
	INCLUDEPATH += $$SRC/Audio/AL
	HEADERS     += $$files($$SRC/Audio/AL/*.h)
	SOURCES     += $$files($$SRC/Audio/AL/*.cpp)
}


portaudio {
	message("Portaudio included")
	win32 { LIBS += $$ROOT/lib/portaudio/lib/portaudio.lib}
	else  { LIBS += -lportaudio}
	DEFINES     += PORTAUDIO
	INCLUDEPATH += $$SRC/Audio/PA $$ROOT/lib/portaudio/include
	HEADERS     += $$files($$SRC/Audio/PA/*.h)
	SOURCES     += $$files($$SRC/Audio/PA/*.cpp)
}

jack {
	message("Jack included")
	LIBS        += -ljack
	DEFINES     += JACK
	INCLUDEPATH += $$SRC/Audio/JA
	HEADERS     += $$files($$SRC/Audio/JA/*.h)
	SOURCES     += $$files($$SRC/Audio/JA/*.cpp)
}

netjack {
	message("NetJack included")
	LIBS        += -ljacknet
	DEFINES     += NETJACK
	INCLUDEPATH += $$SRC/Audio/NJ_Master \
				   $$SRC/Audio/NJ_Slave
	HEADERS     += $$files($$SRC/Audio/NJ_Master/*.h) \
				   $$files($$SRC/Audio/NJ_Slave/*.h)
	SOURCES     += $$files($$SRC/Audio/NJ_Master/*.cpp) \
				   $$files($$SRC/Audio/NJ_Slave/*.cpp)
}

remote {	# remote processing feature
	message("Remote processing included")
	DEFINES += REMOTE
	LIBS    +=-lfaustremote -lcurl -llo
	HEADERS += $$SRC/MenusAndDialogs/FLStatusBar.h
	SOURCES += $$SRC/MenusAndDialogs/FLStatusBar.cpp
}

