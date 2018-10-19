######################################################################
# FAUSTLIVE.PRO
######################################################################
TARGET 	 = FaustLive
win32 {
	TEMPLATE = vcapp
}
else {
	TEMPLATE = app
}
ROOT 	 = $$PWD/..
SRC 	 = $$ROOT/src
DESTDIR  = $$PWD/FaustLive

LOCALLIB 	= $$ROOT/lib

isEmpty(FAUSTDIR) 		{ FAUSTDIR = /usr/local }
isEmpty(LLVM_CONFIG) 	{ LLVM_CONFIG = llvm-config }
## The LLVM version we are building against, for the Version popup.
isEmpty(LLVM_VERSION) 	{ LLVM_VERSION = $$system($$LLVM_CONFIG --version) }

## Output settings
OBJECTS_DIR = tmp
MOC_DIR 	= tmp
RCC_DIR 	= tmp

#QMAKE_EXTRA_TARGETS += all
#QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9

#CONFIG += x86_64
CONFIG += exceptions rtti c++11
#message ("QMAKE_TARGET $$QMAKE_TARGET.arch")

## QT libraries needed
QT += core gui widgets network
DEFINES += APP_VERSION=\\\"2.0\\\" LLVM_VERSION=\\\"$$LLVM_VERSION\\\"

## Images/Examples and other needed resources
RESOURCES        = $$ROOT/Resources/application.qrc
RESOURCES 	    += $$ROOT/Resources/styles.qrc
ICON             = $$ROOT/Resources/Images/FaustLiveIcon.icns

####### INCLUDES PATHS && LIBS PATHS
DEPENDPATH += $$FAUSTDIR/include/faust/gui
INCLUDEPATH += . $$SRC/Audio $$SRC/MainStructure $$SRC/MenusAndDialogs $$SRC/Network $$SRC/Utilities

unix {
	LIBS += -L/usr/local/lib
	QMAKE_CXX_FLAGS = -Wno-unused-parameter -Wno-unused-variable
	INCLUDEPATH += /usr/local/include
}

win32 {
	DEFINES += _WIN32
	msys|mingw {
		DEFINES += USEWINMAIN 
		DEFINES += GCC
    	LIBS += $$FAUSTDIR/lib/libfaust.a
		LIBS += $$FAUSTDIR/lib/libHTTPDFaust.a
		LIBS += $$FAUSTDIR/lib/libOSCFaust.a
		LIBS += $$LOCALLIB/libmicrohttpd/x64/MSYS/libmicrohttpd.lib
    	LIBS += $$system($$LLVM_CONFIG --ldflags) $$system($$LLVM_CONFIG --libs)
 	  	LIBS += -lwinmm -lws2_32
 	}
	else {
#	    CONFIG += console
	    DEFINES += USEWINMAIN
    	LIBS += winmm.lib ws2_32.lib
    	LIBS += $$FAUSTDIR/lib/faust.lib
		LIBS += $$FAUSTDIR/lib/libHTTPDFaust.lib
		LIBS += $$FAUSTDIR/lib/libOSCFaust.lib
    	LIBS += $$LOCALLIB/libmicrohttpd/x64/libmicrohttpd.lib
	}
    INCLUDEPATH += $$FAUSTDIR/include $$LOCALLIB/libmicrohttpd
}
else {
 static {
	message("Uses static link for Faust libs")
	LIBS += -Wl,-static -lfaust -lHTTPDFaust -lOSCFaust
 } else {
	message("Uses dynamic link for Faust libs")
	LIBS += -lHTTPDFaust -lOSCFaust -lfaust
 }
	LIBS += -lmicrohttpd -lcurl -lsndfile
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
			$$FAUSTDIR/include/faust/gui/faustqt.h

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
	LIBS        += -framework CoreAudio -framework AudioUnit -framework CoreServices -framework CoreMIDI
	DEFINES     += COREAUDIO
	INCLUDEPATH += $$SRC/Audio/CA
	HEADERS     += $$files($$SRC/Audio/CA/*.h)
	SOURCES     += $$files($$SRC/Audio/CA/*.cpp)
	QMAKE_INFO_PLIST = ./MacOS/FaustLiveInfo.plist
}

win32 | portaudio {
	win32 {
		isEmpty(PADIR) 		{ PADIR = $$LOCALLIB/portaudio }
		LIBS += $$PADIR/lib/portaudio.lib
		INCLUDEPATH += $$PADIR/include
	}
	else {
		LIBS        += -lportaudio
	}
	DEFINES     += PORTAUDIO
	INCLUDEPATH += $$SRC/Audio/PA
	HEADERS     += $$files($$SRC/Audio/PA/*.h)
	SOURCES     += $$files($$SRC/Audio/PA/*.cpp)
}

# never implemented
unix:!macx {
	LIBS        += -lasound
#	DEFINES     += ALSA
#	INCLUDEPATH += $$SRC/Audio/AL
#	HEADERS     += $$files($$SRC/Audio/AL/*.h)
#	SOURCES     += $$files($$SRC/Audio/AL/*.cpp)
}


############################## 
# optional settings
############################## 
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

