######################################################################
# FAUSTLIVE.PRO
######################################################################

# THANKS TO QMAKE, THIS FILE WILL PRODUCE THE MAKEFILE OF FAUSTLIVE APPLICATION
# IT DESCRIBES ALL THE LINKED LIBRAIRIES, COMPILATION OPTIONS, THE SOURCES TO BE COMPILED

## Application Settings
OBJECTS_DIR += ../../src/objectsFolder

MOC_DIR += ../../src/objectsFolder

RCC_DIR += ../../src/Utilities

QMAKE_EXTRA_TARGETS += all

CONFIG -= x86_64
CONFIG += exceptions rtti

QMAKE_LFLAGS +=/SAFESEH:NO

## QT libraries needed
QT+=widgets
QT+=core
QT+=gui
QT+=network

TARGET = FaustLive
#VERSION = 1.1
DEFINES += APP_VERSION=\\\"2.0\\\"

## Images/Examples and other needed resources
RESOURCES += ../../Resources/application.qrc
RESOURCES += ../../Resources/windows.qrc

####### INCLUDES PATHS && LIBS PATHS

RC_FILE = FaustLive.rc

## VISUAL STUDIO PROJECT
TEMPLATE = vcapp
INCLUDEPATH += C:\Qt\Qt5.2.0\msvc\include
INCLUDEPATH += C:\Users\Sarah\faudiostream-code\architecture
INCLUDEPATH += C:\Users\Sarah\faudiostream-code\architecture\osclib\faust
INCLUDEPATH += C:\Qt\Qt5.2.0\msvc\include\QtWidgets
INCLUDEPATH += C:\Users\Sarah\DevTools\portaudio\include
INCLUDEPATH += C:\Users\Sarah\DevTools\qrencode-win32-681f2ea7a41f\qrencode-win32
INCLUDEPATH += C:\Users\Sarah\DevTools\libmicrohttpd\include
INCLUDEPATH += C:\Users\Sarah\DevTools\curl-7.35.0-win32\include
INCLUDEPATH += C:\Users\Sarah\DevTools\libopenssl\include
INCLUDEPATH += ../../src/Audio
INCLUDEPATH += ../../src/MenusAndDialogs
INCLUDEPATH += ../../src/MainStructure
INCLUDEPATH += ../../src/Network
INCLUDEPATH += ../../src/Utilities

LLVMLIBS = $$system(C:\Users\Sarah\DevTools\llvm-3.4\bin\Release\llvm-config --libs)
#LLVMDIR = $$system(C:\Users\Sarah\DevTools\llvm-3.4\bin\Release\llvm-config --ldflags)

Debug{
		LIBS+=-LC:\Users\Sarah\faudiostream-code\windows\_output\Win32\libfaust_vs2012\Debug
		LIBS+=-LC:\Users\Sarah\faudiostream-code\windows\_output\Win32\libOSCFaust\Debug
		LIBS+=-LC:\Users\Sarah\faudiostream-code\windows\_output\Win32\liboscpack\Debug
		LIBS+=-LC:\Users\Sarah\faudiostream-code\windows\_output\Win32\libHTTPD\Debug
		LIBS+=-LC:\Users\Sarah\DevTools\llvm-3.4\lib\Debug
		LIBS+=-LC:\Users\Sarah\DevTools\portaudio\build\msvc\Win32\Debug
		LIBS+=-LC:\Users\Sarah\DevTools\qrencode-win32-681f2ea7a41f\qrencode-win32\vc8\.build\Debug-Lib
		LIBS+=-LC:\Users\Sarah\DevTools\curl-7.35.0-win32\lib
		LIBS+=-LC:\Users\Sarah\DevTools\libopenssl\lib
		LIBS+=-LC:\Users\Sarah\DevTools\libmicrohttpd\lib
		CONFIG += console
}
Release{
		LIBS+=-LC:\Users\Sarah\faudiostream-code\windows\_output\Win32\libfaust_vs2012\Release
		LIBS+=-LC:\Users\Sarah\faudiostream-code\windows\_output\Win32\libOSCFaust\Release
		LIBS+=-LC:\Users\Sarah\faudiostream-code\windows\_output\Win32\liboscpack\Release
		LIBS+=-LC:\Users\Sarah\faudiostream-code\windows\_output\Win32\libHTTPD\Release
		LIBS+=-LC:\Users\Sarah\DevTools\libmicrohttpd\lib
		LIBS+=-LC:\Users\Sarah\DevTools\llvm-3.4\lib\Release
		LIBS+=-LC:\Users\Sarah\DevTools\portaudio\build\msvc\Win32\Release
		LIBS+=-LC:\Users\Sarah\DevTools\qrencode-win32-681f2ea7a41f\qrencode-win32\vc8\.build\Release-Lib
		LIBS+=-LC:\Users\Sarah\DevTools\curl-7.35.0-win32\lib
		LIBS+=-LC:\Users\Sarah\DevTools\libopenssl\lib
}

LIBS+=-llibeay32
LIBS+=-lssleay32

LIBS+=-lfaust
LIBS+= $$LLVMLIBS
LIBS+=-llibOSCFaust
LIBS+=-lliboscpack

LIBS+=-llibHTTPDFaust
LIBS+=-lqrcodelib
LIBS+=-llibcurl
LIBS+=-llibmicrohttpd.dll.a
DEFINES += HTTPCTRL
DEFINES += QRCODECTRL

LIBS+=-lWs2_32
LIBS+=-lwinmm
	
HEADERS += ../../src/Utilities/utilities.h \
			../../src/Utilities/TMutex.h
SOURCES += ../../src/Utilities/utilities.cpp

########## DEFINES/LIBS/SOURCES/... depending on audio drivers | remote processing feature

equals(REMVAR, 1){
	DEFINES += REMOTE
	LIBS+=-lfaustremote
	LIBS+=-lcurl
	LIBS+=-llo
}

equals(JVAR, 1){
	message("JACK LINKED")
	LIBS+= -ljack
	DEFINES += JACK
	
	INCLUDEPATH += ../../src/Audio/JA
		
	HEADERS += 	../../src/Audio/JA/JA_audioFactory.h \
				../../src/Audio/JA/JA_audioSettings.h \
				../../src/Audio/JA/JA_audioManager.h \
				../../src/Audio/JA/JA_audioFader.h \
	
	SOURCES += 	../../src/Audio/JA/JA_audioSettings.cpp \
				../../src/Audio/JA/JA_audioManager.cpp \
				../../src/Audio/JA/JA_audioFactory.cpp \
				../../src/Audio/JA/JA_audioFader.cpp 
}else{
	message("JACK NOT LINKED")
}	

equals(NJVAR, 1){
	message("NETJACK LINKED")
	LIBS += -ljacknet
	DEFINES += NETJACK
	
	INCLUDEPATH += ../../src/Audio/NJ
	
	HEADERS += 	../../src/Audio/NJ/NJ_audioFactory.h \
				../../src/Audio/NJ/NJ_audioSettings.h \
				../../src/Audio/NJ/NJ_audioManager.h \
				../../src/Audio/NJ/NJ_audioFader.h 
	
	SOURCES += 	../../src/Audio/NJ/NJ_audioFactory.cpp \
				../../src/Audio/NJ/NJ_audioSettings.cpp \
				../../src/Audio/NJ/NJ_audioManager.cpp \
				../../src/Audio/NJ/NJ_audioFader.cpp 
}else{
	message("NETJACK NOT LINKED")
}		
		
win32{
	PAVAR = 1
}

equals(PAVAR, 1){
	message("PORT AUDIO LINKED")
	
	LIBS += -lportaudio_x86
	
	DEFINES += PORTAUDIO
	
	INCLUDEPATH += ../../src/Audio/PA
	
	HEADERS += 	../../src/Audio/PA/PA_audioFactory.h \
				../../src/Audio/PA/PA_audioSettings.h \
				../../src/Audio/PA/PA_audioManager.h \
				../../src/Audio/PA/PA_audioFader.h 
	
	SOURCES += 	../../src/Audio/PA/PA_audioFader.cpp \
				../../src/Audio/PA/PA_audioFactory.cpp \
				../../src/Audio/PA/PA_audioSettings.cpp \
				../../src/Audio/PA/PA_audioManager.cpp 
}else{
	message("PORT AUDIO NOT LINKED")
}		

########## HEADERS AND SOURCES OF PROJECT

HEADERS +=  ../../src/Audio/AudioSettings.h \
			../../src/Audio/AudioManager.h \
			../../src/Audio/AudioFactory.h \
			../../src/Audio/AudioCreator.h \
			../../src/Audio/AudioFader_Interface.h \
            ../../src/Audio/AudioFader_Implementation.h \
			../../src/Audio/FJUI.h \
			../../src/MenusAndDialogs/FLToolBar.h \
			../../src/MenusAndDialogs/FLHelpWindow.h \
			../../src/MenusAndDialogs/FLPresentationWindow.h \
			../../src/MenusAndDialogs/FLErrorWindow.h \
            ../../src/MenusAndDialogs/FLExportManager.h \
			../../src/MenusAndDialogs/FLMessageWindow.h \
            ../../src/MenusAndDialogs/FLPreferenceWindow.h \
			../../src/MenusAndDialogs/FLPresentationWindow.h \
			../../src/MenusAndDialogs/FLStatusBar.h \
			../../src/MenusAndDialogs/SimpleParser.h \
			../../src/MainStructure/FLComponentWindow.h \
			../../src/MainStructure/FLFileWatcher.h \
			../../src/MainStructure/FLSessionManager.h \
			../../src/MainStructure/FLWinSettings.h \
            ../../src/MainStructure/FLWindow.h \
			../../src/Network/FLServerHttp.h \
			../../src/Network/HTTPWindow.h \
			../../src/MainStructure/FLInterfaceManager.h \
            ../../src/MainStructure/FLSettings.h \
            ../../src/MainStructure/FLApp.h \
			C:\Users\Sarah\faudiostream-code\architecture\faust\gui\faustqt.h
			
SOURCES += 	../../src/Audio/AudioCreator.cpp \
            ../../src/Audio/AudioFader_Implementation.cpp \
        	../../src/MenusAndDialogs/FLToolBar.cpp \
			../../src/MenusAndDialogs/FLHelpWindow.cpp \
			../../src/MenusAndDialogs/FLPresentationWindow.cpp \
            ../../src/MenusAndDialogs/FLErrorWindow.cpp \
			../../src/MenusAndDialogs/FLMessageWindow.cpp \
            ../../src/MenusAndDialogs/FLExportManager.cpp \
			../../src/MenusAndDialogs/FLHelpWindow.cpp \
			../../src/MenusAndDialogs/FLStatusBar.cpp \
            ../../src/MainStructure/FLWindow.cpp \
			../../src/Network/FLServerHttp.cpp \
			../../src/Network/HTTPWindow.cpp \
			../../src/MainStructure/FLInterfaceManager.cpp \
            ../../src/MainStructure/FLSettings.cpp \
            ../../src/MenusAndDialogs/FLPreferenceWindow.cpp \
            ../../src/MainStructure/FLApp.cpp	\
			../../src/MainStructure/FLComponentWindow.cpp \
			../../src/MainStructure/FLFileWatcher.cpp \
			../../src/MainStructure/FLSessionManager.cpp \
			../../src/MainStructure/FLWinSettings.cpp \
			../../src/MenusAndDialogs/SimpleParser.cpp \
			../../src/Utilities/main.cpp 
			


