######################################################################
# FAUSTLIVE.PRO
######################################################################

# THANKS TO QMAKE, THIS FILE WILL PRODUCE THE MAKEFILE OF FAUSTLIVE APPLICATION
# IT DESCRIBES ALL THE LINKED LIBRAIRIES, COMPILATION OPTIONS, THE SOURCES TO BE COMPILED

TEMPLATE = app

isEmpty(FAUSTDIR) {
	FAUSTDIR = /usr/local
}

isEmpty(LLVM_CONFIG) {
	LLVM_CONFIG = llvm-config
}

## The LLVM version we are building against, for the Version popup.
isEmpty(LLVM_VERSION) {
	LLVM_VERSION = $$system($$LLVM_CONFIG --version)
}
 
## Application Settings
OBJECTS_DIR += ../../src/objectsFolder
MOC_DIR += ../../src/objectsFolder
RCC_DIR += ../../src/Utilities

QMAKE_EXTRA_TARGETS += all

CONFIG -= x86_64
CONFIG += exceptions rtti
#CONFIG += console

## QT libraries needed
QT+=widgets
QT+=core
QT+=gui
QT+=network

TARGET = FaustLive
DEFINES += APP_VERSION=\\\"2.0\\\" LLVM_VERSION=\\\"$$LLVM_VERSION\\\"

## Images/Examples and other needed resources
RESOURCES = ../../Resources/application.qrc
RESOURCES += ../../Resources/styles.qrc
ICON = Resources/Images/FaustLiveIcon.icns
QMAKE_INFO_PLIST = FaustLiveInfo.plist

####### INCLUDES PATHS && LIBS PATHS

# Use user environment flags
QMAKE_CXXFLAGS += $$(CXXFLAGS)
QMAKE_CFLAGS += $$(CFLAGS)
QMAKE_LFLAGS += $$(LDFLAGS)

DEPENDPATH += $$FAUSTDIR/include/faust/gui
INCLUDEPATH += .
INCLUDEPATH += /opt/local/include	
INCLUDEPATH += ../../src/Audio
INCLUDEPATH += ../../src/MenusAndDialogs
INCLUDEPATH += ../../src/MainStructure
INCLUDEPATH += ../../src/Network
INCLUDEPATH += ../../src/Utilities

LIBS+=-L$$FAUSTDIR/lib/ -L/usr/lib/ -L/opt/local/lib

equals(static, 1){
	LIBS+=-Wl,-static -lHTTPDFaust -lOSCFaust -lfaust -Wl,-Bdynamic
} else {
	LIBS+=-lHTTPDFaust -lOSCFaust -lfaust
}


equals(static, 1){
	#LIBS+=/usr/local/lib/libqrencode.a
	#LIBS+=/usr/local/lib/libmicrohttpd.a
	#LIBS+=-lgnutls
	#LIBS+=/usr/lib/x86_64-linux-gnu/libgnutls.a
	#LIBS+=/lib/x86_64-linux-gnu/libgcrypt.a
	#LIBS+=/usr/lib/x86_64-linux-gnu/libgpg-error.a
	#LIBS+=/usr/lib/x86_64-linux-gnu/libtasn1.a
	#LIBS+=-lp11-kit
	#LIBS+=-lcurl
	#LIBS+=-Wl,-static	
}

	LIBS+=-lqrencode
	LIBS+=-lmicrohttpd
	LIBS+=-lcrypto
	LIBS+=-lcurl

DEFINES += HTTPCTRL
DEFINES += QRCODECTRL

########## DEFINES/LIBS/SOURCES/... depending on audio drivers | remote processing feature

equals(REMVAR, 1){
	DEFINES += REMOTE
	LIBS+=-lfaustremote
	LIBS+=-lcurl
	LIBS+=-llo
	LIBS+=-ldbus-1 -lrt
	
	HEADERS += ../../src/MenusAndDialogs/FLStatusBar.h
	SOURCES += ../../src/MenusAndDialogs/FLStatusBar.cpp
		
	#HEADERS += ../../src/Network/Server.h
	#SOURCES += ../../src/Network/Server.cpp
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
	
	INCLUDEPATH += ../../src/Audio/NJ_Slave
	
	HEADERS += 	../../src/Audio/NJ_Slave/NJs_audioFactory.h \
				../../src/Audio/NJ_Slave/NJs_audioSettings.h \
				../../src/Audio/NJ_Slave/NJs_audioManager.h \
				../../src/Audio/NJ_Slave/NJs_audioFader.h 
	
	SOURCES += 	../../src/Audio/NJ_Slave/NJs_audioFactory.cpp \
				../../src/Audio/NJ_Slave/NJs_audioSettings.cpp \
				../../src/Audio/NJ_Slave/NJs_audioManager.cpp \
				../../src/Audio/NJ_Slave/NJs_audioFader.cpp 
}else{
	message("NETJACK NOT LINKED")
}		

equals(ALVAR, 1){
	message("ALSA LINKED")
	LIBS += -lasound
	DEFINES += ALSA
	
	INCLUDEPATH += ../../src/Audio/AL
	
	HEADERS += 	../../src/Audio/AL/AL_audioFactory.h \
				../../src/Audio/AL/AL_audioSettings.h \
				../../src/Audio/AL/AL_audioManager.h \
				../../src/Audio/AL/AL_audioFader.h \
	
	SOURCES += 	../../src/Audio/AL/AL_audioFactory.cpp \
				../../src/Audio/AL/AL_audioSettings.cpp \
				../../src/Audio/AL/AL_audioManager.cpp 
}else{
	message("ALSA NOT LINKED")
}		

equals(PAVAR, 1){
	message("PORT AUDIO LINKED")
	
	LIBS += -lportaudio
	
	DEFINES += PORTAUDIO
	
	INCLUDEPATH += ../../src/Audio/PA
	
	HEADERS += 	../../src/Audio/PA/PA_audioFactory.h \
				../../src/Audio/PA/PA_audioSettings.h \
				../../src/Audio/PA/PA_audioManager.h \
				../../src/Audio/PA/PA_audioFader.h \
	
	SOURCES += 	../../src/Audio/PA/PA_audioFader.cpp \
				../../src/Audio/PA/PA_audioFactory.cpp \
				../../src/Audio/PA/PA_audioSettings.cpp \
				../../src/Audio/PA/PA_audioManager.cpp 
}else{
	message("PORT AUDIO NOT LINKED")
}		

########## LIBS AND FLAGS

# Make sure to include --ldflags twice, once for the -L flags, and once for
# the system libraries (LLVM 3.4 and earlier have these both in --ldflags).
LIBS+=$$system(C:\\MinGW\\msys\\1.0\\local\\bin\\llvm-config --ldflags --libs)
LIBS+=$$system(C:\\MinGW\\msys\\1.0\\local\\bin\\llvm-config --ldflags)
# The system libraries need a different option in LLVM 3.5 and later.
LIBS+=$$system(C:\\MinGW\\msys\\1.0\\local\\bin\\llvm-config --system-libs)

LIBS+=-lcurldll -lwsock32 -lws2_32 -lwinmm

########## HEADERS AND SOURCES OF PROJECT

HEADERS +=  ../../src/Utilities/utilities.h \
			../../src/Audio/AudioSettings.h \
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
			../../src/MenusAndDialogs/FLMessageWindow.h \
            ../../src/MenusAndDialogs/FLExportManager.h \
            ../../src/MenusAndDialogs/FLPreferenceWindow.h \
            ../../src/MainStructure/FLWindow.h \
            ../../src/MainStructure/FLComponentWindow.h \
            ../../src/MainStructure/FLSettings.h \
            ../../src/MainStructure/FLWinSettings.h \
            ../../src/MainStructure/FLFileWatcher.h \
            ../../src/Network/FLServerHttp.h \
            ../../src/MainStructure/FLSessionManager.h \
            ../../src/MainStructure/FLApp.h \
            ../../src/MainStructure/FLInterfaceManager.h \
            ../../src/MenusAndDialogs/SimpleParser.h \
			../../src/Network/HTTPWindow.h \
			$$FAUSTDIR/include/faust/gui/faustqt.h

SOURCES += 	../../src/Utilities/utilities.cpp \
			../../src/Audio/AudioCreator.cpp \
            ../../src/Audio/AudioFader_Implementation.cpp \
        	../../src/MenusAndDialogs/FLToolBar.cpp \
			../../src/MenusAndDialogs/FLHelpWindow.cpp \
			../../src/MenusAndDialogs/FLPresentationWindow.cpp \
            ../../src/MenusAndDialogs/FLErrorWindow.cpp \
            ../../src/MenusAndDialogs/FLMessageWindow.cpp \
            ../../src/MenusAndDialogs/FLExportManager.cpp \
            ../../src/MainStructure/FLWindow.cpp \
            ../../src/MainStructure/FLComponentWindow.cpp \
            ../../src/MainStructure/FLSettings.cpp \
            ../../src/MainStructure/FLWinSettings.cpp \
            ../../src/MainStructure/FLFileWatcher.cpp \
            ../../src/MenusAndDialogs/FLPreferenceWindow.cpp \
            ../../src/MainStructure/FLSessionManager.cpp \
            ../../src/MainStructure/FLApp.cpp \
            ../../src/MainStructure/FLInterfaceManager.cpp \
			../../src/Network/FLServerHttp.cpp \
			../../src/Network/HTTPWindow.cpp \
			../../src/MenusAndDialogs/SimpleParser.cpp \
			../../src/Utilities/main.cpp



